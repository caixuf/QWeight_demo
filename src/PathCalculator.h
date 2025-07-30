#ifndef PATHCALCULATOR_H
#define PATHCALCULATOR_H

#include "Common.h"
#include "GridPoint.h"
#include "PathResult.h"
#include <QObject>
#include <QThread>
#include <QVector>
#include <QPoint>

class PathCalculator : public QObject {
    Q_OBJECT

public:
    explicit PathCalculator(QObject* parent = nullptr);
    
    void setGrid(const QVector<QVector<GridPoint>>& grid);
    void setAlgorithm(AlgorithmType algorithm);
    
public slots:
    void calculatePath(const QPoint& start, const QPoint& end);
    void pauseCalculation();
    void resumeCalculation();
    void stopCalculation();

signals:
    void pathFound(const PathResult& result);
    void pathNotFound();
    void progressUpdated(int percentage);
    void calculationFinished();

private:
    QVector<QVector<GridPoint>> m_grid;
    AlgorithmType m_algorithm;
    bool m_isPaused;
    bool m_isStopped;
    
    // 算法实现（后续完善）
    QVector<QPoint> calculateAStar(const QPoint& start, const QPoint& end);
    QVector<QPoint> calculateDijkstra(const QPoint& start, const QPoint& end);
    QVector<QPoint> calculateBFS(const QPoint& start, const QPoint& end);
    QVector<QPoint> calculateDFS(const QPoint& start, const QPoint& end);
};

#endif // PATHCALCULATOR_H
