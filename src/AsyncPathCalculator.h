#ifndef ASYNCPATHCALCULATOR_H
#define ASYNCPATHCALCULATOR_H

#include "Common.h"
#include "GridPoint.h"
#include "PathResult.h"
#include <QObject>
#include <QThread>
#include <QVector>
#include <QPoint>
#include <QQueue>
#include <QMutex>
#include <QTimer>
#include <QElapsedTimer>
#include <QWaitCondition>
#include <QHash>
#include <atomic>

// 为QPoint添加哈希函数支持
inline uint qHash(const QPoint &key, uint seed = 0) {
    return qHash(key.x(), seed) ^ qHash(key.y(), seed + 1);
}

// 计算任务结构
struct CalculationTask {
    QPoint start;
    QPoint end;
    AlgorithmType algorithm;
    int taskId;
    
    CalculationTask() : taskId(-1) {}
    CalculationTask(const QPoint& s, const QPoint& e, AlgorithmType algo, int id)
        : start(s), end(e), algorithm(algo), taskId(id) {}
};

// 计算结果结构
struct CalculationResult {
    PathResult pathResult;
    int taskId;
    bool isComplete;  // 是否为完整计算完成
    bool isPartial;   // 是否为部分结果
    
    CalculationResult() : taskId(-1), isComplete(false), isPartial(false) {}
    CalculationResult(const PathResult& result, int id, bool complete = true, bool partial = false)
        : pathResult(result), taskId(id), isComplete(complete), isPartial(partial) {}
};

// 工作线程类
class PathCalculatorWorker : public QObject {
    Q_OBJECT

public:
    explicit PathCalculatorWorker(QObject* parent = nullptr);
    ~PathCalculatorWorker();
    
    void setGrid(const QVector<QVector<GridPoint>>& grid);
    void addTask(const CalculationTask& task);
    void pauseCalculation();
    void resumeCalculation();
    void stopCalculation();
    void stopAllTasks();
    
    // 获取结果队列中的数据（线程安全）
    bool getNextResult(CalculationResult& result);
    int getResultQueueSize();

public slots:
    void processNextTask();

signals:
    void taskStarted(int taskId);
    void taskProgress(int taskId, int percentage);
    void taskCompleted(int taskId);
    void allTasksCompleted();

private:
    QVector<QVector<GridPoint>> m_grid;
    QQueue<CalculationTask> m_taskQueue;
    QQueue<CalculationResult> m_resultQueue;
    QMutex m_taskMutex;
    QMutex m_resultMutex;
    QWaitCondition m_taskCondition;
    
    std::atomic<bool> m_isPaused;
    std::atomic<bool> m_isStopped;
    std::atomic<bool> m_isRunning;
    
    // 算法实现
    void calculatePathAsync(const CalculationTask& task);
    QVector<QPoint> calculateDFS(const QPoint& start, const QPoint& end, int taskId);
    
    // 基于用户算法的哈密顿路径查找
    void findHamiltonianPathsDFS(int x, int y, int endX, int endY, int totalPassableCells,
                               QVector<QPoint>& currentPath, QVector<QVector<QPoint>>& allPaths,
                               QVector<QVector<bool>>& visited, int taskId);
    bool isValidDFS(int x, int y, const QVector<QVector<bool>>& visited) const;
    
    // 工具方法
    bool isValidPoint(const QPoint& point) const;
    QVector<QPoint> getNeighbors(const QPoint& point) const;
    QVector<QPoint> findSimplePath(const QPoint& start, const QPoint& end, const QSet<QPoint>& allPoints);
    void addPartialResult(const CalculationTask& task, const QVector<QPoint>& partialPath, int progress);
    void addPartialPathResult(const QPoint& start, const QPoint& end,
                            const QVector<QPoint>& path, 
                            int pathNumber, int totalPaths, int taskId);
    void addFinalResult(const CalculationTask& task, const QVector<QPoint>& finalPath, qint64 elapsed);
    
    // 等待暂停状态结束
    void waitForResume();
};

// 主异步计算器类
class AsyncPathCalculator : public QObject {
    Q_OBJECT

public:
    explicit AsyncPathCalculator(QObject* parent = nullptr);
    ~AsyncPathCalculator();
    
    void setGrid(const QVector<QVector<GridPoint>>& grid);
    
    // 添加计算任务
    int addCalculationTask(const QPoint& start, const QPoint& end, AlgorithmType algorithm);
    
    // 控制方法
    void pauseAllCalculations();
    void resumeAllCalculations();
    void stopAllCalculations();
    void stopTask(int taskId);
    
    // 启动和停止结果检查器
    void startResultChecker(int intervalMs = 100);
    void stopResultChecker();
    
    // 获取状态信息
    int getPendingTaskCount();
    int getResultQueueSize();

signals:
    void pathFound(const PathResult& result, int taskId);
    void partialPathFound(const PathResult& result, int taskId);
    void pathNotFound(int taskId);
    void calculationProgress(int taskId, int percentage);
    void calculationStarted(int taskId);
    void calculationFinished(int taskId);
    void allCalculationsFinished();

private slots:
    void checkResults();
    void onTaskStarted(int taskId);
    void onTaskProgress(int taskId, int percentage);
    void onTaskCompleted(int taskId);
    void onAllTasksCompleted();

private:
    PathCalculatorWorker* m_worker;
    QThread* m_workerThread;
    QTimer* m_resultTimer;
    
    int m_nextTaskId;
    QMutex m_taskIdMutex;
    
    // 获取下一个任务ID
    int getNextTaskId();
};

#endif // ASYNCPATHCALCULATOR_H
