/**
 * @brief SpeechRecognizer - Whisper 离线语音识别封装
 *
 * 使用 OpenAI Whisper 进行离线中文语音识别：
 *  1. 通过 QProcess 调用 Python whisper 脚本
 *  2. Whisper small 模型自动下载到 ~/.cache/whisper/
 *  3. 识别 WAV 音频并返回中文文本
 *
 * 依赖：pip3 install openai-whisper
 *
 * Whisper 模型大小（越大越准但越慢）：
 *  tiny  (~39MB) — 最快，适合低配设备
 *  base  (~74MB) — 平衡
 *  small (~466MB) — 推荐，中文效果好
 *  medium(~1.5GB) — 更准确
 */
#ifndef SPEECHRECOGNIZER_H
#define SPEECHRECOGNIZER_H

#include <QObject>
#include <QString>
#include <QFile>

class SpeechRecognizer : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 初始化语音识别器
     * @param modelSize Whisper 模型大小（"tiny"/"base"/"small"/"medium"）
     */
    explicit SpeechRecognizer(const QString& modelSize, QObject* parent = nullptr);
    ~SpeechRecognizer();

    /** Whisper 是否加载成功 */
    bool isReady() const { return m_ready; }

    /**
     * @brief 识别 WAV 文件中的语音
     * @param wavFilePath 16kHz 16-bit 单声道 WAV 文件路径
     * @return 识别的中文文本，失败返回空字符串
     */
    QString recognize(const QString& wavFilePath);

private:
    bool m_ready;
    QString m_modelSize;  // Whisper 模型大小：tiny/base/small/medium
};

#endif // SPEECHRECOGNIZER_H
