/**
 * @brief 日程管理系统 - Qt 图形界面版
 *
 * 程序入口：启动时显示登录窗口，登录成功后进入主窗口。
 *
 * 使用 Qt5 Widgets 框架，兼容 Ubuntu 22.04。
 *
 * 编译方式：
 *   mkdir build && cd build
 *   cmake ..
 *   make
 *   ./myschedule
 *
 * 架构说明：
 *   main.cpp         → 入口，创建 QApplication，启动 LoginWindow
 *   loginwindow      → 登录/注册界面，调用 auth.h 验证
 *   mainwindow       → 主界面，表格显示 + 任务操作 + 后台提醒
 *   taskdialog       → 添加/编辑任务的表单对话框
 *   taskmanager      → 数据层，管理任务列表 + 文件读写
 *   task             → 数据模型，任务属性和序列化
 *   auth             → 用户认证，SHA256 密码哈希
 */
#include <QApplication>
#include "loginwindow.h"
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // 设置全局样式
    app.setStyleSheet(
        "QMainWindow { background-color: #fafafa; }"
        "QTableView { font-size: 13px; gridline-color: #e0e0e0; }"
        "QTableView::item { padding: 4px 8px; }"
        "QHeaderView::section { background-color: #e8e8e8; padding: 6px; "
        "  font-weight: bold; border: 1px solid #ddd; }"
    );

    // ===== 显示登录窗口 =====
    LoginWindow loginDlg;
    if (loginDlg.exec() != QDialog::Accepted) {
        return 0;  // 用户取消登录，退出程序
    }

    // ===== 登录成功，进入主界面 =====
    QString username = loginDlg.getUsername();
    MainWindow mainWin(username);
    mainWin.show();

    return app.exec();
}
