#include "PathVisualWidget.h"
#include <QPaintEvent>
#include <QDebug>

PathVisualWidget::PathVisualWidget(QWidget* parent)
    : QWidget(parent)
    , m_arrowStyle(ArrowRenderer::createGreenStyle())
    , m_scale(20.0)
    , m_margin(5)
    , m_showGrid(false)
    , m_showPoints(true)
{
    setMinimumSize(100, 30);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    
    // 设置适合表格显示的箭头样式 - 更精致小巧
    m_arrowStyle.widthRatio = 1.0 / 6.0;  // 稍微粗一点
    m_arrowStyle.headLengthRatio = 0.15;   // 头部稍大
    m_arrowStyle.color = QColor(144, 238, 144, 200);  // 浅绿色
    m_arrowStyle.outlineColor = QColor(60, 179, 113);  // 中等海绿色边框
    m_arrowStyle.outlineWidth = 0.8;      // 适中的边框
}

void PathVisualWidget::setPath(const QVector<QPoint>& path)
{
    if (m_path != path) {
        m_path = path;
        update();
    }
}

void PathVisualWidget::setArrowStyle(const ArrowRenderer::ArrowStyle& style)
{
    m_arrowStyle = style;
    update();
}

QString PathVisualWidget::getPathString() const
{
    if (m_path.size() < 2) {
        return "无路径";
    }
    
    QString result;
    for (int i = 0; i < m_path.size(); ++i) {
        if (i > 0) {
            result += " → ";
        }
        result += QString("(%1,%2)").arg(m_path[i].x()).arg(m_path[i].y());
    }
    
    return result;
}

void PathVisualWidget::setScale(double scale)
{
    if (scale > 0 && scale != m_scale) {
        m_scale = scale;
        update();
    }
}

void PathVisualWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    
    if (m_path.size() < 2) {
        // 绘制"无路径"文本
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "无路径");
        return;
    }
    
    // 设置抗锯齿
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // 转换路径坐标
    QVector<QPointF> transformedPath = transformPath();
    
    if (transformedPath.size() < 2) {
        return;
    }
    
    // 绘制网格背景（如果启用）
    if (m_showGrid) {
        drawGrid(&painter);
    }
    
    // 绘制箭头路径
    ArrowRenderer::drawPathArrows(&painter, transformedPath, m_arrowStyle);
    
    // 绘制路径点（如果启用）
    if (m_showPoints) {
        drawPathPoints(&painter, transformedPath);
    }
}

QSize PathVisualWidget::sizeHint() const
{
    if (m_path.size() < 2) {
        return QSize(100, 30);
    }
    
    QRectF bounds = calculateBounds();
    int width = static_cast<int>(bounds.width() * m_scale) + 2 * m_margin;
    int height = static_cast<int>(bounds.height() * m_scale) + 2 * m_margin;
    
    // 限制最小和最大尺寸
    width = qMax(80, qMin(width, 300));
    height = qMax(25, qMin(height, 100));
    
    return QSize(width, height);
}

QSize PathVisualWidget::minimumSizeHint() const
{
    return QSize(80, 25);
}

QVector<QPointF> PathVisualWidget::transformPath() const
{
    if (m_path.isEmpty()) {
        return QVector<QPointF>();
    }
    
    QRectF bounds = calculateBounds();
    if (bounds.width() <= 0 || bounds.height() <= 0) {
        return QVector<QPointF>();
    }
    
    // 计算可用空间
    QRectF widgetRect = rect().adjusted(m_margin, m_margin, -m_margin, -m_margin);
    
    // 计算缩放比例，确保路径适应widget大小
    double scaleX = widgetRect.width() / bounds.width();
    double scaleY = widgetRect.height() / bounds.height();
    double scale = qMin(scaleX, scaleY);
    
    // 限制最小缩放比例
    scale = qMax(scale, 5.0);
    
    // 计算偏移量以居中显示
    double scaledWidth = bounds.width() * scale;
    double scaledHeight = bounds.height() * scale;
    double offsetX = widgetRect.left() + (widgetRect.width() - scaledWidth) / 2.0;
    double offsetY = widgetRect.top() + (widgetRect.height() - scaledHeight) / 2.0;
    
    // 转换所有点
    QVector<QPointF> transformedPath;
    for (const QPoint& point : m_path) {
        double x = offsetX + (point.x() - bounds.left()) * scale;
        double y = offsetY + (point.y() - bounds.top()) * scale;
        transformedPath.append(QPointF(x, y));
    }
    
    return transformedPath;
}

QRectF PathVisualWidget::calculateBounds() const
{
    if (m_path.isEmpty()) {
        return QRectF();
    }
    
    int minX = m_path[0].x();
    int maxX = m_path[0].x();
    int minY = m_path[0].y();
    int maxY = m_path[0].y();
    
    for (const QPoint& point : m_path) {
        minX = qMin(minX, point.x());
        maxX = qMax(maxX, point.x());
        minY = qMin(minY, point.y());
        maxY = qMax(maxY, point.y());
    }
    
    // 添加小的边距以确保箭头完全可见
    return QRectF(minX - 0.5, minY - 0.5, maxX - minX + 1.0, maxY - minY + 1.0);
}

void PathVisualWidget::drawGrid(QPainter* painter) const
{
    // 可选：绘制轻微的网格背景
    painter->save();
    
    QPen gridPen(QColor(200, 200, 200, 100));
    gridPen.setWidth(1);
    painter->setPen(gridPen);
    
    // 这里可以添加网格绘制逻辑
    // 暂时留空，根据需要可以后续添加
    
    painter->restore();
}

void PathVisualWidget::drawPathPoints(QPainter* painter, const QVector<QPointF>& transformedPath) const
{
    if (transformedPath.size() < 2) {
        return;
    }
    
    painter->save();
    
    // 绘制起点 - 使用淡黄色充满
    painter->setBrush(QBrush(QColor(255, 255, 224)));  // 淡黄色 (LightYellow)
    painter->setPen(QPen(QColor(218, 165, 32), 1.5));  // 金黄色边框
    painter->drawEllipse(transformedPath.first(), 4, 4);  // 稍微大一点
    
    // 绘制终点 - 使用淡黄色充满
    painter->setBrush(QBrush(QColor(255, 255, 224)));  // 淡黄色 (LightYellow)
    painter->setPen(QPen(QColor(218, 165, 32), 1.5));  // 金黄色边框
    painter->drawEllipse(transformedPath.last(), 4, 4);  // 稍微大一点
    
    // 绘制中间点 - 使用淡黄色充满但稍小
    if (transformedPath.size() > 2) {
        painter->setBrush(QBrush(QColor(255, 255, 224, 180)));  // 淡黄色，稍微透明
        painter->setPen(QPen(QColor(218, 165, 32), 1));
        for (int i = 1; i < transformedPath.size() - 1; ++i) {
            painter->drawEllipse(transformedPath[i], 3, 3);
        }
    }
    
    painter->restore();
}
