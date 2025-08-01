#include "AsyncPathCalculator.h"
#include <QDebug>
#include <QApplication>
#include <QThread>
#include <QtMath>
#include <algorithm>

// PathCalculatorWorker 实现

PathCalculatorWorker::PathCalculatorWorker(QObject* parent)
    : QObject(parent)
    , m_isPaused(false)
    , m_isStopped(false)
    , m_isRunning(false)
{
}

PathCalculatorWorker::~PathCalculatorWorker() {
    stopAllTasks();
}

void PathCalculatorWorker::setGrid(const QVector<QVector<GridPoint>>& grid) {
    QMutexLocker locker(&m_taskMutex);
    m_grid = grid;
}

void PathCalculatorWorker::addTask(const CalculationTask& task) {
    QMutexLocker locker(&m_taskMutex);
    m_taskQueue.enqueue(task);
    m_taskCondition.wakeOne();
    
    // 如果没有在运行，启动处理
    if (!m_isRunning) {
        QMetaObject::invokeMethod(this, "processNextTask", Qt::QueuedConnection);
    }
}

void PathCalculatorWorker::pauseCalculation() {
    m_isPaused = true;
}

void PathCalculatorWorker::resumeCalculation() {
    m_isPaused = false;
    m_taskCondition.wakeAll();
}

void PathCalculatorWorker::stopCalculation() {
    m_isStopped = true;
    m_taskCondition.wakeAll();
}

void PathCalculatorWorker::stopAllTasks() {
    m_isStopped = true;
    m_taskCondition.wakeAll();
    
    // 清空任务队列
    QMutexLocker taskLocker(&m_taskMutex);
    m_taskQueue.clear();
    
    // 清空结果队列
    QMutexLocker resultLocker(&m_resultMutex);
    m_resultQueue.clear();
}

bool PathCalculatorWorker::getNextResult(CalculationResult& result) {
    QMutexLocker locker(&m_resultMutex);
    if (!m_resultQueue.isEmpty()) {
        result = m_resultQueue.dequeue();
        return true;
    }
    return false;
}

int PathCalculatorWorker::getResultQueueSize() {
    QMutexLocker locker(&m_resultMutex);
    return m_resultQueue.size();
}

void PathCalculatorWorker::processNextTask() {
    m_isRunning = true;
    
    while (!m_isStopped) {
        CalculationTask task;
        
        // 获取下一个任务
        {
            QMutexLocker locker(&m_taskMutex);
            if (m_taskQueue.isEmpty()) {
                if (!m_isStopped) {
                    // 等待新任务
                    m_taskCondition.wait(&m_taskMutex, 1000); // 1秒超时
                }
                if (m_taskQueue.isEmpty()) {
                    continue;
                }
            }
            task = m_taskQueue.dequeue();
        }
        
        // 处理任务
        if (!m_isStopped) {
            QMetaObject::invokeMethod(this, [this, task]() {
                emit taskStarted(task.taskId);
            }, Qt::QueuedConnection);
            calculatePathAsync(task);
        }
    }
    
    m_isRunning = false;
    QMetaObject::invokeMethod(this, [this]() {
        emit allTasksCompleted();
    }, Qt::QueuedConnection);
}

void PathCalculatorWorker::calculatePathAsync(const CalculationTask& task) {
    QElapsedTimer timer;
    timer.start();
    
    QMetaObject::invokeMethod(this, [this, task]() {
        emit taskProgress(task.taskId, 0);
    }, Qt::QueuedConnection);
    
    QVector<QPoint> path;
    
    try {
        // 固定使用DFS算法
        path = calculateDFS(task.start, task.end, task.taskId);
    } catch (...) {
        // 处理异常
        qDebug() << "计算过程中发生异常，任务ID:" << task.taskId;
    }
    
    qint64 elapsed = timer.elapsed();
    
    if (!m_isStopped) {
        // 添加最终结果
        addFinalResult(task, path, elapsed);
        QMetaObject::invokeMethod(this, [this, task, elapsed]() {
            emit taskProgress(task.taskId, 100);
            emit taskCompleted(task.taskId);
        }, Qt::QueuedConnection);
    }
}

QVector<QPoint> PathCalculatorWorker::calculateDFS(const QPoint& start, const QPoint& end, int taskId) {
    // 前置检查：起点终点有效性
    if (m_grid.isEmpty() || !isValidPoint(start) || !isValidPoint(end)) {
        qDebug() << "前置检查失败: grid空=" << m_grid.isEmpty() 
                 << "start有效=" << isValidPoint(start) 
                 << "end有效=" << isValidPoint(end);
        return QVector<QPoint>();
    }
    
    // 计算总的可通行点数量
    int totalPassableCells = 0;
    for (int i = 0; i < m_grid.size(); i++) {
        for (int j = 0; j < m_grid[0].size(); j++) {
            if (m_grid[i][j].isWalkable()) {
                totalPassableCells++;
            }
        }
    }
    
    qDebug() << "开始查找哈密顿路径 - 起点:" << start << "终点:" << end;
    qDebug() << "网格大小:" << m_grid.size() << "x" << m_grid[0].size() << "可通行点数量:" << totalPassableCells;
    
    // 使用您提供的哈密顿路径算法查找所有路径
    QVector<QVector<QPoint>> allPaths;
    QVector<QPoint> currentPath;
    QVector<QVector<bool>> visited(m_grid.size(), QVector<bool>(m_grid[0].size(), false));
    
    // 开始哈密顿DFS搜索
    findHamiltonianPathsDFS(start.y(), start.x(), end.y(), end.x(), totalPassableCells,
                           currentPath, allPaths, visited, taskId);
    
    qDebug() << "找到哈密顿路径数量:" << allPaths.size();
    
    // 返回第一条路径（如果有的话）
    // 注意：所有路径已经在计算过程中通过 addPartialPathResult 放入队列了
    if (!allPaths.isEmpty()) {
        return allPaths.first();
    }
    
    qDebug() << "未找到哈密顿路径";
    return QVector<QPoint>();
}



bool PathCalculatorWorker::isValidPoint(const QPoint& point) const {
    return point.x() >= 0 && point.x() < m_grid[0].size() &&
           point.y() >= 0 && point.y() < m_grid.size() &&
           m_grid[point.y()][point.x()].isWalkable();
}

QVector<QPoint> PathCalculatorWorker::getNeighbors(const QPoint& point) const {
    QVector<QPoint> neighbors;
    const int dx[] = {-1, 1, 0, 0};  // 左、右、上、下
    const int dy[] = {0, 0, -1, 1};
    
    for (int i = 0; i < 4; ++i) {
        QPoint neighbor(point.x() + dx[i], point.y() + dy[i]);
        if (isValidPoint(neighbor)) {
            neighbors.append(neighbor);
        }
    }
    
    return neighbors;
}

// 基于您提供的哈密顿路径算法的Qt版本实现
void PathCalculatorWorker::findHamiltonianPathsDFS(int x, int y, int endX, int endY, int totalPassableCells,
                                                  QVector<QPoint>& currentPath, QVector<QVector<QPoint>>& allPaths,
                                                  QVector<QVector<bool>>& visited, int taskId) {
    // 检查是否被停止 - 在递归开始时立即检查
    if (m_isStopped || allPaths.size() >= 5000) { // 增加限制到5000条哈密顿路径
        return;
    }
    
    // 检查暂停状态，但要快速响应停止信号
    if (m_isPaused && !m_isStopped) {
        QMutexLocker locker(&m_taskMutex);
        while (m_isPaused && !m_isStopped) {
            m_taskCondition.wait(&m_taskMutex, 50); // 减少等待时间到50ms
        }
        // 暂停恢复后再次检查停止状态
        if (m_isStopped) {
            return;
        }
    }
    
    // 标记当前位置为已访问
    visited[x][y] = true;
    currentPath.append(QPoint(y, x)); // 注意：QPoint是(x,y)，但我们的坐标是(row,col)
    
    // 进度更新
    if (currentPath.size() % 3 == 0) {
        int progress = (currentPath.size() * 80) / totalPassableCells;
        QMetaObject::invokeMethod(this, [this, taskId, progress]() {
            emit taskProgress(taskId, progress);
        }, Qt::QueuedConnection);
    }
    
    // 到达终点且访问了所有可通行点（哈密顿路径条件）
    if (x == endX && y == endY && currentPath.size() == totalPassableCells) {
        allPaths.append(currentPath);
        qDebug() << "找到哈密顿路径" << allPaths.size() << "，长度:" << currentPath.size();
        
        // 立即将这条路径放入结果队列，供主线程取出并显示
        // 注意：currentPath中的QPoint已经是正确的(x,y)坐标了
        QPoint start = currentPath.first();
        QPoint end = currentPath.last();
        addPartialPathResult(start, end, currentPath, allPaths.size(), -1, taskId); // -1表示总数未知，正在计算中
        
        // 回溯
        visited[x][y] = false;
        currentPath.removeLast();
        return;
    }
    
    // 如果还没有访问完所有可通行点，继续搜索
    if (currentPath.size() < totalPassableCells) {
        // 四个方向：上、下、左、右
        const int dx[4] = {-1, 1, 0, 0};
        const int dy[4] = {0, 0, -1, 1};
        
        // 尝试四个方向
        for (int i = 0; i < 4; i++) {
            // 在每次循环迭代中检查停止标志
            if (m_isStopped || allPaths.size() >= 5000) {
                break; // 立即退出循环
            }
            
            int newX = x + dx[i];
            int newY = y + dy[i];
            
            if (isValidDFS(newX, newY, visited)) {
                findHamiltonianPathsDFS(newX, newY, endX, endY, totalPassableCells,
                                      currentPath, allPaths, visited, taskId);
            }
        }
    }
    
    // 回溯
    visited[x][y] = false;
    currentPath.removeLast();
}

bool PathCalculatorWorker::isValidDFS(int x, int y, const QVector<QVector<bool>>& visited) const {
    return x >= 0 && x < m_grid.size() && 
           y >= 0 && y < m_grid[0].size() && 
           m_grid[x][y].isWalkable() && !visited[x][y];
}

void PathCalculatorWorker::addPartialResult(const CalculationTask& task, 
                                          const QVector<QPoint>& partialPath, int progress) {
    PathResult result(QString("部分结果_%1_%2").arg(task.taskId).arg(progress),
                     task.start, task.end, partialPath, task.algorithm, 0);
    
    CalculationResult calcResult(result, task.taskId, false, true);
    
    QMutexLocker locker(&m_resultMutex);
    m_resultQueue.enqueue(calcResult);
}

void PathCalculatorWorker::addPartialPathResult(const QPoint& start, const QPoint& end,
                                              const QVector<QPoint>& path, 
                                              int pathNumber, int totalPaths, int taskId) {
    QString resultName;
    if (totalPaths > 0) {
        // 如果知道总数，显示为 "路径 x/总数"
        resultName = QString("哈密顿路径_%1/%2").arg(pathNumber).arg(totalPaths);
    } else {
        // 如果不知道总数（计算中），显示为 "路径 x (计算中...)"
        resultName = QString("哈密顿路径_%1 (计算中...)").arg(pathNumber);
    }
    
    PathResult result(resultName, start, end, path, AlgorithmType::DFS, 0);
    
    CalculationResult calcResult(result, taskId, false, true);
    
    QMutexLocker locker(&m_resultMutex);
    m_resultQueue.enqueue(calcResult);
    
    qDebug() << "添加哈密顿路径到结果队列:" << resultName << "路径长度:" << path.size();
}

void PathCalculatorWorker::addFinalResult(const CalculationTask& task, 
                                        const QVector<QPoint>& finalPath, qint64 elapsed) {
    QString resultName = QString("任务_%1_最终结果").arg(task.taskId);
    PathResult result(resultName, task.start, task.end, finalPath, task.algorithm, elapsed);
    
    CalculationResult calcResult(result, task.taskId, true, false);
    
    QMutexLocker locker(&m_resultMutex);
    m_resultQueue.enqueue(calcResult);
}

void PathCalculatorWorker::waitForResume() {
    if (m_isPaused && !m_isStopped) {
        QMutexLocker locker(&m_taskMutex);
        while (m_isPaused && !m_isStopped) {
            m_taskCondition.wait(&m_taskMutex, 50); // 减少等待时间到50ms，更快响应停止信号
        }
    }
}

// AsyncPathCalculator 实现

AsyncPathCalculator::AsyncPathCalculator(QObject* parent)
    : QObject(parent)
    , m_worker(nullptr)
    , m_workerThread(nullptr)
    , m_resultTimer(nullptr)
    , m_nextTaskId(1)
{
    // 创建结果检查定时器
    m_resultTimer = new QTimer(this);
    connect(m_resultTimer, &QTimer::timeout, this, &AsyncPathCalculator::checkResults);
    
    // 创建工作线程和worker
    recreateWorkerThread();
}

AsyncPathCalculator::~AsyncPathCalculator() {
    stopResultChecker();
    
    if (m_workerThread && m_workerThread->isRunning()) {
        // 强制终止线程
        m_workerThread->terminate();
        m_workerThread->wait(1000);
    }
    
    if (m_worker) {
        delete m_worker;
        m_worker = nullptr;
    }
    
    if (m_workerThread) {
        delete m_workerThread;
        m_workerThread = nullptr;
    }
}

void AsyncPathCalculator::setGrid(const QVector<QVector<GridPoint>>& grid) {
    // 保存网格数据
    m_gridData = grid;
    
    if (m_worker) {
        m_worker->setGrid(grid);
    }
}

int AsyncPathCalculator::addCalculationTask(const QPoint& start, const QPoint& end, AlgorithmType algorithm) {
    int taskId = getNextTaskId();
    CalculationTask task(start, end, algorithm, taskId);
    
    if (m_worker) {
        m_worker->addTask(task);
    }
    
    return taskId;
}

void AsyncPathCalculator::pauseAllCalculations() {
    if (m_worker) {
        m_worker->pauseCalculation();
    }
}

void AsyncPathCalculator::resumeAllCalculations() {
    if (m_worker) {
        m_worker->resumeCalculation();
    }
}

void AsyncPathCalculator::stopAllCalculations() {
    if (m_worker && m_workerThread) {
        // 停止结果检查器
        stopResultChecker();
        
        // 激进停止策略：直接终止线程并重新创建
        qDebug() << "正在强制停止工作线程...";
        
        // 立即终止线程
        if (m_workerThread->isRunning()) {
            m_workerThread->terminate();
            m_workerThread->wait(500); // 只等待500ms
        }
        
        // 清理旧的worker和线程
        if (m_worker) {
            delete m_worker;
            m_worker = nullptr;
        }
        
        delete m_workerThread;
        m_workerThread = nullptr;
        
        // 重新创建工作线程和worker
        recreateWorkerThread();
        
        qDebug() << "工作线程已重新创建";
        
        // 发出停止完成信号
        emit allCalculationsFinished();
    }
}

void AsyncPathCalculator::recreateWorkerThread() {
    qDebug() << "重新创建工作线程...";
    
    // 创建新的工作线程
    m_workerThread = new QThread(this);
    
    // 创建新的worker
    m_worker = new PathCalculatorWorker();
    m_worker->moveToThread(m_workerThread);
    
    // 连接线程启动信号
    connect(m_workerThread, &QThread::started,
            m_worker, &PathCalculatorWorker::processNextTask);
    
    // 连接worker信号
    connect(m_worker, &PathCalculatorWorker::taskStarted,
            this, &AsyncPathCalculator::onTaskStarted);
    connect(m_worker, &PathCalculatorWorker::taskProgress,
            this, &AsyncPathCalculator::onTaskProgress);
    connect(m_worker, &PathCalculatorWorker::taskCompleted,
            this, &AsyncPathCalculator::onTaskCompleted);
    connect(m_worker, &PathCalculatorWorker::allTasksCompleted,
            this, &AsyncPathCalculator::onAllTasksCompleted);
    
    // 如果有网格数据，重新设置
    if (!m_gridData.isEmpty()) {
        qDebug() << "重新设置网格数据到新worker";
        m_worker->setGrid(m_gridData);
    }
    
    // 启动新线程
    m_workerThread->start();
    qDebug() << "新工作线程已启动";
}

void AsyncPathCalculator::stopTask(int taskId) {
    // 这个功能需要更复杂的实现来停止特定任务
    Q_UNUSED(taskId);
}

void AsyncPathCalculator::startResultChecker(int intervalMs) {
    if (m_resultTimer) {
        m_resultTimer->start(intervalMs);
    }
}

void AsyncPathCalculator::stopResultChecker() {
    if (m_resultTimer) {
        m_resultTimer->stop();
    }
}

int AsyncPathCalculator::getPendingTaskCount() {
    // 需要添加获取待处理任务数量的方法
    return 0;
}

int AsyncPathCalculator::getResultQueueSize() {
    if (m_worker) {
        return m_worker->getResultQueueSize();
    }
    return 0;
}

void AsyncPathCalculator::checkResults() {
    if (!m_worker) return;
    
    CalculationResult result;
    while (m_worker->getNextResult(result)) {
        if (result.isPartial) {
            emit partialPathFound(result.pathResult, result.taskId);
        } else if (result.isComplete) {
            if (result.pathResult.getPath().isEmpty()) {
                emit pathNotFound(result.taskId);
            } else {
                emit pathFound(result.pathResult, result.taskId);
            }
            emit calculationFinished(result.taskId);
        }
    }
}

void AsyncPathCalculator::onTaskStarted(int taskId) {
    emit calculationStarted(taskId);
}

void AsyncPathCalculator::onTaskProgress(int taskId, int percentage) {
    emit calculationProgress(taskId, percentage);
}

void AsyncPathCalculator::onTaskCompleted(int taskId) {
    // 任务完成会在checkResults中处理
    Q_UNUSED(taskId);
}

void AsyncPathCalculator::onAllTasksCompleted() {
    emit allCalculationsFinished();
}

int AsyncPathCalculator::getNextTaskId() {
    QMutexLocker locker(&m_taskIdMutex);
    return m_nextTaskId++;
}
