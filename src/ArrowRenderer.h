#ifndef ARROWRENDERER_H
#define ARROWRENDERER_H

#include <QWidget>
#include <QPainter>
#include <QPointF>
#include <QPolygonF>
#include <QLineF>
#include <QColor>
#include <cmath>

/**
 * @brief 箭头渲染器类
 * 用于绘制美观的箭头，箭头宽度为两圆心距离的七分之一
 */
class ArrowRenderer
{
public:
    // 箭头样式配置
    struct ArrowStyle {
        QColor color;
        QColor outlineColor;
        double widthRatio;
        double headLengthRatio;
        double headWidthRatio;
        double outlineWidth;
        bool showOutline;
        bool antialiasing;
        
        // 构造函数
        ArrowStyle() 
            : color(50, 150, 50)
            , outlineColor(0, 0, 0)
            , widthRatio(1.0 / 7.0)
            , headLengthRatio(0.15)
            , headWidthRatio(2.0)
            , outlineWidth(1.0)
            , showOutline(true)
            , antialiasing(true)
        {
        }
    };

    /**
     * @brief 绘制箭头
     * @param painter 画笔
     * @param startPoint 起点
     * @param endPoint 终点
     * @param style 箭头样式
     */
    static void drawArrow(QPainter* painter, 
                         const QPointF& startPoint, 
                         const QPointF& endPoint, 
                         const ArrowStyle& style);

    /**
     * @brief 绘制路径上的所有箭头
     * @param painter 画笔
     * @param path 路径点列表
     * @param style 箭头样式
     */
    static void drawPathArrows(QPainter* painter, 
                              const QVector<QPointF>& path, 
                              const ArrowStyle& style);

    /**
     * @brief 创建箭头多边形
     * @param startPoint 起点
     * @param endPoint 终点
     * @param style 箭头样式
     * @return 箭头多边形
     */
    static QPolygonF createArrowPolygon(const QPointF& startPoint, 
                                       const QPointF& endPoint, 
                                       const ArrowStyle& style);

    /**
     * @brief 创建预设样式
     */
    static ArrowStyle createGreenStyle();     // 绿色箭头
    static ArrowStyle createBlueStyle();      // 蓝色箭头
    static ArrowStyle createRedStyle();       // 红色箭头
    static ArrowStyle createGoldStyle();      // 金色箭头

private:
    /**
     * @brief 计算两点间距离
     */
    static double distance(const QPointF& p1, const QPointF& p2);

    /**
     * @brief 计算两点间角度
     */
    static double angle(const QPointF& p1, const QPointF& p2);

    /**
     * @brief 检查点是否有效
     */
    static bool isValidPoint(const QPointF& point);
};

// 内联函数实现
inline double ArrowRenderer::distance(const QPointF& p1, const QPointF& p2)
{
    const double dx = p2.x() - p1.x();
    const double dy = p2.y() - p1.y();
    return std::sqrt(dx * dx + dy * dy);
}

inline double ArrowRenderer::angle(const QPointF& p1, const QPointF& p2)
{
    const double dx = p2.x() - p1.x();
    const double dy = p2.y() - p1.y();
    return std::atan2(dy, dx);
}

inline bool ArrowRenderer::isValidPoint(const QPointF& point)
{
    return std::isfinite(point.x()) && std::isfinite(point.y());
}

#endif // ARROWRENDERER_H
