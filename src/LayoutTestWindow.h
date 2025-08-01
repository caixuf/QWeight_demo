#ifndef LAYOUTTESTWINDOW_H
#define LAYOUTTESTWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QFrame>
#include <QGroupBox>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QProgressBar>
#include <QListWidget>
#include <QTreeWidget>
#include <QTabWidget>

class LayoutTestWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LayoutTestWindow(QWidget *parent = nullptr);
    ~LayoutTestWindow();

private slots:
    void onSpacerDemo();
    void onSizePolicyDemo();
    void onStretchFactorDemo();
    void onGridLayoutDemo();

private:
    void setupUI();
    void createBasicLayoutDemo();
    void createGridLayoutDemo();
    void createSplitterDemo();
    void createSpacerDemo();
    void createSizePolicyDemo();
    void createStretchFactorDemo();
    void createRealWorldExample();
    
    // 示例方法
    QWidget* createBasicWidget(const QString& text, const QColor& color);
    QWidget* createControlPanel();
    QWidget* createContentArea();
    QWidget* createStatusArea();
    
    // UI组件
    QWidget* m_centralWidget;
    QTabWidget* m_tabWidget;
    
    // 各个演示页面
    QWidget* m_basicLayoutPage;
    QWidget* m_gridLayoutPage;
    QWidget* m_splitterPage;
    QWidget* m_spacerPage;
    QWidget* m_sizePolicyPage;
    QWidget* m_stretchFactorPage;
    QWidget* m_realWorldPage;
    
    // 演示用的控件
    QSlider* m_stretchSlider;
    QSpinBox* m_stretchSpin;
    QLabel* m_statusLabel;
    
    // 演示布局引用
    QHBoxLayout* m_stretchDemoLayout;
    QPushButton* m_middleButton;
};

#endif // LAYOUTTESTWINDOW_H
