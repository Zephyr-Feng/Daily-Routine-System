
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

    // 显示登录窗口 
    LoginWindow loginDlg;
    if (loginDlg.exec() != QDialog::Accepted) {
        return 0;  // 用户取消登录，退出程序
    }

    // 登录成功，进入主界面 
    QString username = loginDlg.getUsername();
    MainWindow mainWin(username);
    mainWin.show();

    return app.exec();
}
