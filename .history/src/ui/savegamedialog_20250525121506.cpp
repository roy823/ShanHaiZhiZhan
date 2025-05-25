#include "savegamedialog.h"

#include <QMessageBox>
#include <QRegularExpressionValidator>

SaveGameDialog::SaveGameDialog(QWidget* parent)
    : QDialog(parent),
      m_saveNameEdit(nullptr),
      m_saveButton(nullptr),
      m_cancelButton(nullptr) {
    
    setWindowTitle("保存游戏");
    setMinimumSize(300, 150);
    
    setupUI();
}

SaveGameDialog::~SaveGameDialog() {
}

void SaveGameDialog::setupUI() {
    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 添加标题
    QLabel* titleLabel = new QLabel("输入存档名称:", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    mainLayout->addWidget(titleLabel);
    
    // 创建存档名称输入框
    m_saveNameEdit = new QLineEdit(this);
    m_saveNameEdit->setPlaceholderText("存档名称");
    
    // 只允许使用字母、数字、下划线和中文作为存档名
    QRegularExpressionValidator* validator = new QRegularExpressionValidator(QRegularExpression("[a-zA-Z0-9_\\u4e00-\\u9fa5]+"), this);
    m_saveNameEdit->setValidator(validator);
    
    mainLayout->addWidget(m_saveNameEdit);
    
    // 创建按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_saveButton = new QPushButton("保存", this);
    m_saveButton->setEnabled(false);
    buttonLayout->addWidget(m_saveButton);
    
    m_cancelButton = new QPushButton("取消", this);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 设置布局
    setLayout(mainLayout);
    
    // 连接信号
    connect(m_saveButton, &QPushButton::clicked, this, &SaveGameDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SaveGameDialog::onCancelClicked);
    connect(m_saveNameEdit, &QLineEdit::textChanged, this, &SaveGameDialog::onSaveNameChanged);
}

QString SaveGameDialog::getSaveName() const {
    return m_saveNameEdit->text();
}

void SaveGameDialog::onSaveClicked() {
    if (!m_saveNameEdit->text().isEmpty()) {
        accept();
    }
}

void SaveGameDialog::onCancelClicked() {
    reject();
}

void SaveGameDialog::onSaveNameChanged(const QString& text) {
    m_saveButton->setEnabled(!text.isEmpty());
}
