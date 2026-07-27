#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>
#include <QString>
#include <vector>
#include <mutex>
#include "task.h"

/**
 * @brief TaskManager - 任务数据管理器（单例模式）
 *
 * 职责：
 *  1. 在内存中管理任务列表 (vector<Task>)
 *  2. 任务数据的文件读写 (tasks.txt)
 *  3. 提供增删改查接口
 *  4. 检查任务是否需要提醒
 *
 * 使用方式：
 *   TaskManager::instance().loadFromFile("tasks.txt");
 *   auto tasks = TaskManager::instance().allTasks();
 */
class TaskManager : public QObject {
    Q_OBJECT

public:
    /** 获取单例实例 */
    static TaskManager& instance();

    // ========== 文件读写 ==========

    /**
     * @brief 从文件加载任务列表
     * @param filePath 文件路径（默认 "tasks.txt"）
     * @return 加载的任务数量
     *
     * 文件格式：每行一个任务，用 | 分隔字段
     * 加载后自动更新 next_id，确保新任务的 id 不重复
     */
    int loadFromFile(const QString& filePath = "tasks.txt");

    /**
     * @brief 保存所有任务到文件
     * @param filePath 文件路径
     * @return 成功返回 true
     */
    bool saveToFile(const QString& filePath = "tasks.txt");

    // ========== 任务增删改查 ==========

    /**
     * @brief 添加新任务
     * @param task 任务对象
     * @return 成功返回 true
     *
     * 校验规则：
     *  - 开始时间不能与其他任务相同
     *  - 任务名称 + 开始时间必须唯一
     *  - 添加后自动保存到文件
     */
    bool addTask(const Task& task);

    /**
     * @brief 根据 id 删除任务
     * @param id 任务 id
     * @return 成功返回 true
     */
    bool deleteTask(int id);

    /**
     * @brief 更新已有任务
     * @param updated 已更新数据的任务对象（通过 id 匹配）
     * @return 成功返回 true
     */
    bool updateTask(const Task& updated);

    /**
     * @brief 根据 id 查找任务
     * @param id 任务 id
     * @return 找到返回指针，否则返回 nullptr
     */
    Task* findTask(int id);

    /** 获取所有任务的只读引用 */
    const std::vector<Task>& allTasks() const { return m_tasks; }

    /** 获取任务总数 */
    int taskCount() const { return (int)m_tasks.size(); }

    // ========== 提醒检查 ==========

    /**
     * @brief 检查是否有任务需要提醒
     * @return 需要提醒的任务列表
     *
     * 遍历所有任务，如果"提醒时间 <= 当前时间"且"开始时间 >= 当前时间"，
     * 则认为需要提醒。已提醒过的任务会标记避免重复提醒。
     */
    std::vector<Task> checkReminders();

signals:
    /** 当任务数据发生变化时发射（通知 UI 刷新）*/
    void tasksChanged();

    /** 当有任务需要提醒时发射 */
    void reminderTriggered(const QString& taskName, const QString& taskTime);

private:
    TaskManager() {}                       // 私有构造（单例）
    ~TaskManager() {}
    TaskManager(const TaskManager&) = delete;       // 禁止拷贝
    TaskManager& operator=(const TaskManager&) = delete;

    std::vector<Task> m_tasks;              // 任务列表
    std::mutex m_mutex;                     // 线程安全锁
    std::vector<int> m_reminded_ids;        // 已提醒过的任务 id（避免重复提醒）
};

#endif // TASKMANAGER_H
