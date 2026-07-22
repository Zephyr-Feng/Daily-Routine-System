#ifndef AUTH_H
#define AUTH_H

#include <string>
using namespace std;

// SHA256 加密
string sha256(const string& str);

bool register_user(const string& username, const string& password);

bool login(const string& username, const string& password);

bool user_exist(const string &username);

#endif
