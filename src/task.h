#include<iostream>
using namespace std;
struct Time{
    int hour;
    int minute;
};
enum Priority
{
    high,
    medium,
    low,
};
enum Classify
{
    study,
    play,
    life,
};
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
    string show_name()
    {
        return name;
    }
    int show_id(){
        return id;
    }
    Time show_stime(){
        return start_time;
    }
    Time show_rtime(){
        return remind_time;
    }
    Priority show_priority(){
        return priority;
    }
    Classify show_classify(){
        return classify;
    }
};