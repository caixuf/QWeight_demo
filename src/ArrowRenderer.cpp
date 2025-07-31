#include "ArrowRenderer.h"
#include <QDebug>

void ArrowRenderer::drawArrow(QPainter* painter, 
                             const QPointF& startPoint, 
                             const QPointF& endPoint, 
                             const ArrowStyle& style)
{
    if (!painter || !isValidPoint(startPoint) || !isValidPoint(endPoint)) {
        return;
    }

    // 计算距离，如果太小则不绘制
    const double dist = distance(startPoint, endPoint);
    if (dist < 2.0) {
        return;
    }

    // 保存原始状态
    painter->save();

    // 设置抗锯齿
    if (style.antialiasing) {
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    // 创建箭头多边形
    QPolygonF arrowPolygon = createArrowPolygon(startPoint, endPoint, style);
    if (arrowPolygon.isEmpty()) {
        painter->restore();
        return;
    }

    // 绘制箭头主体
    painter->setBrush(QBrush(style.color));
    if (style.showOutline && style.outlineWidth > 0) {
        QPen pen(style.outlineColor);
        pen.setWidthF(style.outlineWidth);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        painter->setPen(pen);
    } else {
        painter->setPen(Qt::NoPen);
    }

    painter->drawPolygon(arrowPolygon);

    // 恢复状态
    painter->restore();
}

void ArrowRenderer::drawPathArrows(QPainter* painter, 
                                  const QVector<QPointF>& path, 
                                  const ArrowStyle& style)
{
    if (!painter || path.size() < 2) {
        return;
    }

    for (int i = 0; i < path.size() - 1; ++i) {
        drawArrow(painter, path[i], path[i + 1], style);
    }
}

QPolygonF ArrowRenderer::createArrowPolygon(const QPointF& startPoint, 
                                           const QPointF& endPoint, 
                                           const ArrowStyle& style)
{
    if (!isValidPoint(startPoint) || !isValidPoint(endPoint)) {
        return QPolygonF();
    }

    const double dx = endPoint.x() - startPoint.x();
    const double dy = endPoint.y() - startPoint.y();
    const double dist = std::sqrt(dx * dx + dy * dy);
    
    if (dist < 2.0) {
        return QPolygonF();
    }

    // 参考代码中的算法，采用更小的比例和更优雅的形状
    const double headLength = dist / 2.8;  // 箭头头部长度调大2.5倍，从1/7改为约1/2.8
    
    // 内角和外角，创建更精致的箭头形状
    const double ARROW_ANGLE = 25.0;        // 外角度，稍小一些让箭头更尖锐
    const double ARROW_INNER_ANGLE = 12.0;  // 内角度，更小让箭头更精致
    
    const double headLength2 = headLength * std::cos(ARROW_ANGLE * M_PI / 180.0) / 
                              std::cos(ARROW_INNER_ANGLE * M_PI / 180.0);
    
    const double angle = std::atan2(dy, dx);
    const double angle1 = angle + (ARROW_ANGLE * M_PI / 180.0);
    const double angle2 = angle - (ARROW_ANGLE * M_PI / 180.0);
    const double angle3 = angle + (ARROW_INNER_ANGLE * M_PI / 180.0);
    const double angle4 = angle - (ARROW_INNER_ANGLE * M_PI / 180.0);
    
    // 计算箭头的关键点
    const QPointF arrowTop = endPoint - QPointF(headLength * std::cos(angle1), 
                                                headLength * std::sin(angle1));
    const QPointF arrowBot = endPoint - QPointF(headLength * std::cos(angle2), 
                                                headLength * std::sin(angle2));
    const QPointF arrowTop2 = endPoint - QPointF(headLength2 * std::cos(angle3), 
                                                 headLength2 * std::sin(angle3));
    const QPointF arrowBot2 = endPoint - QPointF(headLength2 * std::cos(angle4), 
                                                 headLength2 * std::sin(angle4));
    
    // 创建优雅的一笔画箭头多边形
    return QPolygonF() << startPoint << arrowTop2 << arrowTop 
                      << endPoint << arrowBot << arrowBot2;
}

ArrowRenderer::ArrowStyle ArrowRenderer::createGreenStyle()
{
    ArrowStyle style;
    style.color = QColor(144, 238, 144, 200);  // 浅绿色 (LightGreen)
    style.outlineColor = QColor(60, 179, 113);  // 中等海绿色作为边框
    style.widthRatio = 1.0 / 7.0;
    style.headLengthRatio = 0.12;
    style.headWidthRatio = 1.8;
    style.outlineWidth = 1.0;  // 稍微粗一点的边框
    style.showOutline = true;
    style.antialiasing = true;
    return style;
}

ArrowRenderer::ArrowStyle ArrowRenderer::createBlueStyle()
{
    ArrowStyle style;
    style.color = QColor(60, 120, 200, 200);
    style.outlineColor = QColor(40, 80, 140);
    style.widthRatio = 1.0 / 7.0;
    style.headLengthRatio = 0.12;
    style.headWidthRatio = 1.8;
    style.outlineWidth = 0.8;
    style.showOutline = true;
    style.antialiasing = true;
    return style;
}

ArrowRenderer::ArrowStyle ArrowRenderer::createRedStyle()
{
    ArrowStyle style;
    style.color = QColor(220, 60, 60, 200);
    style.outlineColor = QColor(160, 40, 40);
    style.widthRatio = 1.0 / 7.0;
    style.headLengthRatio = 0.12;
    style.headWidthRatio = 1.8;
    style.outlineWidth = 0.8;
    style.showOutline = true;
    style.antialiasing = true;
    return style;
}

ArrowRenderer::ArrowStyle ArrowRenderer::createGoldStyle()
{
    ArrowStyle style;
    style.color = QColor(255, 220, 60, 220);
    style.outlineColor = QColor(200, 160, 20);
    style.widthRatio = 1.0 / 7.0;
    style.headLengthRatio = 0.12;
    style.headWidthRatio = 1.8;
    style.outlineWidth = 0.8;
    style.showOutline = true;
    style.antialiasing = true;
    return style;
}
