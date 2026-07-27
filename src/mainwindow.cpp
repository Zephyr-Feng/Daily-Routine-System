#include "mainwindow.h"
#include "taskdialog.h"
#include "voiceinputdialog.h"
#include <QCoreApplication>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QBrush>

MainWindow::MainWindow(const QString& username, QWidget* parent)
    : QMainWindow(parent), m_username(username), m_filterActive(false)
{
    setupToolBar();
    setupUI();
    setupTable();
    setupCalendar();
    setupStatusBar();

    // ===== 加载任务数据 =====
    int count = TaskManager::instance().loadFromFile("tasks.txt");
    m_statusLabel->setText(
        QString("已加载 %1 个任务").arg(count > 0 ? count : 0)
    );
    refreshTable();
    refreshCalendarMarks();

    // ===== 初始化音频播放器（Stage 2 新增）=====
    m_audioPlayer = new AudioPlayer(this);

    // ===== 初始化语音识别器（Stage 3 新增）=====
    // Whisper small 模型，中文识别效果好
    m_speechRec = new SpeechRecognizer("small", this);

    // ===== 启动提醒定时器（每10秒检查一次）=====
    m_remindTimer = new QTimer(this);
    connect(m_remindTimer, &QTimer::timeout,
            this, &MainWindow::checkReminders);
    m_remindTimer->start(10000);  // 10秒

    // ===== 监听数据变化自动刷新 =====
    connect(&TaskManager::instance(), &TaskManager::tasksChanged,
            this, [this]() {
        refreshTable();
        refreshCalendarMarks();
    });
}

// ========== 整体布局：日历 + 表格 ==========

void MainWindow::setupUI() {
    resize(1050, 560);
    setWindowTitle("日程管理系统 - " + m_username);
    setMinimumSize(800, 420);

    // 中央区域：QSplitter 左右分栏
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    splitter->setStyleSheet(
        "QSplitter::handle { background: #D0D5DD; }"
    );

    // ── 左侧：日历面板 ──
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(8, 8, 8, 8);
    leftLayout->setSpacing(6);

    m_filterLabel = new QLabel("点击日期筛选任务");
    m_filterLabel->setAlignment(Qt::AlignCenter);
    m_filterLabel->setStyleSheet(
        "color: #667085; font-size: 12px; padding: 4px;"
    );
    leftLayout->addWidget(m_filterLabel);

    m_calendar = new QCalendarWidget();
    m_calendar->setMinimumWidth(260);
    m_calendar->setMaximumWidth(320);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    leftLayout->addWidget(m_calendar);

    // "显示全部"按钮
    QPushButton* showAllBtn = new QPushButton("显示全部任务");
    showAllBtn->setCursor(Qt::PointingHandCursor);
    showAllBtn->setStyleSheet(
        "QPushButton { color: #1976D2; border: 1px solid #1976D2; "
        "border-radius: 5px; padding: 5px; font-size: 12px; }"
        "QPushButton:hover { background: #E3F2FD; }"
    );
    connect(showAllBtn, &QPushButton::clicked, this, &MainWindow::onShowAll);
    leftLayout->addWidget(showAllBtn);

    splitter->addWidget(leftPanel);

    // ── 右侧：表格 ──
    m_tableView = new QTableView();
    splitter->addWidget(m_tableView);

    // 左右比例 ~3:7
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 5);

    setCentralWidget(splitter);
}

// ========== 工具栏 ==========

void MainWindow::setupToolBar() {
    m_toolbar = addToolBar("工具栏");
    m_toolbar->setMovable(false);
    m_toolbar->setIconSize(QSize(20, 20));
    m_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toolbar->setStyleSheet(
        "QToolBar { spacing: 2px; padding: 4px 8px; "
        "background: #FFFFFF; border-bottom: 1px solid #E8ECF0; }"
        "QToolBar QToolButton { padding: 5px 10px; border-radius: 5px; "
        "color: #344054; font-size: 13px; }"
        "QToolBar QToolButton:hover { background: #E3F2FD; color: #1976D2; }"
        "QToolBar QToolButton:pressed { background: #BBDEFB; }"
        "QToolBar QToolButton[actionType=\"primary\"] { "
        "background: #1976D2; color: #FFFFFF; border-radius: 5px; }"
        "QToolBar QToolButton[actionType=\"primary\"]:hover { background: #1565C0; }"
        "QToolBar::separator { width: 1px; background: #D0D5DD; margin: 4px 6px; }"
    );

    m_addAction = m_toolbar->addAction("＋ 添加任务");
    m_addAction->setToolTip("添加一个新的日程任务");
    connect(m_addAction, &QAction::triggered, this, &MainWindow::onAddTask);

    m_editAction = m_toolbar->addAction("✎ 编辑");
    m_editAction->setToolTip("编辑选中的任务");
    connect(m_editAction, &QAction::triggered, this, &MainWindow::onEditTask);

    m_deleteAction = m_toolbar->addAction("✕ 删除");
    m_deleteAction->setToolTip("删除选中的任务");
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::onDeleteTask);

    m_toolbar->addSeparator();

    QAction* voiceAction = m_toolbar->addAction("🎤 语音录入");
    voiceAction->setToolTip("通过语音识别录入新任务");
    connect(voiceAction, &QAction::triggered, this, &MainWindow::onVoiceInput);

    m_toolbar->addSeparator();

    QAction* musicAction = m_toolbar->addAction("🎵 选择提醒音乐");
    musicAction->setToolTip("选择自定义 MP3/WAV 提醒音乐");
    connect(musicAction, &QAction::triggered, this, &MainWindow::onSelectMusic);

    QAction* refreshAction = m_toolbar->addAction("↻ 刷新");
    refreshAction->setToolTip("刷新任务列表");
    connect(refreshAction, &QAction::triggered, this, [this]() {
        refreshTable();
        refreshCalendarMarks();
    });
}

// ========== 日历 ==========

void MainWindow::setupCalendar() {
    m_calendar->setSelectedDate(QDate::currentDate());

    connect(m_calendar, &QCalendarWidget::clicked,
            this, &MainWindow::onDateClicked);
}

void MainWindow::refreshCalendarMarks() {
    // 清除所有日期格式 → 然后标记有任务的日期
    QTextCharFormat defaultFmt;
    m_calendar->setDateTextFormat(QDate(), defaultFmt);  // 重置全部

    const auto& tasks = TaskManager::instance().allTasks();
    QTextCharFormat markFmt;

    for (const auto& task : tasks) {
        const Time& st = task.getStartTime();
        QDate d(st.year, st.month, st.day);
        if (d.isValid()) {
            // 用蓝色圆点标记
            markFmt.setBackground(QColor("#E3F2FD"));
            markFmt.setForeground(QColor("#1565C0"));
            markFmt.setFontWeight(QFont::Bold);
            m_calendar->setDateTextFormat(d, markFmt);
        }
    }

    // 如果当前有筛选，保持筛选日期高亮
    if (m_filterActive) {
        QTextCharFormat selFmt;
        selFmt.setBackground(QColor("#1976D2"));
        selFmt.setForeground(QColor("#FFFFFF"));
        selFmt.setFontWeight(QFont::Bold);
        m_calendar->setDateTextFormat(m_filterDate, selFmt);
    }
}

void MainWindow::onDateClicked(const QDate& date) {
    m_filterActive = true;
    m_filterDate = date;
    m_filterLabel->setText(
        QString("筛选：%1  ─ 点击其他日期切换")
        .arg(date.toString("MM月dd日"))
    );
    refreshCalendarMarks();
    refreshTable();
}

void MainWindow::onShowAll() {
    m_filterActive = false;
    m_filterLabel->setText("点击日期筛选任务");
    refreshCalendarMarks();
    refreshTable();
}

// ========== 表格 ==========

void MainWindow::setupTable() {
    m_model = new QStandardItemModel(0, 6, this);

    // ===== 设置表头 =====
    m_model->setHorizontalHeaderLabels({
        "ID", "任务名称", "开始时间", "提醒时间", "优先级", "分类"
    });

    m_tableView->setModel(m_model);

    // ===== 表格设置 =====
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->verticalHeader()->hide();
    m_tableView->setSortingEnabled(false);
    m_tableView->setShowGrid(false);  // 去掉网格线，更干净

    // ===== 列宽 =====
    QHeaderView* header = m_tableView->horizontalHeader();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    header->setDefaultAlignment(Qt::AlignCenter);

    // 双击编辑
    connect(m_tableView, &QTableView::doubleClicked,
            this, &MainWindow::onEditTask);
}

// ========== 表格刷新 ==========

void MainWindow::refreshTable() {
    m_model->removeRows(0, m_model->rowCount());

    const auto& tasks = TaskManager::instance().allTasks();
    int visibleCount = 0;

    for (size_t i = 0; i < tasks.size(); i++) {
        const Task& t = tasks[i];

        // 日期筛选
        if (m_filterActive) {
            const Time& st = t.getStartTime();
            if (QDate(st.year, st.month, st.day) != m_filterDate) {
                continue;  // 跳过非筛选日期的任务
            }
        }
        visibleCount++;

        QList<QStandardItem*> row;

        row.append(new QStandardItem(QString::number(t.getId())));
        row.append(new QStandardItem(QString::fromStdString(t.getName())));
        row.append(new QStandardItem(
            QString::fromStdString(t.getStartTime().toString())));
        row.append(new QStandardItem(
            QString::fromStdString(t.getRemindTime().toString())));

        // 优先级带色标
        QString priStr = QString::fromStdString(t.priorityToString());
        QStandardItem* priItem = new QStandardItem(priStr);
        if (t.getPriority() == HIGH) {
            priItem->setForeground(QBrush(QColor("#D32F2F")));
            priItem->setData(QColor("#FFEBEE"), Qt::BackgroundRole);
        } else if (t.getPriority() == LOW) {
            priItem->setForeground(QBrush(QColor("#9E9E9E")));
        } else {
            priItem->setForeground(QBrush(QColor("#F57C00")));
        }
        row.append(priItem);

        // 分类
        row.append(new QStandardItem(
            QString::fromStdString(t.classifyToString())));

        // 居中显示
        for (auto* item : row) {
            item->setTextAlignment(Qt::AlignCenter);
        }
        // 名称左对齐
        row[1]->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        // 高优先级整行浅红背景
        if (t.getPriority() == HIGH) {
            for (auto* item : row) {
                item->setData(QColor("#FFF5F5"), Qt::BackgroundRole);
            }
        }

        m_model->appendRow(row);
    }

    // 状态栏
    if (m_filterActive) {
        m_statusLabel->setText(
            QString("筛选 %1 · %2 个任务")
            .arg(m_filterDate.toString("MM/dd"))
            .arg(visibleCount)
        );
    } else {
        m_statusLabel->setText(
            QString("共 %1 个任务").arg(visibleCount)
        );
    }
}

// ========== 状态栏 ==========

void MainWindow::setupStatusBar() {
    m_userLabel   = new QLabel("👤 " + m_username);
    m_statusLabel = new QLabel("就绪");

    statusBar()->setStyleSheet(
        "QStatusBar { background: #1D2939; color: #D0D5DD; "
        "font-size: 12px; padding: 2px 0; }"
        "QStatusBar QLabel { color: #D0D5DD; padding: 2px 10px; }"
    );

    statusBar()->addWidget(m_userLabel);
    statusBar()->addPermanentWidget(m_statusLabel);
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
    dlg.setTask(*task);
    dlg.setWindowTitle("编辑任务");

    if (dlg.exec() == QDialog::Accepted) {
        Task updated = dlg.getTask();
        if (TaskManager::instance().updateTask(updated)) {
            m_statusLabel->setText("✅ 任务已更新");
        }
    }
}

// ========== 选择音乐 ==========

void MainWindow::onSelectMusic() {
    QString path = QFileDialog::getOpenFileName(
        this, "选择提醒音乐", "",
        "音频文件 (*.mp3 *.wav *.ogg *.flac);;所有文件 (*)"
    );

    if (!path.isEmpty()) {
        m_musicPath = path;
        m_statusLabel->setText("🎵 已设置提醒音乐");
    }
}

// ========== 提醒检查 ==========

void MainWindow::checkReminders() {
    std::vector<Task> due = TaskManager::instance().checkReminders();

    for (const auto& task : due) {
        QMessageBox* msgBox = new QMessageBox(this);
        msgBox->setWindowTitle("⏰ 日程提醒");
        msgBox->setIcon(QMessageBox::Information);
        msgBox->setText(QString(
            "<h3 style='color:#1976D2;'>任务提醒</h3>"
            "<p><b>任务：</b>%1</p>"
            "<p><b>开始时间：</b>%2</p>"
            "<p><b>优先级：</b>%3 &nbsp;|&nbsp; <b>分类：</b>%4</p>"
        ).arg(
            QString::fromStdString(task.getName()),
            QString::fromStdString(task.getStartTime().toString()),
            QString::fromStdString(task.priorityToString()),
            QString::fromStdString(task.classifyToString())
        ));
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->setAttribute(Qt::WA_DeleteOnClose);
        msgBox->show();

        // ===== 播放提醒音（Stage 2 新增）=====
        m_audioPlayer->playReminder(m_musicPath);
    }
}

// ========== 语音录入（Stage 3 新增）==========

void MainWindow::onVoiceInput() {
    if (!m_speechRec->isReady()) {
        QMessageBox::warning(this, "语音识别未就绪",
            "语音识别引擎未初始化！\n\n"
            "请确认以下环境已配置：\n"
            "1. Python3 已安装\n"
            "2. pip3 install vosk\n"
            "3. 中文语音模型已下载到程序目录\n"
            "   (vosk-model-small-cn-0.22)");
        return;
    }

    VoiceInputDialog dlg(m_speechRec, this);
    dlg.setWindowTitle("🎤 语音录入任务");

    if (dlg.exec() == QDialog::Accepted) {
        Task newTask = dlg.getTask();
        if (TaskManager::instance().addTask(newTask)) {
            m_statusLabel->setText("✅ 语音任务添加成功");
        } else {
            QMessageBox::warning(this, "添加失败",
                "语音任务添加失败！可能的原因：\n"
                "• 开始时间与其他任务冲突\n"
                "• 任务名称 + 开始时间重复");
            m_statusLabel->setText("❌ 语音任务添加失败");
        }
    }
}
