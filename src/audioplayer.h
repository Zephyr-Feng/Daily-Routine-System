/**
 * @brief AudioPlayer - 音频播放器（任务提醒音乐播放）
 *
 * 功能：
 *  1. 播放默认提醒音（自动生成蜂鸣 WAV）
 *  2. 播放用户自定义提醒音（.wav / .mp3）
 *  3. 双重方案：Qt Multimedia + paplay 回退
 *
 * 使用：
 *  AudioPlayer player;
 *  player.playReminder();           // 默认蜂鸣
 *  player.playReminder("alert.wav"); // 自定义音频
 */
#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QString>

class AudioPlayer : public QObject {
    Q_OBJECT

public:
    explicit AudioPlayer(QObject* parent = nullptr);
    ~AudioPlayer();

    /** 播放提醒音（空参数 = 默认蜂鸣声）*/
    void playReminder(const QString& customFile = "");

    /** 停止播放 */
    void stop();

    /** 是否正在播放 */
    bool isPlaying() const;

    /** 获取默认提示音文件路径 */
    QString defaultSoundPath() const { return m_defaultSoundPath; }

    /** 检查 Qt Multimedia 是否可用 */
    bool multimediaAvailable() const { return !m_useFallback; }

private:
    QMediaPlayer* m_player;
    QString m_defaultSoundPath;
    bool m_useFallback;  // true = 退回使用系统命令

    /** 生成默认蜂鸣音 WAV 文件（440Hz 正弦波 0.5秒）*/
    void generateDefaultSound();

    /** 退路方案：调用系统命令播放 */
    void playFallback(const QString& filePath);
};

#endif // AUDIOPLAYER_H
