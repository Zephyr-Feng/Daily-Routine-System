#include "taskmanager.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <algorithm>
#include <set>

// ========== 单例 ==========

TaskManager& TaskManager::instance() {
    static TaskManager mgr;
    return mgr;
}

// ========== 文件读写 ==========

int TaskManager::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.exists()) {
        // 文件不存在时创建空文件，不影响首次使用
        return 0;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return -1;  // 无法打开文件
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_tasks.clear();

    int maxId = 0;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        Task t;
        if (t.deserialize(line.toStdString())) {
            m_tasks.push_back(t);
            if (t.getId() > maxId) {
                maxId = t.getId();
            }
        }
    }

    file.close();

    // 恢复 next_id，保证新任务 id 不重复
    Task::setNextId(maxId + 1);

    // 按开始时间排序
    std::sort(m_tasks.begin(), m_tasks.end(),
              [](const Task& a, const Task& b) {
                  return a.getStartTime() < b.getStartTime();
              });

    return (int)m_tasks.size();
}

bool TaskManager::saveToFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    QTextStream out(&file);
    for (const auto& task : m_tasks) {
        out << QString::fromStdString(task.serialize()) << "\n";
    }

    file.close();
    return true;
}

// ========== 添加任务 ==========

bool TaskManager::addTask(const Task& task) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // 校验1：开始时间不能与其他任务相同
    for (const auto& t : m_tasks) {
        if (t.getStartTime() == task.getStartTime()) {
            return false;  // 时间冲突
        }
    }

    // 校验2：名称 + 开始时间必须唯一
    for (const auto& t : m_tasks) {
        if (t.getName() == task.getName() &&
            t.getStartTime() == task.getStartTime()) {
            return false;  // 名称+时间冲突
        }
    }

    m_tasks.push_back(task);

    // 保持按开始时间排序
    std::sort(m_tasks.begin(), m_tasks.end(),
              [](const Task& a, const Task& b) {
                  return a.getStartTime() < b.getStartTime();
              });

    // 自动保存
    saveToFile();
    emit tasksChanged();
    return true;
}

// ========== 删除任务 ==========

bool TaskManager::deleteTask(int id) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = std::find_if(m_tasks.begin(), m_tasks.end(),
                           [id](const Task& t) { return t.getId() == id; });

    if (it == m_tasks.end()) {
        return false;  // 未找到
    }

    m_tasks.erase(it);
    saveToFile();
    emit tasksChanged();
    return true;
}

// ========== 更新任务 ==========

bool TaskManager::updateTask(const Task& updated) {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& t : m_tasks) {
        if (t.getId() == updated.getId()) {
            t = updated;
            std::sort(m_tasks.begin(), m_tasks.end(),
                      [](const Task& a, const Task& b) {
                          return a.getStartTime() < b.getStartTime();
                      });
            saveToFile();
            emit tasksChanged();
            return true;
        }
    }
    return false;
}

// ========== 查找任务 ==========

Task* TaskManager::findTask(int id) {
    for (auto& t : m_tasks) {
        if (t.getId() == id) return &t;
    }
    return nullptr;
}

// ========== 提醒检查 ==========

std::vector<Task> TaskManager::checkReminders() {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<Task> due;
    QDateTime now = QDateTime::currentDateTime();

    for (const auto& task : m_tasks) {
        Time rt = task.getRemindTime();
        QDateTime remindDateTime(
            QDate(rt.year, rt.month, rt.day),
            QTime(rt.hour, rt.minute)
        );

        // 如果提醒时间已到 且 开始时间还没过
        // 且这个任务今天还没提醒过
        if (remindDateTime <= now) {
            // 检查是否已经提醒过
            if (std::find(m_reminded_ids.begin(), m_reminded_ids.end(),
                          task.getId()) != m_reminded_ids.end()) {
                continue;  // 已提醒过，跳过
            }

            Time st = task.getStartTime();
            QDateTime startDateTime(
                QDate(st.year, st.month, st.day),
                QTime(st.hour, st.minute)
            );

            // 任务开始时间还没过（否则提醒无意义）
            if (now <= startDateTime) {
                due.push_back(task);
                m_reminded_ids.push_back(task.getId());
            }
        }
    }

    return due;
}
