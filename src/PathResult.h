#ifndef PATHRESULT_H
#define PATHRESULT_H

#include "Common.h"
#include <QString>
#include <QPoint>
#include <QVector>
#include <QDateTime>

class PathResult {
public:
    PathResult();
    PathResult(const QString& id,
               const QPoint& startPoint,
               const QPoint& endPoint,
               const QVector<QPoint>& path,
               AlgorithmType algorithm,
               qint64 calculationTime);
    
    // Getter方法
    QString id() const { return m_id; }
    QPoint startPoint() const { return m_startPoint; }
    QPoint endPoint() const { return m_endPoint; }
    QVector<QPoint> path() const { return m_path; }
    QVector<QPoint> getPath() const { return m_path; } // 为兼容性添加
    AlgorithmType algorithm() const { return m_algorithm; }
    QString algorithmString() const { return algorithmTypeToString(m_algorithm); }
    int pathLength() const { return m_path.size(); }
    qint64 calculationTime() const { return m_calculationTime; }
    QDateTime timestamp() const { return m_timestamp; }
    
    // Setter方法
    void setId(const QString& id) { m_id = id; }
    void setStartPoint(const QPoint& point) { m_startPoint = point; }
    void setEndPoint(const QPoint& point) { m_endPoint = point; }
    void setPath(const QVector<QPoint>& path) { m_path = path; }
    void setAlgorithm(AlgorithmType algorithm) { m_algorithm = algorithm; }
    void setCalculationTime(qint64 time) { m_calculationTime = time; }
    void setTimestamp(const QDateTime& timestamp) { m_timestamp = timestamp; }
    
    // 验证路径是否有效
    bool isValid() const;
    
    // 转换为显示字符串
    QString toString() const;

private:
    QString m_id;
    QPoint m_startPoint;
    QPoint m_endPoint;
    QVector<QPoint> m_path;
    AlgorithmType m_algorithm;
    qint64 m_calculationTime;  // 毫秒
    QDateTime m_timestamp;
};

#endif // PATHRESULT_H
