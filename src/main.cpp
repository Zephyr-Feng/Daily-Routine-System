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
 */
#include <QApplication>
#include "loginwindow.h"
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QMainWindow { background-color: #fafafa; }"
        "QTableView { font-size: 13px; gridline-color: #e0e0e0; }"
        "QTableView::item { padding: 4px 8px; }"
        "QHeaderView::section { background-color: #e8e8e8; padding: 6px; "
        "  font-weight: bold; border: 1px solid #ddd; }"
    );

    LoginWindow loginDlg;
    if (loginDlg.exec() != QDialog::Accepted) {
        return 0;
    }

    QString username = loginDlg.getUsername();
    MainWindow mainWin(username);
    mainWin.show();

    return app.exec();
}
