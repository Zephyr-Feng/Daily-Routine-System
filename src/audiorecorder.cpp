/**
 * @brief AudioRecorder 实现 — 录音（Qt Multimedia + arecord 回退）
 *
 * 技术方案：
 *  首选：Qt Multimedia QAudioRecorder（集成好，自动检测麦克风）
 *  回退：QProcess 调用 arecord（ALSA 命令行录音工具）
 *
 * 输出格式：16kHz 16-bit 单声道 WAV（适合语音）
 */
#include "audiorecorder.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QUrl>

AudioRecorder::AudioRecorder(QObject* parent)
    : QObject(parent)
    , m_recorder(nullptr)
    , m_fallbackProcess(nullptr)
    , m_recording(false)
    , m_useFallback(false)
{
    // ===== 方案1：初始化 Qt Multimedia 录音器 =====
    m_recorder = new QAudioRecorder(this);

    // 检查可用音频输入设备
    QStringList inputs = m_recorder->audioInputs();
    if (inputs.isEmpty()) {
        qWarning() << "未检测到音频输入设备，切换为 arecord 回退方案";
        m_recorder->deleteLater();
        m_recorder = nullptr;
        m_useFallback = true;
    } else {
        qDebug() << "可用音频输入：" << inputs;
        m_recorder->setAudioInput(inputs.first());

        // ===== 编码设置 =====
        QAudioEncoderSettings settings;
        settings.setCodec("audio/pcm");       // PCM 原始音频
        settings.setSampleRate(16000);        // 16kHz（适合语音识别）
        settings.setChannelCount(1);          // 单声道
        settings.setQuality(QMultimedia::HighQuality);
        m_recorder->setEncodingSettings(settings);
        m_recorder->setContainerFormat("audio/x-wav");  // WAV 容器
    }
}

AudioRecorder::~AudioRecorder() {
    if (m_recording) {
        stopRecording();
    }
}

// ========== 开始录音 ==========

void AudioRecorder::startRecording(const QString& outputPath) {
    if (m_recording) {
        qWarning() << "已在录音中，请先停止当前录音";
        return;
    }

    // 确保输出目录存在
    QFileInfo fi(outputPath);
    QDir().mkpath(fi.absolutePath());

    m_outputPath = outputPath;
    m_recording = true;

    if (m_useFallback) {
        startFallbackRecording(outputPath);
    } else {
        m_recorder->setOutputLocation(QUrl::fromLocalFile(outputPath));
        m_recorder->record();

        qDebug() << "开始录音 →" << outputPath;
    }
}

// ========== 停止录音 ==========

void AudioRecorder::stopRecording() {
    if (!m_recording) return;

    if (m_useFallback) {
        stopFallbackRecording();
    } else {
        m_recorder->stop();
        qDebug() << "录音已停止 →" << m_outputPath;
    }

    m_recording = false;
    emit recordingFinished(m_outputPath);
}

// ========== 退路方案：arecord ==========

void AudioRecorder::startFallbackRecording(const QString& outputPath) {
    m_fallbackProcess = new QProcess(this);

    // arecord 参数：-r 采样率 -f 格式 -c 声道
    m_fallbackProcess->setProgram("arecord");
    m_fallbackProcess->setArguments({
        "-r", "16000",        // 16kHz 采样率
        "-f", "S16_LE",       // 16-bit 小端
        "-c", "1",            // 单声道
        "-t", "wav",          // WAV 格式
        outputPath
    });

    m_fallbackProcess->start();
    qDebug() << "开始录音(arecord) →" << outputPath;
}

void AudioRecorder::stopFallbackRecording() {
    if (m_fallbackProcess && m_fallbackProcess->state() != QProcess::NotRunning) {
        m_fallbackProcess->terminate();  // 发送 SIGTERM
        m_fallbackProcess->waitForFinished(3000);
        qDebug() << "录音已停止(arecord) →" << m_outputPath;
    }
}
