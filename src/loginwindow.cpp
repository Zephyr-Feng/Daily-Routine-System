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
    setFixedSize(400, 340);
    setStyleSheet("QDialog { background: #F5F7FA; }");

    // ========== 顶部蓝色标题栏 ==========
    QLabel* headerBg = new QLabel();
    headerBg->setFixedHeight(100);
    headerBg->setStyleSheet(
        "background: qlineargradient(x1:0 y1:0, x2:1 y2:0, "
        "stop:0 #1565C0, stop:1 #1976D2);"
    );

    QLabel* iconLabel = new QLabel("📅", headerBg);
    iconLabel->setStyleSheet("font-size: 36px; background: transparent;");
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setGeometry(0, 8, 400, 40);

    QLabel* titleLabel = new QLabel("日程管理系统", headerBg);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 20px; font-weight: bold; color: #FFFFFF; background: transparent;"
    );
    titleLabel->setGeometry(0, 50, 400, 30);

    // ========== 白色卡片区域 ==========
    QWidget* card = new QWidget();
    card->setStyleSheet(
        "QWidget { background: #FFFFFF; border-radius: 10px; }"
    );

    // 用户名
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText("请输入用户名");
    m_usernameEdit->setMinimumHeight(36);

    // 密码
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("请输入密码");
    m_passwordEdit->setMinimumHeight(36);

    // 按钮
    m_loginBtn    = new QPushButton("登 录");
    m_registerBtn = new QPushButton("注 册");

    m_loginBtn->setMinimumHeight(38);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setStyleSheet(
        "QPushButton { background: #1976D2; color: #FFFFFF; border: none; "
        "border-radius: 6px; font-size: 15px; font-weight: bold; }"
        "QPushButton:hover { background: #1565C0; }"
        "QPushButton:pressed { background: #0D47A1; }"
    );

    m_registerBtn->setMinimumHeight(38);
    m_registerBtn->setCursor(Qt::PointingHandCursor);
    m_registerBtn->setStyleSheet(
        "QPushButton { color: #1976D2; border: 1px solid #1976D2; "
        "border-radius: 6px; font-size: 13px; }"
        "QPushButton:hover { background: #E3F2FD; }"
    );

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);
    btnLayout->addWidget(m_registerBtn);
    btnLayout->addWidget(m_loginBtn);

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(30, 24, 30, 24);
    cardLayout->setSpacing(12);
    cardLayout->addWidget(m_usernameEdit);
    cardLayout->addWidget(m_passwordEdit);
    cardLayout->addSpacing(6);
    cardLayout->addLayout(btnLayout);

    // ========== 总体布局 ==========
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 0, 20, 20);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(headerBg);
    mainLayout->addWidget(card, 1);

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
