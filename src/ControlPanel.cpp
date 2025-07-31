#include "ControlPanel.h"
#include <QApplication>

ControlPanel::ControlPanel(QWidget* parent)
    : QWidget(parent)
    , m_currentState(CalculationState::Idle)
{
    setupUI();
    setupConnections();
    setCalculationState(CalculationState::Idle);
}

void ControlPanel::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    setupGridSizeGroup();
    setupPointControlGroup();
    setupCalculationGroup();
    setupSaveGroup();
    
    // 添加弹性空间
    m_mainLayout->addStretch();
    
    setLayout(m_mainLayout);
}

void ControlPanel::setupGridSizeGroup() {
    m_gridSizeGroup = new QGroupBox("网格大小设置", this);
    QGridLayout* layout = new QGridLayout(m_gridSizeGroup);
    
    // 宽度设置
    layout->addWidget(new QLabel("宽度:"), 0, 0);
    m_widthSpinBox = new QSpinBox();
    m_widthSpinBox->setRange(Constants::MIN_GRID_SIZE, Constants::MAX_GRID_SIZE);
    m_widthSpinBox->setValue(Constants::DEFAULT_GRID_WIDTH);
    layout->addWidget(m_widthSpinBox, 0, 1);
    
    // 高度设置
    layout->addWidget(new QLabel("高度:"), 1, 0);
    m_heightSpinBox = new QSpinBox();
    m_heightSpinBox->setRange(Constants::MIN_GRID_SIZE, Constants::MAX_GRID_SIZE);
    m_heightSpinBox->setValue(Constants::DEFAULT_GRID_HEIGHT);
    layout->addWidget(m_heightSpinBox, 1, 1);
    
    // 应用按钮
    m_applyButton = new QPushButton("应用");
    layout->addWidget(m_applyButton, 2, 0, 1, 2);
    
    m_mainLayout->addWidget(m_gridSizeGroup);
}

void ControlPanel::setupPointControlGroup() {
    m_pointControlGroup = new QGroupBox("起点终点设置", this);
    QVBoxLayout* layout = new QVBoxLayout(m_pointControlGroup);
    
    m_setStartButton = new QPushButton("设置起点");
    m_setStartButton->setCheckable(true);
    m_setStartButton->setStyleSheet(
        "QPushButton:checked { background-color: lightgreen; }"
    );
    layout->addWidget(m_setStartButton);
    
    m_setEndButton = new QPushButton("设置终点");
    m_setEndButton->setCheckable(true);
    m_setEndButton->setStyleSheet(
        "QPushButton:checked { background-color: lightcoral; }"
    );
    layout->addWidget(m_setEndButton);
    
    m_mainLayout->addWidget(m_pointControlGroup);
}

void ControlPanel::setupCalculationGroup() {
    m_calculationGroup = new QGroupBox("路径计算", this);
    QVBoxLayout* layout = new QVBoxLayout(m_calculationGroup);
    
    // 算法选择（固定为DFS）
    QHBoxLayout* algoLayout = new QHBoxLayout();
    algoLayout->addWidget(new QLabel("算法:"));
    m_algorithmCombo = new QComboBox();
    m_algorithmCombo->addItem("DFS (深度优先搜索)", static_cast<int>(AlgorithmType::DFS));
    m_algorithmCombo->setEnabled(false);  // 禁用算法选择，固定为DFS
    algoLayout->addWidget(m_algorithmCombo);
    layout->addLayout(algoLayout);
    
    // 控制按钮
    QHBoxLayout* buttonLayout1 = new QHBoxLayout();
    m_startButton = new QPushButton("开始计算");
    m_pauseResumeButton = new QPushButton("暂停");
    buttonLayout1->addWidget(m_startButton);
    buttonLayout1->addWidget(m_pauseResumeButton);
    layout->addLayout(buttonLayout1);
    
    QHBoxLayout* buttonLayout2 = new QHBoxLayout();
    m_stopButton = new QPushButton("停止");
    m_resetButton = new QPushButton("重新开始");
    buttonLayout2->addWidget(m_stopButton);
    buttonLayout2->addWidget(m_resetButton);
    layout->addLayout(buttonLayout2);
    
    // 进度条
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    layout->addWidget(m_progressBar);
    
    // 状态标签
    m_statusLabel = new QLabel("就绪");
    m_statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
    layout->addWidget(m_statusLabel);
    
    m_mainLayout->addWidget(m_calculationGroup);
}

void ControlPanel::setupSaveGroup() {
    m_saveGroup = new QGroupBox("结果保存", this);
    QVBoxLayout* layout = new QVBoxLayout(m_saveGroup);
    
    m_saveXmlButton = new QPushButton("保存到XML");
    m_saveSqliteButton = new QPushButton("保存到SQLite");
    
    layout->addWidget(m_saveXmlButton);
    layout->addWidget(m_saveSqliteButton);
    
    m_mainLayout->addWidget(m_saveGroup);
}

void ControlPanel::setupConnections() {
    // 网格大小
    connect(m_applyButton, &QPushButton::clicked,
            this, &ControlPanel::onApplyGridSize);
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value) { 
                emit gridSizeChanged(value, m_heightSpinBox->value()); 
            });
    connect(m_heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value) { 
                emit gridSizeChanged(m_widthSpinBox->value(), value); 
            });
    
    // 点设置
    connect(m_setStartButton, &QPushButton::clicked,
            this, &ControlPanel::onStartPointMode);
    connect(m_setEndButton, &QPushButton::clicked,
            this, &ControlPanel::onEndPointMode);
    
    // 计算控制
    connect(m_startButton, &QPushButton::clicked,
            this, &ControlPanel::onStartCalculation);
    connect(m_pauseResumeButton, &QPushButton::clicked,
            this, &ControlPanel::onPauseResume);
    connect(m_stopButton, &QPushButton::clicked,
            this, &ControlPanel::onStopCalculation);
    connect(m_resetButton, &QPushButton::clicked,
            this, &ControlPanel::onResetCalculation);
    
    // 保存
    connect(m_saveXmlButton, &QPushButton::clicked,
            this, &ControlPanel::onSaveToXml);
    connect(m_saveSqliteButton, &QPushButton::clicked,
            this, &ControlPanel::onSaveToSqlite);
}

int ControlPanel::getGridWidth() const {
    return m_widthSpinBox->value();
}

int ControlPanel::getGridHeight() const {
    return m_heightSpinBox->value();
}

AlgorithmType ControlPanel::getSelectedAlgorithm() const {
    int index = m_algorithmCombo->currentData().toInt();
    return static_cast<AlgorithmType>(index);
}

void ControlPanel::setCalculationState(CalculationState state) {
    m_currentState = state;
    updateCalculationButtons(state);
    
    switch (state) {
        case CalculationState::Idle:
            m_statusLabel->setText("就绪");
            m_statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
            m_progressBar->setVisible(false);
            break;
            
        case CalculationState::Running:
            m_statusLabel->setText("计算中...");
            m_statusLabel->setStyleSheet("QLabel { color: blue; font-weight: bold; }");
            m_progressBar->setVisible(true);
            break;
            
        case CalculationState::Paused:
            m_statusLabel->setText("已暂停");
            m_statusLabel->setStyleSheet("QLabel { color: orange; font-weight: bold; }");
            break;
            
        case CalculationState::Completed:
            m_statusLabel->setText("计算完成");
            m_statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
            m_progressBar->setVisible(false);
            break;
            
        case CalculationState::Stopped:
            m_statusLabel->setText("已停止");
            m_statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
            m_progressBar->setVisible(false);
            break;
    }
}

void ControlPanel::updateProgress(int percentage) {
    m_progressBar->setValue(percentage);
}

void ControlPanel::setGridSize(int width, int height) {
    m_widthSpinBox->setValue(width);
    m_heightSpinBox->setValue(height);
}

void ControlPanel::updateCalculationButtons(CalculationState state) {
    switch (state) {
        case CalculationState::Idle:
        case CalculationState::Completed:
        case CalculationState::Stopped:
            m_startButton->setEnabled(true);
            m_pauseResumeButton->setEnabled(false);
            m_pauseResumeButton->setText("暂停");
            m_stopButton->setEnabled(false);
            m_resetButton->setEnabled(true);
            break;
            
        case CalculationState::Running:
            m_startButton->setEnabled(false);
            m_pauseResumeButton->setEnabled(true);
            m_pauseResumeButton->setText("暂停");
            m_stopButton->setEnabled(true);
            m_resetButton->setEnabled(false);
            break;
            
        case CalculationState::Paused:
            m_startButton->setEnabled(false);
            m_pauseResumeButton->setEnabled(true);
            m_pauseResumeButton->setText("继续");
            m_stopButton->setEnabled(true);
            m_resetButton->setEnabled(false);
            break;
    }
}

// 槽函数实现
void ControlPanel::onApplyGridSize() {
    emit applyGridSize();
}

void ControlPanel::onStartPointMode() {
    m_setEndButton->setChecked(false);
    emit setStartPointMode();
}

void ControlPanel::onEndPointMode() {
    m_setStartButton->setChecked(false);
    emit setEndPointMode();
}

void ControlPanel::onStartCalculation() {
    emit startCalculation();
}

void ControlPanel::onPauseResume() {
    if (m_currentState == CalculationState::Running) {
        emit pauseCalculation();
    } else if (m_currentState == CalculationState::Paused) {
        emit resumeCalculation();
    }
}

void ControlPanel::onStopCalculation() {
    emit stopCalculation();
}

void ControlPanel::onResetCalculation() {
    emit resetCalculation();
}

void ControlPanel::onSaveToXml() {
    emit saveToXml();
}

void ControlPanel::onSaveToSqlite() {
    emit saveToSqlite();
}
