#include "GridPoint.h"

GridPoint::GridPoint()
    : m_position(0, 0)
    , m_type(PointType::Normal)
    , m_visited(false)
{
}

GridPoint::GridPoint(int x, int y, PointType type)
    : m_position(x, y)
    , m_type(type)
    , m_visited(false)
{
}

void GridPoint::reset() {
    m_type = PointType::Normal;
    m_visited = false;
}

bool GridPoint::operator==(const GridPoint& other) const {
    return m_position == other.m_position;
}

bool GridPoint::operator!=(const GridPoint& other) const {
    return !(*this == other);
}
