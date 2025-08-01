#include "LayoutTestWindow.h"
#include <QApplication>
#include <QDebug>

LayoutTestWindow::LayoutTestWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_tabWidget(nullptr)
    , m_stretchDemoLayout(nullptr)
    , m_middleButton(nullptr)
{
    setupUI();
    setWindowTitle("Qt布局管理最佳实践演示");
    setMinimumSize(1000, 700);
    resize(1200, 800);
}

LayoutTestWindow::~LayoutTestWindow()
{
}

void LayoutTestWindow::setupUI()
{
    // 创建中央部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(m_centralWidget);
    
    // 创建标题
    QLabel* titleLabel = new QLabel("Qt布局管理最佳实践演示", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; padding: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // 创建选项卡
    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);
    
    // 创建各个演示页面
    createBasicLayoutDemo();
    createGridLayoutDemo();
    createSplitterDemo();
    createSpacerDemo();
    createSizePolicyDemo();
    createStretchFactorDemo();
    createRealWorldExample();
}

void LayoutTestWindow::createBasicLayoutDemo()
{
    m_basicLayoutPage = new QWidget();
    
    QVBoxLayout* pageLayout = new QVBoxLayout(m_basicLayoutPage);
    
    // 页面标题
    QLabel* pageTitle = new QLabel("基础布局演示 - QHBoxLayout & QVBoxLayout");
    pageTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #34495e; padding: 5px;");
    pageLayout->addWidget(pageTitle);
    
    // 水平布局演示
    QGroupBox* hboxGroup = new QGroupBox("QHBoxLayout 水平布局演示");
    QHBoxLayout* hboxLayout = new QHBoxLayout(hboxGroup);
    
    hboxLayout->addWidget(createBasicWidget("按钮1", QColor(231, 76, 60)));
    hboxLayout->addWidget(createBasicWidget("按钮2", QColor(46, 204, 113)));
    hboxLayout->addWidget(createBasicWidget("按钮3", QColor(52, 152, 219)));
    hboxLayout->addWidget(createBasicWidget("按钮4", QColor(155, 89, 182)));
    
    pageLayout->addWidget(hboxGroup);
    
    // 垂直布局演示
    QGroupBox* vboxGroup = new QGroupBox("QVBoxLayout 垂直布局演示");
    QVBoxLayout* vboxLayout = new QVBoxLayout(vboxGroup);
    
    vboxLayout->addWidget(createBasicWidget("行1", QColor(241, 196, 15)));
    vboxLayout->addWidget(createBasicWidget("行2", QColor(230, 126, 34)));
    vboxLayout->addWidget(createBasicWidget("行3", QColor(149, 165, 166)));
    
    pageLayout->addWidget(vboxGroup);
    
    // 嵌套布局演示
    QGroupBox* nestedGroup = new QGroupBox("嵌套布局演示");
    QHBoxLayout* nestedMainLayout = new QHBoxLayout(nestedGroup);
    
    // 左侧垂直布局
    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->addWidget(createBasicWidget("左上", QColor(192, 57, 43)));
    leftLayout->addWidget(createBasicWidget("左下", QColor(142, 68, 173)));
    
    // 右侧垂直布局
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->addWidget(createBasicWidget("右上", QColor(39, 174, 96)));
    rightLayout->addWidget(createBasicWidget("右下", QColor(41, 128, 185)));
    
    nestedMainLayout->addLayout(leftLayout);
    nestedMainLayout->addLayout(rightLayout);
    
    pageLayout->addWidget(nestedGroup);
    
    // 添加弹性空间
    pageLayout->addStretch();
    
    m_tabWidget->addTab(m_basicLayoutPage, "基础布局");
}

void LayoutTestWindow::createGridLayoutDemo()
{
    m_gridLayoutPage = new QWidget();
    
    QVBoxLayout* pageLayout = new QVBoxLayout(m_gridLayoutPage);
    
    // 页面标题
    QLabel* pageTitle = new QLabel("栅格布局演示 - QGridLayout");
    pageTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #34495e; padding: 5px;");
    pageLayout->addWidget(pageTitle);
    
    // 基础栅格布局
    QGroupBox* basicGridGroup = new QGroupBox("基础栅格布局 (3x3)");
    QGridLayout* basicGridLayout = new QGridLayout(basicGridGroup);
    
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            QString text = QString("(%1,%2)").arg(row).arg(col);
            QColor color = QColor::fromHsv((row * 3 + col) * 40, 200, 200);
            basicGridLayout->addWidget(createBasicWidget(text, color), row, col);
        }
    }
    
    pageLayout->addWidget(basicGridGroup);
    
    // 跨行跨列演示
    QGroupBox* spanGridGroup = new QGroupBox("跨行跨列演示");
    QGridLayout* spanGridLayout = new QGridLayout(spanGridGroup);
    
    // 添加跨行跨列的部件
    spanGridLayout->addWidget(createBasicWidget("标题 (跨2列)", QColor(231, 76, 60)), 0, 0, 1, 2);
    spanGridLayout->addWidget(createBasicWidget("侧边栏\n(跨2行)", QColor(46, 204, 113)), 1, 0, 2, 1);
    spanGridLayout->addWidget(createBasicWidget("内容区域", QColor(52, 152, 219)), 1, 1);
    spanGridLayout->addWidget(createBasicWidget("状态栏", QColor(155, 89, 182)), 2, 1);
    
    pageLayout->addWidget(spanGridGroup);
    
    // 比例控制演示
    QGroupBox* ratioGridGroup = new QGroupBox("比例控制演示 - 列宽比例设置");
    QGridLayout* ratioGridLayout = new QGridLayout(ratioGridGroup);
    
    ratioGridLayout->addWidget(createBasicWidget("列1\n(比例1)", QColor(241, 196, 15)), 0, 0);
    ratioGridLayout->addWidget(createBasicWidget("列2\n(比例2)", QColor(230, 126, 34)), 0, 1);
    ratioGridLayout->addWidget(createBasicWidget("列3\n(比例3)", QColor(149, 165, 166)), 0, 2);
    
    // 设置列的伸缩比例
    ratioGridLayout->setColumnStretch(0, 1);  // 第0列占1份
    ratioGridLayout->setColumnStretch(1, 2);  // 第1列占2份
    ratioGridLayout->setColumnStretch(2, 3);  // 第2列占3份
    
    pageLayout->addWidget(ratioGridGroup);
    
    pageLayout->addStretch();
    
    m_tabWidget->addTab(m_gridLayoutPage, "栅格布局");
}

void LayoutTestWindow::createSplitterDemo()
{
    m_splitterPage = new QWidget();
    
    QVBoxLayout* pageLayout = new QVBoxLayout(m_splitterPage);
    
    // 页面标题
    QLabel* pageTitle = new QLabel("分割器演示 - QSplitter");
    pageTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #34495e; padding: 5px;");
    pageLayout->addWidget(pageTitle);
    
    // 水平分割器
    QGroupBox* hSplitterGroup = new QGroupBox("水平分割器");
    QVBoxLayout* hSplitterLayout = new QVBoxLayout(hSplitterGroup);
    
    QSplitter* hSplitter = new QSplitter(Qt::Horizontal);
    hSplitter->addWidget(createBasicWidget("左侧面板\n(可调整大小)", QColor(231, 76, 60)));
    hSplitter->addWidget(createBasicWidget("中间面板\n(可调整大小)", QColor(46, 204, 113)));
    hSplitter->addWidget(createBasicWidget("右侧面板\n(可调整大小)", QColor(52, 152, 219)));
    
    // 设置初始比例
    hSplitter->setSizes(QList<int>() << 200 << 400 << 200);
    hSplitter->setStretchFactor(0, 0);  // 左侧不伸缩
    hSplitter->setStretchFactor(1, 1);  // 中间可伸缩
    hSplitter->setStretchFactor(2, 0);  // 右侧不伸缩
    
    hSplitterLayout->addWidget(hSplitter);
    pageLayout->addWidget(hSplitterGroup);
    
    // 垂直分割器
    QGroupBox* vSplitterGroup = new QGroupBox("垂直分割器");
    QVBoxLayout* vSplitterLayout = new QVBoxLayout(vSplitterGroup);
    
    QSplitter* vSplitter = new QSplitter(Qt::Vertical);
    vSplitter->addWidget(createBasicWidget("顶部面板", QColor(155, 89, 182)));
    vSplitter->addWidget(createBasicWidget("中间面板\n(主要内容区域)", QColor(241, 196, 15)));
    vSplitter->addWidget(createBasicWidget("底部面板", QColor(230, 126, 34)));
    
    // 设置初始比例
    vSplitter->setSizes(QList<int>() << 100 << 300 << 100);
    
    vSplitterLayout->addWidget(vSplitter);
    pageLayout->addWidget(vSplitterGroup);
    
    // 嵌套分割器
    QGroupBox* nestedSplitterGroup = new QGroupBox("嵌套分割器演示");
    QVBoxLayout* nestedSplitterLayout = new QVBoxLayout(nestedSplitterGroup);
    
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);
    
    // 左侧固定面板
    mainSplitter->addWidget(createBasicWidget("工具栏\n(固定宽度)", QColor(192, 57, 43)));
    
    // 右侧嵌套垂直分割器
    QSplitter* rightSplitter = new QSplitter(Qt::Vertical);
    rightSplitter->addWidget(createBasicWidget("编辑区域", QColor(39, 174, 96)));
    rightSplitter->addWidget(createBasicWidget("输出区域", QColor(41, 128, 185)));
    
    mainSplitter->addWidget(rightSplitter);
    
    // 设置比例
    mainSplitter->setSizes(QList<int>() << 150 << 600);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);
    
    nestedSplitterLayout->addWidget(mainSplitter);
    pageLayout->addWidget(nestedSplitterGroup);
    
    m_tabWidget->addTab(m_splitterPage, "分割器");
}

void LayoutTestWindow::createSpacerDemo()
{
    m_spacerPage = new QWidget();
    
    QVBoxLayout* pageLayout = new QVBoxLayout(m_spacerPage);
    
    // 页面标题
    QLabel* pageTitle = new QLabel("弹簧演示 - QSpacerItem 精确布局控制");
    pageTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #34495e; padding: 5px;");
    pageLayout->addWidget(pageTitle);
    
    // 固定弹簧演示
    QGroupBox* fixedSpacerGroup = new QGroupBox("固定弹簧演示 - 使用弹簧控制间距");
    QHBoxLayout* fixedSpacerLayout = new QHBoxLayout(fixedSpacerGroup);
    
    fixedSpacerLayout->addWidget(createBasicWidget("左", QColor(231, 76, 60)));
    fixedSpacerLayout->addItem(new QSpacerItem(50, 20, QSizePolicy::Fixed, QSizePolicy::Minimum));
    fixedSpacerLayout->addWidget(createBasicWidget("中", QColor(46, 204, 113)));
    fixedSpacerLayout->addItem(new QSpacerItem(100, 20, QSizePolicy::Fixed, QSizePolicy::Minimum));
    fixedSpacerLayout->addWidget(createBasicWidget("右", QColor(52, 152, 219)));
    
    pageLayout->addWidget(fixedSpacerGroup);
    
    // 可伸缩弹簧演示
    QGroupBox* expandingSpacerGroup = new QGroupBox("可伸缩弹簧演示 - Expanding策略");
    QHBoxLayout* expandingSpacerLayout = new QHBoxLayout(expandingSpacerGroup);
    
    expandingSpacerLayout->addWidget(createBasicWidget("始终左对齐", QColor(155, 89, 182)));
    expandingSpacerLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    expandingSpacerLayout->addWidget(createBasicWidget("始终右对齐", QColor(241, 196, 15)));
    
    pageLayout->addWidget(expandingSpacerGroup);
    
    // 最小大小控制演示
    QGroupBox* minSizeGroup = new QGroupBox("最小大小控制演示 - 精确布局控制");
    QVBoxLayout* minSizeLayout = new QVBoxLayout(minSizeGroup);
    
    QHBoxLayout* controlRow = new QHBoxLayout();
    
    // 创建具有最小大小的控件
    QPushButton* fixedBtn = new QPushButton("固定大小按钮");
    fixedBtn->setFixedSize(150, 40);
    fixedBtn->setStyleSheet("background-color: #e74c3c; color: white; font-weight: bold;");
    
    QPushButton* minBtn = new QPushButton("最小大小按钮");
    minBtn->setMinimumSize(100, 30);
    minBtn->setStyleSheet("background-color: #2ecc71; color: white; font-weight: bold;");
    
    QPushButton* expandBtn = new QPushButton("可扩展按钮");
    expandBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    expandBtn->setStyleSheet("background-color: #3498db; color: white; font-weight: bold;");
    
    controlRow->addWidget(fixedBtn);
    controlRow->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum));
    controlRow->addWidget(minBtn);
    controlRow->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    controlRow->addWidget(expandBtn);
    
    minSizeLayout->addLayout(controlRow);
    
    // 说明文字
    QLabel* explanation = new QLabel(
        "• 红色按钮：固定大小 (150x40)\n"
        "• 绿色按钮：最小大小 (100x30)，可以更大但不会更小\n"
        "• 蓝色按钮：可扩展，会填充剩余空间"
    );
    explanation->setStyleSheet("color: #34495e; padding: 10px; background-color: #ecf0f1; border-radius: 5px;");
    minSizeLayout->addWidget(explanation);
    
    pageLayout->addWidget(minSizeGroup);
    
    pageLayout->addStretch();
    
    m_tabWidget->addTab(m_spacerPage, "弹簧控制");
}

void LayoutTestWindow::createSizePolicyDemo()
{
    m_sizePolicyPage = new QWidget();
    
    QVBoxLayout* pageLayout = new QVBoxLayout(m_sizePolicyPage);
    
    // 页面标题
    QLabel* pageTitle = new QLabel("大小策略演示 - QSizePolicy");
    pageTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #34495e; padding: 5px;");
    pageLayout->addWidget(pageTitle);
    
    // 各种大小策略演示
    QGroupBox* policyGroup = new QGroupBox("不同大小策略对比");
    QVBoxLayout* policyLayout = new QVBoxLayout(policyGroup);
    
    // Fixed策略
    QHBoxLayout* fixedRow = new QHBoxLayout();
    QLabel* fixedLabel = new QLabel("Fixed:");
    fixedLabel->setFixedWidth(80);
    QPushButton* fixedBtn = new QPushButton("Fixed大小策略");
    fixedBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    fixedBtn->setStyleSheet("background-color: #e74c3c; color: white;");
    fixedRow->addWidget(fixedLabel);
    fixedRow->addWidget(fixedBtn);
    fixedRow->addStretch();
    policyLayout->addLayout(fixedRow);
    
    // Minimum策略
    QHBoxLayout* minRow = new QHBoxLayout();
    QLabel* minLabel = new QLabel("Minimum:");
    minLabel->setFixedWidth(80);
    QPushButton* minBtn = new QPushButton("Minimum大小策略");
    minBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    minBtn->setStyleSheet("background-color: #f39c12; color: white;");
    minRow->addWidget(minLabel);
    minRow->addWidget(minBtn);
    minRow->addStretch();
    policyLayout->addLayout(minRow);
    
    // Maximum策略
    QHBoxLayout* maxRow = new QHBoxLayout();
    QLabel* maxLabel = new QLabel("Maximum:");
    maxLabel->setFixedWidth(80);
    QPushButton* maxBtn = new QPushButton("Maximum大小策略");
    maxBtn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    maxBtn->setStyleSheet("background-color: #9b59b6; color: white;");
    maxRow->addWidget(maxLabel);
    maxRow->addWidget(maxBtn);
    maxRow->addStretch();
    policyLayout->addLayout(maxRow);
    
    // Preferred策略
    QHBoxLayout* prefRow = new QHBoxLayout();
    QLabel* prefLabel = new QLabel("Preferred:");
    prefLabel->setFixedWidth(80);
    QPushButton* prefBtn = new QPushButton("Preferred大小策略");
    prefBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    prefBtn->setStyleSheet("background-color: #2ecc71; color: white;");
    prefRow->addWidget(prefLabel);
    prefRow->addWidget(prefBtn);
    prefRow->addStretch();
    policyLayout->addLayout(prefRow);
    
    // Expanding策略
    QHBoxLayout* expRow = new QHBoxLayout();
    QLabel* expLabel = new QLabel("Expanding:");
    expLabel->setFixedWidth(80);
    QPushButton* expBtn = new QPushButton("Expanding大小策略");
    expBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    expBtn->setStyleSheet("background-color: #3498db; color: white;");
    expRow->addWidget(expLabel);
    expRow->addWidget(expBtn);
    policyLayout->addLayout(expRow);
    
    // MinimumExpanding策略
    QHBoxLayout* minExpRow = new QHBoxLayout();
    QLabel* minExpLabel = new QLabel("MinExp:");
    minExpLabel->setFixedWidth(80);
    QPushButton* minExpBtn = new QPushButton("MinimumExpanding大小策略");
    minExpBtn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    minExpBtn->setStyleSheet("background-color: #1abc9c; color: white;");
    minExpRow->addWidget(minExpLabel);
    minExpRow->addWidget(minExpBtn);
    policyLayout->addLayout(minExpRow);
    
    pageLayout->addWidget(policyGroup);
    
    // 策略说明
    QGroupBox* explanationGroup = new QGroupBox("大小策略说明");
    QVBoxLayout* explanationLayout = new QVBoxLayout(explanationGroup);
    
    QLabel* explanationText = new QLabel(
        "• Fixed: 固定大小，不会改变\n"
        "• Minimum: 可以比建议大小更大，但不会更小\n"
        "• Maximum: 可以比建议大小更小，但不会更大\n"
        "• Preferred: 建议大小，可大可小\n"
        "• Expanding: 会扩展以填充可用空间\n"
        "• MinimumExpanding: 至少是最小大小，并且会扩展填充空间"
    );
    explanationText->setStyleSheet("color: #2c3e50; padding: 10px; background-color: #ecf0f1; border-radius: 5px;");
    explanationLayout->addWidget(explanationText);
    
    pageLayout->addWidget(explanationGroup);
    
    pageLayout->addStretch();
    
    m_tabWidget->addTab(m_sizePolicyPage, "大小策略");
}

void LayoutTestWindow::createStretchFactorDemo()
{
    m_stretchFactorPage = new QWidget();
    
    QVBoxLayout* pageLayout = new QVBoxLayout(m_stretchFactorPage);
    
    // 页面标题
    QLabel* pageTitle = new QLabel("伸缩因子演示 - Stretch Factor");
    pageTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #34495e; padding: 5px;");
    pageLayout->addWidget(pageTitle);
    
    // 伸缩因子控制
    QGroupBox* controlGroup = new QGroupBox("伸缩因子控制");
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);
    
    QLabel* controlLabel = new QLabel("调整中间部件的伸缩因子 (0-10):");
    controlLayout->addWidget(controlLabel);
    
    QHBoxLayout* sliderLayout = new QHBoxLayout();
    m_stretchSlider = new QSlider(Qt::Horizontal);
    m_stretchSlider->setRange(0, 10);
    m_stretchSlider->setValue(1);
    
    m_stretchSpin = new QSpinBox();
    m_stretchSpin->setRange(0, 10);
    m_stretchSpin->setValue(1);
    
    sliderLayout->addWidget(new QLabel("伸缩因子:"));
    sliderLayout->addWidget(m_stretchSlider);
    sliderLayout->addWidget(m_stretchSpin);
    sliderLayout->addStretch();
    
    controlLayout->addLayout(sliderLayout);
    
    // 连接信号
    connect(m_stretchSlider, QOverload<int>::of(&QSlider::valueChanged), 
            m_stretchSpin, &QSpinBox::setValue);
    connect(m_stretchSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            m_stretchSlider, &QSlider::setValue);
    connect(m_stretchSlider, &QSlider::valueChanged, this, &LayoutTestWindow::onStretchFactorDemo);
    
    pageLayout->addWidget(controlGroup);
    
    // 演示区域
    QGroupBox* demoGroup = new QGroupBox("伸缩因子演示区域");
    m_stretchDemoLayout = new QHBoxLayout(demoGroup);
    
    // 创建三个部件，中间的可以调整伸缩因子
    QPushButton* leftBtn = new QPushButton("左侧\n(伸缩因子=1)");
    leftBtn->setStyleSheet("background-color: #e74c3c; color: white; font-weight: bold;");
    leftBtn->setObjectName("leftBtn");
    
    m_middleButton = new QPushButton("中间\n(可调整伸缩因子)");
    m_middleButton->setStyleSheet("background-color: #2ecc71; color: white; font-weight: bold;");
    m_middleButton->setObjectName("middleBtn");
    
    QPushButton* rightBtn = new QPushButton("右侧\n(伸缩因子=1)");
    rightBtn->setStyleSheet("background-color: #3498db; color: white; font-weight: bold;");
    rightBtn->setObjectName("rightBtn");
    
    m_stretchDemoLayout->addWidget(leftBtn, 1);        // 伸缩因子 1
    m_stretchDemoLayout->addWidget(m_middleButton, 1); // 伸缩因子 1 (会被动态调整)
    m_stretchDemoLayout->addWidget(rightBtn, 1);       // 伸缩因子 1
    
    pageLayout->addWidget(demoGroup);
    
    // 说明文字
    QLabel* explanation = new QLabel(
        "伸缩因子控制部件在布局中占用空间的比例：\n"
        "• 当窗口大小改变时，空间将按照伸缩因子的比例分配给各个部件\n"
        "• 伸缩因子为0表示部件不会扩展\n"
        "• 伸缩因子越大，部件获得的额外空间越多"
    );
    explanation->setStyleSheet("color: #34495e; padding: 10px; background-color: #ecf0f1; border-radius: 5px;");
    pageLayout->addWidget(explanation);
    
    pageLayout->addStretch();
    
    m_tabWidget->addTab(m_stretchFactorPage, "伸缩因子");
}

void LayoutTestWindow::createRealWorldExample()
{
    m_realWorldPage = new QWidget();
    
    QVBoxLayout* pageLayout = new QVBoxLayout(m_realWorldPage);
    
    // 页面标题
    QLabel* pageTitle = new QLabel("实际应用示例 - 典型应用程序布局");
    pageTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #34495e; padding: 5px;");
    pageLayout->addWidget(pageTitle);
    
    // 模拟IDE布局
    QGroupBox* ideGroup = new QGroupBox("IDE风格布局示例");
    QVBoxLayout* ideLayout = new QVBoxLayout(ideGroup);
    
    // 工具栏
    QFrame* toolBar = new QFrame();
    toolBar->setFixedHeight(40);
    toolBar->setStyleSheet("background-color: #34495e; border: 1px solid #2c3e50;");
    QHBoxLayout* toolBarLayout = new QHBoxLayout(toolBar);
    
    QPushButton* newBtn = new QPushButton("新建");
    QPushButton* openBtn = new QPushButton("打开");
    QPushButton* saveBtn = new QPushButton("保存");
    
    newBtn->setStyleSheet("background-color: #3498db; color: white; border: none; padding: 5px 10px;");
    openBtn->setStyleSheet("background-color: #2ecc71; color: white; border: none; padding: 5px 10px;");
    saveBtn->setStyleSheet("background-color: #e74c3c; color: white; border: none; padding: 5px 10px;");
    
    toolBarLayout->addWidget(newBtn);
    toolBarLayout->addWidget(openBtn);
    toolBarLayout->addWidget(saveBtn);
    toolBarLayout->addStretch();
    
    ideLayout->addWidget(toolBar);
    
    // 主要内容区域 - 使用水平分割器
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);
    
    // 左侧项目浏览器
    QFrame* projectExplorer = new QFrame();
    projectExplorer->setMinimumWidth(200);
    projectExplorer->setStyleSheet("background-color: #ecf0f1; border: 1px solid #bdc3c7;");
    QVBoxLayout* explorerLayout = new QVBoxLayout(projectExplorer);
    
    QLabel* explorerTitle = new QLabel("项目浏览器");
    explorerTitle->setStyleSheet("font-weight: bold; padding: 5px; background-color: #95a5a6; color: white;");
    
    QListWidget* fileList = new QListWidget();
    fileList->addItem("main.cpp");
    fileList->addItem("window.h");
    fileList->addItem("window.cpp");
    fileList->addItem("resource.qrc");
    
    explorerLayout->addWidget(explorerTitle);
    explorerLayout->addWidget(fileList);
    
    mainSplitter->addWidget(projectExplorer);
    
    // 中间和右侧区域 - 使用垂直分割器
    QSplitter* rightSplitter = new QSplitter(Qt::Vertical);
    
    // 编辑区域
    QFrame* editorArea = new QFrame();
    editorArea->setStyleSheet("background-color: #ffffff; border: 1px solid #bdc3c7;");
    QVBoxLayout* editorLayout = new QVBoxLayout(editorArea);
    
    QLabel* editorTitle = new QLabel("代码编辑器");
    editorTitle->setStyleSheet("font-weight: bold; padding: 5px; background-color: #3498db; color: white;");
    
    QTextEdit* codeEditor = new QTextEdit();
    codeEditor->setText(
        "#include <QApplication>\\n"
        "#include <QMainWindow>\\n"
        "\\n"
        "int main(int argc, char *argv[])\\n"
        "{\\n"
        "    QApplication app(argc, argv);\\n"
        "    \\n"
        "    QMainWindow window;\\n"
        "    window.show();\\n"
        "    \\n"
        "    return app.exec();\\n"
        "}"
    );
    codeEditor->setStyleSheet("font-family: 'Courier New'; font-size: 12px;");
    
    editorLayout->addWidget(editorTitle);
    editorLayout->addWidget(codeEditor);
    
    rightSplitter->addWidget(editorArea);
    
    // 底部输出区域
    QFrame* outputArea = new QFrame();
    outputArea->setMinimumHeight(100);
    outputArea->setStyleSheet("background-color: #2c3e50; border: 1px solid #34495e;");
    QVBoxLayout* outputLayout = new QVBoxLayout(outputArea);
    
    QLabel* outputTitle = new QLabel("输出窗口");
    outputTitle->setStyleSheet("font-weight: bold; padding: 5px; background-color: #e74c3c; color: white;");
    
    QTextEdit* outputText = new QTextEdit();
    outputText->setText(
        "编译开始...\\n"
        "正在编译 main.cpp\\n"
        "正在编译 window.cpp\\n"
        "链接中...\\n"
        "编译成功完成。\\n"
    );
    outputText->setStyleSheet("background-color: #34495e; color: #ecf0f1; font-family: 'Courier New';");
    outputText->setMaximumHeight(80);
    
    outputLayout->addWidget(outputTitle);
    outputLayout->addWidget(outputText);
    
    rightSplitter->addWidget(outputArea);
    
    mainSplitter->addWidget(rightSplitter);
    
    // 设置分割器比例
    mainSplitter->setSizes(QList<int>() << 250 << 750);
    mainSplitter->setStretchFactor(0, 0);  // 项目浏览器不伸缩
    mainSplitter->setStretchFactor(1, 1);  // 编辑区域可伸缩
    
    rightSplitter->setSizes(QList<int>() << 400 << 150);
    rightSplitter->setStretchFactor(0, 1);  // 编辑器区域可伸缩
    rightSplitter->setStretchFactor(1, 0);  // 输出区域不伸缩
    
    ideLayout->addWidget(mainSplitter);
    
    // 状态栏
    QFrame* statusBar = new QFrame();
    statusBar->setFixedHeight(25);
    statusBar->setStyleSheet("background-color: #95a5a6; border: 1px solid #7f8c8d;");
    QHBoxLayout* statusLayout = new QHBoxLayout(statusBar);
    
    m_statusLabel = new QLabel("就绪");
    m_statusLabel->setStyleSheet("color: white; font-size: 11px;");
    
    QLabel* positionLabel = new QLabel("行: 12, 列: 5");
    positionLabel->setStyleSheet("color: white; font-size: 11px;");
    
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(positionLabel);
    
    ideLayout->addWidget(statusBar);
    
    pageLayout->addWidget(ideGroup);
    
    m_tabWidget->addTab(m_realWorldPage, "实际应用");
}

QWidget* LayoutTestWindow::createBasicWidget(const QString& text, const QColor& color)
{
    QPushButton* widget = new QPushButton(text);
    widget->setMinimumSize(80, 40);
    widget->setStyleSheet(QString(
        "background-color: %1; "
        "color: white; "
        "border: none; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "padding: 5px;"
    ).arg(color.name()));
    
    return widget;
}

void LayoutTestWindow::onSpacerDemo()
{
    // 这个方法可以用来演示动态调整弹簧
}

void LayoutTestWindow::onSizePolicyDemo()
{
    // 这个方法可以用来演示动态调整大小策略
}

void LayoutTestWindow::onStretchFactorDemo()
{
    // 动态调整伸缩因子
    if (!m_stretchDemoLayout || !m_middleButton) return;
    
    // 获取滑块值
    int stretchValue = m_stretchSlider->value();
    
    // 更新中间按钮的文本
    m_middleButton->setText(QString("中间\n(伸缩因子=%1)").arg(stretchValue));
    
    // 重新设置伸缩因子 - 这是关键！
    m_stretchDemoLayout->setStretchFactor(m_middleButton, stretchValue);
    
    // 更新状态
    if (m_statusLabel) {
        m_statusLabel->setText(QString("中间部件伸缩因子已更新为: %1").arg(stretchValue));
    }
    
    qDebug() << "伸缩因子已更新为:" << stretchValue;
}

void LayoutTestWindow::onGridLayoutDemo()
{
    // 这个方法可以用来演示动态调整栅格布局
}
