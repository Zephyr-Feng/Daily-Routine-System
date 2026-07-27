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

    // 使用 Fusion 风格保证跨平台一致
    app.setStyle("Fusion");

    // ===== 全局样式表 =====
    // 配色：主色 #1976D2 深蓝, 强调 #2196F3, 背景 #F5F7FA, 卡片白 #FFFFFF
    app.setStyleSheet(R"(
        /* ── 全局 ── */
        QMainWindow, QDialog {
            background-color: #F5F7FA;
        }

        /* ── 输入框 ── */
        QLineEdit, QTextEdit, QPlainTextEdit {
            border: 1px solid #D0D5DD;
            border-radius: 6px;
            padding: 6px 10px;
            background: #FFFFFF;
            font-size: 13px;
            color: #1D2939;
        }
        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
            border-color: #2196F3;
        }

        /* ── 按钮 ── */
        QPushButton {
            border: 1px solid #D0D5DD;
            border-radius: 6px;
            padding: 6px 16px;
            background: #FFFFFF;
            font-size: 13px;
            color: #344054;
        }
        QPushButton:hover {
            background: #F0F3F8;
            border-color: #2196F3;
        }
        QPushButton:pressed {
            background: #E3E8EF;
        }
        QPushButton[primary="true"] {
            background: #1976D2;
            color: #FFFFFF;
            border: none;
            font-weight: bold;
        }
        QPushButton[primary="true"]:hover {
            background: #1565C0;
        }
        QPushButton[primary="true"]:pressed {
            background: #0D47A1;
        }

        /* ── 下拉框 ── */
        QComboBox {
            border: 1px solid #D0D5DD;
            border-radius: 6px;
            padding: 5px 10px;
            background: #FFFFFF;
            font-size: 13px;
            min-width: 80px;
        }
        QComboBox:hover { border-color: #2196F3; }
        QComboBox QAbstractItemView {
            background: #FFFFFF;
            border: 1px solid #D0D5DD;
            selection-background-color: #E3F2FD;
            selection-color: #1D2939;
        }

        /* ── 微调框 ── */
        QSpinBox {
            border: 1px solid #D0D5DD;
            border-radius: 6px;
            padding: 4px 8px;
            background: #FFFFFF;
            font-size: 13px;
        }
        QSpinBox:focus { border-color: #2196F3; }

        /* ── GroupBox ── */
        QGroupBox {
            border: 1px solid #D0D5DD;
            border-radius: 8px;
            margin-top: 14px;
            padding: 12px 10px 10px 10px;
            background: #FFFFFF;
            font-size: 13px;
            font-weight: bold;
            color: #344054;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
        }

        /* ── 表格 ── */
        QTableView {
            background: #FFFFFF;
            alternate-background-color: #F8FAFC;
            gridline-color: #E8ECF0;
            font-size: 13px;
            border: none;
            selection-background-color: #E3F2FD;
            selection-color: #1D2939;
        }
        QTableView::item {
            padding: 6px 10px;
        }
        QHeaderView::section {
            background: #1976D2;
            color: #FFFFFF;
            padding: 8px 6px;
            font-size: 13px;
            font-weight: bold;
            border: none;
            border-right: 1px solid #1565C0;
        }

        /* ── 日历 ── */
        QCalendarWidget {
            background: #FFFFFF;
            border: 1px solid #E8ECF0;
            border-radius: 8px;
        }
        QCalendarWidget QToolButton {
            color: #344054;
            font-size: 14px;
            font-weight: bold;
            padding: 6px;
            border: none;
            border-radius: 4px;
        }
        QCalendarWidget QToolButton:hover {
            background: #E3F2FD;
        }
        QCalendarWidget QMenu {
            background: #FFFFFF;
        }
        QCalendarWidget QSpinBox {
            font-size: 13px;
        }
        QCalendarWidget QTableView {
            alternate-background-color: transparent;
            selection-background-color: #E3F2FD;
        }

        /* ── 滚动条 ── */
        QScrollBar:vertical {
            border: none;
            background: #F5F7FA;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #C4CDD5;
            border-radius: 4px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #A0AAB5;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar:horizontal {
            border: none;
            background: #F5F7FA;
            height: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal {
            background: #C4CDD5;
            border-radius: 4px;
            min-width: 30px;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }

        /* ── 工具提示 ── */
        QToolTip {
            background: #1D2939;
            color: #FFFFFF;
            border: none;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 12px;
        }

        /* ── 消息框 ── */
        QMessageBox {
            background: #FFFFFF;
        }
        QMessageBox QLabel {
            font-size: 13px;
            color: #1D2939;
        }
    )");

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
