#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include "Common.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QProgressBar>

class ControlPanel : public QWidget {
    Q_OBJECT

public:
    explicit ControlPanel(QWidget* parent = nullptr);
    
    // 获取设置值
    int getGridWidth() const;
    int getGridHeight() const;
    AlgorithmType getSelectedAlgorithm() const;
    AlgorithmType getCurrentAlgorithm() const { return getSelectedAlgorithm(); }  // 添加别名
    
    // 设置控件状态
    void setCalculationState(CalculationState state);
    void updateProgress(int percentage);
    void setGridSize(int width, int height);

signals:
    // 网格设置信号
    void gridSizeChanged(int width, int height);
    void applyGridSize();
    
    // 点设置信号
    void setStartPointMode();
    void setEndPointMode();
    
    // 计算控制信号
    void startCalculation();
    void pauseCalculation();
    void resumeCalculation();
    void stopCalculation();
    void resetCalculation();
    
    // 保存信号
    void saveToXml();
    void saveToSqlite();

private slots:
    void onApplyGridSize();
    void onStartPointMode();
    void onEndPointMode();
    void onStartCalculation();
    void onPauseResume();
    void onStopCalculation();
    void onResetCalculation();
    void onSaveToXml();
    void onSaveToSqlite();

private:
    void setupUI();
    void setupGridSizeGroup();
    void setupPointControlGroup();
    void setupCalculationGroup();
    void setupSaveGroup();
    void setupConnections();
    void updateCalculationButtons(CalculationState state);
    
    // 网格大小设置
    QGroupBox* m_gridSizeGroup;
    QSpinBox* m_widthSpinBox;
    QSpinBox* m_heightSpinBox;
    QPushButton* m_applyButton;
    
    // 点设置控制
    QGroupBox* m_pointControlGroup;
    QPushButton* m_setStartButton;
    QPushButton* m_setEndButton;
    
    // 计算控制
    QGroupBox* m_calculationGroup;
    QComboBox* m_algorithmCombo;
    QPushButton* m_startButton;
    QPushButton* m_pauseResumeButton;
    QPushButton* m_stopButton;
    QPushButton* m_resetButton;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    
    // 结果保存
    QGroupBox* m_saveGroup;
    QPushButton* m_saveXmlButton;
    QPushButton* m_saveSqliteButton;
    
    // 当前计算状态
    CalculationState m_currentState;
    
    // 主布局
    QVBoxLayout* m_mainLayout;
};

#endif // CONTROLPANEL_H
