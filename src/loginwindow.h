#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

/**
 * @brief LoginWindow - 登录/注册对话框
 *
 * 用户输入用户名和密码，可以进行登录或注册。
 * 密码经过 SHA256 哈希后存储（由 auth.h 的 login/register_user 处理）。
 * 登录成功后 accept()，主程序进入 MainWindow。
 */
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
