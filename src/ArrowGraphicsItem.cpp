#include "ArrowGraphicsItem.h"
#include <QStyleOptionGraphicsItem>

ArrowGraphicsItem::ArrowGraphicsItem(const QPointF& startPoint, 
                                   const QPointF& endPoint, 
                                   const QPen& pen,
                                   QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , m_startPoint(startPoint)
    , m_endPoint(endPoint)
    , m_pen(pen)
    , m_arrowStyle(ArrowRenderer::createGreenStyle())
{
    // 调整箭头样式以适应Graphics Scene显示
    m_arrowStyle.color = pen.color();
    m_arrowStyle.outlineColor = pen.color().darker(150);
    m_arrowStyle.outlineWidth = 0.5;
    m_arrowStyle.widthRatio = 1.0 / 6.0;  // 稍微粗一点，便于在场景中看清楚
}

QRectF ArrowGraphicsItem::boundingRect() const
{
    return calculateBoundingRect();
}

void ArrowGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (m_startPoint == m_endPoint) {
        return;
    }

    // 绘制箭头主体（使用美化的箭头渲染器）
    ArrowRenderer::drawArrow(painter, m_startPoint, m_endPoint, m_arrowStyle);
}

void ArrowGraphicsItem::setPoints(const QPointF& startPoint, const QPointF& endPoint)
{
    if (m_startPoint != startPoint || m_endPoint != endPoint) {
        prepareGeometryChange();
        m_startPoint = startPoint;
        m_endPoint = endPoint;
        update();
    }
}

void ArrowGraphicsItem::setPen(const QPen& pen)
{
    if (m_pen != pen) {
        m_pen = pen;
        m_arrowStyle.color = pen.color();
        m_arrowStyle.outlineColor = pen.color().darker(150);
        update();
    }
}

void ArrowGraphicsItem::setArrowStyle(const ArrowRenderer::ArrowStyle& style)
{
    m_arrowStyle = style;
    update();
}

QRectF ArrowGraphicsItem::calculateBoundingRect() const
{
    if (m_startPoint == m_endPoint) {
        return QRectF();
    }

    // 计算起点和终点的边界矩形
    QRectF rect(m_startPoint, m_endPoint);
    rect = rect.normalized();

    // 添加一些边距以确保箭头完全可见
    double margin = 20.0;  // 足够的边距以包含箭头头部
    return rect.adjusted(-margin, -margin, margin, margin);
}
