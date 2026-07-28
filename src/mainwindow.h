#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QStandardItemModel>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QHeaderView>
#include <QCalendarWidget>
#include <QSet>
#include <thread>
#include <atomic>
#include "taskmanager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const QString& username, QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onAddTask();
    void onDeleteTask();
    void onEditTask();
    void refreshTable();
    void showAllTasks();
    void showReminder(const Task& task);
    void onDateClicked(const QDate& date);
    void updateCalendar();

private:
    void reminderLoop();
    void setupUI();
    void setupToolBar();
    void setupTable();
    void setupCalendar();
    int selectedTaskId() const;

    QTableView*          m_tableView;
    QStandardItemModel*  m_model;
    QCalendarWidget*     m_calendar;
    QLabel*              m_userLabel;
    QLabel*              m_statusLabel;
    std::thread          m_remindThread;
    std::atomic<bool>    m_running;
    QString              m_username;
    QDate                m_selectedDate;
    QSet<QDate>          m_highlightedDates;

    QAction* m_addAction;
    QAction* m_deleteAction;
    QAction* m_editAction;
    QAction* m_refreshAction;
    QAction* m_showAllAction;
};

#endif // MAINWINDOW_H
