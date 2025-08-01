#ifndef GRIDPOINT_H
#define GRIDPOINT_H

#include "Common.h"
#include <QPoint>

class GridPoint {
public:
    GridPoint();
    GridPoint(int x, int y, PointType type = PointType::Normal);
    
    // Getter方法
    int x() const { return m_position.x(); }
    int y() const { return m_position.y(); }
    QPoint position() const { return m_position; }
    PointType type() const { return m_type; }
    bool isVisited() const { return m_visited; }
    
    // 路径计算相关方法
    double getWeight() const;
    bool isWalkable() const;
    
    // Setter方法
    void setPosition(int x, int y) { m_position = QPoint(x, y); }
    void setPosition(const QPoint& pos) { m_position = pos; }
    void setType(PointType type) { m_type = type; }
    void setVisited(bool visited) { m_visited = visited; }
    
    // 重置状态
    void reset();
    
    // 比较操作符
    bool operator==(const GridPoint& other) const;
    bool operator!=(const GridPoint& other) const;

private:
    QPoint m_position;
    PointType m_type;
    bool m_visited;
};

#endif // GRIDPOINT_H
