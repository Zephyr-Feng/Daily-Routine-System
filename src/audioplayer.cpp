/**
 * @brief AudioPlayer 实现 — 音频播放（Qt Multimedia + paplay 回退）
 *
 * 技术方案：
 *  首选：Qt Multimedia QMediaPlayer（集成好，支持 WAV/MP3/OGG）
 *  回退：QProcess 调用 paplay（PulseAudio 命令行播放器）
 *
 * 默认提示音：
 *  首次运行时生成 default_reminder.wav（440Hz 正弦波，0.5 秒）
 *  文件放在可执行文件同目录下
 */
#include "audioplayer.h"
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <cmath>
#include <cstring>

AudioPlayer::AudioPlayer(QObject* parent)
    : QObject(parent)
    , m_player(nullptr)
    , m_useFallback(false)
{
    // ===== 方案1：尝试初始化 Qt Multimedia =====
    m_player = new QMediaPlayer(this, QMediaPlayer::LowLatency);

    if (m_player->error() != QMediaPlayer::NoError) {
        qWarning() << "QMediaPlayer 初始化失败，使用 paplay 回退方案";
        m_player->deleteLater();
        m_player = nullptr;
        m_useFallback = true;
    }

    // ===== 检查/生成默认提示音 =====
    m_defaultSoundPath = QCoreApplication::applicationDirPath()
                         + "/default_reminder.wav";

    if (!QFile::exists(m_defaultSoundPath)) {
        generateDefaultSound();
    }
}

AudioPlayer::~AudioPlayer() {
    stop();
}

bool AudioPlayer::isPlaying() const {
    if (m_player) {
        return m_player->state() == QMediaPlayer::PlayingState;
    }
    return false;
}

// ========== 播放提醒 ==========

void AudioPlayer::playReminder(const QString& customFile) {
    QString filePath = customFile.isEmpty() ? m_defaultSoundPath : customFile;

    // 检查文件是否存在
    if (!QFile::exists(filePath)) {
        qWarning() << "音频文件不存在：" << filePath;
        return;
    }

    // 先停止当前播放
    stop();

    if (m_useFallback) {
        playFallback(filePath);
    } else {
        m_player->setMedia(QUrl::fromLocalFile(filePath));
        m_player->setVolume(80);  // 80% 音量
        m_player->play();
    }
}

void AudioPlayer::stop() {
    if (m_player && m_player->state() != QMediaPlayer::StoppedState) {
        m_player->stop();
    }
}

// ========== 生成默认蜂鸣音 WAV ==========

void AudioPlayer::generateDefaultSound() {
    // WAV 参数
    const int sampleRate = 44100;
    const int durationMs = 500;      // 0.5 秒
    const int frequency  = 440;      // A 音符
    const int numSamples = sampleRate * durationMs / 1000;
    const int bitsPerSample = 16;
    const int numChannels = 1;

    // 生成正弦波数据
    std::vector<int16_t> samples(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        // 加入衰减包络，听起来更自然
        double envelope = 1.0 - static_cast<double>(i) / numSamples;
        samples[i] = static_cast<int16_t>(
            32767 * 0.5 * sin(2.0 * M_PI * frequency * t) * envelope
        );
    }

    // 写入 WAV 文件
    QFile file(m_defaultSoundPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法创建默认提醒音文件：" << m_defaultSoundPath;
        return;
    }

    int dataSize = numSamples * (bitsPerSample / 8) * numChannels;
    int fileSize = 36 + dataSize;  // 文件总大小 - 8

    // RIFF 头
    auto write32 = [&](uint32_t val) {
        file.write(reinterpret_cast<const char*>(&val), 4);
    };
    auto write16 = [&](uint16_t val) {
        file.write(reinterpret_cast<const char*>(&val), 2);
    };

    // RIFF 块
    file.write("RIFF", 4);
    write32(fileSize);
    file.write("WAVE", 4);

    // fmt 子块
    file.write("fmt ", 4);
    write32(16);                    // 子块大小
    write16(1);                     // PCM 格式
    write16(numChannels);           // 声道数
    write32(sampleRate);            // 采样率
    write32(sampleRate * numChannels * bitsPerSample / 8);  // 字节率
    write16(numChannels * bitsPerSample / 8);  // 块对齐
    write16(bitsPerSample);         // 位深度

    // data 子块
    file.write("data", 4);
    write32(dataSize);
    file.write(reinterpret_cast<const char*>(samples.data()), dataSize);

    file.close();
    qDebug() << "默认提醒音已生成：" << m_defaultSoundPath;
}

// ========== 退路方案：paplay ==========

void AudioPlayer::playFallback(const QString& filePath) {
    QProcess* proc = new QProcess(this);
    proc->setProgram("paplay");
    proc->setArguments({ filePath });

    // 尝试 paplay，失败则尝试 aplay
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, filePath](int exitCode, QProcess::ExitStatus) {
        if (exitCode != 0) {
            // paplay 失败，尝试 aplay
            QProcess* fallback = new QProcess(this);
            fallback->setProgram("aplay");
            fallback->setArguments({ filePath });
            fallback->start();
        }
    });

    proc->start();
}
