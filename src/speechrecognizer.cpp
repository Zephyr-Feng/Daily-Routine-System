/**
 * @brief SpeechRecognizer 实现 — faster-whisper 离线中文语音识别
 *
 * 方案：通过 QProcess 调用 Python + faster-whisper 脚本进行语音识别
 *
 * faster-whisper 优势：
 *  1. 使用 CTranslate2 引擎，CPU 上比 openai-whisper 快 4 倍
 *  2. 体积小 10 倍（无需 PyTorch/CUDA）
 *  3. 同样使用 Whisper 模型，中文识别准确率高
 */
#include "speechrecognizer.h"
#include <QProcess>
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

SpeechRecognizer::SpeechRecognizer(const QString& modelSize, QObject* parent)
    : QObject(parent)
    , m_ready(false)
    , m_modelSize(modelSize)
{
    // ===== 检查 Python 环境 =====
    QProcess checkPy;
    checkPy.start("python3", {"--version"});
    checkPy.waitForFinished(3000);

    if (checkPy.exitCode() != 0) {
        qWarning() << "未找到 Python3，语音识别功能不可用";
        return;
    }
    qDebug() << "Python3 已就绪：" << checkPy.readAllStandardOutput().trimmed();

    // ===== 检查 faster-whisper Python 包 =====
    QProcess checkWhisper;
    checkWhisper.start("python3", {"-c", "import faster_whisper; print('ok')"});
    checkWhisper.waitForFinished(5000);

    if (checkWhisper.exitCode() != 0) {
        qWarning() << "faster-whisper 未安装。运行: pip3 install faster-whisper";
        return;
    }
    qDebug() << "faster-whisper 已就绪";

    // ===== 验证模型名称/路径 =====
    // 支持两种方式：模型名("tiny"/"small") 或 本地模型路径
    if (QFileInfo(modelSize).isDir()) {
        // 本地模型目录路径，直接使用
        qDebug() << "使用本地模型路径：" << modelSize;
    } else {
        QStringList validModels = {"tiny", "base", "small", "medium", "large"};
        if (!validModels.contains(modelSize)) {
            qWarning() << "无效的 Whisper 模型：" << modelSize;
            qWarning() << "可选值：tiny, base, small, medium, large 或本地目录路径";
            return;
        }
    }

    m_ready = true;
    qDebug() << "Whisper 语音识别器初始化成功，模型：" << modelSize;
}

SpeechRecognizer::~SpeechRecognizer() {
    // Python 进程会自动管理，无需额外清理
}

QString SpeechRecognizer::recognize(const QString& wavFilePath) {
    if (!m_ready) {
        qWarning() << "SpeechRecognizer 未就绪，无法识别";
        return QString();
    }

    if (!QFileInfo::exists(wavFilePath)) {
        qWarning() << "WAV 文件不存在：" << wavFilePath;
        return QString();
    }

    // ===== 查找 whisper_stt.py 脚本 =====
    // 优先找可执行文件同目录（build/），再找源码目录（src/）
    QString scriptPath = QCoreApplication::applicationDirPath() + "/whisper_stt.py";
    if (!QFileInfo::exists(scriptPath)) {
        QDir dir(QCoreApplication::applicationDirPath());
        dir.cdUp(); // 从 build/ 回到项目根目录
        scriptPath = dir.absolutePath() + "/src/whisper_stt.py";
    }

    if (!QFileInfo::exists(scriptPath)) {
        qWarning() << "whisper_stt.py 未找到：" << scriptPath;
        return QString();
    }

    // ===== 通过 QProcess 调用 Whisper Python 脚本 =====
    QProcess proc;
    proc.start("python3", {
        scriptPath,
        m_modelSize,    // 参数1: 模型大小（small）或本地模型路径
        wavFilePath     // 参数2: WAV 文件路径
    });

    if (!proc.waitForFinished(120000)) {  // 2分钟超时
        qWarning() << "语音识别超时（120秒）";
        proc.kill();
        return QString();
    }

    if (proc.exitCode() != 0) {
        qWarning() << "语音识别失败：" << proc.readAllStandardError();
        return QString();
    }

    QString result = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    qDebug() << "Whisper 识别结果：" << result;
    return result;
}
