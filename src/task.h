#ifndef TASK_H
#define TASK_H

#include <iostream>
#include <vector>
using namespace std;

struct Time {
    int month;
    int date;
};

enum Priority { high, medium, low };
enum Classify { study, play, life };

class Task {
private:
    int id;
    string name;
    Time start_time;
    Time remind_time;
    Priority priority;
    Classify classify;
    static int next_id;

public:
    Task(string n, Time t1, Time t2, Priority p, Classify c);
    Task() {}

    string show_name() const;
    int show_id() const;
    Time show_stime() const;
    Time show_rtime() const;
    Priority show_priority() const;
    Classify show_classify() const;
    void set_id(int new_id);
    void show_task(int m,int d);
    void remove(int i);
};

void save_task(const Task& t);
vector<Task> load_tasks();

#endif
