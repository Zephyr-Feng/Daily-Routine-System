#include "mainwindow.h"
#include "taskdialog.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

MainWindow::MainWindow(const QString& username, QWidget* parent)
    : QMainWindow(parent), m_username(username)
{
    setupUI();
    setupToolBar();
    setupCalendar();
    setupTable();

    int count = TaskManager::instance().loadFromFile("tasks.txt");
    m_statusLabel->setText(
        QString("已加载 %1 个任务").arg(count > 0 ? count : 0)
    );

    m_selectedDate = QDate::currentDate();
    refreshTable();
    updateCalendar();

    m_remindTimer = new QTimer(this);
    connect(m_remindTimer, &QTimer::timeout,
            this, &MainWindow::checkReminders);
    m_remindTimer->start(10000);

    connect(&TaskManager::instance(), &TaskManager::tasksChanged,
            this, &MainWindow::refreshTable);
    connect(&TaskManager::instance(), &TaskManager::tasksChanged,
            this, &MainWindow::updateCalendar);

    connect(m_calendar, &QCalendarWidget::clicked,
            this, &MainWindow::onDateClicked);
}

void MainWindow::setupUI() {
    resize(900, 500);
    setWindowTitle("日程管理系统 - " + m_username);
    setMinimumSize(700, 400);

    m_userLabel   = new QLabel("用户：" + m_username);
    m_statusLabel = new QLabel("就绪");
    m_userLabel->setStyleSheet("padding: 2px 10px;");
    m_statusLabel->setStyleSheet("padding: 2px 10px;");
    statusBar()->addWidget(m_userLabel);
    statusBar()->addPermanentWidget(m_statusLabel);
}

void MainWindow::setupToolBar() {
    QToolBar* toolbar = addToolBar("工具栏");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    m_addAction = toolbar->addAction("＋ 添加任务");
    m_addAction->setToolTip("添加一个新的日程任务");
    connect(m_addAction, &QAction::triggered, this, &MainWindow::onAddTask);

    m_editAction = toolbar->addAction("✎ 编辑任务");
    m_editAction->setToolTip("编辑选中的任务");
    connect(m_editAction, &QAction::triggered, this, &MainWindow::onEditTask);

    m_deleteAction = toolbar->addAction("✕ 删除任务");
    m_deleteAction->setToolTip("删除选中的任务");
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::onDeleteTask);

    toolbar->addSeparator();

    m_showAllAction = toolbar->addAction("📅 显示全部");
    m_showAllAction->setToolTip("取消日期筛选，显示所有任务");
    connect(m_showAllAction, &QAction::triggered, this, &MainWindow::showAllTasks);

    m_refreshAction = toolbar->addAction("↻ 刷新");
    m_refreshAction->setToolTip("刷新任务列表");
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::refreshTable);

    toolbar->setStyleSheet(
        "QToolBar { spacing: 5px; padding: 4px; background: #f5f5f5; border-bottom: 1px solid #ddd; }"
        "QToolButton { padding: 4px 12px; border-radius: 3px; }"
        "QToolButton:hover { background: #e0e0e0; }"
    );
}

void MainWindow::setupCalendar() {
    m_calendar = new QCalendarWidget();
    m_calendar->setGridVisible(true);
    m_calendar->setFirstDayOfWeek(Qt::Monday);
    m_calendar->setFixedWidth(280);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
}

void MainWindow::setupTable() {
    m_tableView = new QTableView();
    m_model     = new QStandardItemModel(0, 6, this);

    m_model->setHorizontalHeaderLabels({
        "ID", "任务名称", "开始时间", "提醒时间", "优先级", "分类"
    });

    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->verticalHeader()->hide();
    m_tableView->setSortingEnabled(false);

    QHeaderView* header = m_tableView->horizontalHeader();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents);

    connect(m_tableView, &QTableView::doubleClicked,
            this, &MainWindow::onEditTask);

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(m_calendar);
    splitter->addWidget(m_tableView);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);
}

void MainWindow::refreshTable() {
    m_model->removeRows(0, m_model->rowCount());

    const auto& tasks = TaskManager::instance().allTasks();

    for (size_t i = 0; i < tasks.size(); i++) {
        const Task& t = tasks[i];
        Time st = t.getStartTime();

        if (m_selectedDate.isValid()) {
            if (st.year  != m_selectedDate.year() ||
                st.month != m_selectedDate.month() ||
                st.day   != m_selectedDate.day()) {
                continue;
            }
        }

        QList<QStandardItem*> row;
        row.append(new QStandardItem(QString::number(t.getId())));
        row.append(new QStandardItem(QString::fromStdString(t.getName())));
        row.append(new QStandardItem(
            QString::fromStdString(st.toString())));
        row.append(new QStandardItem(
            QString::fromStdString(t.getRemindTime().toString())));
        row.append(new QStandardItem(
            QString::fromStdString(t.priorityToString())));
        row.append(new QStandardItem(
            QString::fromStdString(t.classifyToString())));

        for (auto* item : row) {
            item->setTextAlignment(Qt::AlignCenter);
        }
        row[1]->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        if (t.getPriority() == HIGH) {
            for (auto* item : row) {
                item->setForeground(QBrush(QColor("#d32f2f")));
            }
        }

        m_model->appendRow(row);
    }

    int total = TaskManager::instance().taskCount();
    int shown = m_model->rowCount();
    if (m_selectedDate.isValid() && shown < total) {
        m_statusLabel->setText(
            QString("%1 / %2 个任务（筛选：%3）")
                .arg(shown).arg(total)
                .arg(m_selectedDate.toString("yyyy-MM-dd")));
    } else {
        m_statusLabel->setText(
            QString("共 %1 个任务").arg(total));
    }
}

void MainWindow::updateCalendar() {
    m_calendar->setDateTextFormat(QDate(), QTextCharFormat());

    const auto& tasks = TaskManager::instance().allTasks();
    QTextCharFormat taskFormat;
    taskFormat.setFontWeight(QFont::Bold);
    taskFormat.setBackground(QBrush(QColor("#BBDEFB")));

    for (const auto& t : tasks) {
        Time st = t.getStartTime();
        QDate d(st.year, st.month, st.day);
        if (d.isValid()) {
            m_calendar->setDateTextFormat(d, taskFormat);
        }
    }
}

void MainWindow::onDateClicked(const QDate& date) {
    m_selectedDate = date;
    refreshTable();
    m_statusLabel->setText(
        QString("📅 %1").arg(date.toString("yyyy 年 M 月 d 日")));
}

void MainWindow::showAllTasks() {
    m_selectedDate = QDate();
    refreshTable();
    m_statusLabel->setText("显示全部任务");
}

int MainWindow::selectedTaskId() const {
    QModelIndexList selection = m_tableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) return -1;

    int row = selection.first().row();
    return m_model->item(row, 0)->text().toInt();
}

void MainWindow::onAddTask() {
    TaskDialog dlg(this);
    dlg.setAddMode();
    dlg.setWindowTitle("添加新任务");

    if (dlg.exec() == QDialog::Accepted) {
        Task newTask = dlg.getTask();
        if (TaskManager::instance().addTask(newTask)) {
            m_statusLabel->setText("✅ 任务添加成功");
        } else {
            QMessageBox::warning(this, "添加失败",
                "任务添加失败！可能的原因：\n"
                "• 开始时间与其他任务冲突\n"
                "• 任务名称 + 开始时间重复");
            m_statusLabel->setText("❌ 任务添加失败");
        }
    }
}

void MainWindow::onDeleteTask() {
    int id = selectedTaskId();
    if (id < 0) {
        QMessageBox::information(this, "提示", "请先在表格中选中一个任务！");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除",
        QString("确定要删除任务 #%1 吗？\n此操作不可撤销！").arg(id),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (TaskManager::instance().deleteTask(id)) {
            m_statusLabel->setText("🗑 任务已删除");
        }
    }
}

void MainWindow::onEditTask() {
    int id = selectedTaskId();
    if (id < 0) {
        QMessageBox::information(this, "提示", "请先在表格中选中一个任务！");
        return;
    }

    Task* task = TaskManager::instance().findTask(id);
    if (!task) return;

    TaskDialog dlg(this);
    dlg.setTask(*task);
    dlg.setWindowTitle("编辑任务");

    if (dlg.exec() == QDialog::Accepted) {
        Task updated = dlg.getTask();
        if (TaskManager::instance().updateTask(updated)) {
            m_statusLabel->setText("✅ 任务已更新");
        }
    }
}

void MainWindow::checkReminders() {
    std::vector<Task> due = TaskManager::instance().checkReminders();

    for (const auto& task : due) {
        QMessageBox* msgBox = new QMessageBox(this);
        msgBox->setWindowTitle("⏰ 日程提醒");
        msgBox->setIcon(QMessageBox::Information);
        msgBox->setText(QString(
            "<h3>任务提醒</h3>"
            "<p><b>任务：</b>%1</p>"
            "<p><b>开始时间：</b>%2</p>"
            "<p><b>优先级：</b>%3 | <b>分类：</b>%4</p>"
        ).arg(
            QString::fromStdString(task.getName()),
            QString::fromStdString(task.getStartTime().toString()),
            QString::fromStdString(task.priorityToString()),
            QString::fromStdString(task.classifyToString())
        ));
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->setAttribute(Qt::WA_DeleteOnClose);
        msgBox->show();

        // ===== 播放提醒音 =====
        system("aplay remind.wav &");
    }
}
