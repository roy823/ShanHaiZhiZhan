// src/ui/loadgamedialog.h
#ifndef LOADGAMEDIALOG_H
#define LOADGAMEDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "../core/savesystem.h" // 引入存档系统

class LoadGameDialog : public QDialog {
    Q_OBJECT // 声明为QObject，以便使用信号和槽

public:
    // 构造函数，parent为父控件
    explicit LoadGameDialog(QWidget* parent = nullptr);
    // 析构函数
    ~LoadGameDialog();

    // 获取用户选择的存档名称
    QString getSelectedSave() const;

private slots:
    // 按钮点击的槽函数
    void onLoadClicked();   // "载入"按钮点击
    void onDeleteClicked(); // "删除"按钮点击
    void onCancelClicked(); // "取消"按钮点击

    // 列表项选择变化的槽函数
    void onSaveSelected(QListWidgetItem* item); // 当存档列表中的项被点击时

private:
    // UI组件指针
    QListWidget* m_savesList;       // 显示存档的列表
    QPushButton* m_loadButton;      // "载入"按钮
    QPushButton* m_deleteButton;    // "删除"按钮
    QPushButton* m_cancelButton;    // "取消"按钮

    // 存储当前选中的存档名称
    QString m_selectedSave;

    // 初始化UI界面
    void setupUI();
    // 从文件系统加载并显示可用存档
    void loadSaves();
};

#endif // LOADGAMEDIALOG_H