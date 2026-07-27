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
#include <QCalendarWidget>
#include <QSplitter>
#include <QDate>
#include <QTextCharFormat>
#include "taskmanager.h"
#include "audioplayer.h"
#include "speechrecognizer.h"

/**
 * @brief MainWindow - 主窗口
 *
 * 功能：
 *  1. 左侧日历面板：有任务日期高亮，点击筛选
 *  2. 右侧任务表格：按开始时间排序，列对齐
 *  3. 工具栏：添加 / 删除 / 编辑 / 语音录入 / 选择音乐 / 显示全部
 *  4. 状态栏：当前用户 + 任务统计 + 筛选状态
 *  5. 后台定时器：每10秒检查任务提醒
 *  6. 提醒：弹出对话框 + 播放音频
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

    /** 语音录入任务 */
    void onVoiceInput();

    /** 日历日期被点击 → 筛选该日任务 */
    void onDateClicked(const QDate& date);

    /** 显示全部任务（取消日期筛选）*/
    void onShowAll();

    /** 选择提醒音乐 */
    void onSelectMusic();

private:
    /** 初始化整体布局（日历+表格）*/
    void setupUI();

    /** 初始化工具栏 */
    void setupToolBar();

    /** 初始化左侧日历面板 */
    void setupCalendar();

    /** 初始化表格 */
    void setupTable();

    /** 初始化状态栏 */
    void setupStatusBar();

    /** 获取当前选中的任务 id（-1 表示未选中）*/
    int selectedTaskId() const;

    /** 刷新日历上的任务标记 */
    void refreshCalendarMarks();

    // ── 左侧日历 ──
    QCalendarWidget*    m_calendar;
    QDate               m_filterDate;     // 筛选日期，null=全部
    bool                m_filterActive;
    QLabel*             m_filterLabel;    // 日历上方"选择日期"提示

    // ── 右侧表格 ──
    QTableView*         m_tableView;
    QStandardItemModel* m_model;

    // ── 状态栏 ──
    QLabel*             m_userLabel;
    QLabel*             m_statusLabel;

    // ── 核心组件 ──
    QTimer*             m_remindTimer;
    AudioPlayer*        m_audioPlayer;
    SpeechRecognizer*   m_speechRec;
    QString             m_username;
    QString             m_musicPath;      // 用户选择的提醒音乐路径

    // ── 工具栏 ──
    QToolBar*           m_toolbar;
    QAction*            m_addAction;
    QAction*            m_deleteAction;
    QAction*            m_editAction;
    QAction*            m_showAllAction;
};

#endif // MAINWINDOW_H
