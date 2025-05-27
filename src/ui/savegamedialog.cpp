// src/ui/savegamedialog.cpp
#include "savegamedialog.h"
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include "../core/savesystem.h"
#include <QEvent>
#include <QDebug>
#include <QApplication>

SaveGameDialog::SaveGameDialog(QWidget* parent)
    : QDialog(parent),
      m_saveNameEdit(nullptr),
      m_saveButton(nullptr),
      m_cancelButton(nullptr) {

    setWindowTitle("保存游戏");
    setMinimumSize(350, 150);
    setupUI();
}

SaveGameDialog::~SaveGameDialog() {
    // QDialog的子控件会被自动删除
}

bool SaveGameDialog::event(QEvent *event)
{
    // 确保键盘输入和粘贴事件正常工作
    if (event->type() == QEvent::KeyPress) {
        return QDialog::event(event);
    }
    
    // 允许其他事件正常处理
    return QDialog::event(event);
}

void SaveGameDialog::setupUI() {
    // 创建主垂直布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 添加标题标签
    QLabel* titleLabel = new QLabel("输入存档名称:", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; margin-bottom: 5px;");
    mainLayout->addWidget(titleLabel);

    // 创建存档名称输入框
    m_saveNameEdit = new QLineEdit(this);
    m_saveNameEdit->setPlaceholderText("例如：我的冒险_01");
    m_saveNameEdit->setMaxLength(50);

    mainLayout->addWidget(m_saveNameEdit);

    // 创建按钮的水平布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_saveButton = new QPushButton("保存", this);
    m_saveButton->setEnabled(false); // 初始时禁用"保存"按钮
    m_saveButton->setDefault(true);
    buttonLayout->addWidget(m_saveButton);

    m_cancelButton = new QPushButton("取消", this);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    // 连接信号和槽
    connect(m_saveButton, &QPushButton::clicked, this, &SaveGameDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SaveGameDialog::onCancelClicked);
    connect(m_saveNameEdit, &QLineEdit::textChanged, this, &SaveGameDialog::onSaveNameChanged);
    connect(m_saveNameEdit, &QLineEdit::returnPressed, this, [this](){
        if(m_saveButton && m_saveButton->isEnabled()){
            onSaveClicked();
        }
    });
}

QString SaveGameDialog::getSaveName() const {
    return m_saveNameEdit->text().trimmed();
}

void SaveGameDialog::onSaveClicked() {
    QString saveName = getSaveName();

    if (saveName.isEmpty()) {
        QMessageBox::warning(this, "无效名称", "存档名称不能为空。");
        return;
    }

    // 检查存档是否已存在
    SaveSystem* saveSystem = SaveSystem::getInstance();
    QVector<QString> existingSaves = saveSystem->getAvailableSaves();
    if (existingSaves.contains(saveName)) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "确认覆盖",
            QString("存档 '%1' 已存在。确定要覆盖吗?").arg(saveName),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    }

    // 重要修改：在这里直接执行保存操作
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    // 执行实际保存操作并获取结果
    bool saveSuccess = saveSystem->saveGame(saveName);
    
    QApplication::restoreOverrideCursor();
    
    if (saveSuccess) {
        QMessageBox::information(this, "保存成功", 
                                QString("游戏已成功保存为「%1」").arg(saveName));
        accept(); // 成功后关闭对话框
    } else {
        QMessageBox::critical(this, "保存失败", 
                             "无法保存游戏。请检查文件权限或存储空间。");
        // 保存失败不关闭对话框，让用户可以尝试其他名称
    }
}

void SaveGameDialog::onCancelClicked() {
    reject();
}

void SaveGameDialog::onSaveNameChanged(const QString& text) {
    if (m_saveButton) {
        m_saveButton->setEnabled(!text.trimmed().isEmpty());
    }
}