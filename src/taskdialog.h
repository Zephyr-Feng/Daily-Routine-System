#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QGroupBox>
#include "task.h"


class TaskDialog : public QDialog {
    Q_OBJECT

public:
    explicit TaskDialog(QWidget* parent = nullptr);

    /** 获取用户填写的任务数据 */
    Task getTask() const;

    /** 编辑模式：加载已有任务到表单 */
    void setTask(const Task& task);

    /** 设置为新建模式（默认）*/
    void setAddMode() { m_editMode = false; }

private slots:
    /** 点击确定按钮 */
    void onAccept();

private:
    /** 初始化界面 */
    void setupUI();

    /** 验证输入合法性 */
    bool validateInput();

    // ===== 输入控件 =====
    QLineEdit* m_nameEdit;       // 任务名称

    // 开始时间
    QSpinBox* m_startYear;
    QSpinBox* m_startMonth;
    QSpinBox* m_startDay;
    QSpinBox* m_startHour;
    QSpinBox* m_startMinute;

    // 提醒时间
    QSpinBox* m_remindYear;
    QSpinBox* m_remindMonth;
    QSpinBox* m_remindDay;
    QSpinBox* m_remindHour;
    QSpinBox* m_remindMinute;

    QComboBox* m_priorityCmb;    // 优先级下拉框
    QComboBox* m_classifyCmb;    // 分类下拉框

    bool m_editMode = false;     // false=新建, true=编辑
    int  m_editId = -1;          // 编辑模式下的任务 id
};

#endif // TASKDIALOG_H
