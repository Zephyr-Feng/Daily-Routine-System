#include "auth.h"
#include <fstream>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

const string USER_FILE = "users.txt";

string sha256(const string& str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)str.c_str(), str.length(), hash);
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    return ss.str();
}

bool register_user(const string& username, const string& password) {
    ifstream fin(USER_FILE);
    string line;
    while (getline(fin, line)) {
        string existing_user = line.substr(0, line.find(' '));
        if (existing_user == username) {
            return false;
        }
    }
    fin.close();

    ofstream fout(USER_FILE, ios::app);
    fout << username << " " << sha256(password) << endl;
    return true;
}

bool login(const string& username, const string& password) {
    ifstream fin(USER_FILE);
    string line;
    while (getline(fin, line)) {
        size_t pos = line.find(' ');
        string stored_user = line.substr(0, pos);
        string stored_hash = line.substr(pos + 1);

        if (stored_user == username) {
            return stored_hash == sha256(password);
        }
    }
    return false;
}
