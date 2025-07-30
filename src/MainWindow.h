#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Common.h"
#include "GridGraphicsView.h"
#include "ControlPanel.h"
#include "ResultListWidget.h"
#include "PathResult.h"
#include <QMainWindow>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // 网格操作
    void onGridSizeChanged(int width, int height);
    void onApplyGridSize();
    void onPointClicked(QPoint position, PointType currentType);
    void onStartPointSet(QPoint position);
    void onEndPointSet(QPoint position);
    
    // 控制面板操作
    void onSetStartPointMode();
    void onSetEndPointMode();
    void onStartCalculation();
    void onPauseCalculation();
    void onResumeCalculation();
    void onStopCalculation();
    void onResetCalculation();
    void onSaveToXml();
    void onSaveToSqlite();
    
    // 结果列表操作
    void onResultDoubleClicked(const PathResult& result);
    void onResultSelectionChanged(const PathResult& result);
    void onDeleteSelectedResults();
    void onExportResults();
    
    // 历史记录操作
    void onOpenXmlFile();
    void onOpenSqliteFile();
    void onClearCurrentResults();

private:
    void setupUI();
    void setupCentralWidget();
    void setupMenuBar();
    void setupStatusBar();
    void setupHistoryBar();
    void setupConnections();
    void setupStyles();
    
    void updateStatusMessage(const QString& message);
    void showCalculationProgress(bool show);
    void updateCalculationProgress(int percentage);
    void updateGridSizeLabel();
    
    // 路径计算辅助方法
    QVector<QPoint> calculateSimplePath(const QPoint& start, const QPoint& end);
    QVector<QPoint> calculatePathWithAlgorithm(const QPoint& start, const QPoint& end, AlgorithmType algorithm);
    QVector<QVector<QPoint>> calculateAllPossiblePaths(const QPoint& start, const QPoint& end, AlgorithmType algorithm);
    QVector<QPoint> calculatePathVariant(const QPoint& start, const QPoint& end, int variant);
    QVector<QPoint> calculateAStarPath(const QPoint& start, const QPoint& end);
    QVector<QPoint> calculateDijkstraPath(const QPoint& start, const QPoint& end);
    QVector<QPoint> calculateBFSPath(const QPoint& start, const QPoint& end);
    QVector<QPoint> calculateDFSPath(const QPoint& start, const QPoint& end);
    QString getAlgorithmName(AlgorithmType algorithm);
    
    // UI组件
    QWidget* m_centralWidget;
    QSplitter* m_mainSplitter;
    QSplitter* m_topSplitter;
    
    GridGraphicsView* m_gridView;
    ControlPanel* m_controlPanel;
    ResultListWidget* m_resultList;
    
    // 历史记录操作栏
    QWidget* m_historyBar;
    QHBoxLayout* m_historyLayout;
    QPushButton* m_openXmlButton;
    QPushButton* m_openSqliteButton;
    QPushButton* m_clearResultsButton;
    
    // 菜单栏
    QMenuBar* m_menuBar;
    QMenu* m_fileMenu;
    QMenu* m_editMenu;
    QMenu* m_viewMenu;
    QMenu* m_helpMenu;
    
    // 状态栏
    QStatusBar* m_statusBar;
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    QLabel* m_gridSizeLabel;
    
    // 当前状态
    enum PointSetMode {
        None,
        SetStart,
        SetEnd
    } m_pointSetMode;
    
    CalculationState m_calculationState;
    QPoint m_currentStartPoint;
    QPoint m_currentEndPoint;
    
    // 计算控制
    bool m_isCalculating;
    bool m_shouldStopCalculation;
};

#endif // MAINWINDOW_H
