#ifndef COMMON_H
#define COMMON_H

#include <QPoint>
#include <QColor>
#include <QString>

// 点类型枚举
enum class PointType {
    Normal,     // 普通点
    Start,      // 起点
    End,        // 终点
    Path,       // 路径点
    Obstacle    // 障碍点
};

// 算法类型枚举
enum class AlgorithmType {
    AStar,      // A*算法
    Dijkstra,   // Dijkstra算法
    BFS,        // 广度优先搜索
    DFS         // 深度优先搜索
};

// 计算状态枚举
enum class CalculationState {
    Idle,       // 空闲
    Running,    // 运行中
    Paused,     // 暂停
    Completed,  // 完成
    Stopped     // 停止
};

// 常量定义
namespace Constants {
    const int DEFAULT_GRID_WIDTH = 5;
    const int DEFAULT_GRID_HEIGHT = 5;
    const int MIN_GRID_SIZE = 5;
    const int MAX_GRID_SIZE = 100;
    
    const int POINT_RADIUS = 8;
    const int GRID_SPACING = 25;
    
    // 颜色定义
    const QColor NORMAL_POINT_COLOR = QColor(255, 255, 255);   // 白色
    const QColor START_POINT_COLOR = QColor(0, 255, 0);        // 绿色
    const QColor END_POINT_COLOR = QColor(255, 0, 0);          // 红色
    const QColor PATH_POINT_COLOR = QColor(0, 0, 255);         // 蓝色
    const QColor OBSTACLE_POINT_COLOR = QColor(0, 0, 0);       // 黑色
    const QColor GRID_LINE_COLOR = QColor(200, 200, 200);      // 浅灰色
}

// 工具函数
QString algorithmTypeToString(AlgorithmType type);
AlgorithmType stringToAlgorithmType(const QString& str);
QString pointTypeToString(PointType type);
QColor getPointColor(PointType type);

#endif // COMMON_H
