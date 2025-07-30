#include "PathResult.h"
#include <QUuid>

PathResult::PathResult()
    : m_id(QUuid::createUuid().toString())
    , m_startPoint(0, 0)
    , m_endPoint(0, 0)
    , m_algorithm(AlgorithmType::AStar)
    , m_calculationTime(0)
    , m_timestamp(QDateTime::currentDateTime())
{
}

PathResult::PathResult(const QString& id,
                       const QPoint& startPoint,
                       const QPoint& endPoint,
                       const QVector<QPoint>& path,
                       AlgorithmType algorithm,
                       qint64 calculationTime)
    : m_id(id.isEmpty() ? QUuid::createUuid().toString() : id)
    , m_startPoint(startPoint)
    , m_endPoint(endPoint)
    , m_path(path)
    , m_algorithm(algorithm)
    , m_calculationTime(calculationTime)
    , m_timestamp(QDateTime::currentDateTime())
{
}

bool PathResult::isValid() const {
    return !m_path.isEmpty() && 
           m_path.first() == m_startPoint && 
           m_path.last() == m_endPoint;
}

QString PathResult::toString() const {
    return QString("路径 %1->%2 [%3] 长度:%4 时间:%5ms")
           .arg(QString("(%1,%2)").arg(m_startPoint.x()).arg(m_startPoint.y()))
           .arg(QString("(%1,%2)").arg(m_endPoint.x()).arg(m_endPoint.y()))
           .arg(algorithmTypeToString(m_algorithm))
           .arg(pathLength())
           .arg(m_calculationTime);
}
