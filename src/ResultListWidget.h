#ifndef RESULTLISTWIDGET_H
#define RESULTLISTWIDGET_H

#include "Common.h"
#include "PathResult.h"
#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QAction>

class ResultListWidget : public QWidget {
    Q_OBJECT

public:
    explicit ResultListWidget(QWidget* parent = nullptr);
    
    // 结果管理
    void addResult(const PathResult& result);
    void clearResults();
    void removeSelectedResults();
    void recreateTable();  // 新增：完全重新创建表格
    
    // 获取结果
    QVector<PathResult> getAllResults() const;
    PathResult getSelectedResult() const;
    bool hasSelection() const;
    
    // 设置结果列表
    void setResults(const QVector<PathResult>& results);
    
    // 批量添加结果
    void addBatchResults(const QVector<PathResult>& results);
    
    // 调整列宽自适应
    void resizeColumnsToContents();

protected:
    // 重写窗口大小变化事件
    void resizeEvent(QResizeEvent* event) override;

signals:
    void resultDoubleClicked(const PathResult& result);
    void resultSelectionChanged(const PathResult& result);
    void deleteSelectedResults();
    void exportResults();

private slots:
    void onItemDoubleClicked(int row, int column);
    void onSelectionChanged();
    void onContextMenuRequested(const QPoint& pos);
    void onDeleteSelected();
    void onExportResults();
    void onClearAll();

private:
    void setupUI();
    void setupTable();
    void setupButtons();
    void setupContextMenu();
    void setupConnections();
    void updateResultRow(int row, const PathResult& result);
    void updateButtonStates();
    QString formatPathAsArrows(const QVector<QPoint>& path) const;
    
    QVBoxLayout* m_mainLayout;
    QTableView* m_tableView;
    QStandardItemModel* m_model;
    
    // 按钮
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_deleteButton;
    QPushButton* m_exportButton;
    QPushButton* m_clearButton;
    
    // 状态标签
    QLabel* m_statusLabel;
    
    // 右键菜单
    QMenu* m_contextMenu;
    QAction* m_showPathAction;
    QAction* m_deleteAction;
    QAction* m_exportAction;
    
    // 数据存储
    QVector<PathResult> m_results;
    
    // 表格列定义
    enum TableColumns {
        COL_INDEX = 0,      // 序号
        COL_ALGORITHM,      // 算法
        COL_START_POINT,    // 起点
        COL_END_POINT,      // 终点
        COL_PATH_LENGTH,    // 步数
        COL_CALC_TIME,      // 耗时
        COL_TIMESTAMP,      // 时间
        COL_COUNT
    };
};

#endif // RESULTLISTWIDGET_H
