#include "HistoryManager.h"

HistoryManager::HistoryManager(QObject* parent)
    : QObject(parent)
{
}

void HistoryManager::addToHistory(const PathResult& result) {
    m_history.append(result);
    emit historyUpdated();
}

void HistoryManager::clearHistory() {
    m_history.clear();
    emit historyUpdated();
}

QVector<PathResult> HistoryManager::getHistory() const {
    return m_history;
}

bool HistoryManager::saveHistoryToFile(const QString& filename) {
    // 暂时简单实现，后续会完善
    Q_UNUSED(filename)
    return true;
}

bool HistoryManager::loadHistoryFromFile(const QString& filename) {
    // 暂时简单实现，后续会完善
    Q_UNUSED(filename)
    return true;
}
