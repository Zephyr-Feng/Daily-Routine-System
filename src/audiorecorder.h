/**
 * @brief AudioRecorder - 录音器（录制自定义任务提醒音）
 *
 * 功能：
 *  1. 通过系统麦克风录音为 WAV 文件
 *  2. 双重方案：Qt Multimedia QAudioRecorder + arecord 回退
 *  3. 信号通知录音完成/错误
 *
 * 使用：
 *  AudioRecorder recorder;
 *  recorder.startRecording("my_reminder.wav");
 *  // ... 用户说完 ...
 *  recorder.stopRecording();
 */
#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QObject>
#include <QAudioRecorder>
#include <QAudioEncoderSettings>
#include <QProcess>
#include <QString>

class AudioRecorder : public QObject {
    Q_OBJECT

public:
    explicit AudioRecorder(QObject* parent = nullptr);
    ~AudioRecorder();

    /** 开始录音到指定文件 */
    void startRecording(const QString& outputPath);

    /** 停止录音并保存 */
    void stopRecording();

    /** 是否正在录音 */
    bool isRecording() const { return m_recording; }

signals:
    /** 录音完成，返回文件路径 */
    void recordingFinished(const QString& filePath);

    /** 录音出错 */
    void recordingError(const QString& error);

private:
    QAudioRecorder* m_recorder;
    QProcess* m_fallbackProcess;
    QString m_outputPath;
    bool m_recording;
    bool m_useFallback;

    /** 退路方案：用 arecord 命令行录音 */
    void startFallbackRecording(const QString& outputPath);
    void stopFallbackRecording();
};

#endif // AUDIORECORDER_H
