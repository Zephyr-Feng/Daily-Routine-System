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
    if(!login(user_name,password)){
        if(user_exist(user_name)){
            cout << "wrong password!" << endl;
        }
        else{
            register_user(user_name, password);
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
