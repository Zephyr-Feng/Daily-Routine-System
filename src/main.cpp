#include <iostream>
#include "task.h"
#include "auth.h"
using namespace std;

void run_loop(){
    cout << "please type your name: ";
    cout << "please type your password: ";
    string user_name;
    string password;
    cin >> user_name;
    cin >> password;
    while(!login(user_name,password)){
        cin >> user_name;
        cin >> password;
        if (login(user_name, password))
        {
            cout << "log in successfully!" << endl;
            break;
        }
        else if (!login(user_name,password) && user_exist(user_name)){
            cout << "wrong password!" << endl;
        }
        else{
            register_user(user_name, password);
            cout << "register successfully!" << endl;
            break;
        }
    }
    vector<Task> schedule = load_tasks();
    while (true)
    {
        string command;
        cin >> command;
        if(command=="addtask"){
            cout << "Please type your task" << endl;
            string name;
            string date_s, time_s;
            string date_r, time_r;
            string p_;
            string c_;

            cout << "name: ";
            cin >> name;
            cout << "start date (YYYY-MM-DD): ";
            cin >> date_s;
            cout << "start time (HH:MM): ";
            cin >> time_s;
            cout << "remind date (YYYY-MM-DD): ";
            cin >> date_r;
            cout << "remind time (HH:MM): ";
            cin >> time_r;
            cout << "priority (high/medium/low): ";
            cin >> p_;
            cout << "classify (study/play/life): ";
            cin >> c_;

            Time s = parse_time(date_s + "T" + time_s);
            Time r = parse_time(date_r + "T" + time_r);
            Priority p = str_to_priority(p_);
            Classify c = str_to_classify(c_);
            Task t(name, s, r, p, c);
            save_task(t);
            cout << "task added!" << endl;
        }
        else if(command=="showtask"){
            cout << "type m / md / all: ";
            string cmd;
            cin >> cmd;
            if (cmd == "m")
            {
                cout << "month: ";
                int m;
                cin >> m;
                show_task(m);
            }
            else if(cmd=="md"){
                cout << "month day: ";
                int m,d;
                cin>>m>>d;
                show_task(m, d);
            }
            else{
                show_task();
            }
        }
        else if(command=="deltask"){
            cout << "please type the task's id you want to delete" << endl;
            int k;
            cin >> k;
            remove(k);
        }
        else if(command=="exit"){
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    string cmd = argv[1];
    if(cmd=="run"){
        run_loop();
    }
}
