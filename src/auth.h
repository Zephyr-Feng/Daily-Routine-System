#ifndef AUTH_H
#define AUTH_H

#include <string>
using namespace std;

// SHA256 加密
string sha256(const string& str);

// 注册：将用户名和加密密码写入文件
bool register_user(const string& username, const string& password);

// 登录：验证用户名和密码
bool login(const string& username, const string& password);

#endif
