#include "task.h"
#include <sstream>
#include <vector>

// 静态成员初始化：从 1 开始自动分配 id
int Task::next_id = 1;

// ========== 构造函数 ==========

Task::Task(string n, Time start_t, Time remind_t, Priority p, Classify c) {
    id = next_id++;       // 自动分配 id 并递增
    name = n;
    start_time = start_t;
    remind_time = remind_t;
    priority = p;
    classify = c;
}

Task::Task() {
    id = next_id++;
    name = "";
    // Time 有默认构造
    priority = MEDIUM;   // 默认优先级：中
    classify = STUDY;    // 默认分类：学习
}

// ========== 序列化 ==========

/**
 * 将 Task 序列化为用 | 分隔的一行文本
 *
 * 格式说明（共 14 个字段）：
 *   id|name|年|月|日|时|分|提醒年|提醒月|提醒日|提醒时|提醒分|优先级|分类
 *
 * 示例：
 *   1|学习C++|2026|7|23|9|0|2026|7|23|8|55|0|1
 */
string Task::serialize() const {
    stringstream ss;
    ss << id << "|"
       << name << "|"
       << start_time.year  << "|"
       << start_time.month << "|"
       << start_time.day   << "|"
       << start_time.hour  << "|"
       << start_time.minute << "|"
       << remind_time.year  << "|"
       << remind_time.month << "|"
       << remind_time.day   << "|"
       << remind_time.hour  << "|"
       << remind_time.minute << "|"
       << (int)priority << "|"
       << (int)classify;
    return ss.str();
}

/**
 * 从一行序列化文本解析 Task
 * 格式：id|name|年|月|日|时|分|提醒年|提醒月|提醒日|提醒时|提醒分|优先级|分类
 */
bool Task::deserialize(const string& line) {
    if (line.empty()) return false;

    // 用 | 分割字符串
    vector<string> parts;
    stringstream ss(line);
    string part;
    while (getline(ss, part, '|')) {
        parts.push_back(part);
    }

    if (parts.size() != 14) return false;  // 需要恰好 14 个字段

    // 按位置解析
    id   = stoi(parts[0]);
    name = parts[1];
    start_time.year   = stoi(parts[2]);
    start_time.month  = stoi(parts[3]);
    start_time.day    = stoi(parts[4]);
    start_time.hour   = stoi(parts[5]);
    start_time.minute = stoi(parts[6]);
    remind_time.year   = stoi(parts[7]);
    remind_time.month  = stoi(parts[8]);
    remind_time.day    = stoi(parts[9]);
    remind_time.hour   = stoi(parts[10]);
    remind_time.minute = stoi(parts[11]);
    priority = (Priority)stoi(parts[12]);
    classify = (Classify)stoi(parts[13]);

    return true;
}

// ========== 显示辅助 ==========

string Task::priorityToString() const {
    switch (priority) {
        case HIGH:   return "高";
        case MEDIUM: return "中";
        case LOW:    return "低";
        default:     return "未知";
    }
}

string Task::classifyToString() const {
    switch (classify) {
        case STUDY: return "学习";
        case PLAY:  return "娱乐";
        case LIFE:  return "生活";
        default:    return "未知";
    }
}

Priority Task::parsePriority(const string& s) {
    int v = stoi(s);
    return (Priority)v;
}

Classify Task::parseClassify(const string& s) {
    int v = stoi(s);
    return (Classify)v;
}
