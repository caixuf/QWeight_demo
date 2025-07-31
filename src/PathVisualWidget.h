#ifndef PATHVISUALWIDGET_H
#define PATHVISUALWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QVector>
#include <QPoint>
#include "ArrowRenderer.h"

/**
 * @brief 路径可视化widget
 * 用于在表格单元格中显示美化的路径箭头
 */
class PathVisualWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PathVisualWidget(QWidget* parent = nullptr);

    /**
     * @brief 设置要显示的路径
     * @param path 路径点列表
     */
    void setPath(const QVector<QPoint>& path);

    /**
     * @brief 设置箭头样式
     * @param style 箭头样式
     */
    void setArrowStyle(const ArrowRenderer::ArrowStyle& style);

    /**
     * @brief 获取路径字符串表示（用于tooltips等）
     * @return 格式化的路径字符串
     */
    QString getPathString() const;

    /**
     * @brief 设置缩放因子
     * @param scale 缩放因子
     */
    void setScale(double scale);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    /**
     * @brief 将路径点转换为widget坐标
     * @return 转换后的路径点
     */
    QVector<QPointF> transformPath() const;

    /**
     * @brief 计算路径的边界矩形
     * @return 边界矩形
     */
    QRectF calculateBounds() const;

    /**
     * @brief 绘制网格背景（可选）
     */
    void drawGrid(QPainter* painter) const;

    /**
     * @brief 绘制路径点
     */
    void drawPathPoints(QPainter* painter, const QVector<QPointF>& transformedPath) const;

private:
    QVector<QPoint> m_path;                    // 原始路径
    ArrowRenderer::ArrowStyle m_arrowStyle;    // 箭头样式
    double m_scale;                            // 缩放因子
    int m_margin;                              // 边距
    bool m_showGrid;                           // 是否显示网格
    bool m_showPoints;                         // 是否显示路径点
};

#endif // PATHVISUALWIDGET_H
