#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QStandardItemModel>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QTimer>
#include <QAction>
#include <QHeaderView>
#include "taskmanager.h"
#include "audioplayer.h"

/**
 * @brief MainWindow - 主窗口
 *
 * 功能：
 *  1. 任务表格（按开始时间排序，列对齐）
 *  2. 工具栏：添加 / 删除 / 编辑 / 刷新
 *  3. 状态栏：当前用户 + 下次提醒
 *  4. 后台定时器：每10秒检查任务提醒
 *  5. 提醒：弹出对话框
 *
 * 表格列：
 *  ID | 任务名称 | 开始时间 | 提醒时间 | 优先级 | 分类
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const QString& username, QWidget* parent = nullptr);
    ~MainWindow() {}

private slots:
    /** 添加新任务 */
    void onAddTask();

    /** 删除选中任务 */
    void onDeleteTask();

    /** 编辑选中任务 */
    void onEditTask();

    /** 刷新表格显示 */
    void refreshTable();

    /** 定时检查提醒 */
    void checkReminders();

private:
    /** 初始化界面布局 */
    void setupUI();

    /** 初始化工具栏 */
    void setupToolBar();

    /** 初始化表格 */
    void setupTable();

    /** 获取当前选中的任务 id（-1 表示未选中）*/
    int selectedTaskId() const;

    QTableView*          m_tableView;      // 任务表格
    QStandardItemModel*  m_model;          // 表格数据模型
    QLabel*              m_userLabel;      // 状态栏用户标签
    QLabel*              m_statusLabel;    // 状态标签
    QTimer*              m_remindTimer;    // 提醒定时器
    AudioPlayer*         m_audioPlayer;    // 音频播放器（Stage 2 新增）
    QString              m_username;       // 当前用户名

    // 工具栏按钮
    QAction* m_addAction;
    QAction* m_deleteAction;
    QAction* m_editAction;
    QAction* m_refreshAction;
};

#endif // MAINWINDOW_H
