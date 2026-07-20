#include "task.h"
#include <fstream>
int Task::next_id = 1;

const string Tasks = "tasks.txt";
string priority_to_string(Priority p)
{
    if (p == high)
        return "high";
    if (p == medium)
        return "medium";
    return "low";
}

string classify_to_string(Classify c)
{
    if (c == study)
        return "study";
    if (c == play)
        return "play";
    return "life";
}
Task::Task(string n, Time t1, Time t2, Priority p, Classify c)
{
    id = next_id++;
    name = n;
    start_time = t1;
    remind_time = t2;
    priority = p;
    classify = c;
    ofstream fout(Tasks, ios::app);
    fout << id << " " << name << " " << start_time.hour << ":" << start_time.minute << " " << remind_time.hour << ":" << remind_time.minute << " " << priority_to_string(priority) << " " << classify_to_string(classify) << endl;
}

string Task::show_name() { return name; }
int Task::show_id() { return id; }
Time Task::show_stime() { return start_time; }
Time Task::show_rtime() { return remind_time; }
Priority Task::show_priority() { return priority; }
Classify Task::show_classify() { return classify; }
void Task::load()
{
    
}