#ifndef TASK_H
#define TASK_H

#include <iostream>
#include <string>
using namespace std;

/**
 * @brief 时间结构体：包含年月日和时分
 *        用于任务的开始时间和提醒时间
 */
struct Time {
    int year;    // 年，如 2026
    int month;   // 月，1-12
    int day;     // 日，1-31
    int hour;    // 时，0-23
    int minute;  // 分，0-59

    // 默认构造：设为 2026-01-01 00:00
    Time() : year(2026), month(1), day(1), hour(0), minute(0) {}

    Time(int y, int mo, int d, int h, int mi)
        : year(y), month(mo), day(d), hour(h), minute(mi) {}

    /**
     * @brief 比较两个时间是否相等（用于：任务开始时间不能相同）
     */
    bool operator==(const Time& other) const {
        return year == other.year && month == other.month &&
               day == other.day && hour == other.hour && minute == other.minute;
    }

    /**
     * @brief 比较时间先后（用于排序）
     * @return true 如果 this 在 other 之前
     */
    bool operator<(const Time& other) const {
        if (year != other.year)   return year < other.year;
        if (month != other.month) return month < other.month;
        if (day != other.day)     return day < other.day;
        if (hour != other.hour)   return hour < other.hour;
        return minute < other.minute;
    }

    /**
     * @brief 格式化为可读字符串，如 "2026-07-23 09:00"
     */
    string toString() const {
        char buf[32];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d",
                 year, month, day, hour, minute);
        return string(buf);
    }
};

/**
 * @brief 优先级枚举
 */
enum Priority {
    HIGH = 0,
    MEDIUM = 1,
    LOW = 2
};

/**
 * @brief 分类枚举
 */
enum Classify {
    STUDY = 0,  // 学习
    PLAY = 1,   // 娱乐
    LIFE = 2    // 生活
};

/**
 * @brief Task 类：代表一个日程任务
 *
 * 属性：
 *  - id:         唯一标识（自动递增）
 *  - name:       任务名称
 *  - start_time: 任务开始时间
 *  - remind_time:提醒时间
 *  - priority:   优先级（高/中/低）
 *  - classify:   分类（学习/娱乐/生活）
 */
class Task {
private:
    int id;
    string name;
    Time start_time;
    Time remind_time;
    Priority priority;
    Classify classify;
    static int next_id;  // 静态计数器，自动分配 id

public:
    // ========== 构造函数 ==========

    /** 完整构造 */
    Task(string n, Time start_t, Time remind_t, Priority p, Classify c);

    /** 默认构造（用于 vector 等容器） */
    Task();

    // ========== Getter（读取属性）==========

    int getId()        const { return id; }
    string getName()   const { return name; }
    Time getStartTime()   const { return start_time; }
    Time getRemindTime()  const { return remind_time; }
    Priority getPriority()  const { return priority; }
    Classify getClassify()  const { return classify; }

    // ========== Setter（修改属性）==========

    void setName(const string& n)      { name = n; }
    void setStartTime(const Time& t)   { start_time = t; }
    void setRemindTime(const Time& t)  { remind_time = t; }
    void setPriority(Priority p)       { priority = p; }
    void setClassify(Classify c)       { classify = c; }

    /** 为已有任务手动设置 id（从文件加载时使用）*/
    void setId(int newId) { id = newId; }

    // ========== 序列化（文件读写）==========

    /**
     * @brief 将 Task 序列化为一行字符串
     * @return 格式：id|name|year|month|day|hour|min|ry|rm|rd|rh|rmn|priority|classify
     *         其中 r* 是提醒时间字段
     */
    string serialize() const;

    /**
     * @brief 从一行字符串反序列化为 Task
     * @param line 序列化字符串
     * @return 成功返回 true
     */
    bool deserialize(const string& line);

    // ========== 显示 ==========

    /** 获取优先级的字符串表示 */
    string priorityToString() const;
    /** 获取分类的字符串表示 */
    string classifyToString() const;

    /** 从字符串解析优先级（"0"→HIGH, "1"→MEDIUM, "2"→LOW）*/
    static Priority parsePriority(const string& s);
    /** 从字符串解析分类（"0"→STUDY, "1"→PLAY, "2"→LIFE）*/
    static Classify parseClassify(const string& s);

    /** 重置静态 id 计数器（从文件加载后调用）*/
    static void setNextId(int next) { next_id = next; }
    static int getNextId() { return next_id; }
};

#endif // TASK_H
