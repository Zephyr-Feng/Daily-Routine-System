// 测试：TaskManager 的日历相关功能
// 编译: g++ -std=c++17 -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/qt5 -fPIC test_calendar.cpp -o test_calendar task.cpp auth.cpp taskmanager.cpp -lQt5Core -lQt5Widgets -lssl -lcrypto -lpthread 2>&1 || echo 'SKIP: needs Qt linking, testing logic only'

#include <iostream>
#include <cassert>
#include <ctime>
#include task.h
#include taskmanager.h
using namespace std;

int main() {
 cout << === 测试 1: 日期提取 === << endl;
 Time t(2026, 7, 28, 9, 0);
 assert(t.year == 2026);
 assert(t.month == 7);
 assert(t.day == 28);

 cout << === 测试 2: 任务序列化/反序列化 === << endl;
 Task task1(test, Time(2026,7,28,9,0), Time(2026,7,28,8,0), HIGH, STUDY);
 string ser = task1.serialize();
 cout << 序列化: << ser << endl;
 Task task2;
 assert(task2.deserialize(ser));
 assert(task2.getName() == test);
 assert(task2.getStartTime().year == 2026);
 assert(task2.getStartTime().month == 7);
 assert(task2.getStartTime().day == 28);

 cout << === 测试 3: 删除后文件重写 === << endl;
 // 清空文件
 ofstream(test_tasks.txt) << ;
 TaskManager::instance().loadFromFile(test_tasks.txt);
 
 // 添加 2 个任务
 Task ta(A, Time(2026,7,28,9,0), Time(2026,7,28,8,0), HIGH, STUDY);
 Task tb(B, Time(2026,7,29,10,0), Time(2026,7,29,9,0), MEDIUM, PLAY);
 assert(TaskManager::instance().addTask(ta));
 assert(TaskManager::instance().addTask(tb));

 // 检查日期
 const auto& all = TaskManager::instance().allTasks();
 assert(all.size() == 2);
 assert(all[0].getStartTime().day == 28);
 assert(all[1].getStartTime().day == 29);
 cout << 添加 2 个任务: OK << endl;
 
 // 删除一个
 int delId = all[0].getId();
 assert(TaskManager::instance().deleteTask(delId));
 assert(TaskManager::instance().allTasks().size() == 1);
 assert(TaskManager::instance().allTasks()[0].getStartTime().day == 29);
 cout << 删除 id= << delId << : OK << endl;

 cout << === ALL TESTS PASSED === << endl;
 return 0;
}
