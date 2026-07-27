#include "taskdialog.h"

TaskDialog::TaskDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
}

void TaskDialog::setupUI() {
    setWindowTitle("添加任务");
    setFixedSize(420, 420);

    // ========== 任务名称 ==========
    QLabel* nameLabel = new QLabel("任务名称：");
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("例如：学习C++");

    // ========== 开始时间 ==========
    QGroupBox* startGroup = new QGroupBox("开始时间");
    QGridLayout* startLayout = new QGridLayout();

    m_startYear  = new QSpinBox(); m_startYear->setRange(2026, 2030);
    m_startMonth = new QSpinBox(); m_startMonth->setRange(1, 12);
    m_startDay   = new QSpinBox(); m_startDay->setRange(1, 31);
    m_startHour  = new QSpinBox(); m_startHour->setRange(0, 23);
    m_startMinute= new QSpinBox(); m_startMinute->setRange(0, 59);

    // 默认设为当前时间
    m_startYear->setValue(2026);
    m_startMonth->setValue(7);
    m_startDay->setValue(23);
    m_startHour->setValue(9);
    m_startMinute->setValue(0);

    startLayout->addWidget(new QLabel("年"), 0, 0);
    startLayout->addWidget(m_startYear,  0, 1);
    startLayout->addWidget(new QLabel("月"), 0, 2);
    startLayout->addWidget(m_startMonth, 0, 3);
    startLayout->addWidget(new QLabel("日"), 0, 4);
    startLayout->addWidget(m_startDay,   0, 5);
    startLayout->addWidget(new QLabel("时"), 1, 0);
    startLayout->addWidget(m_startHour,  1, 1);
    startLayout->addWidget(new QLabel("分"), 1, 2);
    startLayout->addWidget(m_startMinute,1, 3);
    startGroup->setLayout(startLayout);

    // ========== 提醒时间 ==========
    QGroupBox* remindGroup = new QGroupBox("提醒时间");
    QGridLayout* remindLayout = new QGridLayout();

    m_remindYear  = new QSpinBox(); m_remindYear->setRange(2026, 2030);
    m_remindMonth = new QSpinBox(); m_remindMonth->setRange(1, 12);
    m_remindDay   = new QSpinBox(); m_remindDay->setRange(1, 31);
    m_remindHour  = new QSpinBox(); m_remindHour->setRange(0, 23);
    m_remindMinute= new QSpinBox(); m_remindMinute->setRange(0, 59);

    // 默认提醒时间 = 开始时间前5分钟
    m_remindYear->setValue(2026);
    m_remindMonth->setValue(7);
    m_remindDay->setValue(23);
    m_remindHour->setValue(8);
    m_remindMinute->setValue(55);

    remindLayout->addWidget(new QLabel("年"), 0, 0);
    remindLayout->addWidget(m_remindYear,  0, 1);
    remindLayout->addWidget(new QLabel("月"), 0, 2);
    remindLayout->addWidget(m_remindMonth, 0, 3);
    remindLayout->addWidget(new QLabel("日"), 0, 4);
    remindLayout->addWidget(m_remindDay,   0, 5);
    remindLayout->addWidget(new QLabel("时"), 1, 0);
    remindLayout->addWidget(m_remindHour,  1, 1);
    remindLayout->addWidget(new QLabel("分"), 1, 2);
    remindLayout->addWidget(m_remindMinute,1, 3);
    remindGroup->setLayout(remindLayout);

    // ========== 优先级和分类 ==========
    QLabel* priLabel = new QLabel("优先级：");
    m_priorityCmb = new QComboBox();
    m_priorityCmb->addItem("高", HIGH);
    m_priorityCmb->addItem("中", MEDIUM);
    m_priorityCmb->addItem("低", LOW);
    m_priorityCmb->setCurrentIndex(1);  // 默认"中"

    QLabel* clsLabel = new QLabel("分类：");
    m_classifyCmb = new QComboBox();
    m_classifyCmb->addItem("学习", STUDY);
    m_classifyCmb->addItem("娱乐", PLAY);
    m_classifyCmb->addItem("生活", LIFE);

    QHBoxLayout* attrLayout = new QHBoxLayout();
    attrLayout->addWidget(priLabel);
    attrLayout->addWidget(m_priorityCmb);
    attrLayout->addSpacing(20);
    attrLayout->addWidget(clsLabel);
    attrLayout->addWidget(m_classifyCmb);

    // ========== 按钮 ==========
    QPushButton* okBtn = new QPushButton("确定");
    QPushButton* cancelBtn = new QPushButton("取消");
    okBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; "
        "padding: 6px 25px; border-radius: 4px; }"
    );

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    // ========== 总体布局 ==========
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(m_nameEdit);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(startGroup);
    mainLayout->addWidget(remindGroup);
    mainLayout->addLayout(attrLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(btnLayout);

    connect(okBtn,     &QPushButton::clicked, this, &TaskDialog::onAccept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

bool TaskDialog::validateInput() {
    // 名称不能为空
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入任务名称！");
        m_nameEdit->setFocus();
        return false;
    }

    // 提醒时间不能晚于开始时间
    Time st(m_startYear->value(), m_startMonth->value(), m_startDay->value(),
            m_startHour->value(), m_startMinute->value());
    Time rt(m_remindYear->value(), m_remindMonth->value(), m_remindDay->value(),
            m_remindHour->value(), m_remindMinute->value());

    if (!(rt < st) && !(rt == st)) {  // rt >= st
        QMessageBox::warning(this, "输入错误",
            "提醒时间必须在开始时间之前！");
        return false;
    }

    return true;
}

void TaskDialog::onAccept() {
    if (!validateInput()) return;
    accept();  // QDialog::accept()
}

Task TaskDialog::getTask() const {
    Time st(m_startYear->value(), m_startMonth->value(), m_startDay->value(),
            m_startHour->value(), m_startMinute->value());
    Time rt(m_remindYear->value(), m_remindMonth->value(), m_remindDay->value(),
            m_remindHour->value(), m_remindMinute->value());

    Priority p  = (Priority)m_priorityCmb->currentData().toInt();
    Classify c  = (Classify)m_classifyCmb->currentData().toInt();

    Task task(m_nameEdit->text().trimmed().toStdString(), st, rt, p, c);

    // 编辑模式下保留原 id
    if (m_editMode && m_editId >= 0) {
        task.setId(m_editId);
    }

    return task;
}

void TaskDialog::setTask(const Task& task) {
    m_editMode = true;
    m_editId   = task.getId();
    setWindowTitle("编辑任务");

    // 加载数据到表单
    m_nameEdit->setText(QString::fromStdString(task.getName()));

    Time st = task.getStartTime();
    m_startYear->setValue(st.year);
    m_startMonth->setValue(st.month);
    m_startDay->setValue(st.day);
    m_startHour->setValue(st.hour);
    m_startMinute->setValue(st.minute);

    Time rt = task.getRemindTime();
    m_remindYear->setValue(rt.year);
    m_remindMonth->setValue(rt.month);
    m_remindDay->setValue(rt.day);
    m_remindHour->setValue(rt.hour);
    m_remindMinute->setValue(rt.minute);

    int priIdx = m_priorityCmb->findData((int)task.getPriority());
    m_priorityCmb->setCurrentIndex(priIdx >= 0 ? priIdx : 1);

    int clsIdx = m_classifyCmb->findData((int)task.getClassify());
    m_classifyCmb->setCurrentIndex(clsIdx >= 0 ? clsIdx : 0);
}
