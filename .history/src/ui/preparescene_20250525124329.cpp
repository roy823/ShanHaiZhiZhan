#include "preparescene.h"
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QIcon>
#include <QPixmap>
#include <QMessageBox>
#include <>

// 创建详情部件实现
CreatureDetailWidget::CreatureDetailWidget(QWidget* parent) :
    QWidget(parent),
    m_nameLabel(nullptr),
    m_typeLabel(nullptr),
    m_levelLabel(nullptr),
    m_statsLabel(nullptr),
    m_skillsList(nullptr) {
    
    setupUI();
}

void CreatureDetailWidget::setupUI() {
    // 创建主布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // 创建标签
    m_nameLabel = new QLabel(this);
    m_nameLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    layout->addWidget(m_nameLabel);
    
    m_typeLabel = new QLabel(this);
    layout->addWidget(m_typeLabel);
    
    m_levelLabel = new QLabel(this);
    layout->addWidget(m_levelLabel);
    
    m_statsLabel = new QLabel(this);
    layout->addWidget(m_statsLabel);
    
    // 创建技能列表
    QLabel* skillsTitle = new QLabel("技能:", this);
    skillsTitle->setStyleSheet("font-weight: bold;");
    layout->addWidget(skillsTitle);
    
    m_skillsList = new QListWidget(this);
    m_skillsList->setMaximumHeight(100);
    layout->addWidget(m_skillsList);
    
    // 设置布局
    setLayout(layout);
}

void CreatureDetailWidget::updateCreatureInfo(Creature* creature) {
    if (!creature) {
        m_nameLabel->setText("未选择精灵");
        m_typeLabel->setText("");
        m_levelLabel->setText("");
        m_statsLabel->setText("");
        m_skillsList->clear();
        return;
    }
    
    // 更新基本信息
    m_nameLabel->setText(creature->getName());
    
    // 更新属性信息
    QString typeText = "类型: " + Type::getElementTypeName(creature->getType().getPrimaryType());
    if (creature->getType().getSecondaryType() != ElementType::NONE) {
        typeText += "/" + Type::getElementTypeName(creature->getType().getSecondaryType());
    }
    m_typeLabel->setText(typeText);
    
    // 更新等级信息
    m_levelLabel->setText(QString("等级: %1").arg(creature->getLevel()));
      // 更新能力值信息
    BaseStats stats = creature->getCurrentStats();
    QString statsText = QString(
        "HP: %1/%2\n"
        "攻击: %3\n"
        "防御: %4\n"
        "特攻: %5\n"
        "特防: %6\n"
        "速度: %7"
    ).arg(creature->getCurrentHP()).arg(creature->getMaxHP())
     .arg(stats.getStat(StatType::ATTACK))
     .arg(stats.getStat(StatType::DEFENSE))
     .arg(stats.getStat(StatType::SP_ATTACK))
     .arg(stats.getStat(StatType::SP_DEFENSE))
     .arg(stats.getStat(StatType::SPEED));
    m_statsLabel->setText(statsText);
    
    // 更新技能列表
    m_skillsList->clear();    for (int i = 0; i < creature->getSkillCount(); ++i) {
        Skill* skill = creature->getSkill(i);
        
        QString skillText = QString("%1 (%2, %3) PP: %4/%5")
            .arg(skill->getName())
            .arg(Type::getElementTypeName(skill->getType()))
            .arg(skill->getCategory() == SkillCategory::PHYSICAL ? "物理" : 
                 skill->getCategory() == SkillCategory::SPECIAL ? "特殊" : "状态")
            .arg(skill->getCurrentPP())
            .arg(skill->getPPCost());
        
        QListWidgetItem* item = new QListWidgetItem(skillText);
        
        // 设置颜色 (暂时使用默认颜色)
        item->setForeground(QColor("black"));
        
        m_skillsList->addItem(item);
    }
}

// 准备场景实现
PrepareScene::PrepareScene(GameEngine* gameEngine, QWidget *parent) :
    QWidget(parent),
    m_gameEngine(gameEngine),
    m_selectedPlayerCreatureIndex(-1),
    m_selectedAvailableCreatureIndex(-1),
    m_tabWidget(nullptr),
    m_teamTab(nullptr),
    m_playerCreaturesList(nullptr),
    m_playerCreatureDetail(nullptr),
    m_removeButton(nullptr),
    m_startPvEButton(nullptr),
    m_startPvPButton(nullptr),
    m_creatureLibraryTab(nullptr),
    m_availableCreaturesList(nullptr),
    m_availableCreatureDetail(nullptr),
    m_addButton(nullptr),
    m_bagTab(nullptr),
    m_mainLayout(nullptr) {
    
    setupUI();
    
    // 连接游戏引擎信号
    connect(m_gameEngine, &GameEngine::playerTeamChanged, this, &PrepareScene::onPlayerTeamChanged);
    
    // 初始刷新
    refreshScene();
}

PrepareScene::~PrepareScene() {
    // 资源清理
}

void PrepareScene::setupUI() {
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    
    // 创建选项卡
    m_tabWidget = new QTabWidget(this);
    
    // 创建队伍标签页
    m_teamTab = new QWidget(m_tabWidget);
    QVBoxLayout* teamLayout = new QVBoxLayout(m_teamTab);
    
    QLabel* teamTitle = new QLabel("你的队伍:", m_teamTab);
    teamTitle->setStyleSheet("font-weight: bold; font-size: 16px;");
    teamLayout->addWidget(teamTitle);
    
    QHBoxLayout* teamContentLayout = new QHBoxLayout();
    
    // 创建玩家精灵列表
    m_playerCreaturesList = new QListWidget(m_teamTab);
    m_playerCreaturesList->setMaximumWidth(200);
    teamContentLayout->addWidget(m_playerCreaturesList, 1);
    
    // 创建精灵详情
    m_playerCreatureDetail = new CreatureDetailWidget(m_teamTab);
    teamContentLayout->addWidget(m_playerCreatureDetail, 2);
    
    teamLayout->addLayout(teamContentLayout);
    
    // 创建操作按钮
    QHBoxLayout* teamButtonLayout = new QHBoxLayout();
    
    m_removeButton = new QPushButton("从队伍中移除", m_teamTab);
    m_removeButton->setEnabled(false);
    teamButtonLayout->addWidget(m_removeButton);
    
    m_startPvEButton = new QPushButton("开始PvE对战", m_teamTab);
    m_startPvEButton->setEnabled(false);
    teamButtonLayout->addWidget(m_startPvEButton);
    
    m_startPvPButton = new QPushButton("开始PvP对战", m_teamTab);
    m_startPvPButton->setEnabled(false);
    teamButtonLayout->addWidget(m_startPvPButton);
    
    QPushButton* backButton = new QPushButton("返回主菜单", m_teamTab);
    teamButtonLayout->addWidget(backButton);
    
    teamLayout->addLayout(teamButtonLayout);
    
    m_teamTab->setLayout(teamLayout);
    
    // 创建精灵库标签页
    m_creatureLibraryTab = new QWidget(m_tabWidget);
    QVBoxLayout* libraryLayout = new QVBoxLayout(m_creatureLibraryTab);
    
    QLabel* libraryTitle = new QLabel("可用精灵:", m_creatureLibraryTab);
    libraryTitle->setStyleSheet("font-weight: bold; font-size: 16px;");
    libraryLayout->addWidget(libraryTitle);
    
    QHBoxLayout* libraryContentLayout = new QHBoxLayout();
    
    // 创建可用精灵列表
    m_availableCreaturesList = new QListWidget(m_creatureLibraryTab);
    m_availableCreaturesList->setMaximumWidth(200);
    libraryContentLayout->addWidget(m_availableCreaturesList, 1);
    
    // 创建精灵详情
    m_availableCreatureDetail = new CreatureDetailWidget(m_creatureLibraryTab);
    libraryContentLayout->addWidget(m_availableCreatureDetail, 2);
    
    libraryLayout->addLayout(libraryContentLayout);
    
    // 创建添加按钮
    m_addButton = new QPushButton("添加到队伍", m_creatureLibraryTab);
    m_addButton->setEnabled(false);
    libraryLayout->addWidget(m_addButton);
    
    m_creatureLibraryTab->setLayout(libraryLayout);
    
    // 创建背包标签页
    m_bagTab = new QWidget(m_tabWidget);
    QVBoxLayout* bagLayout = new QVBoxLayout(m_bagTab);
    
    QLabel* bagTitle = new QLabel("道具背包:", m_bagTab);
    bagTitle->setStyleSheet("font-weight: bold; font-size: 16px;");
    bagLayout->addWidget(bagTitle);
    
    QLabel* bagComingSoon = new QLabel("背包功能即将推出...", m_bagTab);
    bagComingSoon->setAlignment(Qt::AlignCenter);
    bagLayout->addWidget(bagComingSoon);
    
    m_bagTab->setLayout(bagLayout);
    
    // 添加标签页到选项卡
    m_tabWidget->addTab(m_teamTab, "队伍");
    m_tabWidget->addTab(m_creatureLibraryTab, "精灵库");
    m_tabWidget->addTab(m_bagTab, "背包");
    
    // 添加选项卡到主布局
    m_mainLayout->addWidget(m_tabWidget);
    
    // 设置布局
    setLayout(m_mainLayout);
    
    // 连接信号
    connect(m_playerCreaturesList, &QListWidget::currentRowChanged, this, &PrepareScene::onPlayerCreatureSelected);
    connect(m_availableCreaturesList, &QListWidget::currentRowChanged, this, &PrepareScene::onAvailableCreatureSelected);
    connect(m_removeButton, &QPushButton::clicked, this, &PrepareScene::onRemoveCreatureClicked);
    connect(m_addButton, &QPushButton::clicked, this, &PrepareScene::onAddCreatureClicked);
    connect(m_startPvEButton, &QPushButton::clicked, this, &PrepareScene::onStartPvEBattleClicked);
    connect(m_startPvPButton, &QPushButton::clicked, this, &PrepareScene::onStartPvPBattleClicked);
    connect(backButton, &QPushButton::clicked, this, &PrepareScene::onBackToMainMenuClicked);
}

void PrepareScene::refreshScene() {
    updatePlayerCreaturesList();
    updateAvailableCreaturesList();
    
    // 重置选择
    m_selectedPlayerCreatureIndex = -1;
    m_playerCreaturesList->setCurrentRow(-1);
    m_playerCreatureDetail->updateCreatureInfo(nullptr);
    m_removeButton->setEnabled(false);
    
    m_selectedAvailableCreatureIndex = -1;
    m_availableCreaturesList->setCurrentRow(-1);
    m_availableCreatureDetail->updateCreatureInfo(nullptr);
    m_addButton->setEnabled(false);
    
    // 更新战斗按钮状态
    m_startPvEButton->setEnabled(m_gameEngine->getPlayerTeam().size() > 0);
    m_startPvPButton->setEnabled(m_gameEngine->getPlayerTeam().size() > 0);
}

void PrepareScene::updatePlayerCreaturesList() {
    m_playerCreaturesList->clear();
    
    // 获取玩家队伍
    const QVector<Creature*>& playerTeam = m_gameEngine->getPlayerTeam();
    
    // 添加精灵到列表
    for (Creature* creature : playerTeam) {
        QString itemText = QString("%1 (Lv.%2)")
            .arg(creature->getName())
            .arg(creature->getLevel());
        
        QListWidgetItem* item = new QListWidgetItem(itemText);
        
        // 尝试加载精灵图标
        QString iconPath = QString(":/sprites/%1_icon.png").arg(creature->getName().toLower().replace(' ', '_'));
        if (QFile::exists(iconPath)) {
            item->setIcon(QIcon(iconPath));
        } else {
            item->setIcon(QIcon(":/sprites/default_icon.png"));
        }
        
        m_playerCreaturesList->addItem(item);
    }
}

void PrepareScene::updateAvailableCreaturesList() {
    m_availableCreaturesList->clear();
    
    // 获取所有可用精灵
    const QVector<Creature*>& availableCreatures = m_gameEngine->getAvailableCreatures();
    
    // 添加精灵到列表
    for (Creature* creature : availableCreatures) {
        QString itemText = QString("%1 (Lv.%2)")
            .arg(creature->getName())
            .arg(creature->getLevel());
        
        QListWidgetItem* item = new QListWidgetItem(itemText);
        
        // 尝试加载精灵图标
        QString iconPath = QString(":/sprites/%1_icon.png").arg(creature->getName().toLower().replace(' ', '_'));
        if (QFile::exists(iconPath)) {
            item->setIcon(QIcon(iconPath));
        } else {
            item->setIcon(QIcon(":/sprites/default_icon.png"));
        }
        
        m_availableCreaturesList->addItem(item);
    }
}

Creature* PrepareScene::getSelectedPlayerCreature() const {
    if (m_selectedPlayerCreatureIndex < 0 || m_selectedPlayerCreatureIndex >= m_gameEngine->getPlayerTeam().size()) {
        return nullptr;
    }
    
    return m_gameEngine->getPlayerTeam()[m_selectedPlayerCreatureIndex];
}

Creature* PrepareScene::getSelectedAvailableCreature() const {
    if (m_selectedAvailableCreatureIndex < 0 || m_selectedAvailableCreatureIndex >= m_gameEngine->getAvailableCreatures().size()) {
        return nullptr;
    }
    
    return m_gameEngine->getAvailableCreatures()[m_selectedAvailableCreatureIndex];
}

void PrepareScene::onPlayerCreatureSelected(int index) {
    m_selectedPlayerCreatureIndex = index;
    m_removeButton->setEnabled(index >= 0);
    
    m_playerCreatureDetail->updateCreatureInfo(getSelectedPlayerCreature());
}

void PrepareScene::onAvailableCreatureSelected(int index) {
    m_selectedAvailableCreatureIndex = index;
    m_addButton->setEnabled(index >= 0);
    
    m_availableCreatureDetail->updateCreatureInfo(getSelectedAvailableCreature());
}

void PrepareScene::onAddCreatureClicked() {
    Creature* creature = getSelectedAvailableCreature();
    
    if (creature && m_gameEngine->getPlayerTeam().size() < 6) {
        m_gameEngine->addCreatureToPlayerTeam(creature);
        refreshScene();
    } else if (m_gameEngine->getPlayerTeam().size() >= 6) {
        QMessageBox::information(this, "队伍已满", "你的队伍已经有6只精灵了，无法再添加。");
    }
}

void PrepareScene::onRemoveCreatureClicked() {
    if (m_selectedPlayerCreatureIndex >= 0) {
        m_gameEngine->removeCreatureFromPlayerTeam(m_selectedPlayerCreatureIndex);
        refreshScene();
    }
}

void PrepareScene::onPlayerTeamChanged() {
    refreshScene();
}

void PrepareScene::onStartPvEBattleClicked() {
    if (m_gameEngine->getPlayerTeam().size() > 0) {
        m_gameEngine->startPvEBattle();
    } else {
        QMessageBox::information(this, "无法开始战斗", "你需要至少一只精灵才能开始战斗。");
    }
}

void PrepareScene::onStartPvPBattleClicked() {
    if (m_gameEngine->getPlayerTeam().size() > 0) {
        m_gameEngine->startPvPBattle();
    } else {
        QMessageBox::information(this, "无法开始战斗", "你需要至少一只精灵才能开始战斗。");
    }
}

void PrepareScene::onBackToMainMenuClicked() {
    emit m_gameEngine->returnToMainMenu();
}
