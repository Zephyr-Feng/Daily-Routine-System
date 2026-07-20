#ifndef TASK_H
#define TASK_H

#include <iostream>
using namespace std;

struct Time {
    int hour;
    int minute;
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

    string show_name();
    int show_id();
    Time show_stime();
    Time show_rtime();
    Priority show_priority();
    Classify show_classify();
    void store();
    void load();
    
};

#endif
