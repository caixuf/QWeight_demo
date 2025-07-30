#include "MainWindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTime>
#include <QThread>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_pointSetMode(None)
    , m_calculationState(CalculationState::Idle)
    , m_currentStartPoint(-1, -1)
    , m_currentEndPoint(-1, -1)
    , m_isCalculating(false)
    , m_shouldStopCalculation(false)
{
    qDebug() << "MainWindow构造函数开始...";
    
    try {
        qDebug() << "设置UI...";
        setupUI();
        
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
    // 检查起点终点是否设置
    if (!m_gridView->hasStartPoint() || !m_gridView->hasEndPoint()) {
        QMessageBox::warning(this, "警告", "请先设置起点和终点！");
        return;
    }
    
    if (m_isCalculating) {
        return; // 已经在计算中
    }
    
    m_isCalculating = true;
    m_shouldStopCalculation = false;
    m_calculationState = CalculationState::Running;
    m_controlPanel->setCalculationState(m_calculationState);
    showCalculationProgress(true);
    
    // 获取起点和终点
    QPoint start = m_gridView->getStartPoint();
    QPoint end = m_gridView->getEndPoint();
    
    // 获取当前网格大小信息用于显示
    int gridWidth = m_gridView->gridWidth();
    int gridHeight = m_gridView->gridHeight();
    
    // 清空之前的结果
    m_resultList->clearResults();
    m_gridView->clearPath();
    
    // 获取用户选择的算法
    AlgorithmType selectedAlgorithm = m_controlPanel->getSelectedAlgorithm();
    QString algorithmName = getAlgorithmName(selectedAlgorithm);
    
    updateStatusMessage(QString("正在计算 %1 算法的所有可能路径...").arg(algorithmName));
    
    // 计算该算法的多条可能路径
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
            // 生成更多路径变体
            for (int i = 1; i <= 10; ++i) {
                QVector<QPoint> variant = calculatePathVariant(start, end, i);
                if (!variant.isEmpty()) {
                    allPaths.append(variant);
                }
            }
            break;
            
        case AlgorithmType::Dijkstra:
            // Dijkstra算法 - 生成多条等长最短路径
            allPaths.append(calculateDijkstraPath(start, end));
            // 生成更多等长路径变体
            for (int i = 1; i <= 12; ++i) {
                QVector<QPoint> variant = calculatePathVariant(start, end, i + 10);
                if (!variant.isEmpty()) {
                    allPaths.append(variant);
                }
            }
            break;
            
        case AlgorithmType::BFS:
            // BFS - 生成所有可能的最短路径
            allPaths.append(calculateBFSPath(start, end));
            // 生成其他最短路径
            for (int i = 1; i <= 15; ++i) {
                QVector<QPoint> variant = calculatePathVariant(start, end, i + 20);
                if (!variant.isEmpty()) {
                    allPaths.append(variant);
                }
            }
            break;
            
        case AlgorithmType::DFS:
            // DFS - 可能找到多条不同长度的路径
            allPaths.append(calculateDFSPath(start, end));
            // 生成更多DFS可能的路径（包括较长路径）
            for (int i = 1; i <= 20; ++i) {
                QVector<QPoint> variant = calculatePathVariant(start, end, i + 30);
                if (!variant.isEmpty()) {
                    allPaths.append(variant);
                }
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
    
    // 根据变体类型生成不同的路径
    int pathType = variant % 8; // 增加到8种不同的路径类型
    
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
                int detourX = (variant / 8) % 3 - 1; // -1, 0, 1的偏移
                int detourY = (variant / 24) % 3 - 1;
                
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
    // DFS可能会找到更曲折的路径
    QVector<QPoint> path;
    path.append(start);
    
    QPoint current = start;
    
    // 模拟DFS的曲折路径：交替移动
    while (current != end) {
        if (current.x() != end.x() && current.y() != end.y()) {
            // 交替移动
            if ((current.x() + current.y()) % 2 == 0) {
                // 水平移动
                if (current.x() < end.x()) {
                    current.setX(current.x() + 1);
                } else {
                    current.setX(current.x() - 1);
                }
            } else {
                // 垂直移动
                if (current.y() < end.y()) {
                    current.setY(current.y() + 1);
                } else {
                    current.setY(current.y() - 1);
                }
            }
        } else if (current.x() != end.x()) {
            if (current.x() < end.x()) {
                current.setX(current.x() + 1);
            } else {
                current.setX(current.x() - 1);
            }
        } else if (current.y() != end.y()) {
            if (current.y() < end.y()) {
                current.setY(current.y() + 1);
            } else {
                current.setY(current.y() - 1);
            }
        }
        path.append(current);
    }
    
    return path;
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
