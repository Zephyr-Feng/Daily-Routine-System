#include "task.h"
#include <fstream>
#include <sstream>

int Task::next_id = 1;
const string TASK_FILE = "tasks.txt";

Priority str_to_priority(const string &s)
{
    if (s == "high")
        return high;
    if (s == "medium")
        return medium;
    return low;
}

Classify str_to_classify(const string &s)
{
    if (s == "study")
        return study;
    if (s == "play")
        return play;
    return life;
}

string priority_to_str(Priority p)
{
    if (p == high)
        return "high";
    if (p == medium)
        return "medium";
    return "low";
}

string classify_to_str(Classify c)
{
    if (c == study)
        return "study";
    if (c == play)
        return "play";
    return "life";
}

Time parse_time(const string &s)
{
    Time t;
    size_t comma = s.find(',');
    t.month = stoi(s.substr(0, comma));
    t.date = stoi(s.substr(comma + 1));
    return t;
}

string time_to_str(Time t)
{
    return to_string(t.month) + "," + to_string(t.date);
}

Task::Task(string n, Time t1, Time t2, Priority p, Classify c)
{
    id = next_id++;
    name = n;
    start_time = t1;
    remind_time = t2;
    priority = p;
    classify = c;
}

void Task::set_id(int new_id) { id = new_id; }

void save_task(const Task &t)
{
    ofstream fout(TASK_FILE, ios::app);
    fout << t.show_id() << " " << t.show_name() << " "
         << time_to_str(t.show_stime()) << " "
         << time_to_str(t.show_rtime()) << " "
         << priority_to_str(t.show_priority()) << " "
         << classify_to_str(t.show_classify()) << endl;
}

vector<Task> load_tasks()
{
    vector<Task> tasks;
    ifstream fin(TASK_FILE);
    string line;

    while (getline(fin, line))
    {
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
void Task::show_task(int m, int d)
{
    ifstream fin(TASK_FILE);
    string line;

    while (getline(fin, line))
    {
        stringstream ss(line);
        int id;
        string name, time1, time2, pri_str, cat_str;

        ss >> id >> name >> time1 >> time2 >> pri_str >> cat_str;
        Time time_start = parse_time(time1);
        if (time_start.month == m && time_start.date == d)
        {
            cout << id << " " << name << " " << time1 << " " << time2 << " " << pri_str << cat_str << endl;
        }
    }
}
void Task::remove(int k){
    vector<Task> t = load_tasks();
    vector<Task> ans;
    for (int i = 0; i < t.size(); i++)
    {
        if(t[i].show_id()==k){
            t.erase(t.begin() + i);
        }
    }
    ofstream fout(TASK_FILE);
    for (int i = 0; i < t.size();i++){
        save_task(t[i]);
    }
}
string Task::show_name() const { return name; }
int Task::show_id() const { return id; }
Time Task::show_stime() const { return start_time; }
Time Task::show_rtime() const { return remind_time; }
Priority Task::show_priority() const { return priority; }
Classify Task::show_classify() const { return classify; }
