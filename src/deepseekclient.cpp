/**
 * @brief DeepSeekClient 实现 — 调用 DeepSeek Chat API 进行 NLP 解析
 *
 * API 文档：https://api-docs.deepseek.com/
 * 兼容 OpenAI Chat Completions 格式
 */
#include "deepseekclient.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QRegExp>
#include <QDebug>

DeepSeekClient::DeepSeekClient(QObject* parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_apiUrl("https://api.deepseek.com/chat/completions")
{
}

ParsedTask DeepSeekClient::parseTask(const QString& text, const QDate& today) {
    ParsedTask result;

    if (m_apiKey.isEmpty()) {
        qWarning() << "DeepSeek API Key 未设置";
        return result;
    }

    // ===== 构造 API 请求 =====
    QUrl url(m_apiUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

    // ===== 构造 System Prompt =====
    QString systemPrompt = QString(
        "你是一个日程管理助手。请从用户的口语输入中提取任务信息，返回JSON格式。\n"
        "\n"
        "规则：\n"
        "1. 任务名称(taskName)：提取核心任务内容，去掉时间/日期/优先级等修饰词\n"
        "2. 日期(year/month/day)：理解[明天][后天][下周三][大后天]等相对日期\n"
        "   今天是 %1年%2月%3日（周%4），以此推算具体日期\n"
        "3. 时间(hour/minute)：24小时制，识别[上午/下午/晚上/中午]并正确转换\n"
        "   [上午九点]->9:00, [下午三点]->15:00, [晚上八点]->20:00\n"
        "   如果未提到具体时间，默认 hour=9, minute=0\n"
        "4. 优先级(priority)：识别[紧急/重要/高优先级]->high, [不急/低优先级]->low, 其他->medium\n"
        "5. 分类(classify)：识别[学习/作业/课程/考试/复习]->study, "
        "[娱乐/玩/游戏/电影/音乐/运动]->play, "
        "[生活/日常/购物/吃饭/睡觉/约会]->life, 其他->study\n"
        "\n"
        "只返回JSON，不要任何其他内容，不要markdown代码块标记：\n"
        "{\"taskName\":\"...\",\"year\":%1,\"month\":%2,\"day\":%3,\"hour\":9,\"minute\":0,\"priority\":\"medium\",\"classify\":\"study\"}"
    ).arg(today.year()).arg(today.month()).arg(today.day()).arg(today.dayOfWeek());

    // ===== 构造消息 =====
    QJsonArray messages;
    QJsonObject sysMsg;
    sysMsg["role"] = "system";
    sysMsg["content"] = systemPrompt;
    messages.append(sysMsg);

    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = text;
    messages.append(userMsg);

    // ===== 构造请求体 =====
    QJsonObject body;
    body["model"] = "deepseek-chat";
    body["messages"] = messages;
    body["temperature"] = 0.1;   // 低温度，保证稳定输出
    body["max_tokens"] = 300;
    body["stream"] = false;

    QJsonDocument doc(body);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    qDebug() << "Sending DeepSeek request:" << text;

    // ===== 发送请求（同步等待）=====
    QNetworkReply* reply = m_manager->post(request, jsonData);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();  // 阻塞等待

    // ===== 处理响应 =====
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "DeepSeek API error:" << reply->errorString();
        qWarning() << "Response body:" << reply->readAll();
        reply->deleteLater();
        return result;
    }

    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    qDebug() << "DeepSeek raw response:" << responseData;

    // ===== 解析响应 JSON =====
    QJsonParseError parseError;
    QJsonDocument respDoc = QJsonDocument::fromJson(responseData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "DeepSeek JSON parse error:" << parseError.errorString();
        return result;
    }

    QJsonObject respObj = respDoc.object();
    QJsonArray choices = respObj["choices"].toArray();
    if (choices.isEmpty()) {
        qWarning() << "DeepSeek response has no choices";
        return result;
    }

    QString content = choices[0].toObject()["message"].toObject()["content"].toString().trimmed();

    // 去掉可能的 markdown 代码块标记
    if (content.startsWith("```")) {
        content.remove(QRegExp("^```(?:json)?\\s*"));
        content.remove(QRegExp("\\s*```$"));
        content = content.trimmed();
    }

    qDebug() << "DeepSeek parsed result:" << content;

    // ===== 解析返回的 JSON =====
    QJsonDocument taskDoc = QJsonDocument::fromJson(content.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "DeepSeek task JSON parse error:" << parseError.errorString();
        qWarning() << "Content:" << content;
        return result;
    }

    QJsonObject taskObj = taskDoc.object();

    result.taskName = taskObj["taskName"].toString();
    result.year     = taskObj["year"].toInt(today.year());
    result.month    = taskObj["month"].toInt(today.month());
    result.day      = taskObj["day"].toInt(today.day());
    result.hour     = taskObj["hour"].toInt(9);
    result.minute   = taskObj["minute"].toInt(0);
    result.priority = taskObj["priority"].toString("medium");
    result.classify = taskObj["classify"].toString("study");
    result.valid    = !result.taskName.isEmpty();

    qDebug() << "Parse success:" << result.taskName
             << QString("%1-%2-%3 %4:%5").arg(result.year).arg(result.month)
                    .arg(result.day).arg(result.hour).arg(result.minute)
             << result.priority << result.classify;

    return result;
}
