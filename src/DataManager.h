#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "Common.h"
#include "PathResult.h"
#include <QObject>
#include <QString>
#include <QVector>

class DataManager : public QObject {
    Q_OBJECT

public:
    explicit DataManager(QObject* parent = nullptr);
    
    // XML操作
    bool saveToXml(const QString& filename, const QVector<PathResult>& results);
    QVector<PathResult> loadFromXml(const QString& filename);
    
    // SQLite操作
    bool saveToSqlite(const QString& filename, const QVector<PathResult>& results);
    QVector<PathResult> loadFromSqlite(const QString& filename);
    
    // 数据库初始化
    bool initializeDatabase(const QString& filename);

signals:
    void operationFinished(bool success, const QString& message);

private:
    bool createXmlDocument(const QString& filename, const QVector<PathResult>& results);
    bool parseXmlDocument(const QString& filename, QVector<PathResult>& results);
    
    bool createSqliteConnection(const QString& filename);
    bool createTables();
    void closeSqliteConnection();
    
    QString m_currentDbFile;
};

#endif // DATAMANAGER_H
