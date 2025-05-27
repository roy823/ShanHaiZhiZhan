#include "loadgamedialog.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QDateTime>
#include <QStandardPaths>

LoadGameDialog::LoadGameDialog(QWidget* parent)
    : QDialog(parent),
      m_savesList(nullptr),
      m_loadButton(nullptr),
      m_deleteButton(nullptr),
      m_cancelButton(nullptr),
      m_selectedSave("") {
    
    setWindowTitle("载入游戏");
    setMinimumSize(400, 300);
    
    setupUI();
    loadSaves();
}

LoadGameDialog::~LoadGameDialog() {
}

void LoadGameDialog::setupUI() {
    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 添加标题
    QLabel* titleLabel = new QLabel("选择存档:", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    mainLayout->addWidget(titleLabel);
    
    // 创建存档列表
    m_savesList = new QListWidget(this);
    mainLayout->addWidget(m_savesList);
    
    // 创建按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_loadButton = new QPushButton("载入", this);
    m_loadButton->setEnabled(false);
    buttonLayout->addWidget(m_loadButton);
    
    m_deleteButton = new QPushButton("删除", this);
    m_deleteButton->setEnabled(false);
    buttonLayout->addWidget(m_deleteButton);
    
    m_cancelButton = new QPushButton("取消", this);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 设置布局
    setLayout(mainLayout);
    
    // 连接信号
    connect(m_loadButton, &QPushButton::clicked, this, &LoadGameDialog::onLoadClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &LoadGameDialog::onDeleteClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &LoadGameDialog::onCancelClicked);
    connect(m_savesList, &QListWidget::itemClicked, this, &LoadGameDialog::onSaveSelected);
}

void LoadGameDialog::loadSaves() {
    m_savesList->clear();
    
    SaveSystem* saveSystem = SaveSystem::getInstance();
    QVector<QString> saves = saveSystem->getAvailableSaves();
    
    for (const QString& saveName : saves) {
        QListWidgetItem* item = new QListWidgetItem(saveName);
        
        // 尝试获取存档文件的创建时间
        QString savePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + 
                           "/saves/" + 
                           saveName + 
                           ".json";
        
        QFileInfo fileInfo(savePath);
        if (fileInfo.exists()) {
            QDateTime creationTime = fileInfo.birthTime();
            item->setToolTip(QString("创建时间: %1").arg(creationTime.toString()));
        }
        
        m_savesList->addItem(item);
    }
}

QString LoadGameDialog::getSelectedSave() const {
    return m_selectedSave;
}

void LoadGameDialog::onLoadClicked() {
    if (!m_selectedSave.isEmpty()) {
        accept();
    }
}

void LoadGameDialog::onDeleteClicked() {
    if (!m_selectedSave.isEmpty()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "确认删除", QString("确定要删除存档 '%1' 吗?").arg(m_selectedSave),
                                     QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            SaveSystem* saveSystem = SaveSystem::getInstance();
            if (saveSystem->deleteSave(m_selectedSave)) {
                loadSaves();
                m_selectedSave = "";
                m_loadButton->setEnabled(false);
                m_deleteButton->setEnabled(false);
            } else {
                QMessageBox::critical(this, "错误", "无法删除存档");
            }
        }
    }
}

void LoadGameDialog::onCancelClicked() {
    reject();
}

void LoadGameDialog::onSaveSelected(QListWidgetItem* item) {
    if (item) {
        m_selectedSave = item->text();
        m_loadButton->setEnabled(true);
        m_deleteButton->setEnabled(true);
    }
}
