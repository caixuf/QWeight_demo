#ifndef ARROWGRAPHICSITEM_H
#define ARROWGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPen>
#include "ArrowRenderer.h"

/**
 * @brief 美化箭头的Graphics Item
 * 用于在QGraphicsScene中显示美化的箭头
 */
class ArrowGraphicsItem : public QGraphicsItem
{
public:
    explicit ArrowGraphicsItem(const QPointF& startPoint, 
                              const QPointF& endPoint, 
                              const QPen& pen = QPen(),
                              QGraphicsItem* parent = nullptr);

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // 设置箭头参数
    void setPoints(const QPointF& startPoint, const QPointF& endPoint);
    void setPen(const QPen& pen);
    void setArrowStyle(const ArrowRenderer::ArrowStyle& style);

private:
    QPointF m_startPoint;
    QPointF m_endPoint;
    QPen m_pen;
    ArrowRenderer::ArrowStyle m_arrowStyle;
    
    // 计算边界矩形的辅助方法
    QRectF calculateBoundingRect() const;
};

#endif // ARROWGRAPHICSITEM_H
