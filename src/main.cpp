#include <iostream>
#include"task.h"
#include"auth.h"
using namespace std;
void run_loop(){
    cout << "please type your name" << endl;
    cout << "please type your password" << endl;
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
        else if(!login(user_name,password)&&user_exist(user_name)){
            cout << "wrong password!" << endl;
        }
        else{
            register_user(user_name, password);
            cout << "register successfully!" << endl;
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
            string s_time;
            string r_time;
            string p_;
            string c_;
            cin >> name >> s_time >> r_time >> p_ >> c_;
            Time s = parse_time(s_time);
            Time r = parse_time(r_time);
            Priority p = str_to_priority(p_);
            Classify c = str_to_classify(c_);
            Task t(name, s, r, p, c);
            save_task(t);
        }
        else if(command=="showtask"){
            cout << "please type m/md/all";
            string cmd;
            cin >> cmd;
            if (cmd == "m")
            {
                int m;
                cin >> m;
                show_task(m);
            }
            else if(cmd=="md"){
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
    }
}
int main(int argc, char *argv[])
{
    string cmd = argv[1];
    if(cmd=="run"){
        run_loop();
    }
}