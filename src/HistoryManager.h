#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include "Common.h"
#include "PathResult.h"
#include <QObject>
#include <QString>
#include <QVector>

class HistoryManager : public QObject {
    Q_OBJECT

public:
    explicit HistoryManager(QObject* parent = nullptr);
    
    // 历史记录管理
    void addToHistory(const PathResult& result);
    void clearHistory();
    QVector<PathResult> getHistory() const;
    
    // 文件操作
    bool saveHistoryToFile(const QString& filename);
    bool loadHistoryFromFile(const QString& filename);

signals:
    void historyUpdated();
    void historyLoaded(const QVector<PathResult>& results);

private:
    QVector<PathResult> m_history;
    QString m_currentHistoryFile;
};

#endif // HISTORYMANAGER_H
