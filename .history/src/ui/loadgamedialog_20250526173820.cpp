// src/ui/loadgamedialog.cpp
#include "loadgamedialog.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QDateTime>
#include <QStandardPaths> // 用于获取标准路径

LoadGameDialog::LoadGameDialog(QWidget* parent)
    : QDialog(parent),
      m_savesList(nullptr),
      m_loadButton(nullptr),
      m_deleteButton(nullptr),
      m_cancelButton(nullptr),
      m_selectedSave("") { // 初始化选中的存档名为空

    setWindowTitle("载入游戏"); // 设置窗口标题
    setMinimumSize(400, 300);  // 设置最小尺寸

    setupUI();   // 初始化UI组件
    loadSaves(); // 加载已有存档列表
}

LoadGameDialog::~LoadGameDialog() {
    // QDialog的子控件会被自动删除，无需手动delete m_savesList等
}

void LoadGameDialog::setupUI() {
    // 创建主垂直布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 添加标题标签
    QLabel* titleLabel = new QLabel("选择存档:", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    mainLayout->addWidget(titleLabel);

    // 创建存档列表控件
    m_savesList = new QListWidget(this);
    mainLayout->addWidget(m_savesList);

    // 创建按钮水平布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_loadButton = new QPushButton("载入", this);
    m_loadButton->setEnabled(false); // 初始时禁用，直到有存档被选中
    buttonLayout->addWidget(m_loadButton);

    m_deleteButton = new QPushButton("删除", this);
    m_deleteButton->setEnabled(false); // 初始时禁用
    buttonLayout->addWidget(m_deleteButton);

    m_cancelButton = new QPushButton("取消", this);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout); // 将按钮布局添加到主布局

    // 设置对话框的布局
    setLayout(mainLayout);

    // 连接信号和槽
    connect(m_loadButton, &QPushButton::clicked, this, &LoadGameDialog::onLoadClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &LoadGameDialog::onDeleteClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &LoadGameDialog::onCancelClicked);
    connect(m_savesList, &QListWidget::itemClicked, this, &LoadGameDialog::onSaveSelected); // 列表项点击时
}

void LoadGameDialog::loadSaves() {
    m_savesList->clear(); // 清空现有列表项

    SaveSystem* saveSystem = SaveSystem::getInstance(); // 获取存档系统实例
    QVector<QString> saves = saveSystem->getAvailableSaves(); // 获取所有可用存档名称

    for (const QString& saveName : saves) {
        QListWidgetItem* item = new QListWidgetItem(saveName); // 为每个存档创建一个列表项

        // 尝试获取存档文件的元数据，例如创建时间，作为提示信息
        QString savePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) +
                           "/saves/" +
                           saveName +
                           ".json"; // 构建存档文件的完整路径

        QFileInfo fileInfo(savePath);
        if (fileInfo.exists()) {
            // QDateTime creationTime = fileInfo.birthTime(); // 获取创建时间 (birthTime可能平台不一致)
            QDateTime lastModifiedTime = fileInfo.lastModified(); // 使用最后修改时间更普遍
            item->setToolTip(QString("最后修改: %1").arg(lastModifiedTime.toString("yyyy-MM-dd hh:mm:ss")));
        }

        m_savesList->addItem(item); // 将列表项添加到列表中
    }
}

QString LoadGameDialog::getSelectedSave() const {
    return m_selectedSave; // 返回当前选中的存档名称
}

void LoadGameDialog::onLoadClicked() {
    if (!m_selectedSave.isEmpty()) {
        accept(); // 如果有选中的存档，则接受对话框 (关闭并返回QDialog::Accepted)
    }
}

void LoadGameDialog::onDeleteClicked() {
    if (!m_selectedSave.isEmpty()) {
        // 弹出确认删除对话框
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "确认删除", QString("确定要删除存档 '%1' 吗? 此操作不可恢复。").arg(m_selectedSave),
                                     QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            SaveSystem* saveSystem = SaveSystem::getInstance();
            if (saveSystem->deleteSave(m_selectedSave)) {
                loadSaves(); // 删除成功后重新加载存档列表
                m_selectedSave = ""; // 清空选中项
                m_loadButton->setEnabled(false);  // 禁用按钮
                m_deleteButton->setEnabled(false);
            } else {
                QMessageBox::critical(this, "错误", "无法删除存档文件。请检查文件权限或是否存在。");
            }
        }
    }
}

void LoadGameDialog::onCancelClicked() {
    reject(); // 用户点击取消，则拒绝对话框 (关闭并返回QDialog::Rejected)
}

void LoadGameDialog::onSaveSelected(QListWidgetItem* item) {
    if (item) {
        m_selectedSave = item->text(); // 更新选中的存档名称
        m_loadButton->setEnabled(true);   // 启用载入和删除按钮
        m_deleteButton->setEnabled(true);
    } else { // 如果没有项被选中 (例如列表被清空后)
        m_selectedSave = "";
        m_loadButton->setEnabled(false);
        m_deleteButton->setEnabled(false);
    }
}