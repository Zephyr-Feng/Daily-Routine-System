#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>


class LoginWindow : public QDialog {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget* parent = nullptr);
    ~LoginWindow() {}

    /** 获取登录成功的用户名 */
    QString getUsername() const { return m_username; }

private slots:
    /** 点击"登录"按钮 */
    void onLogin();

    /** 点击"注册"按钮 */
    void onRegister();

private:
    /** 初始化界面布局 */
    void setupUI();

    QLineEdit* m_usernameEdit;    // 用户名输入框
    QLineEdit* m_passwordEdit;    // 密码输入框
    QPushButton* m_loginBtn;      // 登录按钮
    QPushButton* m_registerBtn;   // 注册按钮
    QString m_username;           // 登录成功的用户名
};

#endif // LOGINWINDOW_H
