#include "ResultListWidget.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QStandardItem>
#include <QDebug>

ResultListWidget::ResultListWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupConnections();
    updateButtonStates();
}

void ResultListWidget::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(5);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    
    setupTable();
    setupButtons();
    setupContextMenu();
    
    // 状态标签
    m_statusLabel = new QLabel("计算结果: 0 条记录");
    m_statusLabel->setStyleSheet("QLabel { color: gray; font-size: 12px; }");
    m_statusLabel->setMaximumHeight(20); // 限制状态标签高度
    m_mainLayout->addWidget(m_statusLabel, 0); // 状态标签不拉伸
    
    setLayout(m_mainLayout);
}

void ResultListWidget::setupTable() {
    // 创建模型
    m_model = new QStandardItemModel(0, COL_COUNT, this);
    
    // 设置表头
    QStringList headers;
    headers << "序号" << "算法" << "起点" << "终点" << "路径" << "步数" << "耗时" << "时间";
    m_model->setHorizontalHeaderLabels(headers);
    
    // 创建表格视图
    m_tableView = new QTableView(this);
    m_tableView->setModel(m_model);
    
    // 设置表格属性
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSortingEnabled(true);
    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableView->setGridStyle(Qt::SolidLine);
    m_tableView->setShowGrid(true);
    
    // 设置表格大小政策，确保可以充满空间
    m_tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tableView->setMinimumHeight(200); // 设置最小高度
    
    // 确保表格可以滚动显示所有行
    m_tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    m_tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    
    // 设置列宽度
    QHeaderView* header = m_tableView->horizontalHeader();
    header->setStretchLastSection(false);
    header->resizeSection(COL_INDEX, 50);        // 序号
    header->resizeSection(COL_ALGORITHM, 80);    // 算法
    header->resizeSection(COL_START_POINT, 70);  // 起点
    header->resizeSection(COL_END_POINT, 70);    // 终点
    header->resizeSection(COL_PATH, 200);        // 路径（箭头形式）- 需要更多空间
    header->resizeSection(COL_PATH_LENGTH, 60);  // 步数
    header->resizeSection(COL_CALC_TIME, 80);    // 耗时
    header->resizeSection(COL_TIMESTAMP, 70);    // 时间
    
    // 设置行高
    m_tableView->verticalHeader()->setDefaultSectionSize(25);
    m_tableView->verticalHeader()->hide(); // 隐藏行号
    
    // 让表格占用更多垂直空间
    m_mainLayout->addWidget(m_tableView, 1); // 添加拉伸因子1
}

void ResultListWidget::setupButtons() {
    m_buttonLayout = new QHBoxLayout();
    
    m_deleteButton = new QPushButton("删除选中");
    m_deleteButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    
    m_exportButton = new QPushButton("导出结果");
    m_exportButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    
    m_clearButton = new QPushButton("清空全部");
    m_clearButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
    
    m_buttonLayout->addWidget(m_deleteButton);
    m_buttonLayout->addWidget(m_exportButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_clearButton);
    
    // 设置按钮布局为紧凑模式
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonLayout->setSpacing(5);
    
    m_mainLayout->addLayout(m_buttonLayout, 0); // 按钮布局不拉伸
}

void ResultListWidget::setupContextMenu() {
    m_contextMenu = new QMenu(this);
    
    m_showPathAction = new QAction("显示路径", this);
    m_showPathAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    
    m_deleteAction = new QAction("删除", this);
    m_deleteAction->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    
    m_exportAction = new QAction("导出", this);
    m_exportAction->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    
    m_contextMenu->addAction(m_showPathAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_deleteAction);
    m_contextMenu->addAction(m_exportAction);
}

void ResultListWidget::setupConnections() {
    // 表格事件
    connect(m_tableView, &QTableView::doubleClicked,
            this, [this](const QModelIndex& index) {
                onItemDoubleClicked(index.row(), index.column());
            });
    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ResultListWidget::onSelectionChanged);
    connect(m_tableView, &QTableView::customContextMenuRequested,
            this, &ResultListWidget::onContextMenuRequested);
    
    // 按钮事件
    connect(m_deleteButton, &QPushButton::clicked,
            this, &ResultListWidget::onDeleteSelected);
    connect(m_exportButton, &QPushButton::clicked,
            this, &ResultListWidget::onExportResults);
    connect(m_clearButton, &QPushButton::clicked,
            this, &ResultListWidget::onClearAll);
    
    // 右键菜单事件
    connect(m_showPathAction, &QAction::triggered,
            this, [this]() {
                if (hasSelection()) {
                    emit resultDoubleClicked(getSelectedResult());
                }
            });
    connect(m_deleteAction, &QAction::triggered,
            this, &ResultListWidget::onDeleteSelected);
    connect(m_exportAction, &QAction::triggered,
            this, &ResultListWidget::onExportResults);
}

void ResultListWidget::addResult(const PathResult& result) {
    m_results.append(result);
    
    int row = m_model->rowCount();
    m_model->insertRow(row);
    updateResultRow(row, result);
    
    // 调试输出 - 添加详细信息
    qDebug() << "添加结果到第" << row << "行，算法:" << result.algorithmString() 
             << "，路径长度:" << result.pathLength() << "，总行数:" << m_model->rowCount();
    
    // 更新状态标签
    m_statusLabel->setText(QString("计算结果: %1 条记录").arg(m_results.size()));
    updateButtonStates();
    
    // 滚动到新添加的行
    m_tableView->scrollToBottom();
}

void ResultListWidget::clearResults() {
    m_results.clear();
    m_model->setRowCount(0);
    m_statusLabel->setText("计算结果: 0 条记录");
    updateButtonStates();
}

void ResultListWidget::removeSelectedResults() {
    QList<int> selectedRows;
    for (const QModelIndex& index : m_tableView->selectionModel()->selectedRows()) {
        selectedRows.append(index.row());
    }
    
    if (selectedRows.isEmpty()) return;
    
    // 从后往前删除，避免索引变化问题
    std::sort(selectedRows.begin(), selectedRows.end(), std::greater<int>());
    
    for (int row : selectedRows) {
        if (row >= 0 && row < m_results.size()) {
            m_results.removeAt(row);
            m_model->removeRow(row);
        }
    }
    
    // 重新更新行号
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QStandardItem* item = m_model->item(i, COL_INDEX);
        if (item) {
            item->setText(QString::number(i + 1));
        }
    }
    
    m_statusLabel->setText(QString("计算结果: %1 条记录").arg(m_results.size()));
    updateButtonStates();
}

QVector<PathResult> ResultListWidget::getAllResults() const {
    return m_results;
}

PathResult ResultListWidget::getSelectedResult() const {
    QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedRows();
    if (!selectedIndexes.isEmpty()) {
        int currentRow = selectedIndexes.first().row();
        if (currentRow >= 0 && currentRow < m_results.size()) {
            return m_results[currentRow];
        }
    }
    return PathResult();
}

bool ResultListWidget::hasSelection() const {
    return !m_tableView->selectionModel()->selectedRows().isEmpty();
}

void ResultListWidget::setResults(const QVector<PathResult>& results) {
    clearResults();
    m_results = results;
    
    m_model->setRowCount(results.size());
    for (int i = 0; i < results.size(); ++i) {
        updateResultRow(i, results[i]);
    }
    
    m_statusLabel->setText(QString("计算结果: %1 条记录").arg(results.size()));
    updateButtonStates();
}

void ResultListWidget::updateResultRow(int row, const PathResult& result) {
    // 序号
    QStandardItem* indexItem = new QStandardItem(QString::number(row + 1));
    indexItem->setFlags(indexItem->flags() & ~Qt::ItemIsEditable);
    indexItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, COL_INDEX, indexItem);
    
    // 算法名称（更突出显示）
    QStandardItem* algoItem = new QStandardItem(result.algorithmString());
    algoItem->setFlags(algoItem->flags() & ~Qt::ItemIsEditable);
    algoItem->setTextAlignment(Qt::AlignCenter);
    
    // 根据算法类型设置不同的背景色
    switch (result.algorithm()) {
        case AlgorithmType::AStar:
            algoItem->setBackground(QColor(255, 230, 230)); // 浅红色
            break;
        case AlgorithmType::Dijkstra:
            algoItem->setBackground(QColor(230, 255, 230)); // 浅绿色
            break;
        case AlgorithmType::BFS:
            algoItem->setBackground(QColor(230, 230, 255)); // 浅蓝色
            break;
        case AlgorithmType::DFS:
            algoItem->setBackground(QColor(255, 255, 230)); // 浅黄色
            break;
    }
    m_model->setItem(row, COL_ALGORITHM, algoItem);
    
    // 起点
    QString startText = QString("(%1,%2)").arg(result.startPoint().x()).arg(result.startPoint().y());
    QStandardItem* startItem = new QStandardItem(startText);
    startItem->setFlags(startItem->flags() & ~Qt::ItemIsEditable);
    startItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, COL_START_POINT, startItem);
    
    // 终点
    QString endText = QString("(%1,%2)").arg(result.endPoint().x()).arg(result.endPoint().y());
    QStandardItem* endItem = new QStandardItem(endText);
    endItem->setFlags(endItem->flags() & ~Qt::ItemIsEditable);
    endItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, COL_END_POINT, endItem);
    
    // 路径（箭头形式）
    QString pathText = formatPathAsArrows(result.path());
    QStandardItem* pathItem = new QStandardItem(pathText);
    pathItem->setFlags(pathItem->flags() & ~Qt::ItemIsEditable);
    pathItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter); // 左对齐，因为路径可能很长
    pathItem->setToolTip(pathText); // 添加工具提示以显示完整路径
    m_model->setItem(row, COL_PATH, pathItem);
    
    // 路径长度（步数）
    QString lengthText = QString("%1 步").arg(result.pathLength());
    QStandardItem* lengthItem = new QStandardItem(lengthText);
    lengthItem->setFlags(lengthItem->flags() & ~Qt::ItemIsEditable);
    lengthItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, COL_PATH_LENGTH, lengthItem);
    
    // 计算时间（毫秒）
    QString timeText = QString("%1 ms").arg(result.calculationTime());
    QStandardItem* timeItem = new QStandardItem(timeText);
    timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
    timeItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, COL_CALC_TIME, timeItem);
    
    // 时间戳（简化显示）
    QString timestampText = result.timestamp().toString("hh:mm:ss");
    QStandardItem* timestampItem = new QStandardItem(timestampText);
    timestampItem->setFlags(timestampItem->flags() & ~Qt::ItemIsEditable);
    timestampItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, COL_TIMESTAMP, timestampItem);
}

void ResultListWidget::updateButtonStates() {
    bool hasResults = !m_results.isEmpty();
    bool hasSelection = this->hasSelection();
    
    m_deleteButton->setEnabled(hasSelection);
    m_exportButton->setEnabled(hasResults);
    m_clearButton->setEnabled(hasResults);
    
    if (m_showPathAction) m_showPathAction->setEnabled(hasSelection);
    if (m_deleteAction) m_deleteAction->setEnabled(hasSelection);
    if (m_exportAction) m_exportAction->setEnabled(hasResults);
}

// 槽函数实现
void ResultListWidget::onItemDoubleClicked(int row, int column) {
    Q_UNUSED(column)
    if (row >= 0 && row < m_results.size()) {
        emit resultDoubleClicked(m_results[row]);
    }
}

void ResultListWidget::onSelectionChanged() {
    updateButtonStates();
    if (hasSelection()) {
        emit resultSelectionChanged(getSelectedResult());
    }
}

void ResultListWidget::onContextMenuRequested(const QPoint& pos) {
    QModelIndex index = m_tableView->indexAt(pos);
    if (index.isValid()) {
        m_contextMenu->exec(m_tableView->mapToGlobal(pos));
    }
}

void ResultListWidget::onDeleteSelected() {
    if (!hasSelection()) return;
    
    int ret = QMessageBox::question(this, "确认删除",
                                   "确定要删除选中的结果吗？",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        removeSelectedResults();
        emit deleteSelectedResults();
    }
}

void ResultListWidget::onExportResults() {
    emit exportResults();
}

void ResultListWidget::onClearAll() {
    if (m_results.isEmpty()) return;
    
    int ret = QMessageBox::question(this, "确认清空",
                                   "确定要清空所有结果吗？",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        clearResults();
    }
}

QString ResultListWidget::formatPathAsArrows(const QVector<QPoint>& path) const {
    if (path.size() < 2) {
        return "无路径";
    }
    
    QString result;
    for (int i = 0; i < path.size() - 1; ++i) {
        QPoint current = path[i];
        QPoint next = path[i + 1];
        
        // 添加当前点坐标
        result += QString("(%1,%2)").arg(current.x()).arg(current.y());
        
        // 确定箭头方向
        QString arrow;
        if (next.x() > current.x()) {
            arrow = "→";  // 向右
        } else if (next.x() < current.x()) {
            arrow = "←";  // 向左
        } else if (next.y() > current.y()) {
            arrow = "↓";  // 向下
        } else if (next.y() < current.y()) {
            arrow = "↑";  // 向上
        } else {
            arrow = "●";  // 相同位置（不应该发生）
        }
        
        result += arrow;
    }
    
    // 添加最后一个点
    QPoint lastPoint = path.last();
    result += QString("(%1,%2)").arg(lastPoint.x()).arg(lastPoint.y());
    
    return result;
}
