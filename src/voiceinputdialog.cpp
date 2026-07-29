/**
 * @brief VoiceInputDialog 实现 — 语音录入任务对话框
 *
 * 完整流程：
 *  1. 用户按住 🎤 按钮 → 开始录音
 *  2. 松开按钮 → 停止录音 → WAV 文件保存
 *  3. 调用 faster-whisper（Python）离线识别 → 中文文本
 *  4. 调用 DeepSeek API 解析中文文本 → 结构化任务字段
 *  5. 显示解析结果，用户确认/调整 → accept()
 */
#include "voiceinputdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QRegularExpression>
#include <QDir>
#include <QFileInfo>

VoiceInputDialog::VoiceInputDialog(SpeechRecognizer* recognizer,
                                     QWidget* parent)
    : QDialog(parent)
    , m_recorder(new AudioRecorder(this))
    , m_recognizer(recognizer)
    , m_deepseek(new DeepSeekClient(this))
    , m_recording(false)
{
    // 设置 DeepSeek API Key
    m_deepseek->setApiKey("sk-3b3ff5c707de47de8beb33aed7301bee");

    setupUI();

    // 录音完成 → 开始识别
    connect(m_recorder, &AudioRecorder::recordingFinished,
            this, &VoiceInputDialog::onRecordingFinished);
    connect(m_recorder, &AudioRecorder::recordingError,
            this, [this](const QString& err) {
                m_statusLabel->setText("❌ " + err);
                m_statusLabel->setStyleSheet("color: #e53935; font-size: 13px;");
                m_recordBtn->setEnabled(true);
            });
}

VoiceInputDialog::~VoiceInputDialog() {
    if (m_recording) {
        m_recorder->stopRecording();
    }
    // 清理临时录音文件
    if (!m_audioFile.isEmpty() && QFile::exists(m_audioFile)) {
        QFile::remove(m_audioFile);
    }
}

// =========================== UI 构建 ===========================

void VoiceInputDialog::setupUI() {
    setWindowTitle("🎙️ 语音录入任务");
    setFixedSize(460, 520);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 15, 20, 15);

    // ===== 提示文字 =====
    QLabel* hintLabel = new QLabel("按住按钮说话，松开后自动识别\n"
                                   "例如：「明天下午三点学C++ 高优先级」");
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setStyleSheet("color: #666; font-size: 12px;");
    mainLayout->addWidget(hintLabel);

    // ===== 录音按钮 =====
    m_recordBtn = new QPushButton("🎤 按住录音");
    m_recordBtn->setMinimumHeight(55);
    m_recordBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #1976D2; color: white;"
        "  font-size: 16px; font-weight: bold;"
        "  border-radius: 10px; border: none;"
        "}"
        "QPushButton:hover { background-color: #1565C0; }"
        "QPushButton:pressed { background-color: #e53935; }"
    );
    m_recordBtn->setCursor(Qt::PointingHandCursor);

    // 使用 pressed/released 信号实现按住录音
    connect(m_recordBtn, &QPushButton::pressed,
            this, &VoiceInputDialog::onRecordPressed);
    connect(m_recordBtn, &QPushButton::released,
            this, &VoiceInputDialog::onRecordReleased);

    mainLayout->addWidget(m_recordBtn);

    // ===== 状态标签 =====
    m_statusLabel = new QLabel("准备就绪");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #888; font-size: 13px;");
    mainLayout->addWidget(m_statusLabel);

    // ===== 识别文本显示 =====
    QGroupBox* textGroup = new QGroupBox("识别结果");
    QVBoxLayout* textLayout = new QVBoxLayout();
    m_rawTextEdit = new QTextEdit();
    m_rawTextEdit->setReadOnly(true);
    m_rawTextEdit->setMaximumHeight(60);
    m_rawTextEdit->setPlaceholderText("识别到的文本将显示在这里…");
    m_rawTextEdit->setStyleSheet("background-color: #f5f5f5; font-size: 12px;");
    textLayout->addWidget(m_rawTextEdit);
    textGroup->setLayout(textLayout);
    mainLayout->addWidget(textGroup);

    // ===== 解析结果编辑区 =====
    QGroupBox* parseGroup = new QGroupBox("任务解析结果（可修改）");
    QGridLayout* parseLayout = new QGridLayout();

    // 任务名称
    parseLayout->addWidget(new QLabel("任务："), 0, 0);
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("识别出的任务名称");
    parseLayout->addWidget(m_nameEdit, 0, 1, 1, 4);

    // 日期
    parseLayout->addWidget(new QLabel("日期："), 1, 0);
    m_yearSp  = new QSpinBox(); m_yearSp->setRange(2026, 2030);
    m_monthSp = new QSpinBox(); m_monthSp->setRange(1, 12);
    m_daySp   = new QSpinBox(); m_daySp->setRange(1, 31);
    parseLayout->addWidget(m_yearSp,  1, 1);
    parseLayout->addWidget(new QLabel("年"), 1, 2);
    parseLayout->addWidget(m_monthSp, 1, 3);
    parseLayout->addWidget(new QLabel("月"), 1, 4);
    parseLayout->addWidget(m_daySp,   1, 5);
    parseLayout->addWidget(new QLabel("日"), 1, 6);

    // 时间
    parseLayout->addWidget(new QLabel("时间："), 2, 0);
    m_hourSp   = new QSpinBox(); m_hourSp->setRange(0, 23);
    m_minuteSp = new QSpinBox(); m_minuteSp->setRange(0, 59);
    m_hourSp->setValue(9);
    parseLayout->addWidget(m_hourSp,   2, 1);
    parseLayout->addWidget(new QLabel("时"), 2, 2);
    parseLayout->addWidget(m_minuteSp, 2, 3);
    parseLayout->addWidget(new QLabel("分"), 2, 4);

    // 优先级和分类
    parseLayout->addWidget(new QLabel("优先级："), 3, 0);
    m_priorityCmb = new QComboBox();
    m_priorityCmb->addItem("高", HIGH);
    m_priorityCmb->addItem("中", MEDIUM);
    m_priorityCmb->addItem("低", LOW);
    m_priorityCmb->setCurrentIndex(1);
    parseLayout->addWidget(m_priorityCmb, 3, 1, 1, 2);

    parseLayout->addWidget(new QLabel("分类："), 3, 3);
    m_classifyCmb = new QComboBox();
    m_classifyCmb->addItem("学习", STUDY);
    m_classifyCmb->addItem("娱乐", PLAY);
    m_classifyCmb->addItem("生活", LIFE);
    parseLayout->addWidget(m_classifyCmb, 3, 5, 1, 2);

    parseGroup->setLayout(parseLayout);
    mainLayout->addWidget(parseGroup);

    // ===== 按钮 =====
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_confirmBtn = new QPushButton("✓ 确认添加");
    m_confirmBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; "
        "padding: 8px 20px; border-radius: 5px; font-size: 13px; }"
        "QPushButton:hover { background-color: #43A047; }"
    );
    m_cancelBtn = new QPushButton("取消");
    m_cancelBtn->setStyleSheet(
        "QPushButton { padding: 8px 20px; border-radius: 5px; }"
    );
    connect(m_confirmBtn, &QPushButton::clicked, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "输入错误", "请输入任务名称！");
            return;
        }
        accept();
    });
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    btnLayout->addWidget(m_confirmBtn);
    btnLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(btnLayout);
}

// =========================== 录音控制 ===========================

void VoiceInputDialog::onRecordPressed() {
    if (m_recording) return;
    m_recording = true;

    // 临时录音文件
    m_audioFile = QDir::temp().absoluteFilePath("voice_task.wav");

    m_recordBtn->setText("🔴 正在录音…");
    m_recordBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #e53935; color: white;"
        "  font-size: 16px; font-weight: bold;"
        "  border-radius: 10px; border: none;"
        "}"
    );
    m_statusLabel->setText("🔴 录音中，请说话…");
    m_statusLabel->setStyleSheet("color: #e53935; font-size: 13px; font-weight: bold;");
    m_confirmBtn->setEnabled(false);

    QApplication::processEvents();
    m_recorder->startRecording(m_audioFile);
}

void VoiceInputDialog::onRecordReleased() {
    if (!m_recording) return;
    m_recording = false;

    m_recordBtn->setEnabled(false);
    m_recordBtn->setText("⏳ 识别中…");
    m_recordBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #FF9800; color: white;"
        "  font-size: 16px; font-weight: bold;"
        "  border-radius: 10px; border: none;"
        "}"
    );
    m_statusLabel->setText("⏳ 正在处理语音…");
    m_statusLabel->setStyleSheet("color: #FF9800; font-size: 13px;");

    QApplication::processEvents();
    m_recorder->stopRecording();
}

// =========================== 语音识别 & NLP 解析 ===========================

void VoiceInputDialog::onRecordingFinished(const QString& filePath) {
    m_statusLabel->setText("⏳ 语音识别中…");
    QApplication::processEvents();

    if (!m_recognizer || !m_recognizer->isReady()) {
        m_statusLabel->setText("❌ 语音识别引擎未就绪\n请安装: pip3 install faster-whisper");
        m_statusLabel->setStyleSheet("color: #e53935; font-size: 12px;");
        resetRecordButton();
        return;
    }

    // 第一步：Whisper 语音转文字
    QString text = m_recognizer->recognize(filePath);

    // 清理临时文件
    if (QFile::exists(filePath)) {
        QFile::remove(filePath);
    }

    if (text.isEmpty()) {
        m_statusLabel->setText("❌ 未能识别到语音内容，请重试");
        m_statusLabel->setStyleSheet("color: #e53935; font-size: 12px;");
        m_rawTextEdit->setPlainText("（无识别结果）");
        resetRecordButton();
        return;
    }

    // 显示原始识别文本
    m_rawTextEdit->setPlainText(text);

    // 第二步：DeepSeek NLP 解析
    parseText(text);
}

void VoiceInputDialog::parseText(const QString& text) {
    m_statusLabel->setText("🤖 AI 解析中…");
    m_statusLabel->setStyleSheet("color: #1976D2; font-size: 13px;");
    QApplication::processEvents();

    // 先用 DeepSeek API 解析
    ParsedTask parsed = m_deepseek->parseTask(text, QDate::currentDate());

    if (parsed.valid) {
        // DeepSeek 解析成功
        m_nameEdit->setText(parsed.taskName);
        m_yearSp->setValue(parsed.year);
        m_monthSp->setValue(parsed.month);
        m_daySp->setValue(parsed.day);
        m_hourSp->setValue(parsed.hour);
        m_minuteSp->setValue(parsed.minute);

        // 优先级
        if (parsed.priority == "high") m_priorityCmb->setCurrentIndex(0);
        else if (parsed.priority == "low") m_priorityCmb->setCurrentIndex(2);
        else m_priorityCmb->setCurrentIndex(1);

        // 分类
        if (parsed.classify == "study") m_classifyCmb->setCurrentIndex(0);
        else if (parsed.classify == "play") m_classifyCmb->setCurrentIndex(1);
        else if (parsed.classify == "life") m_classifyCmb->setCurrentIndex(2);

        m_statusLabel->setText("✅ AI 解析完成，请核对并修改");
        m_statusLabel->setStyleSheet("color: #4CAF50; font-size: 13px; font-weight: bold;");
    } else {
        // DeepSeek 失败 → 本地简单解析
        m_statusLabel->setText("⚠️ AI 解析失败，使用本地解析结果");
        m_statusLabel->setStyleSheet("color: #FF9800; font-size: 12px;");

        m_nameEdit->setText(extractTaskName(text));

        int yr, mo, dy, hr, mn;
        extractDateTime(text, yr, mo, dy, hr, mn);
        m_yearSp->setValue(yr);
        m_monthSp->setValue(mo);
        m_daySp->setValue(dy);
        m_hourSp->setValue(hr);
        m_minuteSp->setValue(mn);

        Priority pri = extractPriority(text);
        m_priorityCmb->setCurrentIndex((int)pri);

        Classify cls = extractClassify(text);
        m_classifyCmb->setCurrentIndex((int)cls);
    }

    m_confirmBtn->setEnabled(true);
    resetRecordButton();
}

void VoiceInputDialog::resetRecordButton() {
    m_recordBtn->setEnabled(true);
    m_recordBtn->setText("🎤 按住录音");
    m_recordBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #1976D2; color: white;"
        "  font-size: 16px; font-weight: bold;"
        "  border-radius: 10px; border: none;"
        "}"
        "QPushButton:hover { background-color: #1565C0; }"
        "QPushButton:pressed { background-color: #e53935; }"
    );
}

// =========================== 本地 NLP 解析（降级方案）===========================

QString VoiceInputDialog::extractTaskName(const QString& text) {
    QString t = text;

    // 去掉常见时间表达
    QStringList timePatterns = {
        "今天", "明天", "后天", "大后天",
        "上午", "下午", "晚上", "中午", "早晨", "傍晚",
        "下周一", "下周二", "下周三", "下周四", "下周五", "下周六", "下周日",
        "下星期", "下周",
        "周一", "周二", "周三", "周四", "周五", "周六", "周日",
        "星期", "礼拜",
    };

    for (const QString& pat : timePatterns) {
        t.replace(pat, "");
    }

    // 去掉数字时间（如 "三点"、"9点"、"15:00"）
    static QRegularExpression timeRe(
        R"((\d{1,2}[点:：]\d{0,2}[分]?|[零一二三四五六七八九十两]+点[零一二三四五六七八九十半]+分?))");
    t.replace(timeRe, "");

    // 去掉优先级/分类词
    QStringList attrWords = {
        "紧急", "高优先级", "重要", "不急", "低优先级", "不着急",
        "学习", "娱乐", "生活", "运动",
    };
    for (const QString& w : attrWords) {
        t.replace(w, "");
    }

    // 去掉多余空格
    t = t.simplified();

    return t.isEmpty() ? text.simplified() : t;
}

void VoiceInputDialog::extractDateTime(const QString& text,
                                        int& year, int& month, int& day,
                                        int& hour, int& minute) {
    QDate today = QDate::currentDate();
    year  = today.year();
    month = today.month();
    day   = today.day();
    hour  = 9;
    minute = 0;

    // 检测相对日期
    if (text.contains("明天"))      { QDate d = today.addDays(1);  month=d.month(); day=d.day(); }
    else if (text.contains("后天")) { QDate d = today.addDays(2);  month=d.month(); day=d.day(); }
    else if (text.contains("大后天")){ QDate d = today.addDays(3); month=d.month(); day=d.day(); }
    else if (text.contains("今天")) { /* 默认今天 */ }

    // 检测星期
    QStringList weekDays = {"周一","周二","周三","周四","周五","周六","周日"};
    for (int i = 0; i < weekDays.size(); ++i) {
        if (text.contains(weekDays[i])) {
            int targetDow = i + 1;
            int daysAhead = targetDow - today.dayOfWeek();
            if (daysAhead <= 0) daysAhead += 7;
            QDate d = today.addDays(daysAhead);
            month = d.month(); day = d.day();
            break;
        }
    }

    // 检测时间
    static QRegularExpression hourRe(R"((\d{1,2})\s*[点时:：]\s*(\d{0,2}))");
    auto match = hourRe.match(text);
    if (match.hasMatch()) {
        int h = match.captured(1).toInt();
        int m = match.captured(2).isEmpty() ? 0 : match.captured(2).toInt();
        // 下午/晚上 +12
        if ((text.contains("下午") || text.contains("晚上")) && h <= 12 && h >= 1) {
            if (h != 12) h += 12;
        } else if (text.contains("中午") && h >= 1 && h <= 2) {
            h += 12;
        }
        hour = h;
        minute = m;
    } else {
        // 中文数字
        if (text.contains("上午")) { hour = 9; }
        else if (text.contains("下午")) { hour = 15; }
        else if (text.contains("晚上")) { hour = 20; }
        else if (text.contains("中午")) { hour = 12; }
    }

    // 范围约束
    year  = qBound(2026, year, 2030);
    month = qBound(1, month, 12);
    day   = qBound(1, day, 31);
    hour  = qBound(0, hour, 23);
    minute= qBound(0, minute, 59);
}

Priority VoiceInputDialog::extractPriority(const QString& text) {
    if (text.contains("紧急") || text.contains("高优先级") || text.contains("重要"))
        return HIGH;
    if (text.contains("不急") || text.contains("低优先级") || text.contains("不着急"))
        return LOW;
    return MEDIUM;
}

Classify VoiceInputDialog::extractClassify(const QString& text) {
    if (text.contains("学习") || text.contains("作业") || text.contains("课程") ||
        text.contains("考试") || text.contains("复习") || text.contains("看书"))
        return STUDY;
    if (text.contains("娱乐") || text.contains("玩") || text.contains("游戏") ||
        text.contains("电影") || text.contains("音乐") || text.contains("运动") ||
        text.contains("锻炼"))
        return PLAY;
    if (text.contains("生活") || text.contains("购物") || text.contains("吃饭") ||
        text.contains("睡觉") || text.contains("约会") || text.contains("日常"))
        return LIFE;
    return STUDY;
}

// =========================== 获取结果 ===========================

Task VoiceInputDialog::getTask() const {
    Time st(m_yearSp->value(), m_monthSp->value(), m_daySp->value(),
            m_hourSp->value(), m_minuteSp->value());

    // 提醒时间 = 开始时间前 5 分钟
    Time rt = st;
    rt.minute -= 5;
    if (rt.minute < 0) {
        rt.minute += 60;
        rt.hour -= 1;
        if (rt.hour < 0) {
            rt.hour = 23;
            rt.day -= 1;
        }
    }

    Priority p = (Priority)m_priorityCmb->currentData().toInt();
    Classify c = (Classify)m_classifyCmb->currentData().toInt();

    return Task(m_nameEdit->text().trimmed().toStdString(), st, rt, p, c);
}
