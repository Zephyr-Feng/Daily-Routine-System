#include "task.h"
#include <fstream>
#include <sstream>
#include <iomanip>

int Task::next_id = 1;
const string TASK_FILE = "tasks.txt";

Priority str_to_priority(const string &s) {
    if (s == "high") return high;
    if (s == "medium") return medium;
    return low;
}

Classify str_to_classify(const string &s) {
    if (s == "study") return study;
    if (s == "play") return play;
    return life;
}

string priority_to_str(Priority p) {
    if (p == high) return "high";
    if (p == medium) return "medium";
    return "low";
}

string classify_to_str(Classify c) {
    if (c == study) return "study";
    if (c == play) return "play";
    return "life";
}

Time parse_time(const string &s) {
    Time t;
    t.year   = stoi(s.substr(0, 4));
    t.month  = stoi(s.substr(5, 2));
    t.day    = stoi(s.substr(8, 2));
    t.hour   = stoi(s.substr(11, 2));
    t.minute = stoi(s.substr(14, 2));
    return t;
}

string two_digits(int n) {
    if (n < 10) return "0" + to_string(n);
    return to_string(n);
}

string time_to_str(Time t) {
    return to_string(t.year) + "-"
         + two_digits(t.month) + "-"
         + two_digits(t.day) + "T"
         + two_digits(t.hour) + ":"
         + two_digits(t.minute);
}

string time_to_display(Time t) {
    return to_string(t.year) + "-"
         + two_digits(t.month) + "-"
         + two_digits(t.day) + " "
         + two_digits(t.hour) + ":"
         + two_digits(t.minute);
}

Task::Task(string n, Time t1, Time t2, Priority p, Classify c) {
    id = next_id++;
    name = n;
    start_time = t1;
    remind_time = t2;
    priority = p;
    classify = c;
}

void Task::set_id(int new_id) { id = new_id; }

void save_task(const Task &t) {
    ofstream fout(TASK_FILE, ios::app);
    fout << t.show_id() << " " << t.show_name() << " "
         << time_to_str(t.show_stime()) << " "
         << time_to_str(t.show_rtime()) << " "
         << priority_to_str(t.show_priority()) << " "
         << classify_to_str(t.show_classify()) << endl;
}

vector<Task> load_tasks() {
    vector<Task> tasks;
    ifstream fin(TASK_FILE);
    string line;
    while (getline(fin, line)) {
        stringstream ss(line);
        int id;
        string name, time1, time2, pri_str, cat_str;
        ss >> id >> name >> time1 >> time2 >> pri_str >> cat_str;
        Task t(name, parse_time(time1), parse_time(time2),
               str_to_priority(pri_str), str_to_classify(cat_str));
        t.set_id(id);
        tasks.push_back(t);
    }
    return tasks;
}

void remove(int k) {
    vector<Task> t = load_tasks();
    for (int i = 0; i < t.size(); i++) {
        if (t[i].show_id() == k) {
            t.erase(t.begin() + i);
            break;
        }
    }
    ofstream fout(TASK_FILE);
    for (int i = 0; i < t.size(); i++) {
        save_task(t[i]);
    }
}

void show_task(int m, int d) {
    ifstream fin(TASK_FILE);
    string line;
    while (getline(fin, line)) {
        stringstream ss(line);
        int id;
        string name, time1, time2, pri_str, cat_str;
        ss >> id >> name >> time1 >> time2 >> pri_str >> cat_str;
        Time t = parse_time(time1);
        if (t.month == m && t.day == d) {
            cout << id << "  " << name << "  "
                 << time_to_display(t) << "  "
                 << pri_str << "  " << cat_str << endl;
        }
    }
}

void show_task(int m) {
    fstream fin(TASK_FILE);
    string line;
    while (getline(fin, line)) {
        stringstream ss(line);
        int id;
        string name, time1, time2, pri_str, cat_str;
        ss >> id >> name >> time1 >> time2 >> pri_str >> cat_str;
        Time t = parse_time(time1);
        if (t.month == m) {
            cout << id << "  " << name << "  "
                 << time_to_display(t) << "  "
                 << pri_str << "  " << cat_str << endl;
        }
    }
}

void show_task() {
    fstream fin(TASK_FILE);
    string line;
    while (getline(fin, line)) {
        stringstream ss(line);
        int id;
        string name, time1, time2, pri_str, cat_str;
        ss >> id >> name >> time1 >> time2 >> pri_str >> cat_str;
        Time t = parse_time(time1);
        cout << id << "  " << name << "  "
             << time_to_display(t) << "  "
             << pri_str << "  " << cat_str << endl;
    }
}

string Task::show_name() const { return name; }
int Task::show_id() const { return id; }
Time Task::show_stime() const { return start_time; }
Time Task::show_rtime() const { return remind_time; }
Priority Task::show_priority() const { return priority; }
Classify Task::show_classify() const { return classify; }
