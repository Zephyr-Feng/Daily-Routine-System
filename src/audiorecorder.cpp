/**
 * @brief AudioRecorder 实现 — arecord 录音
 *
 * arecord 参数说明：
 *   -f cd  : 16-bit little-endian, 44100Hz, stereo → 改为手动指定
 *   -f S16_LE : 16-bit 采样
 *   -r 16000  : 16000Hz 采样率（Whisper 推荐）
 *   -c 1      : 单声道
 *   -t wav    : WAV 格式输出
 */
#include "audiorecorder.h"
#include <QDebug>
#include <csignal>

AudioRecorder::AudioRecorder(QObject* parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_recording(false)
{
}

AudioRecorder::~AudioRecorder() {
    if (m_recording) {
        stopRecording();
    }
}

void AudioRecorder::startRecording(const QString& filePath) {
    if (m_recording) {
        qWarning() << "已经在录音中";
        return;
    }

    m_filePath = filePath;
    m_recording = true;

    // arecord: 16kHz 16-bit 单声道 WAV
    // -q 静默模式，不打印信息
    QStringList args;
    args << "-q"
         << "-f" << "S16_LE"
         << "-r" << "16000"
         << "-c" << "1"
         << "-t" << "wav"
         << filePath;

    qDebug() << "开始录音:" << filePath;
    m_process->start("arecord", args);

    if (!m_process->waitForStarted(3000)) {
        m_recording = false;
        qWarning() << "arecord 启动失败:" << m_process->errorString();
        emit recordingError("录音设备启动失败：" + m_process->errorString());
    }
}

void AudioRecorder::stopRecording() {
    if (!m_recording) {
        return;
    }

    qDebug() << "停止录音:" << m_filePath;

    // 发送 SIGINT 信号优雅地终止 arecord
    // arecord 收到 SIGINT 后会正确写入 WAV 头并退出
    m_process->terminate();

    if (!m_process->waitForFinished(5000)) {
        qWarning() << "arecord 未能正常退出，强制终止";
        m_process->kill();
        m_process->waitForFinished(2000);
    }

    m_recording = false;

    if (m_process->exitStatus() == QProcess::NormalExit) {
        qDebug() << "录音已保存到:" << m_filePath;
        emit recordingFinished(m_filePath);
    } else {
        qWarning() << "录音异常退出";
        emit recordingError("录音过程异常退出");
    }
}
