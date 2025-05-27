// src/ui/savegamedialog.h
#ifndef SAVEGAMEDIALOG_H
#define SAVEGAMEDIALOG_H

#include <QDialog>
#include <QLineEdit>      // 单行文本输入框
#include <QPushButton>    // 按钮
#include <QVBoxLayout>    // 垂直布局
#include <QHBoxLayout>    // 水平布局
#include <QLabel>         // 标签

class SaveGameDialog : public QDialog {
    Q_OBJECT // 声明为QObject，以便使用信号和槽

public:
    // 构造函数，parent为父控件
    explicit SaveGameDialog(QWidget* parent = nullptr);
    // 析构函数
    ~SaveGameDialog();

    // 获取用户输入的存档名称
    QString getSaveName() const;

protected:
    // 重写accept函数，当用户点击"保存"按钮时调用
    void accept() override;

    // 重写reject函数，当用户点击"取消"按钮或关闭对话框时调用
    void reject() override;

private slots:
    // 按钮点击的槽函数
    void onSaveClicked();   // "保存"按钮点击
    void onCancelClicked(); // "取消"按钮点击

    // 输入框文本变化的槽函数
    void onSaveNameChanged(const QString& text); // 当存档名称输入框内容改变时

private:
    // UI组件指针
    QLineEdit* m_saveNameEdit;     // 存档名称输入框
    QPushButton* m_saveButton;     // "保存"按钮
    QPushButton* m_cancelButton;   // "取消"按钮

    // 初始化UI界面
    void setupUI();
};

#endif // SAVEGAMEDIALOG_H