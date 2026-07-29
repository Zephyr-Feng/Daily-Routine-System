/**
 * @brief DeepSeekClient - DeepSeek AI API 客户端
 *
 * 调用 DeepSeek Chat API 进行中文自然语言解析：
 *  将用户的口语输入（如"下周三下午有高数考试挺急的"）
 *  解析为结构化的任务字段（任务名/日期/时间/优先级/分类）
 *
 * API: https://api.deepseek.com/chat/completions (OpenAI 兼容格式)
 * 模型: deepseek-chat
 */
#ifndef DEEPSEEKCLIENT_H
#define DEEPSEEKCLIENT_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QDate>

/**
 * @brief DeepSeek 解析结果
 */
struct ParsedTask {
    QString taskName;       // 任务名称
    int     year   = 2026;
    int     month  = 1;
    int     day    = 1;
    int     hour   = 9;
    int     minute = 0;
    QString priority;       // "high" / "medium" / "low"
    QString classify;       // "study" / "play" / "life"
    bool    valid  = false; // 解析是否成功
};

class DeepSeekClient : public QObject {
    Q_OBJECT

public:
    explicit DeepSeekClient(QObject* parent = nullptr);

    /**
     * @brief 设置 API Key
     */
    void setApiKey(const QString& key) { m_apiKey = key; }

    /**
     * @brief 调用 DeepSeek API 解析中文自然语言为结构化任务
     * @param text 用户说出的中文文本（来自语音识别）
     * @param today 今天的日期（用于解析"明天""下周三"等）
     * @return 解析结果
     *
     * 注意：这是同步调用，会阻塞直到 API 返回（约1-3秒）
     */
    ParsedTask parseTask(const QString& text, const QDate& today = QDate::currentDate());

private:
    QNetworkAccessManager* m_manager;
    QString m_apiKey;
    QString m_apiUrl;  // https://api.deepseek.com/chat/completions
};

#endif // DEEPSEEKCLIENT_H
