#include "DataManager.h"
#include <QDomDocument>
#include <QFile>
#include <QTextStream>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

DataManager::DataManager(QObject* parent)
    : QObject(parent)
{
}

bool DataManager::saveToXml(const QString& filename, const QVector<PathResult>& results) {
    QDomDocument doc;
    
    // 创建根元素
    QDomElement root = doc.createElement("PathResults");
    doc.appendChild(root);
    
    // 添加版本信息
    root.setAttribute("version", "1.0");
    root.setAttribute("count", results.size());
    
    // 添加每个路径结果
    for (const PathResult& result : results) {
        QDomElement resultElement = doc.createElement("PathResult");
        
        // 基本信息
        resultElement.setAttribute("algorithm", static_cast<int>(result.algorithm()));
        resultElement.setAttribute("algorithmName", result.algorithmString());
        resultElement.setAttribute("pathLength", result.pathLength());
        resultElement.setAttribute("calculationTime", result.calculationTime());
        resultElement.setAttribute("timestamp", result.timestamp().toString(Qt::ISODate));
        
        // 起点和终点
        QDomElement startElement = doc.createElement("StartPoint");
        startElement.setAttribute("x", result.startPoint().x());
        startElement.setAttribute("y", result.startPoint().y());
        resultElement.appendChild(startElement);
        
        QDomElement endElement = doc.createElement("EndPoint");
        endElement.setAttribute("x", result.endPoint().x());
        endElement.setAttribute("y", result.endPoint().y());
        resultElement.appendChild(endElement);
        
        // 路径点
        QDomElement pathElement = doc.createElement("Path");
        for (const QPoint& point : result.path()) {
            QDomElement pointElement = doc.createElement("Point");
            pointElement.setAttribute("x", point.x());
            pointElement.setAttribute("y", point.y());
            pathElement.appendChild(pointElement);
        }
        resultElement.appendChild(pathElement);
        
        root.appendChild(resultElement);
    }
    
    // 保存到文件
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit operationFinished(false, "无法创建XML文件: " + file.errorString());
        return false;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << doc.toString(4); // 4个空格缩进
    
    file.close();
    
    emit operationFinished(true, QString("成功保存 %1 条路径结果到XML文件").arg(results.size()));
    return true;
}

QVector<PathResult> DataManager::loadFromXml(const QString& filename) {
    QVector<PathResult> results;
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit operationFinished(false, "无法打开XML文件: " + file.errorString());
        return results;
    }
    
    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
        file.close();
        emit operationFinished(false, QString("XML解析失败 (行%1, 列%2): %3")
                              .arg(errorLine).arg(errorColumn).arg(errorMsg));
        return results;
    }
    
    file.close();
    
    QDomElement root = doc.documentElement();
    if (root.tagName() != "PathResults") {
        emit operationFinished(false, "无效的XML格式：根元素应为PathResults");
        return results;
    }
    
    // 解析每个路径结果
    QDomNodeList resultNodes = root.childNodes();
    for (int i = 0; i < resultNodes.count(); ++i) {
        QDomElement resultElement = resultNodes.at(i).toElement();
        if (resultElement.tagName() != "PathResult") {
            continue;
        }
        
        // 解析基本信息
        AlgorithmType algorithm = static_cast<AlgorithmType>(resultElement.attribute("algorithm").toInt());
        qint64 calculationTime = resultElement.attribute("calculationTime").toLongLong();
        QDateTime timestamp = QDateTime::fromString(resultElement.attribute("timestamp"), Qt::ISODate);
        
        // 解析起点
        QDomElement startElement = resultElement.firstChildElement("StartPoint");
        QPoint startPoint(startElement.attribute("x").toInt(), startElement.attribute("y").toInt());
        
        // 解析终点
        QDomElement endElement = resultElement.firstChildElement("EndPoint");
        QPoint endPoint(endElement.attribute("x").toInt(), endElement.attribute("y").toInt());
        
        // 解析路径
        QVector<QPoint> path;
        QDomElement pathElement = resultElement.firstChildElement("Path");
        QDomNodeList pointNodes = pathElement.childNodes();
        for (int j = 0; j < pointNodes.count(); ++j) {
            QDomElement pointElement = pointNodes.at(j).toElement();
            if (pointElement.tagName() == "Point") {
                QPoint point(pointElement.attribute("x").toInt(), pointElement.attribute("y").toInt());
                path.append(point);
            }
        }
        
        // 创建路径结果
        PathResult result("", startPoint, endPoint, path, algorithm, calculationTime);
        result.setTimestamp(timestamp);
        results.append(result);
    }
    
    emit operationFinished(true, QString("成功从XML文件加载 %1 条路径结果").arg(results.size()));
    return results;
}

bool DataManager::saveToSqlite(const QString& filename, const QVector<PathResult>& results) {
    // 创建数据库连接
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "save_connection");
    db.setDatabaseName(filename);
    
    if (!db.open()) {
        emit operationFinished(false, "无法打开SQLite数据库: " + db.lastError().text());
        return false;
    }
    
    // 创建表
    QSqlQuery query(db);
    QString createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS path_results (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            algorithm INTEGER NOT NULL,
            algorithm_name TEXT NOT NULL,
            start_x INTEGER NOT NULL,
            start_y INTEGER NOT NULL,
            end_x INTEGER NOT NULL,
            end_y INTEGER NOT NULL,
            path_length INTEGER NOT NULL,
            calculation_time INTEGER NOT NULL,
            timestamp TEXT NOT NULL,
            path_data TEXT NOT NULL
        )
    )";
    
    if (!query.exec(createTableSQL)) {
        db.close();
        QSqlDatabase::removeDatabase("save_connection");
        emit operationFinished(false, "无法创建数据表: " + query.lastError().text());
        return false;
    }
    
    // 开始事务
    db.transaction();
    
    // 插入数据
    QString insertSQL = R"(
        INSERT INTO path_results (
            algorithm, algorithm_name, start_x, start_y, end_x, end_y,
            path_length, calculation_time, timestamp, path_data
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    query.prepare(insertSQL);
    
    int successCount = 0;
    for (const PathResult& result : results) {
        // 将路径点序列化为JSON字符串
        QJsonArray pathArray;
        for (const QPoint& point : result.path()) {
            QJsonObject pointObj;
            pointObj["x"] = point.x();
            pointObj["y"] = point.y();
            pathArray.append(pointObj);
        }
        QJsonDocument pathDoc(pathArray);
        QString pathData = pathDoc.toJson(QJsonDocument::Compact);
        
        query.bindValue(0, static_cast<int>(result.algorithm()));
        query.bindValue(1, result.algorithmString());
        query.bindValue(2, result.startPoint().x());
        query.bindValue(3, result.startPoint().y());
        query.bindValue(4, result.endPoint().x());
        query.bindValue(5, result.endPoint().y());
        query.bindValue(6, result.pathLength());
        query.bindValue(7, result.calculationTime());
        query.bindValue(8, result.timestamp().toString(Qt::ISODate));
        query.bindValue(9, pathData);
        
        if (query.exec()) {
            successCount++;
        }
    }
    
    // 提交事务
    db.commit();
    db.close();
    QSqlDatabase::removeDatabase("save_connection");
    
    if (successCount == results.size()) {
        emit operationFinished(true, QString("成功保存 %1 条路径结果到SQLite数据库").arg(successCount));
        return true;
    } else {
        emit operationFinished(false, QString("部分保存失败：成功 %1/%2 条记录").arg(successCount).arg(results.size()));
        return false;
    }
}

QVector<PathResult> DataManager::loadFromSqlite(const QString& filename) {
    QVector<PathResult> results;
    
    // 检查文件是否存在
    QFile file(filename);
    if (!file.exists()) {
        emit operationFinished(false, "SQLite数据库文件不存在");
        return results;
    }
    
    // 创建数据库连接
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "load_connection");
    db.setDatabaseName(filename);
    
    if (!db.open()) {
        emit operationFinished(false, "无法打开SQLite数据库: " + db.lastError().text());
        return results;
    }
    
    // 查询数据
    QSqlQuery query(db);
    QString selectSQL = R"(
        SELECT algorithm, algorithm_name, start_x, start_y, end_x, end_y,
               path_length, calculation_time, timestamp, path_data
        FROM path_results
        ORDER BY id
    )";
    
    if (!query.exec(selectSQL)) {
        db.close();
        QSqlDatabase::removeDatabase("load_connection");
        emit operationFinished(false, "无法查询数据: " + query.lastError().text());
        return results;
    }
    
    // 读取数据
    while (query.next()) {
        AlgorithmType algorithm = static_cast<AlgorithmType>(query.value(0).toInt());
        QPoint startPoint(query.value(2).toInt(), query.value(3).toInt());
        QPoint endPoint(query.value(4).toInt(), query.value(5).toInt());
        qint64 calculationTime = query.value(7).toLongLong();
        QDateTime timestamp = QDateTime::fromString(query.value(8).toString(), Qt::ISODate);
        QString pathData = query.value(9).toString();
        
        // 反序列化路径数据
        QVector<QPoint> path;
        QJsonParseError error;
        QJsonDocument pathDoc = QJsonDocument::fromJson(pathData.toUtf8(), &error);
        
        if (error.error == QJsonParseError::NoError && pathDoc.isArray()) {
            QJsonArray pathArray = pathDoc.array();
            for (const QJsonValue& pointValue : pathArray) {
                if (pointValue.isObject()) {
                    QJsonObject pointObj = pointValue.toObject();
                    QPoint point(pointObj["x"].toInt(), pointObj["y"].toInt());
                    path.append(point);
                }
            }
        }
        
        // 创建路径结果
        PathResult result("", startPoint, endPoint, path, algorithm, calculationTime);
        result.setTimestamp(timestamp);
        results.append(result);
    }
    
    db.close();
    QSqlDatabase::removeDatabase("load_connection");
    
    emit operationFinished(true, QString("成功从SQLite数据库加载 %1 条路径结果").arg(results.size()));
    return results;
}

bool DataManager::initializeDatabase(const QString& filename) {
    // 暂时简单实现，后续会完善
    Q_UNUSED(filename)
    return true;
}

bool DataManager::createXmlDocument(const QString& filename, const QVector<PathResult>& results) {
    // 待实现
    Q_UNUSED(filename)
    Q_UNUSED(results)
    return true;
}

bool DataManager::parseXmlDocument(const QString& filename, QVector<PathResult>& results) {
    // 待实现
    Q_UNUSED(filename)
    Q_UNUSED(results)
    return true;
}

bool DataManager::createSqliteConnection(const QString& filename) {
    // 待实现
    Q_UNUSED(filename)
    return true;
}

bool DataManager::createTables() {
    // 待实现
    return true;
}

void DataManager::closeSqliteConnection() {
    // 待实现
}
