/**
 * @brief AudioRecorder - Linux WAV 录音器
 *
 * 通过 QProcess 调用 arecord 录制 WAV 音频。
 * arecord 是 ALSA 工具包的一部分，Ubuntu 默认安装。
 *
 * 录音参数：
 *   - 格式: WAV (16-bit, 16kHz, 单声道)
 *   - 16kHz 单声道满足 Whisper 语音识别的最低要求
 *
 * 使用方式：
 *   AudioRecorder* rec = new AudioRecorder(this);
 *   connect(rec, &AudioRecorder::recordingFinished,
 *           this, &MyClass::onRecordDone);
 *   rec->startRecording("/tmp/voice.wav");
 *   // ... 用户操作 ...
 *   rec->stopRecording();  // 信号发出时文件已写入完毕
 */
#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QObject>
#include <QProcess>
#include <QString>

class AudioRecorder : public QObject {
    Q_OBJECT

public:
    explicit AudioRecorder(QObject* parent = nullptr);
    ~AudioRecorder();

    /** 开始录音，写入指定 WAV 文件路径 */
    void startRecording(const QString& filePath);

    /** 停止录音，触发 recordingFinished 信号 */
    void stopRecording();

    /** 是否正在录音 */
    bool isRecording() const { return m_recording; }

signals:
    /** 录音完成，filePath 为 WAV 文件路径 */
    void recordingFinished(const QString& filePath);

    /** 录音出错 */
    void recordingError(const QString& error);

private:
    QProcess* m_process;
    QString   m_filePath;
    bool      m_recording;
};

#endif // AUDIORECORDER_H
