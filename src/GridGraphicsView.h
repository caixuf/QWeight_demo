#ifndef GRIDGRAPHICSVIEW_H
#define GRIDGRAPHICSVIEW_H

#include "Common.h"
#include "GridPoint.h"
#include "ArrowRenderer.h"
#include "ArrowGraphicsItem.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QVector>
#include <QMouseEvent>
#include <QWheelEvent>

class GridGraphicsView : public QGraphicsView {
    Q_OBJECT

public:
    explicit GridGraphicsView(QWidget* parent = nullptr);
    ~GridGraphicsView();
    
    // 网格操作
    void createGrid(int width, int height);
    void clearGrid();
    void resetGrid();
    
    // 点操作
    void setPointType(int x, int y, PointType type);
    PointType getPointType(int x, int y) const;
    
    // 路径显示
    void showPath(const QVector<QPoint>& path);
    void clearPath();
    
    // 网格信息
    int gridWidth() const { return m_gridWidth; }
    int gridHeight() const { return m_gridHeight; }
    
    // 获取网格数据
    const QVector<QVector<GridPoint>>& getGrid() const { return m_gridData; }
    
    // 起点终点操作
    void setStartPoint(const QPoint& point);
    void setEndPoint(const QPoint& point);
    QPoint getStartPoint() const { return m_startPoint; }
    QPoint getEndPoint() const { return m_endPoint; }
    bool hasStartPoint() const { return m_startPoint != QPoint(-1, -1); }
    bool hasEndPoint() const { return m_endPoint != QPoint(-1, -1); }

signals:
    void pointClicked(QPoint position, PointType currentType);
    void startPointSet(QPoint position);
    void endPointSet(QPoint position);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onSceneSelectionChanged();

private:
    void setupScene();
    void createGridLines();
    void createGridPoints();
    void updatePointAppearance(int x, int y);
    void drawArrow(const QPointF& start, const QPointF& end, const QPen& pen);
    QPoint screenToGrid(const QPointF& screenPos) const;
    QPointF gridToScene(int x, int y) const;
    void fitGridInView();
    
    QGraphicsScene* m_scene;
    int m_gridWidth;
    int m_gridHeight;
    
    // 网格数据
    QVector<QVector<GridPoint>> m_gridData;
    
    // 图形项
    QVector<QVector<QGraphicsEllipseItem*>> m_pointItems;
    QVector<QGraphicsLineItem*> m_gridLines;
    QVector<ArrowGraphicsItem*> m_pathArrows;  // 改为箭头图形项
    
    // 起点终点
    QPoint m_startPoint;
    QPoint m_endPoint;
    
    // 缩放相关
    double m_scaleFactor;
    static constexpr double SCALE_MIN = 0.1;
    static constexpr double SCALE_MAX = 5.0;
};

#endif // GRIDGRAPHICSVIEW_H
