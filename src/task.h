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
Priority str_to_priority(const string &s);
string priority_to_str(Priority p);
Classify str_to_classify(const string &s);
string classify_to_str(Classify c);
class Task
{
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
    void remove(int i);
};
void show_task(int m,int d);
void show_task(int m);
void show_task();
void save_task(const Task& t);
void remove(int k);
vector<Task> load_tasks();
Time parse_time(const string &s);
#endif
