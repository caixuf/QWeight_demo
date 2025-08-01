#include "GridGraphicsView.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QPen>
#include <QBrush>
#include <cmath>
#include <QTime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

GridGraphicsView::GridGraphicsView(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(nullptr)
    , m_gridWidth(5)  // 直接使用数值
    , m_gridHeight(5) // 直接使用数值
    , m_startPoint(0, 0)
    , m_endPoint(1, 1)
    , m_scaleFactor(1.0)
{
    qDebug() << "GridGraphicsView构造函数开始...";
    
    try {
        qDebug() << "设置场景...";
        setupScene();
        
        qDebug() << "创建网格...";
        createGrid(m_gridWidth, m_gridHeight);
        
        qDebug() << "GridGraphicsView构造函数完成";
    } catch (const std::exception& e) {
        qDebug() << "GridGraphicsView构造函数异常:" << e.what();
    } catch (...) {
        qDebug() << "GridGraphicsView构造函数未知异常";
    }
}

GridGraphicsView::~GridGraphicsView() {
    clearGrid();
}

void GridGraphicsView::setupScene() {
    qDebug() << "setupScene 开始...";
    
    try {
        qDebug() << "创建场景...";
        m_scene = new QGraphicsScene(this);
        
        qDebug() << "设置场景到视图...";
        setScene(m_scene);
        
        qDebug() << "设置视图属性...";
        setRenderHint(QPainter::Antialiasing);
        
        qDebug() << "setupScene 完成";
    } catch (const std::exception& e) {
        qDebug() << "setupScene 异常:" << e.what();
    } catch (...) {
        qDebug() << "setupScene 未知异常";
    }
}

void GridGraphicsView::createGrid(int width, int height) {
    qDebug() << "createGrid 开始，大小:" << width << "x" << height;
    
    try {
        qDebug() << "清理旧网格...";
        clearGrid();
        
        qDebug() << "设置网格大小...";
        m_gridWidth = width;
        m_gridHeight = height;
        
        qDebug() << "初始化网格数据...";
        m_gridData.resize(m_gridHeight);
        m_pointItems.resize(m_gridHeight);
        
        for (int y = 0; y < m_gridHeight; ++y) {
            m_gridData[y].resize(m_gridWidth);
            m_pointItems[y].resize(m_gridWidth);
            
            for (int x = 0; x < m_gridWidth; ++x) {
                m_gridData[y][x] = GridPoint(x, y, PointType::Normal);
                m_pointItems[y][x] = nullptr;
            }
        }
        
        qDebug() << "创建网格点（不创建网格线）...";
        createGridPoints();
        
        qDebug() << "调整视图...";
        fitGridInView();
        
        qDebug() << "createGrid 完成";
    } catch (const std::exception& e) {
        qDebug() << "createGrid 异常:" << e.what();
    } catch (...) {
        qDebug() << "createGrid 未知异常";
    }
}

void GridGraphicsView::clearGrid() {
    qDebug() << "clearGrid 开始...";
    
    try {
        if (!m_scene) {
            qDebug() << "场景为空，跳过清理";
            return;
        }
        
        qDebug() << "清理图形项...";
        // 简单清理：直接清空场景
        m_scene->clear();
        
        qDebug() << "清理数据结构...";
        m_gridData.clear();
        m_pointItems.clear();
        m_gridLines.clear();
        m_pathArrows.clear();  // 清空路径箭头容器
        
        m_startPoint = QPoint(-1, -1);
        m_endPoint = QPoint(-1, -1);
        
        qDebug() << "clearGrid 完成";
    } catch (const std::exception& e) {
        qDebug() << "clearGrid 异常:" << e.what();
    } catch (...) {
        qDebug() << "clearGrid 未知异常";
    }
}

void GridGraphicsView::resetGrid() {
    for (int y = 0; y < m_gridHeight; ++y) {
        for (int x = 0; x < m_gridWidth; ++x) {
            m_gridData[y][x].reset();
            updatePointAppearance(x, y);
        }
    }
    
    m_startPoint = QPoint(-1, -1);
    m_endPoint = QPoint(-1, -1);
    clearPath();
}

void GridGraphicsView::createGridLines() {
    QPen pen(Constants::GRID_LINE_COLOR, 1);
    
    // 创建水平线
    for (int y = 0; y <= m_gridHeight; ++y) {
        QGraphicsLineItem* line = m_scene->addLine(
            0, y * Constants::GRID_SPACING,
            m_gridWidth * Constants::GRID_SPACING, y * Constants::GRID_SPACING,
            pen);
        m_gridLines.append(line);
    }
    
    // 创建垂直线
    for (int x = 0; x <= m_gridWidth; ++x) {
        QGraphicsLineItem* line = m_scene->addLine(
            x * Constants::GRID_SPACING, 0,
            x * Constants::GRID_SPACING, m_gridHeight * Constants::GRID_SPACING,
            pen);
        m_gridLines.append(line);
    }
}

void GridGraphicsView::createGridPoints() {
    for (int y = 0; y < m_gridHeight; ++y) {
        for (int x = 0; x < m_gridWidth; ++x) {
            QPointF scenePos = gridToScene(x, y);
            
            QPen pen(Qt::black, 1);
            QBrush brush(getPointColor(PointType::Normal));
            
            QGraphicsEllipseItem* item = m_scene->addEllipse(
                scenePos.x() - Constants::POINT_RADIUS,
                scenePos.y() - Constants::POINT_RADIUS,
                Constants::POINT_RADIUS * 2,
                Constants::POINT_RADIUS * 2,
                pen, brush);
            
            item->setFlag(QGraphicsItem::ItemIsSelectable);
            m_pointItems[y][x] = item;
        }
    }
}

void GridGraphicsView::setPointType(int x, int y, PointType type) {
    if (x < 0 || x >= m_gridWidth || y < 0 || y >= m_gridHeight) {
        return;
    }
    
    m_gridData[y][x].setType(type);
    updatePointAppearance(x, y);
    
    if (type == PointType::Start) {
        m_startPoint = QPoint(x, y);
        emit startPointSet(m_startPoint);
    } else if (type == PointType::End) {
        m_endPoint = QPoint(x, y);
        emit endPointSet(m_endPoint);
    }
}

PointType GridGraphicsView::getPointType(int x, int y) const {
    if (x < 0 || x >= m_gridWidth || y < 0 || y >= m_gridHeight) {
        return PointType::Normal;
    }
    
    return m_gridData[y][x].type();
}

void GridGraphicsView::updatePointAppearance(int x, int y) {
    if (x < 0 || x >= m_gridWidth || y < 0 || y >= m_gridHeight) {
        return;
    }
    
    QGraphicsEllipseItem* item = m_pointItems[y][x];
    if (!item) return;
    
    PointType type = m_gridData[y][x].type();
    QColor color = getPointColor(type);
    
    item->setBrush(QBrush(color));
    item->setPen(QPen(Qt::black, 1));
}

void GridGraphicsView::showPath(const QVector<QPoint>& path) {
    clearPath();
    
    if (path.size() < 2) return;
    
    QPen pen(Constants::PATH_POINT_COLOR, 3);
    QBrush brush(Constants::PATH_POINT_COLOR);
    
    for (int i = 0; i < path.size() - 1; ++i) {
        QPointF start = gridToScene(path[i].x(), path[i].y());
        QPointF end = gridToScene(path[i + 1].x(), path[i + 1].y());
        
        // 绘制箭头线条
        drawArrow(start, end, pen);
        
        // 标记路径点（除了起点和终点）
        if (i > 0) { // 不标记起点
            setPointType(path[i].x(), path[i].y(), PointType::Path);
        }
    }
}

void GridGraphicsView::clearPath() {
    qDebug() << "clearPath 开始...";
    
    try {
        if (!m_scene) {
            qDebug() << "场景为空，跳过清理路径";
            return;
        }
        
        // 清理路径箭头
        for (ArrowGraphicsItem* arrow : m_pathArrows) {
            if (arrow) {
                m_scene->removeItem(arrow);
                delete arrow;
            }
        }
        m_pathArrows.clear();
        
        // 重置路径点的颜色
        for (int y = 0; y < m_gridHeight; ++y) {
            for (int x = 0; x < m_gridWidth; ++x) {
                if (m_gridData[y][x].type() == PointType::Path) {
                    m_gridData[y][x].setType(PointType::Normal);
                    updatePointAppearance(x, y);
                }
            }
        }
        
        qDebug() << "clearPath 完成";
    } catch (const std::exception& e) {
        qDebug() << "clearPath 异常:" << e.what();
    } catch (...) {
        qDebug() << "clearPath 未知异常";
    }
}

void GridGraphicsView::drawArrow(const QPointF& start, const QPointF& end, const QPen& pen) {
    if (!m_scene) return;
    
    // 计算方向向量和距离
    QPointF direction = end - start;
    double length = sqrt(direction.x() * direction.x() + direction.y() * direction.y());
    if (length == 0) return;
    
    // 标准化方向向量
    direction /= length;
    
    // 从圆边缘开始和结束（考虑圆的半径）
    double radius = Constants::POINT_RADIUS;
    QPointF adjustedStart = start + direction * radius;
    QPointF adjustedEnd = end - direction * radius;
    
    // 创建美化的箭头图形项
    ArrowGraphicsItem* arrowItem = new ArrowGraphicsItem(adjustedStart, adjustedEnd, pen);
    
    // 设置箭头样式 - 根据pen颜色选择合适的样式
    ArrowRenderer::ArrowStyle style;
    if (pen.color() == Qt::red) {
        style = ArrowRenderer::createRedStyle();
    } else if (pen.color() == Qt::blue) {
        style = ArrowRenderer::createBlueStyle();
    } else if (pen.color() == QColor(255, 215, 0)) { // Gold
        style = ArrowRenderer::createGoldStyle();
    } else {
        style = ArrowRenderer::createGreenStyle();
    }
    
    // 调整样式以适应场景显示
    style.color = QColor(144, 238, 144, 200);  // 统一使用浅绿色
    style.outlineColor = QColor(60, 179, 113);  // 中等海绿色边框
    style.outlineWidth = 1.0;
    style.widthRatio = 1.0 / 4.0;  // 更大的宽度比例，让箭头更明显
    
    arrowItem->setArrowStyle(style);
    
    // 添加到场景和列表
    m_scene->addItem(arrowItem);
    m_pathArrows.append(arrowItem);
}

void GridGraphicsView::setStartPoint(const QPoint& point) {
    // 清除旧的起点
    if (hasStartPoint()) {
        setPointType(m_startPoint.x(), m_startPoint.y(), PointType::Normal);
    }
    
    setPointType(point.x(), point.y(), PointType::Start);
}

void GridGraphicsView::setEndPoint(const QPoint& point) {
    // 清除旧的终点
    if (hasEndPoint()) {
        setPointType(m_endPoint.x(), m_endPoint.y(), PointType::Normal);
    }
    
    setPointType(point.x(), point.y(), PointType::End);
}

QPoint GridGraphicsView::screenToGrid(const QPointF& screenPos) const {
    QPointF scenePos = mapToScene(screenPos.toPoint());
    
    int x = static_cast<int>(std::round(scenePos.x() / Constants::GRID_SPACING));
    int y = static_cast<int>(std::round(scenePos.y() / Constants::GRID_SPACING));
    
    return QPoint(x, y);
}

QPointF GridGraphicsView::gridToScene(int x, int y) const {
    return QPointF(x * Constants::GRID_SPACING, y * Constants::GRID_SPACING);
}

void GridGraphicsView::fitGridInView() {
    if (m_gridWidth <= 0 || m_gridHeight <= 0) return;
    
    QRectF gridRect(0, 0,
                    m_gridWidth * Constants::GRID_SPACING,
                    m_gridHeight * Constants::GRID_SPACING);
    
    m_scene->setSceneRect(gridRect);
    fitInView(gridRect, Qt::KeepAspectRatio);
}

void GridGraphicsView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        QPoint gridPos = screenToGrid(QPointF(event->pos()));
        
        if (gridPos.x() >= 0 && gridPos.x() < m_gridWidth &&
            gridPos.y() >= 0 && gridPos.y() < m_gridHeight) {
            
            PointType currentType = getPointType(gridPos.x(), gridPos.y());
            emit pointClicked(gridPos, currentType);
        }
    }
    
    QGraphicsView::mousePressEvent(event);
}

void GridGraphicsView::wheelEvent(QWheelEvent* event) {
    // 实现缩放功能
    const double scaleFactor = 1.15;
    
    if (event->angleDelta().y() > 0) {
        // 放大
        if (m_scaleFactor < SCALE_MAX) {
            scale(scaleFactor, scaleFactor);
            m_scaleFactor *= scaleFactor;
        }
    } else {
        // 缩小
        if (m_scaleFactor > SCALE_MIN) {
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);
            m_scaleFactor /= scaleFactor;
        }
    }
}

void GridGraphicsView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    // 可以在这里添加响应式布局逻辑
}

void GridGraphicsView::onSceneSelectionChanged() {
    // 处理场景选择变化
}
