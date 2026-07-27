#include "taskdialog.h"

TaskDialog::TaskDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
}

void TaskDialog::setupUI() {
    setWindowTitle("添加任务");
    setFixedSize(440, 400);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(20, 16, 20, 16);

    // ========== 任务名称 ==========
    QLabel* nameLabel = new QLabel("任务名称");
    nameLabel->setStyleSheet("font-weight: bold; color: #344054;");
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("例如：学习C++");
    m_nameEdit->setMinimumHeight(34);

    // ========== 开始时间 ==========
    QGroupBox* startGroup = new QGroupBox("开始时间");
    QVBoxLayout* startLayout = new QVBoxLayout(startGroup);
    m_startTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    m_startTimeEdit->setDisplayFormat("yyyy-MM-dd  HH:mm");
    m_startTimeEdit->setCalendarPopup(true);  // 弹出日历选择器
    m_startTimeEdit->setMinimumHeight(34);
    startLayout->addWidget(m_startTimeEdit);

    // ========== 提醒时间 ==========
    QGroupBox* remindGroup = new QGroupBox("提醒时间");
    QVBoxLayout* remindLayout = new QVBoxLayout(remindGroup);
    m_remindTimeEdit = new QDateTimeEdit(
        QDateTime::currentDateTime().addSecs(-300));  // 默认5分钟前
    m_remindTimeEdit->setDisplayFormat("yyyy-MM-dd  HH:mm");
    m_remindTimeEdit->setCalendarPopup(true);
    m_remindTimeEdit->setMinimumHeight(34);
    remindLayout->addWidget(m_remindTimeEdit);

    // ========== 优先级和分类 ==========
    QGroupBox* attrGroup = new QGroupBox("属性");
    QHBoxLayout* attrLayout = new QHBoxLayout(attrGroup);

    QLabel* priLabel = new QLabel("优先级：");
    priLabel->setStyleSheet("font-weight: bold;");
    m_priorityCmb = new QComboBox();
    m_priorityCmb->addItem("🔴 高", HIGH);
    m_priorityCmb->addItem("🟡 中", MEDIUM);
    m_priorityCmb->addItem("⚪ 低", LOW);
    m_priorityCmb->setCurrentIndex(1);

    QLabel* clsLabel = new QLabel("分类：");
    clsLabel->setStyleSheet("font-weight: bold;");
    m_classifyCmb = new QComboBox();
    m_classifyCmb->addItem("📖 学习", STUDY);
    m_classifyCmb->addItem("🎮 娱乐", PLAY);
    m_classifyCmb->addItem("🏠 生活", LIFE);

    attrLayout->addWidget(priLabel);
    attrLayout->addWidget(m_priorityCmb, 1);
    attrLayout->addSpacing(16);
    attrLayout->addWidget(clsLabel);
    attrLayout->addWidget(m_classifyCmb, 1);

    // ========== 按钮 ==========
    QPushButton* okBtn = new QPushButton("确定");
    QPushButton* cancelBtn = new QPushButton("取消");

    okBtn->setProperty("primary", true);
    okBtn->setMinimumHeight(34);
    okBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setMinimumHeight(34);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(
        "QPushButton { color: #667085; }"
        "QPushButton:hover { background: #F0F3F8; }"
    );

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(okBtn);

    // ========== 总体布局 ==========
    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(m_nameEdit);
    mainLayout->addWidget(startGroup);
    mainLayout->addWidget(remindGroup);
    mainLayout->addWidget(attrGroup);
    mainLayout->addSpacing(4);
    mainLayout->addLayout(btnLayout);

    connect(okBtn,     &QPushButton::clicked, this, &TaskDialog::onAccept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

bool TaskDialog::validateInput() {
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入任务名称！");
        m_nameEdit->setFocus();
        return false;
    }

    // 提醒时间不能晚于开始时间
    if (m_remindTimeEdit->dateTime() >= m_startTimeEdit->dateTime()) {
        QMessageBox::warning(this, "输入错误",
            "提醒时间必须在开始时间之前！");
        return false;
    }

    return true;
}

void TaskDialog::onAccept() {
    if (!validateInput()) return;
    accept();
}

Task TaskDialog::getTask() const {
    QDateTime st = m_startTimeEdit->dateTime();
    QDateTime rt = m_remindTimeEdit->dateTime();

    Time startTime(st.date().year(), st.date().month(), st.date().day(),
                   st.time().hour(), st.time().minute());
    Time remindTime(rt.date().year(), rt.date().month(), rt.date().day(),
                    rt.time().hour(), rt.time().minute());

    Priority p  = (Priority)m_priorityCmb->currentData().toInt();
    Classify c  = (Classify)m_classifyCmb->currentData().toInt();

    Task task(m_nameEdit->text().trimmed().toStdString(), startTime, remindTime, p, c);

    if (m_editMode && m_editId >= 0) {
        task.setId(m_editId);
    }

    return task;
}

void TaskDialog::setTask(const Task& task) {
    m_editMode = true;
    m_editId   = task.getId();
    setWindowTitle("编辑任务");

    m_nameEdit->setText(QString::fromStdString(task.getName()));

    Time st = task.getStartTime();
    m_startTimeEdit->setDateTime(
        QDateTime(QDate(st.year, st.month, st.day),
                  QTime(st.hour, st.minute)));

    Time rt = task.getRemindTime();
    m_remindTimeEdit->setDateTime(
        QDateTime(QDate(rt.year, rt.month, rt.day),
                  QTime(rt.hour, rt.minute)));

    int priIdx = m_priorityCmb->findData((int)task.getPriority());
    m_priorityCmb->setCurrentIndex(priIdx >= 0 ? priIdx : 1);

    int clsIdx = m_classifyCmb->findData((int)task.getClassify());
    m_classifyCmb->setCurrentIndex(clsIdx >= 0 ? clsIdx : 0);
}
