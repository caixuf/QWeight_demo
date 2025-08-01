#include "MainWindow.h"
#include "LayoutTestWindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTime>
#include <QThread>
#include <QSet>
#include <algorithm>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_pointSetMode(None)
    , m_calculationState(CalculationState::Idle)
    , m_currentStartPoint(-1, -1)
    , m_currentEndPoint(-1, -1)
    , m_isCalculating(false)
    , m_shouldStopCalculation(false)
    , m_totalPathCount(0)
    , m_layoutTestWindow(nullptr)
{
    qDebug() << "MainWindow构造函数开始...";
    
    try {
        qDebug() << "初始化数据管理器...";
        m_dataManager = new DataManager(this);
        connect(m_dataManager, &DataManager::operationFinished,
                this, &MainWindow::onDataOperationFinished);
        connect(m_dataManager, &DataManager::progressChanged,
                this, &MainWindow::onDataProgress);
        
        qDebug() << "初始化异步路径计算器...";
        m_asyncCalculator = new AsyncPathCalculator(this);
        connect(m_asyncCalculator, &AsyncPathCalculator::pathFound,
                this, &MainWindow::onAsyncPathFound);
        connect(m_asyncCalculator, &AsyncPathCalculator::partialPathFound,
                this, &MainWindow::onAsyncPartialPathFound);
        connect(m_asyncCalculator, &AsyncPathCalculator::pathNotFound,
                this, &MainWindow::onAsyncPathNotFound);
        connect(m_asyncCalculator, &AsyncPathCalculator::calculationProgress,
                this, &MainWindow::onAsyncCalculationProgress);
        connect(m_asyncCalculator, &AsyncPathCalculator::calculationStarted,
                this, &MainWindow::onAsyncCalculationStarted);
        connect(m_asyncCalculator, &AsyncPathCalculator::calculationFinished,
                this, &MainWindow::onAsyncCalculationFinished);
        connect(m_asyncCalculator, &AsyncPathCalculator::allCalculationsFinished,
                this, &MainWindow::onAsyncAllCalculationsFinished);
        
        // 启动结果检查定时器（每100ms检查一次）
        m_asyncCalculator->startResultChecker(100);
        
        qDebug() << "设置UI...";
        setupUI();
        
        qDebug() << "初始化批量处理定时器...";
        m_batchTimer = new QTimer(this);
        m_batchTimer->setInterval(100); // 每100ms处理一次队列，提高响应性
        connect(m_batchTimer, &QTimer::timeout, this, &MainWindow::processBatchQueue);
        
        qDebug() << "设置连接...";
        setupConnections();
        
        qDebug() << "设置样式...";
        setupStyles();
        
        // 设置窗口属性
        qDebug() << "设置窗口属性...";
        setWindowTitle("Qt路径计算可视化工具 v1.0");
        setMinimumSize(1000, 700);
        resize(1400, 900);
        
        qDebug() << "更新状态消息...";
        updateStatusMessage("就绪");
        
        qDebug() << "更新网格大小标签...";
        updateGridSizeLabel();  // 在所有组件初始化完成后更新标签
        
        qDebug() << "MainWindow构造函数完成";
    } catch (const std::exception& e) {
        qDebug() << "MainWindow构造函数异常:" << e.what();
    } catch (...) {
        qDebug() << "MainWindow构造函数未知异常";
    }
}

MainWindow::~MainWindow() {
    // 析构函数，Qt会自动清理子对象
}

void MainWindow::setupUI() {
    setupCentralWidget();
    setupMenuBar();
    setupStatusBar();
}

void MainWindow::setupCentralWidget() {
    qDebug() << "setupCentralWidget 开始...";
    
    try {
        qDebug() << "创建中央部件...";
        m_centralWidget = new QWidget(this);
        setCentralWidget(m_centralWidget);
        
        qDebug() << "创建主布局...";
        QVBoxLayout* mainLayout = new QVBoxLayout(m_centralWidget);
        mainLayout->setContentsMargins(5, 5, 5, 5);
        
        // 创建上半部分的水平布局
        QHBoxLayout* topLayout = new QHBoxLayout();
        
        qDebug() << "创建网格视图...";
        m_gridView = new GridGraphicsView();
        topLayout->addWidget(m_gridView, 3);  // 占3/4的空间
        
        qDebug() << "创建控制面板...";
        m_controlPanel = new ControlPanel();
        m_controlPanel->setFixedWidth(300);
        topLayout->addWidget(m_controlPanel, 1);  // 占1/4的空间
        
        // 添加上半部分到主布局
        mainLayout->addLayout(topLayout, 2);  // 占2/3空间
        
        qDebug() << "创建结果列表...";
        m_resultList = new ResultListWidget();
        mainLayout->addWidget(m_resultList, 1);  // 占1/3空间
        
        qDebug() << "setupCentralWidget 完成";
    } catch (const std::exception& e) {
        qDebug() << "setupCentralWidget 异常:" << e.what();
    } catch (...) {
        qDebug() << "setupCentralWidget 未知异常";
    }
}

void MainWindow::setupHistoryBar() {
    m_historyBar = new QWidget();
    m_historyBar->setFixedHeight(60);
    m_historyBar->setStyleSheet("QWidget { background-color: #f0f0f0; border-top: 1px solid #d0d0d0; }");
    
    m_historyLayout = new QHBoxLayout(m_historyBar);
    m_historyLayout->setContentsMargins(10, 10, 10, 10);
    
    // 历史记录按钮
    m_openXmlButton = new QPushButton("打开XML文件");
    m_openXmlButton->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    
    m_openSqliteButton = new QPushButton("打开SQLite数据库");
    m_openSqliteButton->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    
    m_clearResultsButton = new QPushButton("清空当前结果");
    m_clearResultsButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
    
    m_historyLayout->addWidget(new QLabel("历史记录:"));
    m_historyLayout->addWidget(m_openXmlButton);
    m_historyLayout->addWidget(m_openSqliteButton);
    m_historyLayout->addStretch();
    m_historyLayout->addWidget(m_clearResultsButton);
}

void MainWindow::setupMenuBar() {
    qDebug() << "setupMenuBar 开始...";
    
    try {
        m_menuBar = menuBar();
        m_fileMenu = m_menuBar->addMenu("文件(&F)");
        
        // 创建工具菜单
        QMenu* toolsMenu = m_menuBar->addMenu("工具(&T)");
        
        // 创建菜单动作
        m_openAction = new QAction("打开数据文件(&O)", this);
        m_openAction->setShortcut(QKeySequence::Open);
        m_openAction->setStatusTip("打开保存的路径计算结果");
        
        m_saveXmlAction = new QAction("保存为XML(&X)", this);
        m_saveXmlAction->setShortcut(QKeySequence("Ctrl+Shift+X"));
        m_saveXmlAction->setStatusTip("将结果保存为XML格式");
        
        m_saveSqliteAction = new QAction("保存为SQLite(&S)", this);
        m_saveSqliteAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
        m_saveSqliteAction->setStatusTip("将结果保存为SQLite数据库");
        
        m_saveCsvAction = new QAction("保存为CSV(&C)", this);
        m_saveCsvAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
        m_saveCsvAction->setStatusTip("将结果导出为CSV格式");
        
        m_exitAction = new QAction("退出(&Q)", this);
        m_exitAction->setShortcut(QKeySequence::Quit);
        m_exitAction->setStatusTip("退出应用程序");
        
        // 布局测试窗口动作
        QAction* layoutTestAction = new QAction("布局管理演示(&L)", this);
        layoutTestAction->setStatusTip("打开Qt布局管理最佳实践演示窗口");
        
        // 添加到菜单
        m_fileMenu->addAction(m_openAction);
        m_fileMenu->addSeparator();
        m_fileMenu->addAction(m_saveXmlAction);
        m_fileMenu->addAction(m_saveSqliteAction);
        m_fileMenu->addAction(m_saveCsvAction);
        m_fileMenu->addSeparator();
        m_fileMenu->addAction(m_exitAction);
        
        // 添加工具菜单项
        toolsMenu->addAction(layoutTestAction);
        
        // 连接信号
        connect(m_openAction, &QAction::triggered, this, &MainWindow::onOpenDataFile);
        connect(m_saveXmlAction, &QAction::triggered, this, &MainWindow::onSaveToXml);
        connect(m_saveSqliteAction, &QAction::triggered, this, &MainWindow::onSaveToSqlite);
        connect(m_saveCsvAction, &QAction::triggered, this, &MainWindow::onSaveToCsv);
        connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
        connect(layoutTestAction, &QAction::triggered, this, &MainWindow::onOpenLayoutTestWindow);
        
        // 初始状态设置
        updateMenuStates();
        
        qDebug() << "setupMenuBar 完成";
    } catch (const std::exception& e) {
        qDebug() << "setupMenuBar 异常:" << e.what();
    } catch (...) {
        qDebug() << "setupMenuBar 未知异常";
    }
}

void MainWindow::setupStatusBar() {
    qDebug() << "setupStatusBar 开始...";
    
    try {
        m_statusBar = statusBar();
        m_statusLabel = new QLabel("就绪");
        m_statusBar->addWidget(m_statusLabel);
        
        // 添加网格大小标签
        m_gridSizeLabel = new QLabel("网格: 20×15");
        m_statusBar->addPermanentWidget(m_gridSizeLabel);
        
        qDebug() << "setupStatusBar 完成";
    } catch (const std::exception& e) {
        qDebug() << "setupStatusBar 异常:" << e.what();
    } catch (...) {
        qDebug() << "setupStatusBar 未知异常";
    }
}

void MainWindow::setupConnections() {
    qDebug() << "setupConnections 开始...";
    
    try {
        // 网格视图连接
        if (m_gridView) {
            connect(m_gridView, &GridGraphicsView::pointClicked,
                    this, &MainWindow::onPointClicked);
            connect(m_gridView, &GridGraphicsView::startPointSet,
                    this, &MainWindow::onStartPointSet);
            connect(m_gridView, &GridGraphicsView::endPointSet,
                    this, &MainWindow::onEndPointSet);
        }
        
        // 控制面板基本连接
        if (m_controlPanel) {
            connect(m_controlPanel, &ControlPanel::applyGridSize,
                    this, &MainWindow::onApplyGridSize);
            connect(m_controlPanel, &ControlPanel::setStartPointMode,
                    this, &MainWindow::onSetStartPointMode);
            connect(m_controlPanel, &ControlPanel::setEndPointMode,
                    this, &MainWindow::onSetEndPointMode);
            connect(m_controlPanel, &ControlPanel::startCalculation,
                    this, &MainWindow::onStartCalculation);
            connect(m_controlPanel, &ControlPanel::stopCalculation,
                    this, &MainWindow::onStopCalculation);
            connect(m_controlPanel, &ControlPanel::resetCalculation,
                    this, &MainWindow::onResetCalculation);
            connect(m_controlPanel, &ControlPanel::saveToXml,
                    this, &MainWindow::onSaveToXml);
            connect(m_controlPanel, &ControlPanel::saveToSqlite,
                    this, &MainWindow::onSaveToSqlite);
        }
        
        // 结果列表连接
        if (m_resultList) {
            connect(m_resultList, &ResultListWidget::resultDoubleClicked,
                    this, &MainWindow::onResultDoubleClicked);
            connect(m_resultList, &ResultListWidget::exportResults,
                    this, &MainWindow::onExportResults);
            connect(m_resultList, &ResultListWidget::resultsChanged,
                    this, &MainWindow::updateMenuStates);
        }
        
        // 批量结果信号连接
        connect(this, &MainWindow::batchAddResults,
                this, &MainWindow::onBatchAddResults);
        
        qDebug() << "setupConnections 完成";
    } catch (const std::exception& e) {
        qDebug() << "setupConnections 异常:" << e.what();
    } catch (...) {
        qDebug() << "setupConnections 未知异常";
    }
}

void MainWindow::setupStyles() {
    qDebug() << "setupStyles 开始...";
    
    try {
        // 简单样式
        setStyleSheet("QMainWindow { background-color: #f5f5f5; }");
        
        qDebug() << "setupStyles 完成";
    } catch (const std::exception& e) {
        qDebug() << "setupStyles 异常:" << e.what();
    } catch (...) {
        qDebug() << "setupStyles 未知异常";
    }
}

void MainWindow::updateStatusMessage(const QString& message) {
    m_statusLabel->setText(message);
}

void MainWindow::showCalculationProgress(bool show) {
    // 目前没有进度条，这个方法暂时留空
    Q_UNUSED(show)
}

void MainWindow::updateCalculationProgress(int percentage) {
    m_progressBar->setValue(percentage);
}

void MainWindow::updateGridSizeLabel() {
    qDebug() << "updateGridSizeLabel 开始...";
    
    try {
        if (!m_gridView || !m_gridSizeLabel) {
            qDebug() << "网格视图或标签为空";
            return;
        }
        
        QString text = QString("网格: %1×%2")
                       .arg(m_gridView->gridWidth())
                       .arg(m_gridView->gridHeight());
        m_gridSizeLabel->setText(text);
        
        qDebug() << "updateGridSizeLabel 完成:" << text;
    } catch (const std::exception& e) {
        qDebug() << "updateGridSizeLabel 异常:" << e.what();
    } catch (...) {
        qDebug() << "updateGridSizeLabel 未知异常";
    }
}

// 槽函数实现
void MainWindow::onGridSizeChanged(int width, int height) {
    Q_UNUSED(width)
    Q_UNUSED(height)
    // 实时更新状态，但不重建网格
}

void MainWindow::onApplyGridSize() {
    qDebug() << "onApplyGridSize 被调用";
    
    int width = m_controlPanel->getGridWidth();
    int height = m_controlPanel->getGridHeight();
    
    qDebug() << "控制面板返回的网格大小:" << width << "x" << height;
    
    m_gridView->createGrid(width, height);
    
    // 验证网格是否正确设置
    int actualWidth = m_gridView->gridWidth();
    int actualHeight = m_gridView->gridHeight();
    qDebug() << "网格视图实际大小:" << actualWidth << "x" << actualHeight;
    
    updateGridSizeLabel();
    updateStatusMessage(QString("网格大小已更新为 %1×%2").arg(actualWidth).arg(actualHeight));
    
    // 重置起点终点
    m_currentStartPoint = QPoint(-1, -1);
    m_currentEndPoint = QPoint(-1, -1);
    
    // 重新创建表格 - 完全销毁之前的表格
    if (m_resultList) {
        m_resultList->recreateTable();
        qDebug() << "结果表格已重新创建";
    }
}

void MainWindow::onPointClicked(QPoint position, PointType currentType) {
    switch (m_pointSetMode) {
        case SetStart:
            if (currentType != PointType::End) {
                m_gridView->setStartPoint(position);
                m_pointSetMode = None;
                updateStatusMessage(QString("起点已设置为 (%1,%2)").arg(position.x()).arg(position.y()));
            }
            break;
            
        case SetEnd:
            if (currentType != PointType::Start) {
                m_gridView->setEndPoint(position);
                m_pointSetMode = None;
                updateStatusMessage(QString("终点已设置为 (%1,%2)").arg(position.x()).arg(position.y()));
            }
            break;
            
        case None:
            // 普通点击，可以切换障碍点
            if (currentType == PointType::Normal) {
                m_gridView->setPointType(position.x(), position.y(), PointType::Obstacle);
                updateStatusMessage(QString("在 (%1,%2) 设置障碍点").arg(position.x()).arg(position.y()));
            } else if (currentType == PointType::Obstacle) {
                m_gridView->setPointType(position.x(), position.y(), PointType::Normal);
                updateStatusMessage(QString("移除 (%1,%2) 的障碍点").arg(position.x()).arg(position.y()));
            }
            break;
    }
}

void MainWindow::onStartPointSet(QPoint position) {
    m_currentStartPoint = position;
}

void MainWindow::onEndPointSet(QPoint position) {
    m_currentEndPoint = position;
}

void MainWindow::onSetStartPointMode() {
    m_pointSetMode = SetStart;
    updateStatusMessage("点击网格设置起点");
}

void MainWindow::onSetEndPointMode() {
    m_pointSetMode = SetEnd;
    updateStatusMessage("点击网格设置终点");
}

void MainWindow::onStartCalculation() {
    qDebug() << "onStartCalculation 开始...";
    
    // 检查起点终点是否设置
    if (!m_gridView->hasStartPoint() || !m_gridView->hasEndPoint()) {
        QMessageBox::warning(this, "警告", "请先设置起点和终点！");
        return;
    }
    
    if (m_isCalculating) {
        qDebug() << "已经在计算中，返回";
        return; // 已经在计算中
    }
    
    qDebug() << "设置计算状态...";
    m_isCalculating = true;
    m_shouldStopCalculation = false;
    m_totalPathCount = 0;  // 重置路径计数器
    m_calculationState = CalculationState::Running;
    m_controlPanel->setCalculationState(m_calculationState);
    showCalculationProgress(true);
    
    // 获取起点和终点
    QPoint start = m_gridView->getStartPoint();
    QPoint end = m_gridView->getEndPoint();
    qDebug() << "起点:" << start << "终点:" << end;
    
    // 获取当前网格大小信息用于显示
    int gridWidth = m_gridView->gridWidth();
    int gridHeight = m_gridView->gridHeight();
    qDebug() << "网格大小:" << gridWidth << "x" << gridHeight;
    
    // 清空之前的结果
    qDebug() << "清空之前的结果...";
    m_resultList->clearResults();
    m_gridView->clearPath();
    
    // 获取当前选择的算法
    AlgorithmType algorithm = m_controlPanel->getCurrentAlgorithm();
    QString algorithmName = algorithmTypeToString(algorithm);
    qDebug() << "使用算法:" << algorithmName;
    
    updateStatusMessage(QString("正在使用 %1 算法异步计算路径...").arg(algorithmName));
    
    // 设置网格到异步计算器
    m_asyncCalculator->setGrid(m_gridView->getGrid());
    
    // 确保结果检查器启动
    m_asyncCalculator->startResultChecker(100);
    
    // 添加计算任务到异步计算器
    qDebug() << "=== 开始异步路径计算 ===";
    qDebug() << "起点:" << start << "终点:" << end;
    qDebug() << "网格大小:" << gridWidth << "x" << gridHeight;
    qDebug() << "算法:" << algorithmName;
    
    int taskId = m_asyncCalculator->addCalculationTask(start, end, algorithm);
    m_activeTaskAlgorithms[taskId] = algorithm;
    m_activeTaskNames[taskId] = QString("任务_%1_%2").arg(taskId).arg(algorithmName);
    
    qDebug() << "添加了计算任务，任务ID:" << taskId;
}

QVector<QPoint> MainWindow::calculateSimplePath(const QPoint& start, const QPoint& end) {
    QVector<QPoint> path;
    path.append(start);
    
    QPoint current = start;
    
    // 简单的路径：先水平移动，再垂直移动（只能走四个方向）
    while (current.x() != end.x()) {
        if (current.x() < end.x()) {
            current.setX(current.x() + 1);
        } else {
            current.setX(current.x() - 1);
        }
        path.append(current);
    }
    
    while (current.y() != end.y()) {
        if (current.y() < end.y()) {
            current.setY(current.y() + 1);
        } else {
            current.setY(current.y() - 1);
        }
        path.append(current);
    }

    return path;
}

QVector<QVector<QPoint>> MainWindow::calculateAllPossiblePaths(const QPoint& start, const QPoint& end, AlgorithmType algorithm) {
    QVector<QVector<QPoint>> allPaths;
    
    // 根据选择的算法生成多条可能的路径
    switch (algorithm) {
        case AlgorithmType::AStar:
            // A*算法 - 生成多条可能的最优路径变体
            allPaths.append(calculateAStarPath(start, end));
            // 生成更多路径变体 - 增加到50条
            for (int i = 1; i <= 50; ++i) {
                QVector<QPoint> variant = calculatePathVariant(start, end, i);
                if (!variant.isEmpty()) {
                    allPaths.append(variant);
                }
            }
            break;
            
        case AlgorithmType::Dijkstra:
            // Dijkstra算法 - 生成多条等长最短路径
            allPaths.append(calculateDijkstraPath(start, end));
            // 生成更多等长路径变体 - 增加到60条
            for (int i = 1; i <= 60; ++i) {
                QVector<QPoint> variant = calculatePathVariant(start, end, i + 50);
                if (!variant.isEmpty()) {
                    allPaths.append(variant);
                }
            }
            break;
            
        case AlgorithmType::BFS:
            // BFS - 生成所有可能的最短路径
            allPaths.append(calculateBFSPath(start, end));
            // 生成其他最短路径 - 增加到70条
            for (int i = 1; i <= 70; ++i) {
                QVector<QPoint> variant = calculatePathVariant(start, end, i + 100);
                if (!variant.isEmpty()) {
                    allPaths.append(variant);
                }
            }
            break;
            
        case AlgorithmType::DFS:
            // DFS - 使用回溯法找到所有可能的路径，边计算边返回
            {
                // 直接开始DFS计算，不预先计算所有路径
                calculateAllDFSPathsProgressive(start, end, algorithm);
                return allPaths; // 返回空的，因为路径会逐步添加到结果列表中
            }
            break;
    }
    
    // 去除重复路径
    QVector<QVector<QPoint>> uniquePaths;
    for (const QVector<QPoint>& path : allPaths) {
        bool isDuplicate = false;
        for (const QVector<QPoint>& existingPath : uniquePaths) {
            if (path == existingPath) {
                isDuplicate = true;
                break;
            }
        }
        if (!isDuplicate && !path.isEmpty()) {
            uniquePaths.append(path);
        }
    }
    
    // 用去重后的路径替换原来的
    allPaths = uniquePaths;
    
    // 确保至少有一条路径
    if (allPaths.isEmpty()) {
        allPaths.append(calculateSimplePath(start, end));
    }
    
    return allPaths;
}

QVector<QPoint> MainWindow::calculatePathVariant(const QPoint& start, const QPoint& end, int variant) {
    QVector<QPoint> path;
    path.append(start);
    
    QPoint current = start;
    
    // 根据变体类型生成不同的路径，增加更多类型
    int pathType = variant % 16; // 增加到16种不同的路径类型
    
    switch (pathType) {
        case 0: // 先水平后垂直
            while (current.x() != end.x()) {
                current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                path.append(current);
            }
            while (current.y() != end.y()) {
                current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                path.append(current);
            }
            break;
            
        case 1: // 先垂直后水平
            while (current.y() != end.y()) {
                current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                path.append(current);
            }
            while (current.x() != end.x()) {
                current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                path.append(current);
            }
            break;
            
        case 2: // 阶梯式路径1 (一步水平一步垂直)
            while (current.x() != end.x() || current.y() != end.y()) {
                if (current.x() != end.x()) {
                    current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
                if (current.y() != end.y()) {
                    current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                    path.append(current);
                }
            }
            break;
            
        case 3: // 阶梯式路径2 (一步垂直一步水平)
            while (current.x() != end.x() || current.y() != end.y()) {
                if (current.y() != end.y()) {
                    current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                    path.append(current);
                }
                if (current.x() != end.x()) {
                    current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
            }
            break;
            
        case 4: // 两步水平一步垂直
            while (current.x() != end.x() || current.y() != end.y()) {
                // 水平移动两步（如果可能）
                for (int i = 0; i < 2 && current.x() != end.x(); ++i) {
                    current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
                // 垂直移动一步
                if (current.y() != end.y()) {
                    current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                    path.append(current);
                }
            }
            break;
            
        case 5: // 一步水平两步垂直
            while (current.x() != end.x() || current.y() != end.y()) {
                // 水平移动一步
                if (current.x() != end.x()) {
                    current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
                // 垂直移动两步（如果可能）
                for (int i = 0; i < 2 && current.y() != end.y(); ++i) {
                    current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                    path.append(current);
                }
            }
            break;
            
        case 6: // 中点路径 (先到中点再到终点)
            {
                QPoint mid((start.x() + end.x()) / 2, (start.y() + end.y()) / 2);
                // 到中点
                while (current.x() != mid.x()) {
                    current.setX(current.x() + (mid.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
                while (current.y() != mid.y()) {
                    current.setY(current.y() + (mid.y() > current.y() ? 1 : -1));
                    path.append(current);
                }
                // 从中点到终点
                while (current.x() != end.x()) {
                    current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
                while (current.y() != end.y()) {
                    current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                    path.append(current);
                }
            }
            break;
            
        case 7: // 绕行路径 (稍微绕一下)
            {
                int detourX = (variant / 16) % 3 - 1; // -1, 0, 1的偏移
                int detourY = (variant / 48) % 3 - 1;
                
                QPoint detour(start.x() + detourX, start.y() + detourY);
                
                // 确保绕行点在合理范围内
                if (detour.x() >= 0 && detour.y() >= 0) {
                    // 先到绕行点
                    while (current.x() != detour.x()) {
                        current.setX(current.x() + (detour.x() > current.x() ? 1 : -1));
                        path.append(current);
                    }
                    while (current.y() != detour.y()) {
                        current.setY(current.y() + (detour.y() > current.y() ? 1 : -1));
                        path.append(current);
                    }
                }
                
                // 从绕行点到终点
                while (current.x() != end.x()) {
                    current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
                while (current.y() != end.y()) {
                    current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                    path.append(current);
                }
            }
            break;
            
        case 8: // 三步水平一步垂直模式
            while (current.x() != end.x() || current.y() != end.y()) {
                for (int i = 0; i < 3 && current.x() != end.x(); ++i) {
                    current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
                if (current.y() != end.y()) {
                    current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                    path.append(current);
                }
            }
            break;
            
        case 9: // 一步水平三步垂直模式
            while (current.x() != end.x() || current.y() != end.y()) {
                if (current.x() != end.x()) {
                    current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
                for (int i = 0; i < 3 && current.y() != end.y(); ++i) {
                    current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                    path.append(current);
                }
            }
            break;
            
        case 10: // 分段式路径：先移动1/3，再移动2/3
            {
                int totalX = end.x() - start.x();
                QPoint mid1(start.x() + totalX / 3, start.y());
                QPoint mid2(start.x() + totalX / 3, end.y());
                
                // 到第一个中间点
                while (current.x() != mid1.x()) {
                    current.setX(current.x() + (mid1.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
                // 到第二个中间点
                while (current.y() != mid2.y()) {
                    current.setY(current.y() + (mid2.y() > current.y() ? 1 : -1));
                    path.append(current);
                }
                // 到终点
                while (current.x() != end.x()) {
                    current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
            }
            break;
            
        case 11: // L形路径变体
            {
                bool horizontalFirst = variant % 2 == 0;
                if (horizontalFirst) {
                    // 先走一半水平，再垂直，再水平
                    int halfX = (end.x() - start.x()) / 2;
                    while (current.x() != start.x() + halfX) {
                        current.setX(current.x() + (halfX > 0 ? 1 : -1));
                        path.append(current);
                    }
                    while (current.y() != end.y()) {
                        current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                        path.append(current);
                    }
                    while (current.x() != end.x()) {
                        current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                        path.append(current);
                    }
                } else {
                    // 先走一半垂直，再水平，再垂直
                    int halfY = (end.y() - start.y()) / 2;
                    while (current.y() != start.y() + halfY) {
                        current.setY(current.y() + (halfY > 0 ? 1 : -1));
                        path.append(current);
                    }
                    while (current.x() != end.x()) {
                        current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                        path.append(current);
                    }
                    while (current.y() != end.y()) {
                        current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                        path.append(current);
                    }
                }
            }
            break;
            
        case 12: // 螺旋式接近
            {
                int spiralSize = 1 + (variant % 3);
                while (current != end) {
                    // 向右
                    for (int i = 0; i < spiralSize && current.x() < end.x(); ++i) {
                        current.setX(current.x() + 1);
                        path.append(current);
                        if (current == end) break;
                    }
                    if (current == end) break;
                    
                    // 向下
                    for (int i = 0; i < spiralSize && current.y() < end.y(); ++i) {
                        current.setY(current.y() + 1);
                        path.append(current);
                        if (current == end) break;
                    }
                    if (current == end) break;
                    
                    // 如果还没到达，用简单路径补齐
                    while (current.x() != end.x()) {
                        current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                        path.append(current);
                    }
                    while (current.y() != end.y()) {
                        current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                        path.append(current);
                    }
                    break;
                }
            }
            break;
            
        case 13: // 锯齿形路径
            while (current.x() != end.x() || current.y() != end.y()) {
                // 水平移动1步
                if (current.x() != end.x()) {
                    current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
                // 垂直移动1步
                if (current.y() != end.y()) {
                    current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                    path.append(current);
                }
                // 如果还有距离，再来一次小的锯齿
                if (current.x() != end.x() && current.y() != end.y()) {
                    if (current.x() != end.x()) {
                        current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                        path.append(current);
                    }
                }
            }
            break;
            
        case 14: // 角点式路径
            {
                // 选择一个角点
                QPoint corner;
                if (variant % 4 == 0) {
                    corner = QPoint(end.x(), start.y()); // 右上角点
                } else if (variant % 4 == 1) {
                    corner = QPoint(start.x(), end.y()); // 左下角点
                } else if (variant % 4 == 2) {
                    corner = QPoint((start.x() + end.x()) / 2, start.y()); // 中上角点
                } else {
                    corner = QPoint(start.x(), (start.y() + end.y()) / 2); // 左中角点
                }
                
                // 先到角点
                while (current.x() != corner.x()) {
                    current.setX(current.x() + (corner.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
                while (current.y() != corner.y()) {
                    current.setY(current.y() + (corner.y() > current.y() ? 1 : -1));
                    path.append(current);
                }
                // 从角点到终点
                while (current.x() != end.x()) {
                    current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                    path.append(current);
                }
                while (current.y() != end.y()) {
                    current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                    path.append(current);
                }
            }
            break;
            
        case 15: // 随机偏移路径
            {
                int offset = (variant % 5) - 2; // -2, -1, 0, 1, 2的偏移
                while (current.x() != end.x() || current.y() != end.y()) {
                    // 主要移动方向
                    if (abs(end.x() - current.x()) > abs(end.y() - current.y())) {
                        // 水平距离更大，优先水平移动
                        if (current.x() != end.x()) {
                            current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                            path.append(current);
                        }
                        // 偶尔垂直移动
                        if (path.size() % 3 == offset % 3 && current.y() != end.y()) {
                            current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                            path.append(current);
                        }
                    } else {
                        // 垂直距离更大，优先垂直移动
                        if (current.y() != end.y()) {
                            current.setY(current.y() + (end.y() > current.y() ? 1 : -1));
                            path.append(current);
                        }
                        // 偶尔水平移动
                        if (path.size() % 3 == offset % 3 && current.x() != end.x()) {
                            current.setX(current.x() + (end.x() > current.x() ? 1 : -1));
                            path.append(current);
                        }
                    }
                }
            }
            break;
    }
    
    return path;
}

// 哈密顿路径计算（从起点到终点并且经过所有点）
void MainWindow::calculateSimpleDFSPath(const QPoint& start, const QPoint& end) {
    qDebug() << "开始哈密顿路径搜索，从" << start << "到" << end;
    
    int gridWidth = m_gridView->gridWidth();
    int gridHeight = m_gridView->gridHeight();
    int totalPoints = gridWidth * gridHeight;
    
    qDebug() << "网格大小:" << gridWidth << "x" << gridHeight << "，总共" << totalPoints << "个点";
    
    // 清空队列
    {
        QMutexLocker locker(&m_queueMutex);
        m_pathQueue.clear();
        m_totalPathCount = 0;
    }
    
    // 启动定时器进行实时UI更新
    if (!m_batchTimer->isActive()) {
        m_batchTimer->start();
        qDebug() << "启动批处理定时器";
    }
    
    QElapsedTimer timer;
    timer.start();
    
    // 启动异步哈密顿路径搜索
    findAllHamiltonianPathsAsync(start, end);
    
    qint64 elapsed = timer.elapsed();
    qDebug() << "哈密顿路径搜索启动用时:" << elapsed << "ms";
    
    updateStatusMessage(QString("正在搜索哈密顿路径（经过所有 %1 个点）...")
                       .arg(totalPoints));
}

// 添加针对不同算法的路径计算方法
QVector<QPoint> MainWindow::calculatePathWithAlgorithm(const QPoint& start, const QPoint& end, AlgorithmType algorithm) {
    // 这里根据不同算法返回不同的路径
    // 为了演示，我们创建不同风格的路径
    
    switch (algorithm) {
        case AlgorithmType::AStar:
            return calculateAStarPath(start, end);
        case AlgorithmType::Dijkstra:
            return calculateDijkstraPath(start, end);
        case AlgorithmType::BFS:
            return calculateBFSPath(start, end);
        case AlgorithmType::DFS:
            return calculateDFSPath(start, end);
        default:
            return calculateSimplePath(start, end);
    }
}

QVector<QPoint> MainWindow::calculateAStarPath(const QPoint& start, const QPoint& end) {
    // A*算法倾向于直接路径
    return calculateSimplePath(start, end);
}

QVector<QPoint> MainWindow::calculateDijkstraPath(const QPoint& start, const QPoint& end) {
    // Dijkstra算法可能会找到稍微不同的路径
    QVector<QPoint> path;
    path.append(start);
    
    QPoint current = start;
    
    // 稍微不同的路径策略：优先垂直移动
    while (current.y() != end.y()) {
        if (current.y() < end.y()) {
            current.setY(current.y() + 1);
        } else {
            current.setY(current.y() - 1);
        }
        path.append(current);
    }
    
    while (current.x() != end.x()) {
        if (current.x() < end.x()) {
            current.setX(current.x() + 1);
        } else {
            current.setX(current.x() - 1);
        }
        path.append(current);
    }
    
    return path;
}

QVector<QPoint> MainWindow::calculateBFSPath(const QPoint& start, const QPoint& end) {
    // BFS通常找到最短路径之一
    return calculateSimplePath(start, end);
}

QVector<QPoint> MainWindow::calculateDFSPath(const QPoint& start, const QPoint& end) {
    // 使用DFS回溯算法找到一条路径
    QVector<QVector<QPoint>> allPaths = findAllDFSPaths(start, end);
    if (!allPaths.isEmpty()) {
        return allPaths.first();
    }
    
    // 如果DFS没找到路径，使用简单路径作为备用
    return calculateSimplePath(start, end);
}

// 添加算法名称获取方法
QString MainWindow::getAlgorithmName(AlgorithmType algorithm) {
    switch (algorithm) {
        case AlgorithmType::AStar: return "A*";
        case AlgorithmType::Dijkstra: return "Dijkstra";
        case AlgorithmType::BFS: return "BFS";
        case AlgorithmType::DFS: return "DFS";
        default: return "未知";
    }
}

void MainWindow::onPauseCalculation() {
    m_calculationState = CalculationState::Paused;
    m_controlPanel->setCalculationState(m_calculationState);
    
    // 暂停异步计算器
    if (m_asyncCalculator) {
        m_asyncCalculator->pauseAllCalculations();
    }
    
    updateStatusMessage("计算已暂停");
}

void MainWindow::onResumeCalculation() {
    m_calculationState = CalculationState::Running;
    m_controlPanel->setCalculationState(m_calculationState);
    
    // 恢复异步计算器
    if (m_asyncCalculator) {
        m_asyncCalculator->resumeAllCalculations();
    }
    
    updateStatusMessage("继续计算...");
}

void MainWindow::onStopCalculation() {
    if (m_isCalculating) {
        m_shouldStopCalculation = true;
        
        // 停止异步计算器
        if (m_asyncCalculator) {
            m_asyncCalculator->stopAllCalculations();
        }
        
        // 重置计算状态，允许重新开始计算
        m_isCalculating = false;
        m_calculationState = CalculationState::Idle;  // 改为Idle而不是Stopped
        m_controlPanel->setCalculationState(m_calculationState);
        
        // 清理活动任务记录
        m_activeTaskAlgorithms.clear();
        m_activeTaskNames.clear();
        
        // 隐藏进度条
        showCalculationProgress(false);
        
        updateStatusMessage("计算已停止，可以重新开始");
    }
}

void MainWindow::onResetCalculation() {
    m_gridView->resetGrid();
    m_gridView->clearPath();
    m_calculationState = CalculationState::Idle;
    m_controlPanel->setCalculationState(m_calculationState);
    showCalculationProgress(false);
    updateStatusMessage("网格已重置");
    
    m_currentStartPoint = QPoint(-1, -1);
    m_currentEndPoint = QPoint(-1, -1);
}

void MainWindow::onSaveToXml() {
    qDebug() << "onSaveToXml() 被调用";
    if (!m_resultList) return;
    
    // 获取选中的结果，如果没有选中则获取所有结果
    QVector<PathResult> results = m_resultList->getSelectedResults();
    qDebug() << "选中的结果数量:" << results.size();
    if (results.isEmpty()) {
        results = m_resultList->getAllResults();
        qDebug() << "所有结果数量:" << results.size();
        if (results.isEmpty()) {
            QMessageBox::information(this, "提示", "没有可保存的结果数据");
            return;
        }
        QMessageBox::information(this, "提示", 
            QString("没有选中任何结果，将保存所有 %1 条结果").arg(results.size()));
    } else {
        int ret = QMessageBox::question(this, "确认保存", 
            QString("确定要保存选中的 %1 条结果到XML文件吗？").arg(results.size()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret != QMessageBox::Yes) return;
    }
    
    QString defaultFile = m_dataManager->getDefaultXmlFile();
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "保存为XML文件",
        defaultFile,
        "XML文件 (*.xml)"
    );
    
    if (!fileName.isEmpty()) {
        qDebug() << "开始保存XML文件到:" << fileName;
        qDebug() << "选中的结果数量:" << results.size();
        bool success = m_dataManager->saveToXml(fileName, results);
        qDebug() << "XML保存结果:" << success;
    }
}

void MainWindow::onSaveToSqlite() {
    qDebug() << "onSaveToSqlite() 被调用";
    if (!m_resultList) return;
    
    // 获取选中的结果，如果没有选中则获取所有结果
    QVector<PathResult> results = m_resultList->getSelectedResults();
    qDebug() << "选中的结果数量:" << results.size();
    if (results.isEmpty()) {
        results = m_resultList->getAllResults();
        qDebug() << "所有结果数量:" << results.size();
        if (results.isEmpty()) {
            QMessageBox::information(this, "提示", "没有可保存的结果数据");
            return;
        }
        QMessageBox::information(this, "提示", 
            QString("没有选中任何结果，将保存所有 %1 条结果").arg(results.size()));
    } else {
        int ret = QMessageBox::question(this, "确认保存", 
            QString("确定要保存选中的 %1 条结果到SQLite数据库吗？").arg(results.size()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret != QMessageBox::Yes) return;
    }
    
    QString defaultFile = m_dataManager->getDefaultSqliteFile();
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "保存为SQLite数据库",
        defaultFile,
        "SQLite数据库 (*.db *.sqlite)"
    );
    
    if (!fileName.isEmpty()) {
        qDebug() << "开始保存SQLite文件到:" << fileName;
        qDebug() << "选中的结果数量:" << results.size();
        bool success = m_dataManager->saveToSqlite(fileName, results);
        qDebug() << "SQLite保存结果:" << success;
    }
}

void MainWindow::onSaveToCsv() {
    if (!m_resultList) return;
    
    // 获取选中的结果，如果没有选中则获取所有结果
    QVector<PathResult> results = m_resultList->getSelectedResults();
    if (results.isEmpty()) {
        results = m_resultList->getAllResults();
        if (results.isEmpty()) {
            QMessageBox::information(this, "提示", "没有可保存的结果数据");
            return;
        }
        QMessageBox::information(this, "提示", 
            QString("没有选中任何结果，将导出所有 %1 条结果").arg(results.size()));
    } else {
        int ret = QMessageBox::question(this, "确认导出", 
            QString("确定要导出选中的 %1 条结果到CSV文件吗？").arg(results.size()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret != QMessageBox::Yes) return;
    }
    
    QString defaultFile = m_dataManager->getDefaultCsvFile();
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出为CSV文件",
        defaultFile,
        "CSV文件 (*.csv)"
    );
    
    if (!fileName.isEmpty()) {
        m_dataManager->saveToCsv(fileName, results);
    }
}

void MainWindow::onDataOperationFinished(bool success, const QString& message) {
    if (success) {
        updateStatusMessage(message);
        updateMenuStates();
    } else {
        QMessageBox::warning(this, "操作失败", message);
        updateStatusMessage("操作失败: " + message);
    }
}

void MainWindow::onDataProgress(int percentage) {
    updateStatusMessage(QString("处理进度: %1%").arg(percentage));
}

void MainWindow::onResultDoubleClicked(const PathResult& result) {
    // 在网格中显示选中的路径
    m_gridView->showPath(result.path());
    updateStatusMessage(QString("显示路径: %1").arg(result.toString()));
}

void MainWindow::onResultSelectionChanged(const PathResult& result) {
    // 可以在这里添加选择变化的处理逻辑
    Q_UNUSED(result)
}

void MainWindow::onBatchAddResults(const QVector<PathResult>& results) {
    // 批量添加结果到表格 - 线程安全的UI更新
    for (const PathResult& result : results) {
        if (m_resultList) {
            m_resultList->addResult(result);
        }
    }
    
    // 更新状态信息
    updateStatusMessage(QString("已添加 %1 条路径结果").arg(results.size()));
}

void MainWindow::processBatchQueue() {
    QMutexLocker locker(&m_queueMutex);
    
    if (m_pathQueue.isEmpty()) {
        return;
    }
    
    qDebug() << "processBatchQueue: 队列中有" << m_pathQueue.size() << "个结果";
    
    // 每次最多处理10个结果（提高批处理效率）
    QVector<PathResult> batchResults;
    int processCount = qMin(10, m_pathQueue.size());
    
    for (int i = 0; i < processCount; ++i) {
        batchResults.append(m_pathQueue.dequeue());
    }
    
    qDebug() << "取出" << batchResults.size() << "个结果，队列剩余" << m_pathQueue.size() << "个";
    
    // 释放锁后更新UI
    locker.unlock();
    
    // 批量添加到UI
    if (m_resultList && !batchResults.isEmpty()) {
        m_resultList->addBatchResults(batchResults);
    }
    
    // 更新状态信息
    updateStatusMessage(QString("已处理 %1 条路径结果，队列剩余 %2 条")
                       .arg(batchResults.size()).arg(m_pathQueue.size()));
}

void MainWindow::onDeleteSelectedResults() {
    updateStatusMessage("已删除选中的结果");
}

void MainWindow::onExportResults() {
    qDebug() << "MainWindow::onExportResults() 被调用";
    // 直接调用CSV保存功能
    onSaveToCsv();
}

void MainWindow::onOpenXmlFile() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "打开XML文件",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "XML文件 (*.xml)");
    
    if (!fileName.isEmpty()) {
        // 这里将来会实现XML加载逻辑
        updateStatusMessage("已打开XML文件: " + fileName);
    }
}

void MainWindow::onOpenSqliteFile() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "打开SQLite数据库",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "SQLite数据库 (*.db)");
    
    if (!fileName.isEmpty()) {
        // 这里将来会实现SQLite加载逻辑
        updateStatusMessage("已打开SQLite数据库: " + fileName);
    }
}

void MainWindow::onClearCurrentResults() {
    if (m_resultList->getAllResults().isEmpty()) return;
    
    int ret = QMessageBox::question(this, "确认清空",
                                   "确定要清空当前所有结果吗？",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        m_resultList->clearResults();
        m_gridView->clearPath();
        updateStatusMessage("当前结果已清空");
    }
}

// 使用DFS回溯算法找到所有可能的路径
QVector<QVector<QPoint>> MainWindow::findAllDFSPaths(const QPoint& start, const QPoint& end) {
    QVector<QVector<QPoint>> allPaths;
    QVector<QPoint> currentPath;
    QSet<QPoint> visited;
    
    currentPath.append(start);
    visited.insert(start);
    
    // 限制搜索深度，避免路径过长
    int maxDepth = abs(end.x() - start.x()) + abs(end.y() - start.y()) + 20;
    
    dfsBacktrack(start, end, currentPath, visited, allPaths, maxDepth);
    
    return allPaths; // 不再限制路径数量
}

// 哈密顿路径搜索：找到从起点到终点并经过所有网格点的路径
QVector<QVector<QPoint>> MainWindow::findAllHamiltonianPaths(const QPoint& start, const QPoint& end) {
    QVector<QVector<QPoint>> allPaths;
    QVector<QPoint> currentPath;
    QSet<QPoint> visited;
    
    int gridWidth = m_gridView->gridWidth();
    int gridHeight = m_gridView->gridHeight();
    int totalPoints = gridWidth * gridHeight;
    
    qDebug() << "开始哈密顿路径搜索，总点数:" << totalPoints;
    
    currentPath.append(start);
    visited.insert(start);
    
    hamiltonianDFS(start, end, currentPath, visited, allPaths, totalPoints);
    
    qDebug() << "哈密顿路径搜索完成，找到" << allPaths.size() << "条路径";
    return allPaths;
}

// 异步哈密顿路径搜索（使用队列机制）
void MainWindow::findAllHamiltonianPathsAsync(const QPoint& start, const QPoint& end) {
    int gridWidth = m_gridView->gridWidth();
    int gridHeight = m_gridView->gridHeight();
    int totalPoints = gridWidth * gridHeight;
    
    qDebug() << "开始异步哈密顿路径搜索，总点数:" << totalPoints;
    
    QVector<QPoint> currentPath;
    QSet<QPoint> visited;
    
    currentPath.append(start);
    visited.insert(start);
    
    // 重置停止标志
    m_shouldStopCalculation = false;
    
    hamiltonianDFSAsync(start, end, currentPath, visited, totalPoints);
    
    qDebug() << "异步哈密顿路径搜索启动完成";
}

// 哈密顿路径的DFS回溯实现
void MainWindow::hamiltonianDFS(const QPoint& current, const QPoint& end, QVector<QPoint>& currentPath, 
                               QSet<QPoint>& visited, QVector<QVector<QPoint>>& allPaths, int totalPoints) {
    
    // 如果当前路径已经访问了所有点，并且当前位置是终点
    if (visited.size() == totalPoints && current == end) {
        allPaths.append(currentPath);
        qDebug() << "找到哈密顿路径，长度:" << currentPath.size();
        return;
    }
    
    // 如果已经访问了所有点但不在终点，返回
    if (visited.size() == totalPoints) {
        return;
    }
    
    // 如果找到的路径数量过多，可以适当限制（可选）
    if (allPaths.size() >= 5000) { // 增加到5000条路径限制
        return;
    }
    
    // 获取相邻的有效位置
    QVector<QPoint> neighbors = getValidNeighbors(current);
    
    for (const QPoint& neighbor : neighbors) {
        if (!visited.contains(neighbor)) {
            // 选择这个邻居
            currentPath.append(neighbor);
            visited.insert(neighbor);
            
            // 递归搜索
            hamiltonianDFS(neighbor, end, currentPath, visited, allPaths, totalPoints);
            
            // 回溯
            currentPath.removeLast();
            visited.remove(neighbor);
        }
    }
}

// 获取指定点的有效邻居点（上下左右四个方向）
QVector<QPoint> MainWindow::getValidNeighbors(const QPoint& point) {
    QVector<QPoint> neighbors;
    int gridWidth = m_gridView->gridWidth();
    int gridHeight = m_gridView->gridHeight();
    
    // 四个方向：上、下、左、右
    QVector<QPoint> directions = {
        QPoint(0, -1),  // 上
        QPoint(0, 1),   // 下
        QPoint(-1, 0),  // 左
        QPoint(1, 0)    // 右
    };
    
    for (const QPoint& dir : directions) {
        QPoint neighbor = point + dir;
        
        // 检查是否在网格范围内
        if (neighbor.x() >= 0 && neighbor.x() < gridWidth &&
            neighbor.y() >= 0 && neighbor.y() < gridHeight) {
            neighbors.append(neighbor);
        }
    }
    
    return neighbors;
}

// DFS回溯核心算法
void MainWindow::dfsBacktrack(const QPoint& current, const QPoint& end, 
                              QVector<QPoint>& currentPath, QSet<QPoint>& visited, 
                              QVector<QVector<QPoint>>& allPaths, int maxDepth) {
    
    // 如果到达终点，保存路径
    if (current == end) {
        allPaths.append(currentPath);
        return;
    }
    
    // 如果路径太长，剪枝（但不限制找到的路径总数）
    if (currentPath.size() > maxDepth) {
        return;
    }
    
    // 获取当前点的所有邻居
    QVector<QPoint> neighbors = getNeighbors(current);
    
    // 按照启发式排序邻居（优先选择更接近终点的邻居）
    std::sort(neighbors.begin(), neighbors.end(), [&](const QPoint& a, const QPoint& b) {
        int distA = abs(a.x() - end.x()) + abs(a.y() - end.y());
        int distB = abs(b.x() - end.x()) + abs(b.y() - end.y());
        return distA < distB;
    });
    
    // 尝试每个邻居
    for (const QPoint& neighbor : neighbors) {
        // 检查是否可以移动到这个邻居
        if (!visited.contains(neighbor) && isValidMove(current, neighbor)) {
            // 标记为已访问
            visited.insert(neighbor);
            currentPath.append(neighbor);
            
            // 递归搜索
            dfsBacktrack(neighbor, end, currentPath, visited, allPaths, maxDepth);
            
            // 回溯：撤销选择
            currentPath.removeLast();
            visited.remove(neighbor);
        }
    }
}

// 检查移动是否有效
bool MainWindow::isValidMove(const QPoint& from, const QPoint& to) {
    // 检查是否是相邻的点（4方向连通）
    int dx = abs(to.x() - from.x());
    int dy = abs(to.y() - from.y());
    
    // 只允许上下左右移动，不允许对角线移动
    if ((dx == 1 && dy == 0) || (dx == 0 && dy == 1)) {
        // 检查目标点是否在网格范围内
        if (m_gridView) {
            int gridWidth = m_gridView->gridWidth();
            int gridHeight = m_gridView->gridHeight();
            
            if (to.x() >= 0 && to.x() < gridWidth && to.y() >= 0 && to.y() < gridHeight) {
                // 这里可以添加更多约束，比如检查是否是障碍物等
                return true;
            }
        }
    }
    
    return false;
}

// 获取一个点的所有有效邻居
QVector<QPoint> MainWindow::getNeighbors(const QPoint& point) {
    QVector<QPoint> neighbors;
    
    // 四个方向：上、下、左、右
    QVector<QPoint> directions = {
        QPoint(0, -1),  // 上
        QPoint(0, 1),   // 下
        QPoint(-1, 0),  // 左
        QPoint(1, 0)    // 右
    };
    
    for (const QPoint& dir : directions) {
        QPoint neighbor = point + dir;
        
        // 检查邻居是否在网格范围内
        if (m_gridView) {
            int gridWidth = m_gridView->gridWidth();
            int gridHeight = m_gridView->gridHeight();
            
            if (neighbor.x() >= 0 && neighbor.x() < gridWidth && 
                neighbor.y() >= 0 && neighbor.y() < gridHeight) {
                neighbors.append(neighbor);
            }
        }
    }
    
    return neighbors;
}

// 渐进式DFS计算：使用队列系统
void MainWindow::calculateAllDFSPathsProgressive(const QPoint& start, const QPoint& end, AlgorithmType algorithm) {
    qDebug() << "calculateAllDFSPathsProgressive 开始 - 起点:" << start << "终点:" << end;
    
    QVector<QPoint> currentPath;
    QSet<QPoint> visited;
    
    currentPath.append(start);
    visited.insert(start);
    
    qDebug() << "启动批量处理定时器...";
    // 启动批量处理定时器
    m_batchTimer->start();
    
    // 设置搜索深度为所有点数（哈密顿路径必须经过所有点）
    int totalPoints = m_gridView->gridWidth() * m_gridView->gridHeight();
    int maxDepth = totalPoints;  // 哈密顿路径的长度必须等于总点数
    qDebug() << "哈密顿路径搜索深度:" << maxDepth << "（必须经过所有点）";
    
    qDebug() << "开始DFS回溯搜索...";
    // 开始渐进式DFS搜索
    dfsBacktrackProgressive(start, end, currentPath, visited, algorithm, start, maxDepth);
    
    qDebug() << "DFS搜索完成，停止定时器...";
    // 停止定时器
    m_batchTimer->stop();
    
    qDebug() << "处理队列中剩余的结果...";
    // 处理队列中剩余的所有结果
    processBatchQueue();
    qDebug() << "calculateAllDFSPathsProgressive 完成";
}

// 渐进式DFS回溯算法：将找到的路径添加到队列中
void MainWindow::dfsBacktrackProgressive(const QPoint& current, const QPoint& end, 
                                        QVector<QPoint>& currentPath, QSet<QPoint>& visited, 
                                        AlgorithmType algorithm, const QPoint& startPoint, int maxDepth) {
    
    // 检查是否应该停止计算（包括路径数量限制）
    if (m_shouldStopCalculation || m_totalPathCount >= MAX_PATHS) {
        qDebug() << "DFS停止条件触发: shouldStop=" << m_shouldStopCalculation 
                 << "totalPaths=" << m_totalPathCount << "maxPaths=" << MAX_PATHS;
        return;
    }
    
    // 添加调试输出，但限制频率
    static int debugCounter = 0;
    if (debugCounter % 1000 == 0) {
        qDebug() << "DFS搜索中... 当前点:" << current << "路径长度:" << currentPath.size();
    }
    debugCounter++;
    
    // 如果到达终点，检查是否为哈密顿路径（经过所有点）
    if (current == end) {
        // 计算网格中的总点数
        int totalPoints = m_gridView->gridWidth() * m_gridView->gridHeight();
        
        // 检查路径是否经过了所有点
        if (currentPath.size() == totalPoints) {
            qDebug() << "找到哈密顿路径! 长度:" << currentPath.size() << "当前路径数:" << m_totalPathCount;
            
            // 先检查是否已经达到最大路径限制
            if (m_totalPathCount >= MAX_PATHS) {
                qDebug() << "已达到最大哈密顿路径限制:" << MAX_PATHS << "，停止搜索";
                m_shouldStopCalculation = true;  // 设置停止标志
                return;
            }
            
            // 计算用时（模拟）
            qint64 calcTime = QTime::currentTime().msecsSinceStartOfDay() % 100 + 1;
            
            // 创建路径结果
            PathResult result("", startPoint, end, currentPath, algorithm, calcTime);
            
            // 线程安全地添加到队列
            {
                QMutexLocker locker(&m_queueMutex);
                m_pathQueue.enqueue(result);
                m_totalPathCount++;
                qDebug() << "哈密顿路径已添加到队列，队列大小:" << m_pathQueue.size() << "总路径数:" << m_totalPathCount;
            }
        } else {
            // 到达终点但不是哈密顿路径，跳过
            qDebug() << "到达终点但路径不完整，只经过了" << currentPath.size() << "个点，总共需要" << totalPoints << "个点";
        }
        
        return;
    }
    
    // 如果路径长度不等于总点数且已经到达最大深度，剪枝
    if (currentPath.size() > maxDepth) {
        return;
    }
    
    // 如果还没有经过所有点但已经到了终点，这不是有效的哈密顿路径
    if (current == end && currentPath.size() < maxDepth) {
        return;  // 剪枝，因为这不是完整的哈密顿路径
    }
    
    // 获取当前点的所有邻居
    QVector<QPoint> neighbors = getNeighbors(current);
    
    // 按照启发式排序邻居（优先选择更接近终点的邻居）
    std::sort(neighbors.begin(), neighbors.end(), [&](const QPoint& a, const QPoint& b) {
        int distA = abs(a.x() - end.x()) + abs(a.y() - end.y());
        int distB = abs(b.x() - end.x()) + abs(b.y() - end.y());
        return distA < distB;
    });
    
    // 尝试每个邻居
    for (const QPoint& neighbor : neighbors) {
        // 检查是否应该停止计算（包括路径数量限制）
        if (m_shouldStopCalculation || m_totalPathCount >= MAX_PATHS) {
            qDebug() << "在邻居循环中检测到停止条件";
            return;
        }
        
        // 检查是否可以移动到这个邻居
        if (!visited.contains(neighbor) && isValidMove(current, neighbor)) {
            // 标记为已访问
            visited.insert(neighbor);
            currentPath.append(neighbor);
            
            // 递归搜索
            dfsBacktrackProgressive(neighbor, end, currentPath, visited, algorithm, startPoint, maxDepth);
            
            // 回溯：撤销选择
            currentPath.removeLast();
            visited.remove(neighbor);
        }
    }
}

// 异步哈密顿路径的DFS回溯实现（使用队列机制）
void MainWindow::hamiltonianDFSAsync(const QPoint& current, const QPoint& end, QVector<QPoint>& currentPath, 
                                     QSet<QPoint>& visited, int totalPoints) {
    
    // 检查是否应该停止计算
    if (m_shouldStopCalculation || m_totalPathCount >= MAX_PATHS) {
        return;
    }
    
    // 如果当前路径已经访问了所有点，并且当前位置是终点
    if (visited.size() == totalPoints && current == end) {
        PathResult result;
        result.setAlgorithm(AlgorithmType::DFS);
        result.setStartPoint(currentPath.first());
        result.setEndPoint(end);
        result.setPath(currentPath);
        result.setCalculationTime(1); // 临时设置，实际时间可以后续计算
        result.setTimestamp(QDateTime::currentDateTime());
        
        // 线程安全地添加到队列
        {
            QMutexLocker locker(&m_queueMutex);
            m_pathQueue.enqueue(result);
            m_totalPathCount++;
            qDebug() << "哈密顿路径已添加到队列，队列大小:" << m_pathQueue.size() << "总路径数:" << m_totalPathCount;
        }
        
        return;
    }
    
    // 如果已经访问了所有点但不在终点，返回
    if (visited.size() == totalPoints) {
        return;
    }
    
    // 如果找到的路径数量过多，限制搜索
    if (m_totalPathCount >= 1000) {
        return;
    }
    
    // 获取相邻的有效位置
    QVector<QPoint> neighbors = getValidNeighbors(current);
    
    for (const QPoint& neighbor : neighbors) {
        // 检查停止条件
        if (m_shouldStopCalculation || m_totalPathCount >= MAX_PATHS) {
            return;
        }
        
        if (!visited.contains(neighbor)) {
            // 选择这个邻居
            currentPath.append(neighbor);
            visited.insert(neighbor);
            
            // 递归搜索
            hamiltonianDFSAsync(neighbor, end, currentPath, visited, totalPoints);
            
            // 回溯
            currentPath.removeLast();
            visited.remove(neighbor);
        }
    }
}

// 数据管理相关方法实现
void MainWindow::updateMenuStates() {
    bool hasResults = m_resultList && !m_resultList->getAllResults().isEmpty();
    
    if (m_saveXmlAction) m_saveXmlAction->setEnabled(hasResults);
    if (m_saveSqliteAction) m_saveSqliteAction->setEnabled(hasResults);
    if (m_saveCsvAction) m_saveCsvAction->setEnabled(hasResults);
}

void MainWindow::onOpenDataFile() {
    QString dataDir = m_dataManager->getDataDirectory();
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "打开数据文件",
        dataDir,
        "所有支持的格式 (*.xml *.db *.sqlite *.sqlite3);;XML文件 (*.xml);;SQLite数据库 (*.db *.sqlite *.sqlite3)"
    );
    
    if (!fileName.isEmpty()) {
        QVector<PathResult> results = m_dataManager->loadFromFile(fileName);
        if (!results.isEmpty()) {
            // 从路径结果中推断矩阵大小
            int maxX = 0, maxY = 0;
            for (const PathResult& result : results) {
                for (const QPoint& point : result.path()) {
                    maxX = qMax(maxX, point.x());
                    maxY = qMax(maxY, point.y());
                }
                // 也检查起点和终点
                maxX = qMax(maxX, qMax(result.startPoint().x(), result.endPoint().x()));
                maxY = qMax(maxY, qMax(result.startPoint().y(), result.endPoint().y()));
            }
            
            // 设置矩阵大小（加1因为坐标从0开始）
            int gridWidth = maxX + 1;
            int gridHeight = maxY + 1;
            
            qDebug() << "从文件推断矩阵大小: " << gridWidth << "x" << gridHeight;
            
            // 更新控制面板的矩阵大小设置
            if (m_controlPanel) {
                m_controlPanel->setGridSize(gridWidth, gridHeight);
            }
            
            // 应用新的矩阵大小
            onApplyGridSize();
            
            // 重新创建表格并加载数据
            m_resultList->recreateTable();
            m_resultList->addBatchResults(results);
            updateMenuStates();
            
            updateStatusMessage(QString("成功加载 %1 条路径结果，矩阵大小: %2x%3")
                .arg(results.size()).arg(gridWidth).arg(gridHeight));
        } else {
            QMessageBox::warning(this, "加载失败", "文件中没有找到有效的路径结果数据");
        }
    }
}

// 异步计算器相关槽函数实现

void MainWindow::onAsyncPathFound(const PathResult& result, int taskId) {
    qDebug() << "异步计算完成最终结果，任务ID:" << taskId;
    
    // 对于哈密顿路径，所有结果已经通过 onAsyncPartialPathFound 处理了
    // 这里只更新状态，不重复添加结果
    AlgorithmType algorithm = m_activeTaskAlgorithms.value(taskId, AlgorithmType::DFS);
    QString algorithmName = algorithmTypeToString(algorithm);
    
    if (result.getPath().isEmpty()) {
        updateStatusMessage(QString("%1 算法计算完成，未找到路径").arg(algorithmName));
    } else {
        updateStatusMessage(QString("%1 算法计算完成，共找到 %2 条哈密顿路径")
                           .arg(algorithmName).arg(m_totalPathCount));
    }
}

void MainWindow::onAsyncPartialPathFound(const PathResult& result, int taskId) {
    qDebug() << "异步计算找到部分路径，任务ID:" << taskId << "路径名称:" << result.id();
    
    // 将部分路径（即每个找到的哈密顿路径）添加到结果列表中显示
    if (m_resultList) {
        m_resultList->addResult(result);
        qDebug() << "已添加哈密顿路径到结果列表:" << result.id() << "路径长度:" << result.getPath().size();
        
        // 显示第一条路径在网格上
        if (m_totalPathCount == 0 && m_gridView) {
            m_gridView->showPath(result.getPath());
            qDebug() << "在网格上显示第一条哈密顿路径";
        }
        
        m_totalPathCount++;
    }
    
    // 更新状态信息
    AlgorithmType algorithm = m_activeTaskAlgorithms.value(taskId, AlgorithmType::DFS);
    QString algorithmName = algorithmTypeToString(algorithm);
    updateStatusMessage(QString("找到 %1: %2 (总计 %3 条)")
                       .arg(algorithmName).arg(result.id()).arg(m_totalPathCount));
}

void MainWindow::onAsyncPathNotFound(int taskId) {
    qDebug() << "异步计算未找到路径，任务ID:" << taskId;
    
    AlgorithmType algorithm = m_activeTaskAlgorithms.value(taskId, AlgorithmType::AStar);
    QString algorithmName = algorithmTypeToString(algorithm);
    updateStatusMessage(QString("使用 %1 算法未找到路径 (任务ID: %2)")
                       .arg(algorithmName).arg(taskId));
}

void MainWindow::onAsyncCalculationProgress(int taskId, int percentage) {
    // 更新进度条
    updateCalculationProgress(percentage);
    
    // 可以选择显示更详细的任务进度信息
    if (percentage % 10 == 0) { // 每10%显示一次
        AlgorithmType algorithm = m_activeTaskAlgorithms.value(taskId, AlgorithmType::AStar);
        QString algorithmName = algorithmTypeToString(algorithm);
        updateStatusMessage(QString("正在使用 %1 算法计算路径... %2% (任务ID: %3)")
                           .arg(algorithmName).arg(percentage).arg(taskId));
    }
}

void MainWindow::onAsyncCalculationStarted(int taskId) {
    qDebug() << "异步计算开始，任务ID:" << taskId;
    
    AlgorithmType algorithm = m_activeTaskAlgorithms.value(taskId, AlgorithmType::AStar);
    QString algorithmName = algorithmTypeToString(algorithm);
    updateStatusMessage(QString("开始使用 %1 算法计算路径 (任务ID: %2)")
                       .arg(algorithmName).arg(taskId));
}

void MainWindow::onAsyncCalculationFinished(int taskId) {
    qDebug() << "异步计算完成，任务ID:" << taskId;
    
    // 从活动任务中移除
    m_activeTaskAlgorithms.remove(taskId);
    m_activeTaskNames.remove(taskId);
    
    // 如果没有更多活动任务，标记计算完成
    if (m_activeTaskAlgorithms.isEmpty()) {
        m_isCalculating = false;
        m_calculationState = CalculationState::Idle;
        m_controlPanel->setCalculationState(m_calculationState);
        showCalculationProgress(false);
        
        int pathCount = m_resultList->getAllResults().size();
        updateStatusMessage(QString("所有计算任务完成 - 找到 %1 条路径").arg(pathCount));
    }
}

void MainWindow::onAsyncAllCalculationsFinished() {
    qDebug() << "所有异步计算任务完成";
    
    // 确保状态正确
    m_isCalculating = false;
    m_calculationState = CalculationState::Idle;
    m_controlPanel->setCalculationState(m_calculationState);
    showCalculationProgress(false);
    
    // 清理活动任务记录
    m_activeTaskAlgorithms.clear();
    m_activeTaskNames.clear();
    
    int pathCount = m_resultList->getAllResults().size();
    updateStatusMessage(QString("全部计算完成 - 总共找到 %1 条路径").arg(pathCount));
}

void MainWindow::onOpenLayoutTestWindow() {
    qDebug() << "打开布局管理演示窗口";
    
    // 如果窗口还没有创建或已经被删除，创建新的窗口
    if (!m_layoutTestWindow) {
        m_layoutTestWindow = new LayoutTestWindow(this);
        
        // 连接窗口关闭信号，确保指针被重置
        connect(m_layoutTestWindow, &QObject::destroyed, this, [this]() {
            m_layoutTestWindow = nullptr;
            qDebug() << "布局测试窗口已关闭";
        });
    }
    
    // 显示窗口
    m_layoutTestWindow->show();
    m_layoutTestWindow->raise();
    m_layoutTestWindow->activateWindow();
    
    updateStatusMessage("已打开Qt布局管理最佳实践演示窗口");
}
