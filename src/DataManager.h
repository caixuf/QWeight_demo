#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "Common.h"
#include "PathResult.h"
#include <QObject>
#include <QString>
#include <QVector>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>

class DataManager : public QObject {
    Q_OBJECT

public:
    explicit DataManager(QObject* parent = nullptr);
    ~DataManager();
    
    // 文件类型检测
    enum FileType {
        Unknown,
        Xml,
        Sqlite,
        Csv
    };
    
    // XML操作
    bool saveToXml(const QString& filename, const QVector<PathResult>& results);
    QVector<PathResult> loadFromXml(const QString& filename);
    
    // SQLite操作
    bool saveToSqlite(const QString& filename, const QVector<PathResult>& results);
    QVector<PathResult> loadFromSqlite(const QString& filename);
    
    // CSV操作
    bool saveToCsv(const QString& filename, const QVector<PathResult>& results);
    
    // 自动检测并加载
    QVector<PathResult> loadFromFile(const QString& filename);
    FileType detectFileType(const QString& filename) const;
    
    // 数据目录管理
    QString getDataDirectory() const;
    QString getDefaultXmlFile() const;
    QString getDefaultSqliteFile() const;
    QString getDefaultCsvFile() const;
    
    // 数据库初始化
    bool initializeDatabase(const QString& filename);

signals:
    void operationFinished(bool success, const QString& message);
    void progressChanged(int percentage);

private:
    // XML相关
    bool createXmlDocument(const QString& filename, const QVector<PathResult>& results);
    bool parseXmlDocument(const QString& filename, QVector<PathResult>& results);
    void writePathResultToXml(QXmlStreamWriter& writer, const PathResult& result, int index);
    PathResult readPathResultFromXml(QXmlStreamReader& reader);
    
    // SQLite相关
    bool createSqliteConnection(const QString& filename);
    bool createTables();
    void closeSqliteConnection();
    
    // 工具函数
    void initializeDataDirectory();
    QString formatPath(const QVector<QPoint>& path) const;
    QVector<QPoint> parsePath(const QString& pathString) const;
    
    QString m_currentDbFile;
    QString m_dataDirectory;
    QSqlDatabase m_database;
};

#endif // DATAMANAGER_H
