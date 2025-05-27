// src/ui/savegamedialog.cpp
#include "savegamedialog.h"
#include <QMessageBox> // 用于显示消息框
#include <QRegularExpressionValidator> // 用于验证输入框内容
#include <QRegularExpression>       // 正则表达式
#include "../core/savesystem.h" // 存档系统类

SaveGameDialog::SaveGameDialog(QWidget* parent)
    : QDialog(parent),
      m_saveNameEdit(nullptr),
      m_saveButton(nullptr),
      m_cancelButton(nullptr) {

    setWindowTitle("保存游戏"); // 设置窗口标题
    setMinimumSize(350, 150);  // 设置对话框最小尺寸

    setupUI(); // 初始化UI组件
}

SaveGameDialog::~SaveGameDialog() {
    // QDialog的子控件会被自动删除
}

bool SaveGameDialog::event(QEvent *event)
{
    // 处理键盘事件
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        
        // 处理 ESC 键
        if (keyEvent->key() == Qt::Key_Escape) {
            onCancelClicked();
            return true;
        }
        
        // 处理回车键 - 在输入框有焦点且输入有效时触发保存
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (m_saveNameEdit->hasFocus() && !m_saveNameEdit->text().trimmed().isEmpty()) {
                onSaveClicked();
                return true;
            }
        }
        
        // 如果是中文输入法相关键，则允许通过
        if (keyEvent->text().contains(QRegularExpression("[\\u4e00-\\u9fa5]"))) {
            return QDialog::event(event);
        }
    }
    
    // 默认处理
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
    m_saveNameEdit->setPlaceholderText("例如：我的冒险_01"); // 设置占位提示文本
    m_saveNameEdit->setMaxLength(50); // 限制存档名称最大长度

    // 设置输入验证器：只允许字母、数字、下划线、连字符和中文
    // \u4e00-\u9fa5 是中文字符的Unicode范围
    QRegularExpression rx("[a-zA-Z0-9_\\-\\u4e00-\\u9fa5]+");
    QValidator* validator = new QRegularExpressionValidator(rx, this);
    m_saveNameEdit->setValidator(validator);

    mainLayout->addWidget(m_saveNameEdit);

    // 创建按钮的水平布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_saveButton = new QPushButton("保存", this);
    m_saveButton->setEnabled(true); // 初始时禁用"保存"按钮，直到用户输入内容
    m_saveButton->setDefault(true); // 设置为默认按钮 (按Enter键时触发)
    buttonLayout->addWidget(m_saveButton);

    m_cancelButton = new QPushButton("取消", this);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout); // 将按钮布局添加到主布局

    // 设置对话框的布局
    setLayout(mainLayout);

    // 连接信号和槽
    connect(m_saveButton, &QPushButton::clicked, this, &SaveGameDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SaveGameDialog::onCancelClicked);
    // 当输入框文本改变时，更新"保存"按钮的启用状态
    connect(m_saveNameEdit, &QLineEdit::textChanged, this, &SaveGameDialog::onSaveNameChanged);
    // 当按下回车键时，如果保存按钮可用，则触发保存
    connect(m_saveNameEdit, &QLineEdit::returnPressed, this, [this](){
        if(m_saveButton && m_saveButton->isEnabled()){
            onSaveClicked();
        }
    });
}

QString SaveGameDialog::getSaveName() const {
    // 返回用户输入的、经过修剪（去除首尾空格）的存档名称
    return m_saveNameEdit->text().trimmed();
}

void SaveGameDialog::onSaveClicked() {
    QString saveName = getSaveName(); // 获取存档名
    if (saveName.isEmpty()) { // 再次检查是否为空 (虽然按钮状态已控制，但作为保险)
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
            return; // 用户取消覆盖
        }
    }
    accept(); // 接受对话框 (关闭并返回QDialog::Accepted)
}

void SaveGameDialog::onCancelClicked() {
    reject(); // 拒绝对话框 (关闭并返回QDialog::Rejected)
}

void SaveGameDialog::onSaveNameChanged(const QString& text) {
    // 当输入框文本改变时，如果文本非空，则启用"保存"按钮，否则禁用
    if (m_saveButton) {
        m_saveButton->setEnabled(!text.trimmed().isEmpty());
    }
}