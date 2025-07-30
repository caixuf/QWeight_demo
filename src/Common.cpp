#include "Common.h"

QString algorithmTypeToString(AlgorithmType type) {
    switch (type) {
        case AlgorithmType::AStar: return "A*";
        case AlgorithmType::Dijkstra: return "Dijkstra";
        case AlgorithmType::BFS: return "BFS";
        case AlgorithmType::DFS: return "DFS";
        default: return "Unknown";
    }
}

AlgorithmType stringToAlgorithmType(const QString& str) {
    if (str == "A*") return AlgorithmType::AStar;
    if (str == "Dijkstra") return AlgorithmType::Dijkstra;
    if (str == "BFS") return AlgorithmType::BFS;
    if (str == "DFS") return AlgorithmType::DFS;
    return AlgorithmType::AStar; // 默认值
}

QString pointTypeToString(PointType type) {
    switch (type) {
        case PointType::Normal: return "Normal";
        case PointType::Start: return "Start";
        case PointType::End: return "End";
        case PointType::Path: return "Path";
        case PointType::Obstacle: return "Obstacle";
        default: return "Unknown";
    }
}

QColor getPointColor(PointType type) {
    switch (type) {
        case PointType::Normal: return Constants::NORMAL_POINT_COLOR;
        case PointType::Start: return Constants::START_POINT_COLOR;
        case PointType::End: return Constants::END_POINT_COLOR;
        case PointType::Path: return Constants::PATH_POINT_COLOR;
        case PointType::Obstacle: return Constants::OBSTACLE_POINT_COLOR;
        default: return Constants::NORMAL_POINT_COLOR;
    }
}
