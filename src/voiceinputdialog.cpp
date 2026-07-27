/**
 * @brief VoiceInputDialog 实现 — 按住录音 + 自动识别 + 中文解析
 */
#include "voiceinputdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QDateTime>
#include <QDir>
#include <QCoreApplication>
#include <QMessageBox>
#include <QDebug>
#include <QRegExp>

VoiceInputDialog::VoiceInputDialog(SpeechRecognizer* recognizer, QWidget* parent)
    : QDialog(parent)
    , m_recorder(new AudioRecorder(this))
    , m_recognizer(recognizer)
    , m_deepseek(new DeepSeekClient(this))
    , m_recording(false)
    , m_isCommand(false)
{
    // ===== 设置 DeepSeek API Key =====
    m_deepseek->setApiKey("sk-f7008524c8f34432811a93f6987239fb");

    setupUI();

    // 临时录音文件
    m_audioFile = QCoreApplication::applicationDirPath() + "/_temp_voice.wav";

    // 录音完成后 → 自动识别
    connect(m_recorder, &AudioRecorder::recordingFinished,
            this, &VoiceInputDialog::onRecordingFinished);
}

VoiceInputDialog::~VoiceInputDialog() {
    // 清理临时文件
    QFile::remove(m_audioFile);
}

// ========== 界面 ==========

void VoiceInputDialog::setupUI() {
    setWindowTitle("🎤 语音录入任务");
    setMinimumSize(480, 520);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    // ===== 提示文字 =====
    QLabel* tipLabel = new QLabel(
        "<div style='text-align:center; color:#666;'>"
        "按住按钮说出任务，松开自动识别<br>"
        "例如：<b>明天上午九点学习C++</b></div>"
    );
    tipLabel->setWordWrap(true);
    mainLayout->addWidget(tipLabel);

    // ===== 麦克风按钮 =====
    m_recordBtn = new QPushButton("🎤 按住说话");
    m_recordBtn->setMinimumHeight(80);
    m_recordBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #2196F3; color: white;"
        "  font-size: 20px; font-weight: bold;"
        "  border-radius: 40px; border: none;"
        "  padding: 20px;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #D32F2F;"  // 按住变红
        "}"
    );
    // 按下开始录音，松开停止
    connect(m_recordBtn, &QPushButton::pressed,
            this, &VoiceInputDialog::onRecordPressed);
    connect(m_recordBtn, &QPushButton::released,
            this, &VoiceInputDialog::onRecordReleased);
    mainLayout->addWidget(m_recordBtn);

    // ===== 状态 =====
    m_statusLabel = new QLabel("就绪，请按住按钮说话");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #666; font-size: 13px;");
    mainLayout->addWidget(m_statusLabel);

    // ===== 识别原始文本 =====
    QLabel* rawLabel = new QLabel("识别结果：");
    mainLayout->addWidget(rawLabel);
    m_rawTextEdit = new QTextEdit();
    m_rawTextEdit->setReadOnly(true);
    m_rawTextEdit->setMaximumHeight(60);
    m_rawTextEdit->setPlaceholderText("识别出的文本将显示在这里...");
    mainLayout->addWidget(m_rawTextEdit);

    // ===== 解析结果编辑区 =====
    QGroupBox* editGroup = new QGroupBox("解析结果（可手动修改）");
    QGridLayout* editLayout = new QGridLayout(editGroup);

    editLayout->addWidget(new QLabel("任务名称："), 0, 0);
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("语音识别出的任务名");
    editLayout->addWidget(m_nameEdit, 0, 1, 1, 4);

    editLayout->addWidget(new QLabel("日期："), 1, 0);
    m_yearSp  = new QSpinBox(); m_yearSp->setRange(2026, 2030);
    m_monthSp = new QSpinBox(); m_monthSp->setRange(1, 12);
    m_daySp   = new QSpinBox(); m_daySp->setRange(1, 31);
    QDate today = QDate::currentDate();
    m_yearSp->setValue(today.year());
    m_monthSp->setValue(today.month());
    m_daySp->setValue(today.day());
    editLayout->addWidget(m_yearSp, 1, 1);
    editLayout->addWidget(new QLabel("年"), 1, 1, Qt::AlignRight);
    editLayout->addWidget(m_monthSp, 1, 2);
    editLayout->addWidget(new QLabel("月"), 1, 2, Qt::AlignRight);
    editLayout->addWidget(m_daySp, 1, 3);
    editLayout->addWidget(new QLabel("日"), 1, 3, Qt::AlignRight);

    editLayout->addWidget(new QLabel("时间："), 2, 0);
    m_hourSp   = new QSpinBox(); m_hourSp->setRange(0, 23);
    m_minuteSp = new QSpinBox(); m_minuteSp->setRange(0, 59);
    m_hourSp->setValue(9);
    editLayout->addWidget(m_hourSp, 2, 1);
    editLayout->addWidget(new QLabel("时"), 2, 1, Qt::AlignRight);
    editLayout->addWidget(m_minuteSp, 2, 2);
    editLayout->addWidget(new QLabel("分"), 2, 2, Qt::AlignRight);

    editLayout->addWidget(new QLabel("优先级："), 3, 0);
    m_priorityCmb = new QComboBox();
    m_priorityCmb->addItem("低", LOW);
    m_priorityCmb->addItem("中", MEDIUM);
    m_priorityCmb->addItem("高", HIGH);
    m_priorityCmb->setCurrentIndex(1);
    editLayout->addWidget(m_priorityCmb, 3, 1, 1, 2);

    editLayout->addWidget(new QLabel("分类："), 3, 2);
    m_classifyCmb = new QComboBox();
    m_classifyCmb->addItem("学习", STUDY);
    m_classifyCmb->addItem("娱乐", PLAY);
    m_classifyCmb->addItem("生活", LIFE);
    editLayout->addWidget(m_classifyCmb, 3, 3, 1, 2);

    mainLayout->addWidget(editGroup);

    // ===== 按钮 =====
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_confirmBtn = new QPushButton("✓ 确认添加");
    m_confirmBtn->setProperty("primary", true);
    m_confirmBtn->setMinimumHeight(34);
    m_confirmBtn->setCursor(Qt::PointingHandCursor);

    m_cancelBtn = new QPushButton("取消");
    m_cancelBtn->setMinimumHeight(34);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setStyleSheet(
        "QPushButton { color: #667085; }"
        "QPushButton:hover { background: #F0F3F8; }"
    );
    btnLayout->addStretch();
    btnLayout->addWidget(m_confirmBtn);
    btnLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_confirmBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelBtn,  &QPushButton::clicked, this, &QDialog::reject);
}

// ========== 录音控制 ==========

void VoiceInputDialog::onRecordPressed() {
    if (!m_recognizer->isReady()) {
        QMessageBox::warning(this, "不可用",
            "语音识别引擎未就绪！\n请确认：\n"
            "1. pip3 install vosk\n"
            "2. 下载中文模型到项目目录");
        return;
    }

    m_recording = true;
    m_recordBtn->setText("🔴 正在录音...松开识别");
    m_statusLabel->setText("录音中...");
    m_statusLabel->setStyleSheet("color: #D32F2F; font-size: 13px; font-weight: bold;");

    // 删除旧临时文件
    QFile::remove(m_audioFile);
    m_recorder->startRecording(m_audioFile);

    qDebug() << "🎤 开始录音 →" << m_audioFile;
}

void VoiceInputDialog::onRecordReleased() {
    if (!m_recording) return;

    m_recording = false;
    m_recordBtn->setText("🔵 正在识别...");
    m_recordBtn->setEnabled(false);
    m_statusLabel->setText("识别中，请稍候...");
    m_statusLabel->setStyleSheet("color: #2196F3; font-size: 13px;");

    m_recorder->stopRecording();

    qDebug() << "🎤 停止录音，开始识别";
}

// ========== 识别回调 ==========

void VoiceInputDialog::onRecordingFinished(const QString& filePath) {
    qDebug() << "录音文件：" << filePath;

    // 检查文件大小
    QFileInfo fi(filePath);
    if (fi.size() < 1000) {
        m_statusLabel->setText("❌ 录音太短，请重新录制");
        m_statusLabel->setStyleSheet("color: #D32F2F; font-size: 13px;");
        m_recordBtn->setText("🎤 按住说话");
        m_recordBtn->setEnabled(true);
        return;
    }

    // ===== 调用 Vosk 识别 =====
    QString text = m_recognizer->recognize(filePath);

    m_rawTextEdit->setPlainText(text);
    m_recordBtn->setText("🎤 按住说话");
    m_recordBtn->setEnabled(true);

    if (text.isEmpty() || text == "[未识别到语音内容]") {
        m_statusLabel->setText("⚠ 未识别到内容，请重新录制");
        m_statusLabel->setStyleSheet("color: #FF9800; font-size: 13px;");
        return;
    }

    m_statusLabel->setText("✅ 识别完成");
    m_statusLabel->setStyleSheet("color: #4CAF50; font-size: 13px;");

    // 解析文本
    parseText(text);
}

// ========== 中文自然语言解析（DeepSeek AI + 正则回退）==========

void VoiceInputDialog::parseText(const QString& text) {
    // ===== 第一步：尝试 DeepSeek AI 解析 =====
    ParsedTask parsed = m_deepseek->parseTask(text);

    if (parsed.valid) {
        // DeepSeek 解析成功
        m_nameEdit->setText(parsed.taskName);
        m_yearSp->setValue(parsed.year);
        m_monthSp->setValue(parsed.month);
        m_daySp->setValue(parsed.day);
        m_hourSp->setValue(parsed.hour);
        m_minuteSp->setValue(parsed.minute);

        // 优先级映射
        if (parsed.priority == "high") {
            m_priorityCmb->setCurrentIndex(m_priorityCmb->findData(HIGH));
        } else if (parsed.priority == "low") {
            m_priorityCmb->setCurrentIndex(m_priorityCmb->findData(LOW));
        } else {
            m_priorityCmb->setCurrentIndex(m_priorityCmb->findData(MEDIUM));
        }

        // 分类映射
        if (parsed.classify == "play") {
            m_classifyCmb->setCurrentIndex(m_classifyCmb->findData(PLAY));
        } else if (parsed.classify == "life") {
            m_classifyCmb->setCurrentIndex(m_classifyCmb->findData(LIFE));
        } else {
            m_classifyCmb->setCurrentIndex(m_classifyCmb->findData(STUDY));
        }

        m_statusLabel->setText("✅ AI 解析完成（可手动修改）");
        m_statusLabel->setStyleSheet("color: #4CAF50; font-size: 13px;");
        qDebug() << "🤖 DeepSeek 解析成功";
        return;
    }

    // ===== 第二步：DeepSeek 失败，回退到正则解析 =====
    qDebug() << "⚠ DeepSeek 解析失败，回退到正则匹配";
    m_statusLabel->setText("⚠ AI 暂不可用，使用本地解析（可手动修改）");
    m_statusLabel->setStyleSheet("color: #FF9800; font-size: 13px;");

    // 提取任务名
    QString name = extractTaskName(text);
    m_nameEdit->setText(name.isEmpty() ? text : name);

    // 提取日期时间
    int y, mo, d, h, mi;
    extractDateTime(text, y, mo, d, h, mi);
    m_yearSp->setValue(y);
    m_monthSp->setValue(mo);
    m_daySp->setValue(d);
    m_hourSp->setValue(h);
    m_minuteSp->setValue(mi);

    // 提取优先级
    Priority pri = extractPriority(text);
    int priIdx = m_priorityCmb->findData((int)pri);
    if (priIdx >= 0) m_priorityCmb->setCurrentIndex(priIdx);

    // 提取分类
    Classify cls = extractClassify(text);
    int clsIdx = m_classifyCmb->findData((int)cls);
    if (clsIdx >= 0) m_classifyCmb->setCurrentIndex(clsIdx);
}

QString VoiceInputDialog::extractTaskName(const QString& text) {
    QString result = text;

    // 去掉时间相关关键词
    QStringList timeWords = {
        "明天", "后天", "今天", "大后天",
        "上午", "下午", "晚上", "中午", "早上", "傍晚",
        "周一", "周二", "周三", "周四", "周五", "周六", "周日",
        "星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日",
    };
    for (const auto& w : timeWords) {
        result.replace(w, "");
    }

    // 去掉数字时间如 "九点"、"三点半"、"9点"、"14:30"
    QRegExp timeRe("(零|一|二|三|四|五|六|七|八|九|十|两|\\d+)+[点:：](半|(零|一|二|三|四|五|六|七|八|九|十|\\d+)*[分]?)?");
    result.replace(timeRe, "");

    // 去掉优先级关键词
    result.replace(QRegExp("高优先级|紧急|重要|低优先级"), "");

    // 去掉分类关键词（如果它们出现在末尾）
    result.replace(QRegExp("学习|娱乐|生活"), "");

    // 去掉空白
    result = result.simplified().trimmed();

    return result;
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

    // ===== 处理"今天"/"明天"/"后天" =====
    if (text.contains("后天") || text.contains("後天")) {
        today = today.addDays(2);
    } else if (text.contains("明天")) {
        today = today.addDays(1);
    } else if (text.contains("大后天") || text.contains("大後天")) {
        today = today.addDays(3);
    }
    // "今天"不改变日期

    // ===== 处理星期几 =====
    QStringList weekdaysCN = {"周一", "周二", "周三", "周四", "周五", "周六", "周日",
                              "星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"};
    for (int i = 0; i < weekdaysCN.size(); i++) {
        if (text.contains(weekdaysCN[i])) {
            int targetDow = (i % 7) + 1;  // Mon=1, Sun=7
            int currentDow = today.dayOfWeek();
            int diff = targetDow - currentDow;
            if (diff <= 0) diff += 7;  // 本周或下周
            today = today.addDays(diff);
            break;
        }
    }

    year  = today.year();
    month = today.month();
    day   = today.day();

    // ===== 处理"上午"/"下午"/"晚上" =====
    int baseHour = 0;
    if (text.contains("下午")) {
        baseHour = 12;
    } else if (text.contains("晚上") || text.contains("傍晚")) {
        baseHour = 12;
    }
    // "上午"/"早上"/"中午" baseHour = 0

    // ===== 提取具体时间 =====
    // 匹配: "九点" "9点" "三点半" "14:30" "九点十五分"等
    QRegExp timeRe("(\\S*?)点(半|(\\S*?)分)?");
    if (timeRe.indexIn(text) >= 0) {
        QString hourStr = timeRe.cap(1);
        bool isHalf = (timeRe.cap(2) == "半");
        QString minuteStr = timeRe.cap(3);

        // 中文数字转阿拉伯
        auto cnToInt = [](const QString& s) -> int {
            // 简单映射
            if (s == "零" || s == "0") return 0;
            if (s == "一" || s == "1") return 1;
            if (s == "二" || s == "两" || s == "2") return 2;
            if (s == "三" || s == "3") return 3;
            if (s == "四" || s == "4") return 4;
            if (s == "五" || s == "5") return 5;
            if (s == "六" || s == "6") return 6;
            if (s == "七" || s == "7") return 7;
            if (s == "八" || s == "8") return 8;
            if (s == "九" || s == "9") return 9;
            if (s == "十" || s == "10") return 10;
            bool ok;
            int val = s.toInt(&ok);
            if (ok) return val;
            return 0;
        };

        int h = cnToInt(hourStr);
        // 修正：如果 h < 8 且是 12 小时制，这是下午时段
        if (baseHour == 12 && h <= 12) {
            h += 12;
        }
        if (h < 8) h += 12;  // "九点"在上午是9，在下午是21

        // 修正hour范围
        if (h > 23) h = h % 24;

        hour = h;
        minute = isHalf ? 30 : cnToInt(minuteStr);
    }

    // ===== 匹配 "14:30" 格式 =====
    QRegExp colonRe("(\\d{1,2}):(\\d{2})");
    if (colonRe.indexIn(text) >= 0) {
        hour = colonRe.cap(1).toInt();
        minute = colonRe.cap(2).toInt();
    }
}

Priority VoiceInputDialog::extractPriority(const QString& text) {
    if (text.contains("高优先级") || text.contains("紧急") || text.contains("重要"))
        return HIGH;
    if (text.contains("低优先级") || text.contains("不急"))
        return LOW;
    return MEDIUM;
}

Classify VoiceInputDialog::extractClassify(const QString& text) {
    if (text.contains("学习") || text.contains("课程") || text.contains("作业"))
        return STUDY;
    if (text.contains("娱乐") || text.contains("玩") || text.contains("游戏")
        || text.contains("电影") || text.contains("音乐"))
        return PLAY;
    if (text.contains("生活") || text.contains("日常") || text.contains("购物")
        || text.contains("吃饭"))
        return LIFE;
    return STUDY;  // 默认学习
}

// ========== 获取任务 ==========

Task VoiceInputDialog::getTask() const {
    Time st(m_yearSp->value(), m_monthSp->value(), m_daySp->value(),
            m_hourSp->value(), m_minuteSp->value());

    // 提醒时间 = 开始时间前5分钟
    Time rt = st;
    rt.minute -= 5;
    if (rt.minute < 0) {
        rt.minute += 60;
        rt.hour -= 1;
        if (rt.hour < 0) {
            rt.hour = 0;
        }
    }

    Priority p  = (Priority)m_priorityCmb->currentData().toInt();
    Classify c  = (Classify)m_classifyCmb->currentData().toInt();

    return Task(m_nameEdit->text().trimmed().toStdString(), st, rt, p, c);
}
