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
#include "speechrecognizer.h"


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
    SpeechRecognizer*    m_speechRec;      // 语音识别器（Stage 3 新增）
    QString              m_username;       // 当前用户名

    // 工具栏按钮
    QAction* m_addAction;
    QAction* m_deleteAction;
    QAction* m_editAction;
    QAction* m_refreshAction;
};

#endif // MAINWINDOW_H
