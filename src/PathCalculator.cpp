#include "PathCalculator.h"
#include <QElapsedTimer>

PathCalculator::PathCalculator(QObject* parent)
    : QObject(parent)
    , m_algorithm(AlgorithmType::AStar)
    , m_isPaused(false)
    , m_isStopped(false)
{
}

void PathCalculator::setGrid(const QVector<QVector<GridPoint>>& grid) {
    m_grid = grid;
}

void PathCalculator::setAlgorithm(AlgorithmType algorithm) {
    m_algorithm = algorithm;
}

void PathCalculator::calculatePath(const QPoint& start, const QPoint& end) {
    m_isPaused = false;
    m_isStopped = false;
    
    emit progressUpdated(0);
    
    QElapsedTimer timer;
    timer.start();
    
    QVector<QPoint> path;
    
    // 根据选择的算法计算路径
    switch (m_algorithm) {
        case AlgorithmType::AStar:
            path = calculateAStar(start, end);
            break;
        case AlgorithmType::Dijkstra:
            path = calculateDijkstra(start, end);
            break;
        case AlgorithmType::BFS:
            path = calculateBFS(start, end);
            break;
        case AlgorithmType::DFS:
            path = calculateDFS(start, end);
            break;
    }
    
    qint64 elapsed = timer.elapsed();
    
    if (!m_isStopped) {
        if (!path.isEmpty()) {
            PathResult result("", start, end, path, m_algorithm, elapsed);
            emit pathFound(result);
        } else {
            emit pathNotFound();
        }
    }
    
    emit progressUpdated(100);
    emit calculationFinished();
}

void PathCalculator::pauseCalculation() {
    m_isPaused = true;
}

void PathCalculator::resumeCalculation() {
    m_isPaused = false;
}

void PathCalculator::stopCalculation() {
    m_isStopped = true;
}

// 算法实现（暂时简单实现，后续会完善）
QVector<QPoint> PathCalculator::calculateAStar(const QPoint& start, const QPoint& end) {
    // 简单的直线路径作为占位符
    QVector<QPoint> path;
    path.append(start);
    path.append(end);
    return path;
}

QVector<QPoint> PathCalculator::calculateDijkstra(const QPoint& start, const QPoint& end) {
    // 简单的直线路径作为占位符
    QVector<QPoint> path;
    path.append(start);
    path.append(end);
    return path;
}

QVector<QPoint> PathCalculator::calculateBFS(const QPoint& start, const QPoint& end) {
    // 简单的直线路径作为占位符
    QVector<QPoint> path;
    path.append(start);
    path.append(end);
    return path;
}

QVector<QPoint> PathCalculator::calculateDFS(const QPoint& start, const QPoint& end) {
    // 简单的直线路径作为占位符
    QVector<QPoint> path;
    path.append(start);
    path.append(end);
    return path;
}
