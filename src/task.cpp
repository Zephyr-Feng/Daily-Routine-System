#include"task.h"

int Task::next_id = 1;
Task::Task(string n, Time t1, Time t2, Priority p, Classify c)
{
    id = next_id;
    next_id++;
    name = n;
    start_time = t1;
    remind_time = t2;
    priority = p;
    classify = c;
}
