#ifndef TASK_H
#define TASK_H

#include <iostream>
#include <vector>
using namespace std;

struct Time {
 int year;
 int month;
 int day;
 int hour;
 int minute;
};

enum Priority { high, medium, low };
enum Classify { study, play, life };
Priority str_to_priority(const string &s);
string priority_to_str(Priority p);
Classify str_to_classify(const string &s);
string classify_to_str(Classify c);

Time parse_time(const string &s);
string time_to_str(Time t);
string time_to_display(Time t);

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
 static void reset_next_id(int id);
 void remove(int i);
};
void show_task(int m,int d);
void show_task(int m);
void show_task();
void save_task(const Task& t);
void remove(int k);
vector<Task> load_tasks();
#endif
