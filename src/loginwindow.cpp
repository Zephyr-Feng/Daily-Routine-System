#include "loginwindow.h"
#include "auth.h"

LoginWindow::LoginWindow(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
}

void LoginWindow::setupUI() {
    // ========== 窗口基本设置 ==========
    setWindowTitle("日程管理系统 - 登录");
    setFixedSize(380, 250);

    // ========== 标题 ==========
    QLabel* titleLabel = new QLabel("📅 日程管理系统");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 20px; font-weight: bold; margin-bottom: 10px;"
    );

    // ========== 用户名输入行 ==========
    QLabel* userLabel = new QLabel("用户名：");
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText("请输入用户名");

    QHBoxLayout* userLayout = new QHBoxLayout();
    userLayout->addWidget(userLabel);
    userLayout->addWidget(m_usernameEdit);

    // ========== 密码输入行 ==========
    QLabel* passLabel = new QLabel("密　码：");
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);  // 密码显示为 ●●●
    m_passwordEdit->setPlaceholderText("请输入密码");

    QHBoxLayout* passLayout = new QHBoxLayout();
    passLayout->addWidget(passLabel);
    passLayout->addWidget(m_passwordEdit);

    // ========== 按钮行 ==========
    m_loginBtn    = new QPushButton("登 录");
    m_registerBtn = new QPushButton("注 册");

    m_loginBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; "
        "padding: 8px 30px; font-size: 14px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #45a049; }"
    );
    m_registerBtn->setStyleSheet(
        "QPushButton { padding: 8px 30px; font-size: 14px; border-radius: 4px; }"
    );

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(m_loginBtn);
    btnLayout->addWidget(m_registerBtn);
    btnLayout->addStretch();

    // ========== 总体布局 ==========
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(userLayout);
    mainLayout->addSpacing(8);
    mainLayout->addLayout(passLayout);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(btnLayout);
    mainLayout->addStretch();

    // ========== 信号连接 ==========
    connect(m_loginBtn,    &QPushButton::clicked, this, &LoginWindow::onLogin);
    connect(m_registerBtn, &QPushButton::clicked, this, &LoginWindow::onRegister);

    // 按回车直接登录
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLogin);
}

void LoginWindow::onLogin() {
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    // ===== 输入校验 =====
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名和密码不能为空！");
        return;
    }

    // ===== 调用 auth.h 的登录函数 =====
    if (login(username.toStdString(), password.toStdString())) {
        m_username = username;
        QMessageBox::information(this, "成功",
            QString("欢迎回来，%1！").arg(username));
        accept();  // QDialog::accept() → 返回 QDialog::Accepted
    } else {
        QMessageBox::warning(this, "登录失败",
            "用户名或密码错误，请重试。\n如果没有账号，请先注册。");
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
    }
}

void LoginWindow::onRegister() {
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    // ===== 输入校验 =====
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名和密码不能为空！");
        return;
    }

    if (password.length() < 3) {
        QMessageBox::warning(this, "提示", "密码至少需要 3 位！");
        return;
    }

    // ===== 调用 auth.h 的注册函数 =====
    if (register_user(username.toStdString(), password.toStdString())) {
        QMessageBox::information(this, "注册成功",
            QString("账号 %1 注册成功！现在可以登录了。").arg(username));
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
    } else {
        QMessageBox::warning(this, "注册失败",
            QString("用户名 %1 已存在，请换一个用户名。").arg(username));
    }
}
