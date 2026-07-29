/**
 * @brief VoiceInputDialog - 语音录入任务对话框
 *
 * 交互流程：
 *  1. 用户按住 🎤 按钮开始录音
 *  2. 松开按钮停止录音
 *  3. 自动调用 Vosk 离线语音识别
 *  4. 解析中文文本 → 提取任务名称/时间/优先级/分类
 *  5. 显示解析结果，用户确认后添加
 *
 * 支持的自然语言模式：
 *  - "明天上午九点学习C++"       → 提取时间+任务名
 *  - "下午三点开会 高优先级"      → 提取时间+优先级
 *  - "周五晚上八点看电影 娱乐"    → 提取时间+分类
 */
#ifndef VOICEINPUTDIALOG_H
#define VOICEINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QTimer>
#include "audiorecorder.h"
#include "speechrecognizer.h"
#include "deepseekclient.h"
#include "task.h"

class VoiceInputDialog : public QDialog {
    Q_OBJECT

public:
    explicit VoiceInputDialog(SpeechRecognizer* recognizer,
                              QWidget* parent = nullptr);
    ~VoiceInputDialog();

    /** 获取解析出的任务对象 */
    Task getTask() const;

    /** 设置语音命令模式（识别特定命令）*/
    QString getCommand() const { return m_command; }
    bool isCommandMode() const { return m_isCommand; }

private slots:
    /** 按下录音按钮 */
    void onRecordPressed();

    /** 松开录音按钮 */
    void onRecordReleased();

    /** 录音完成，开始识别 */
    void onRecordingFinished(const QString& filePath);

    /** 解析中文文本为任务参数 */
    void parseText(const QString& text);

    /** 重置录音按钮样式和状态 */
    void resetRecordButton();

private:
    void setupUI();

    // ===== 自然语言解析 =====
    /** 从中文文本提取任务名 */
    QString extractTaskName(const QString& text);

    /** 从中文文本提取日期时间 */
    void extractDateTime(const QString& text,
                         int& year, int& month, int& day,
                         int& hour, int& minute);

    /** 从中文文本提取优先级 */
    Priority extractPriority(const QString& text);

    /** 从中文文本提取分类 */
    Classify extractClassify(const QString& text);

    // ===== UI 控件 =====
    QPushButton*   m_recordBtn;       // 录音按钮
    QLabel*        m_statusLabel;     // 状态提示
    QTextEdit*     m_rawTextEdit;     // 显示原始识别文本

    // 解析结果编辑区
    QLineEdit*     m_nameEdit;
    QSpinBox*      m_yearSp;
    QSpinBox*      m_monthSp;
    QSpinBox*      m_daySp;
    QSpinBox*      m_hourSp;
    QSpinBox*      m_minuteSp;
    QComboBox*     m_priorityCmb;
    QComboBox*     m_classifyCmb;

    QPushButton*   m_confirmBtn;
    QPushButton*   m_cancelBtn;

    // ===== 核心组件 =====
    AudioRecorder*    m_recorder;
    SpeechRecognizer* m_recognizer;
    DeepSeekClient*   m_deepseek;     // DeepSeek AI 解析器

    QString m_audioFile;    // 临时录音文件路径
    QString m_command;      // 命令模式识别的命令
    bool    m_isCommand;    // 是否为语音命令
    bool    m_recording;
};

#endif // VOICEINPUTDIALOG_H
