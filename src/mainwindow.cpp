#include "mainwindow.h"
#include "taskdialog.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

MainWindow::MainWindow(const QString& username, QWidget* parent)
    : QMainWindow(parent), m_username(username)
{
    setupUI();
    setupToolBar();
    setupTable();

    // ===== 加载任务数据 =====
    int count = TaskManager::instance().loadFromFile("tasks.txt");
    m_statusLabel->setText(
        QString("已加载 %1 个任务").arg(count > 0 ? count : 0)
    );
    refreshTable();

    // ===== 初始化音频播放器（Stage 2 新增）=====
    m_audioPlayer = new AudioPlayer(this);

    // ===== 启动提醒定时器（每10秒检查一次）=====
    m_remindTimer = new QTimer(this);
    connect(m_remindTimer, &QTimer::timeout,
            this, &MainWindow::checkReminders);
    m_remindTimer->start(10000);  // 10秒

    // ===== 监听数据变化自动刷新表格 =====
    connect(&TaskManager::instance(), &TaskManager::tasksChanged,
            this, &MainWindow::refreshTable);
}

// ========== 界面初始化 ==========

void MainWindow::setupUI() {
    // 窗口尺寸：800x500
    resize(800, 500);
    setWindowTitle("日程管理系统 - " + m_username);
    setMinimumSize(600, 400);

    // 状态栏
    m_userLabel   = new QLabel("用户：" + m_username);
    m_statusLabel = new QLabel("就绪");

    // 设置样式：让表格列自动拉伸
    m_userLabel->setStyleSheet("padding: 2px 10px;");
    m_statusLabel->setStyleSheet("padding: 2px 10px;");

    statusBar()->addWidget(m_userLabel);
    statusBar()->addPermanentWidget(m_statusLabel);
}

void MainWindow::setupToolBar() {
    QToolBar* toolbar = addToolBar("工具栏");
    toolbar->setMovable(false);       // 固定位置
    toolbar->setIconSize(QSize(24, 24));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // 添加任务
    m_addAction = toolbar->addAction("＋ 添加任务");
    m_addAction->setToolTip("添加一个新的日程任务");
    connect(m_addAction, &QAction::triggered, this, &MainWindow::onAddTask);

    // 编辑任务
    m_editAction = toolbar->addAction("✎ 编辑任务");
    m_editAction->setToolTip("编辑选中的任务");
    connect(m_editAction, &QAction::triggered, this, &MainWindow::onEditTask);

    // 删除任务
    m_deleteAction = toolbar->addAction("✕ 删除任务");
    m_deleteAction->setToolTip("删除选中的任务");
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::onDeleteTask);

    toolbar->addSeparator();

    // 刷新
    m_refreshAction = toolbar->addAction("↻ 刷新");
    m_refreshAction->setToolTip("刷新任务列表");
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::refreshTable);

    // 样式美化
    toolbar->setStyleSheet(
        "QToolBar { spacing: 5px; padding: 4px; background: #f5f5f5; border-bottom: 1px solid #ddd; }"
        "QToolButton { padding: 4px 12px; border-radius: 3px; }"
        "QToolButton:hover { background: #e0e0e0; }"
    );
}

void MainWindow::setupTable() {
    m_tableView = new QTableView(this);
    m_model     = new QStandardItemModel(0, 6, this);

    // ===== 设置表头 =====
    m_model->setHorizontalHeaderLabels({
        "ID", "任务名称", "开始时间", "提醒时间", "优先级", "分类"
    });

    m_tableView->setModel(m_model);

    // ===== 表格设置 =====
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  // 选中整行
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection); // 单选
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);   // 禁止编辑
    m_tableView->setAlternatingRowColors(true);  // 交替行颜色
    m_tableView->verticalHeader()->hide();        // 隐藏行号
    m_tableView->setSortingEnabled(false);        // 我们自己排序

    // ===== 列宽 =====
    QHeaderView* header = m_tableView->horizontalHeader();
    header->setStretchLastSection(true);    // 最后一列拉伸
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);  // ID
    header->setSectionResizeMode(1, QHeaderView::Stretch);            // 名称
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);   // 开始时间
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);   // 提醒时间
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents);   // 优先级
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents);   // 分类

    // 双击编辑任务
    connect(m_tableView, &QTableView::doubleClicked,
            this, &MainWindow::onEditTask);

    // 设为中央控件
    setCentralWidget(m_tableView);
}

// ========== 表格刷新 ==========

void MainWindow::refreshTable() {
    m_model->removeRows(0, m_model->rowCount());  // 清空

    const auto& tasks = TaskManager::instance().allTasks();

    for (size_t i = 0; i < tasks.size(); i++) {
        const Task& t = tasks[i];
        QList<QStandardItem*> row;

        // ID
        row.append(new QStandardItem(QString::number(t.getId())));
        // 任务名称
        row.append(new QStandardItem(QString::fromStdString(t.getName())));
        // 开始时间
        row.append(new QStandardItem(
            QString::fromStdString(t.getStartTime().toString())));
        // 提醒时间
        row.append(new QStandardItem(
            QString::fromStdString(t.getRemindTime().toString())));
        // 优先级
        row.append(new QStandardItem(
            QString::fromStdString(t.priorityToString())));
        // 分类
        row.append(new QStandardItem(
            QString::fromStdString(t.classifyToString())));

        // 居中显示
        for (auto* item : row) {
            item->setTextAlignment(Qt::AlignCenter);
        }
        // 名称左对齐
        row[1]->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        // 高优先级的行标红
        if (t.getPriority() == HIGH) {
            for (auto* item : row) {
                item->setForeground(QBrush(QColor("#d32f2f")));  // 红色
            }
        }

        m_model->appendRow(row);
    }

    m_statusLabel->setText(
        QString("共 %1 个任务").arg(TaskManager::instance().taskCount())
    );
}

// ========== 选中任务 ==========

int MainWindow::selectedTaskId() const {
    QModelIndexList selection = m_tableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) return -1;

    int row = selection.first().row();
    return m_model->item(row, 0)->text().toInt();
}

// ========== 添加任务 ==========

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

// ========== 删除任务 ==========

void MainWindow::onDeleteTask() {
    int id = selectedTaskId();
    if (id < 0) {
        QMessageBox::information(this, "提示", "请先在表格中选中一个任务！");
        return;
    }

    // 确认对话框
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

// ========== 编辑任务 ==========

void MainWindow::onEditTask() {
    int id = selectedTaskId();
    if (id < 0) {
        QMessageBox::information(this, "提示", "请先在表格中选中一个任务！");
        return;
    }

    Task* task = TaskManager::instance().findTask(id);
    if (!task) return;

    TaskDialog dlg(this);
    dlg.setTask(*task);  // 加载已有数据
    dlg.setWindowTitle("编辑任务");

    if (dlg.exec() == QDialog::Accepted) {
        Task updated = dlg.getTask();
        if (TaskManager::instance().updateTask(updated)) {
            m_statusLabel->setText("✅ 任务已更新");
        }
    }
}

// ========== 提醒检查 ==========

void MainWindow::checkReminders() {
    std::vector<Task> due = TaskManager::instance().checkReminders();

    for (const auto& task : due) {
        // 弹窗提醒
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
        msgBox->setAttribute(Qt::WA_DeleteOnClose);  // 自动释放
        msgBox->show();  // 非阻塞，可以同时弹出多个提醒

        // ===== 播放提醒音（Stage 2 新增）=====
        m_audioPlayer->playReminder();
    }
}
