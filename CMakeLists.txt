cmake_minimum_required(VERSION 3.10)
project(ScheduleManager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== 启用 Qt 自动处理 (MOC, UIC, RCC) =====
set(CMAKE_AUTOMOC ON)

# ===== 查找 Qt5 依赖 =====
find_package(Qt5 REQUIRED COMPONENTS Widgets Multimedia)

# ===== 可执行文件：包含所有 .cpp 源文件 =====
add_executable(myschedule
    src/main.cpp
    src/task.cpp
    src/auth.cpp
    src/taskmanager.cpp
    src/loginwindow.cpp
    src/mainwindow.cpp
    src/taskdialog.cpp
    src/audioplayer.cpp
    src/audiorecorder.cpp
)

# ===== 链接库 =====
# ssl + crypto: OpenSSL（密码哈希）
# Qt5::Widgets: Qt 图形界面
# pthread: 多线程支持
target_link_libraries(myschedule
    Qt5::Widgets
    Qt5::Multimedia
    ssl
    crypto
    pthread
)
