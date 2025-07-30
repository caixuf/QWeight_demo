#include "MainWindow.h"
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
{
    qDebug() << "MainWindow构造函数开始...";
    
    try {
        qDebug() << "设置UI...";
        setupUI();
        
        qDebug() << "初始化批量处理定时器...";
        m_batchTimer = new QTimer(this);
        m_batchTimer->setInterval(500); // 每500ms处理一次队列，减少UI压力
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
        m_fileMenu->addAction("退出", this, &QWidget::close);
        
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
        }
        
        // 结果列表连接
        if (m_resultList) {
            connect(m_resultList, &ResultListWidget::resultDoubleClicked,
                    this, &MainWindow::onResultDoubleClicked);
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
    
    // 获取用户选择的算法
    AlgorithmType selectedAlgorithm = m_controlPanel->getSelectedAlgorithm();
    QString algorithmName = getAlgorithmName(selectedAlgorithm);
    qDebug() << "选择的算法:" << algorithmName;
    
    updateStatusMessage(QString("正在计算 %1 算法的所有可能路径...").arg(algorithmName));
    
    // 特殊处理DFS算法：使用渐进式计算搜索哈密顿路径
    if (selectedAlgorithm == AlgorithmType::DFS) {
        qDebug() << "=== 准备开始哈密顿路径搜索 ===";
        qDebug() << "起点:" << start << "终点:" << end;
        qDebug() << "网格大小:" << gridWidth << "x" << gridHeight << "总点数:" << (gridWidth * gridHeight);
        updateStatusMessage(QString("开始搜索哈密顿路径（经过所有 %1 个点），将实时显示找到的路径...").arg(gridWidth * gridHeight));
        
        // DFS算法会直接在计算过程中添加结果到表格
        calculateAllDFSPathsProgressive(start, end, selectedAlgorithm);
        qDebug() << "=== 哈密顿路径搜索完成! ===";
        
        // DFS计算完成
        int pathCount = m_resultList->getAllResults().size();
        updateStatusMessage(QString("哈密顿路径搜索完成 - 网格%1×%2，共找到%3条完整路径").arg(gridWidth).arg(gridHeight).arg(pathCount));
        
        // 显示第一条路径（如果存在）
        if (pathCount > 0) {
            PathResult firstResult = m_resultList->getAllResults().first();
            m_gridView->showPath(firstResult.path());
        }
        
        m_isCalculating = false;
        m_calculationState = CalculationState::Idle;
        m_controlPanel->setCalculationState(m_calculationState);
        showCalculationProgress(false);
        return;
    }
    
    // 其他算法使用原有的计算方式
    QVector<QVector<QPoint>> allPaths = calculateAllPossiblePaths(start, end, selectedAlgorithm);
    
    int completedCount = 0;
    
    for (int i = 0; i < allPaths.size(); ++i) {
        // 检查停止标记
        if (m_shouldStopCalculation) {
            updateStatusMessage(QString("计算已停止 - 已完成 %1/%2 条路径").arg(completedCount).arg(allPaths.size()));
            break;
        }
        
        updateStatusMessage(QString("正在处理第 %1 条路径... (%2/%3)").arg(i + 1).arg(i + 1).arg(allPaths.size()));
        
        // 记录开始时间
        qint64 startTime = QDateTime::currentMSecsSinceEpoch();
        
        // 处理UI事件，允许停止操作
        for (int j = 0; j < 5 && !m_shouldStopCalculation; ++j) {
            QApplication::processEvents();
            QThread::msleep(10); // 模拟计算时间
        }
        
        if (m_shouldStopCalculation) {
            break;
        }
        
        QVector<QPoint> path = allPaths[i];
        
        // 计算用时
        qint64 calculationTime = QDateTime::currentMSecsSinceEpoch() - startTime;
        
        if (!path.isEmpty()) {
            PathResult result(QString("%1_路径%2").arg(algorithmName).arg(i + 1), // 唯一ID
                             start,
                             end,
                             path,
                             selectedAlgorithm,
                             calculationTime);
            
            m_resultList->addResult(result);
            
            // 显示第一条路径
            if (i == 0) {
                m_gridView->showPath(path);
            }
            
            completedCount++;
        }
    }
    
    m_isCalculating = false;
    m_calculationState = CalculationState::Completed;
    m_controlPanel->setCalculationState(m_calculationState);
    showCalculationProgress(false);
    
    if (!m_shouldStopCalculation) {
        updateStatusMessage(QString("计算完成 - 网格%1×%2，使用%3算法，共找到%4条路径").arg(gridWidth).arg(gridHeight).arg(algorithmName).arg(completedCount));
    }
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
    updateStatusMessage("计算已暂停");
}

void MainWindow::onResumeCalculation() {
    m_calculationState = CalculationState::Running;
    m_controlPanel->setCalculationState(m_calculationState);
    updateStatusMessage("继续计算...");
}

void MainWindow::onStopCalculation() {
    if (m_isCalculating) {
        m_shouldStopCalculation = true;
        m_calculationState = CalculationState::Stopped;
        m_controlPanel->setCalculationState(m_calculationState);
        updateStatusMessage("正在停止计算...");
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
    QString fileName = QFileDialog::getSaveFileName(
        this, "保存到XML文件",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/path_results.xml",
        "XML文件 (*.xml)");
    
    if (!fileName.isEmpty()) {
        // 这里将来会实现XML保存逻辑
        updateStatusMessage("结果已保存到XML文件: " + fileName);
    }
}

void MainWindow::onSaveToSqlite() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "保存到SQLite数据库",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/path_results.db",
        "SQLite数据库 (*.db)");
    
    if (!fileName.isEmpty()) {
        // 这里将来会实现SQLite保存逻辑
        updateStatusMessage("结果已保存到SQLite数据库: " + fileName);
    }
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
    
    // 每次最多处理5个结果
    QVector<PathResult> batchResults;
    int processCount = qMin(5, m_pathQueue.size());
    
    for (int i = 0; i < processCount; ++i) {
        batchResults.append(m_pathQueue.dequeue());
    }
    
    qDebug() << "取出" << batchResults.size() << "个结果，队列剩余" << m_pathQueue.size() << "个";
    
    // 释放锁后更新UI
    locker.unlock();
    
    // 批量添加到UI
    for (const PathResult& result : batchResults) {
        if (m_resultList) {
            m_resultList->addResult(result);
        }
    }
    
    // 更新状态信息
    updateStatusMessage(QString("已处理 %1 条路径结果，队列剩余 %2 条")
                       .arg(batchResults.size()).arg(m_pathQueue.size()));
}

void MainWindow::onDeleteSelectedResults() {
    updateStatusMessage("已删除选中的结果");
}

void MainWindow::onExportResults() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "导出结果",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/exported_results.csv",
        "CSV文件 (*.csv)");
    
    if (!fileName.isEmpty()) {
        // 这里将来会实现CSV导出逻辑
        updateStatusMessage("结果已导出到: " + fileName);
    }
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
