cmake_minimum_required(VERSION 3.10)
project(ScheduleManager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== 启用 Qt 自动处理 (MOC, UIC, RCC) =====
set(CMAKE_AUTOMOC ON)

# ===== 查找依赖 =====
find_package(Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Threads REQUIRED)

# ===== 可执行文件 =====
add_executable(myschedule
 src/main.cpp
 src/task.cpp
 src/auth.cpp
 src/taskmanager.cpp
 src/loginwindow.cpp
 src/mainwindow.cpp
 src/taskdialog.cpp
 src/audiorecorder.cpp
 src/speechrecognizer.cpp
 src/deepseekclient.cpp
 src/voiceinputdialog.cpp
)

# ===== 链接库 =====
target_link_libraries(myschedule
 Qt5::Widgets
 Qt5::Network
 Threads::Threads
 ssl
 crypto
 pthread
)