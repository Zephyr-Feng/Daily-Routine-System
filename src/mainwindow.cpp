#include "mainwindow.h"
#include "taskdialog.h"
#include "voiceinputdialog.h"
#include "speechrecognizer.h"
#include "deepseekclient.h"
#include <QDir>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <chrono>


// ======================== 构造函数 ========================
// MainWindow 初始化入口，按顺序：
//   1. 初始化语音识别引擎（本地 tiny 模型，离线运行）
//   2. 构建 UI 界面（工具栏 + 日历 + 表格）
//   3. 从文件加载已保存的任务
//   4. 默认选中今天，刷新显示
//   5. 启动后台提醒线程（每 10 秒检查）
//   6. 连接信号与槽（数据变更自动刷新界面）
MainWindow::MainWindow(const QString& username, QWidget* parent)
    : QMainWindow(parent), m_username(username), m_running(true)
{
    // 初始化语音识别引擎（tiny 本地模型，无需网络）
    m_recognizer = new SpeechRecognizer(
        QDir::homePath() + "/whisper-models/tiny", this);

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

    // 启动后台提醒线程
    m_remindThread = std::thread(&MainWindow::reminderLoop, this);

    connect(&TaskManager::instance(), &TaskManager::tasksChanged,
            this, &MainWindow::refreshTable);
    connect(&TaskManager::instance(), &TaskManager::tasksChanged,
            this, &MainWindow::updateCalendar);

    connect(m_calendar, &QCalendarWidget::clicked,
            this, &MainWindow::onDateClicked);
}


// ======================== 析构函数 ========================
// 设置退出标记 → 等待提醒线程安全结束
MainWindow::~MainWindow() {
    m_running = false;
    if (m_remindThread.joinable())
        m_remindThread.join();
}


// ======================== 设置界面基本元素 ========================
// 状态栏：左侧显示用户名，右侧显示状态信息
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


// ======================== 工具栏 ========================
// 添加/编辑/删除、显示全部、刷新、语音录入等快捷操作按钮
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

    toolbar->addSeparator();

    m_voiceAction = toolbar->addAction("🎙 语音录入");
    m_voiceAction->setToolTip("通过语音录入新任务（按住说话，AI 自动识别）");
    connect(m_voiceAction, &QAction::triggered, this, &MainWindow::onVoiceInput);

    toolbar->setStyleSheet(
        "QToolBar { spacing: 5px; padding: 4px; background: #f5f5f5; border-bottom: 1px solid #ddd; }"
        "QToolButton { padding: 4px 12px; border-radius: 3px; }"
        "QToolButton:hover { background: #e0e0e0; }"
    );
}


// ======================== 设置日历控件 ========================
// 左侧日历：周一起始，固定宽度，隐藏垂直表头
void MainWindow::setupCalendar() {
    m_calendar = new QCalendarWidget();
    m_calendar->setGridVisible(true);
    m_calendar->setFirstDayOfWeek(Qt::Monday);
    m_calendar->setFixedWidth(280);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
}


// ======================== 设置任务表格 ========================
// 右侧表格：6 列（ID、名称、开始时间、提醒时间、优先级、分类）
// 与日历用 QSplitter 左右分栏显示
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


// ======================== 刷新表格显示 ========================
// 清空旧数据 → 从 TaskManager 读取 → 按日期筛选 → 填充表格行
// 高优先级任务显示红色文字
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


// ======================== 更新日历高亮 ========================
// 遍历所有任务，将有任务的日期用蓝底粗体标注
void MainWindow::updateCalendar() {
    // 清除旧高亮
    for (const QDate& d : m_highlightedDates) {
        m_calendar->setDateTextFormat(d, QTextCharFormat());
    }
    m_highlightedDates.clear();

    const auto& tasks = TaskManager::instance().allTasks();
    QTextCharFormat taskFormat;
    taskFormat.setFontWeight(QFont::Bold);
    taskFormat.setBackground(QBrush(QColor("#BBDEFB")));

    for (const auto& t : tasks) {
        Time st = t.getStartTime();
        QDate d(st.year, st.month, st.day);
        if (d.isValid()) {
            m_calendar->setDateTextFormat(d, taskFormat);
            m_highlightedDates.insert(d);
        }
    }
}


// ======================== 日历点击事件 ========================
// 用户点击某个日期 → 筛选该日任务并刷新表格
void MainWindow::onDateClicked(const QDate& date) {
    m_selectedDate = date;
    refreshTable();
    m_statusLabel->setText(
        QString("📅 %1").arg(date.toString("yyyy 年 M 月 d 日")));
}


// ======================== 显示全部 ========================
// 取消日期筛选（m_selectedDate 置为无效），显示所有任务
void MainWindow::showAllTasks() {
    m_selectedDate = QDate();
    refreshTable();
    m_statusLabel->setText("显示全部任务");
}


// ======================== 获取选中任务 ID ========================
// 从表格当前选中行提取第一列（ID），无选中时返回 -1
int MainWindow::selectedTaskId() const {
    QModelIndexList selection = m_tableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) return -1;

    int row = selection.first().row();
    return m_model->item(row, 0)->text().toInt();
}


// ======================== 添加任务 ========================
// 弹出 TaskDialog 让用户填写 → 确认后加入 TaskManager
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


// ======================== 删除任务 ========================
// 先检查是否有选中 → 确认弹窗 → 执行删除
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


// ======================== 编辑任务 ========================
// 选中任务 → 弹出预先填好的对话框 → 保存修改
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


// ======================== 后台提醒线程 ========================
// 独立线程循环，每 10 秒检查一次是否有任务到提醒时间
// 用 QMetaObject::invokeMethod + QueuedConnection 把 UI 操作发回主线程
void MainWindow::reminderLoop() {
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        if (!m_running) break;

        std::vector<Task> due = TaskManager::instance().checkReminders();
        if (!due.empty()) {
            QMetaObject::invokeMethod(this, [this, due]() {
                for (const auto& task : due) {
                    showReminder(task);
                }
            }, Qt::QueuedConnection);
        }
    }
}


// ======================== 显示提醒弹窗 ========================
// 弹出 QMessageBox 显示任务详情，同时播放 WAV 提示音
void MainWindow::showReminder(const Task& task) {
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

// ======================== 语音录入 ========================
// 检查语音引擎是否就绪 → 打开语音录入对话框 → AI 解析结果并添加任务
void MainWindow::onVoiceInput() {
    if (!m_recognizer->isReady()) {
        QMessageBox::warning(this, "语音录入不可用",
            "语音识别引擎未就绪。\n\n"
            "请确保已安装 faster-whisper：\n"
            "  pip3 install faster-whisper\n\n"
            "并已下载中文模型：\n"
            "  python3 -c \"from faster_whisper import WhisperModel; "
            "WhisperModel('small', device='cpu', compute_type='int8')\"");
        return;
    }

    VoiceInputDialog dlg(m_recognizer, this);
    if (dlg.exec() == QDialog::Accepted) {
        Task newTask = dlg.getTask();
        if (TaskManager::instance().addTask(newTask)) {
            m_statusLabel->setText("✅ 语音任务添加成功");
        } else {
            QMessageBox::warning(this, "添加失败",
                "任务添加失败！可能的原因：\n"
                "• 开始时间与其他任务冲突\n"
                "• 任务名称 + 开始时间重复");
            m_statusLabel->setText("❌ 语音任务添加失败");
        }
    }
}
