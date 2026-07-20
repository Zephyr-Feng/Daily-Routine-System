#include "task.h"

int Task::next_id = 1;

Task::Task(string n, Time t1, Time t2, Priority p, Classify c) {
    id = next_id++;
    name = n;
    start_time = t1;
    remind_time = t2;
    priority = p;
    classify = c;
}

string Task::show_name() { return name; }
int Task::show_id() { return id; }
Time Task::show_stime() { return start_time; }
Time Task::show_rtime() { return remind_time; }
Priority Task::show_priority() { return priority; }
Classify Task::show_classify() { return classify; }
