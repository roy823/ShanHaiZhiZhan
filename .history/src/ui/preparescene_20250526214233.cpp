// src/ui/preparescene.cpp
#include "preparescene.h"
#include "savegamedialog.h" 
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
#include <QFile> // 用于检查图标文件是否存在

// CreatureDetailWidget 实现 (精灵详情显示部件)
CreatureDetailWidget::CreatureDetailWidget(QWidget* parent) :
    QWidget(parent),
    m_nameLabel(nullptr),
    m_typeLabel(nullptr),
    m_levelLabel(nullptr),
    m_statsLabel(nullptr),
    m_skillsList(nullptr) {

    setupUI(); // 初始化UI
}

void CreatureDetailWidget::setupUI() {
    // 主垂直布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(5); // 设置控件间距

    // 名称标签
    m_nameLabel = new QLabel("精灵名称", this);
    m_nameLabel->setStyleSheet("font-weight: bold; font-size: 18px; margin-bottom: 5px;");
    layout->addWidget(m_nameLabel);

    // 类型标签
    m_typeLabel = new QLabel("类型: ", this);
    m_typeLabel->setStyleSheet("font-size: 14px;");
    layout->addWidget(m_typeLabel);

    // 等级标签
    m_levelLabel = new QLabel("等级: ", this);
    m_levelLabel->setStyleSheet("font-size: 14px;");
    layout->addWidget(m_levelLabel);

    // 属性统计标签 (HP, 攻击等)
    m_statsLabel = new QLabel("属性: ", this);
    m_statsLabel->setStyleSheet("font-size: 14px; line-height: 1.5;"); // 增加行高使多行文本更易读
    m_statsLabel->setWordWrap(true);
    layout->addWidget(m_statsLabel);

    layout->addSpacing(10); // 添加一些垂直间距

    // 技能列表标题
    QLabel* skillsTitle = new QLabel("技能:", this);
    skillsTitle->setStyleSheet("font-weight: bold; font-size: 16px; margin-top: 10px;");
    layout->addWidget(skillsTitle);

    // 技能列表控件
    m_skillsList = new QListWidget(this);
    m_skillsList->setMaximumHeight(150); // 限制最大高度
    m_skillsList->setStyleSheet("font-size: 13px;");
    layout->addWidget(m_skillsList);

    layout->addStretch(); // 添加弹性空间，将内容推向上方

    // 设置部件的布局
    setLayout(layout);
    // 设置一个最小宽度，防止窗口过窄时显示不佳
    setMinimumWidth(250);
}

void CreatureDetailWidget::updateCreatureInfo(Creature* creature) {
    if (!creature) { // 如果没有精灵被选中
        m_nameLabel->setText("未选择精灵");
        m_typeLabel->setText("类型: --");
        m_levelLabel->setText("等级: --");
        m_statsLabel->setText("HP: --/--\n攻击: --\n防御: --\n特攻: --\n特防: --\n速度: --");
        m_skillsList->clear();
        return;
    }

    // 更新基本信息
    m_nameLabel->setText(creature->getName());

    // 更新属性信息 (主属性/副属性)
    QString typeText = "类型: " + Type::getElementTypeName(creature->getType().getPrimaryType());
    if (creature->getType().getSecondaryType() != ElementType::NONE && creature->getType().getSecondaryType() != creature->getType().getPrimaryType()) { // 副属性不为NONE且不等于主属性
        typeText += " / " + Type::getElementTypeName(creature->getType().getSecondaryType());
    }
    m_typeLabel->setText(typeText);

    // 更新等级信息
    m_levelLabel->setText(QString("等级: %1").arg(creature->getLevel()));

    // 更新能力值信息 (当前HP/最大HP, 及其他基础属性)
    BaseStats currentStats = creature->getCurrentStats(); // 获取应用了能力等级的当前属性
    BaseStats baseStats = creature->getBaseStats(); // 基础属性用于显示面板，当前属性用于战斗计算

    QString statsText = QString(
        "HP: %1/%2\n"
        "物攻: %3 (基础: %4)\n"
        "物防: %5 (基础: %6)\n"
        "特攻: %7 (基础: %8)\n"
        "特防: %9 (基础: %10)\n"
        "速度: %11 (基础: %12)\n"
        "总PP: %13/%14" // 显示精灵的全局PP
    ).arg(creature->getCurrentHP()).arg(creature->getMaxHP())
     .arg(currentStats.getStat(StatType::ATTACK)).arg(baseStats.getStat(StatType::ATTACK))
     .arg(currentStats.getStat(StatType::DEFENSE)).arg(baseStats.getStat(StatType::DEFENSE))
     .arg(currentStats.getStat(StatType::SP_ATTACK)).arg(baseStats.getStat(StatType::SP_ATTACK))
     .arg(currentStats.getStat(StatType::SP_DEFENSE)).arg(baseStats.getStat(StatType::SP_DEFENSE))
     .arg(currentStats.getStat(StatType::SPEED)).arg(baseStats.getStat(StatType::SPEED))
     .arg(creature->getCurrentPP()).arg(creature->getMaxPP());
    m_statsLabel->setText(statsText);

    // 更新技能列表
    m_skillsList->clear();
    for (int i = 0; i < creature->getSkillCount(); ++i) {
        Skill* skill = creature->getSkill(i);
        if (skill) {
            QString skillCategoryText;
            switch (skill->getCategory()) {
                case SkillCategory::PHYSICAL: skillCategoryText = "物"; break;
                case SkillCategory::SPECIAL:  skillCategoryText = "特"; break;
                case SkillCategory::STATUS:   skillCategoryText = "属"; break;
            }
            // 显示技能名，类型，分类，消耗
            QString skillText = QString("%1 (%2, %3) - 消耗: %4")
                .arg(skill->getName())
                .arg(Type::getElementTypeName(skill->getType()))
                .arg(skillCategoryText)
                .arg(skill->getPPCost());

            QListWidgetItem* item = new QListWidgetItem(skillText);
            // 可以根据技能类型设置item的颜色或图标
            // item->setForeground(QColor(Type::getElementTypeColor(skill->getType())));
            m_skillsList->addItem(item);
        }
    }
    // 显示第五技能 (如果存在)
    Skill* fifthSkill = creature->getFifthSkill();
    if (fifthSkill) {
        QString skillCategoryText;
        switch (fifthSkill->getCategory()) {
            case SkillCategory::PHYSICAL: skillCategoryText = "物"; break;
            case SkillCategory::SPECIAL:  skillCategoryText = "特"; break;
            case SkillCategory::STATUS:   skillCategoryText = "属"; break;
        }
        QString skillText = QString("[五] %1 (%2, %3) - 消耗: %4")
            .arg(fifthSkill->getName())
            .arg(Type::getElementTypeName(fifthSkill->getType()))
            .arg(skillCategoryText)
            .arg(fifthSkill->getPPCost());
        QListWidgetItem* item = new QListWidgetItem(skillText);
        item->setForeground(QColor("purple")); // 第五技能用紫色区分
        m_skillsList->addItem(item);
    }
}

// PrepareScene 实现 (备战场景)
PrepareScene::PrepareScene(GameEngine* gameEngine, QWidget *parent) :
    QWidget(parent),
    m_gameEngine(gameEngine),
    m_selectedPlayerCreatureIndex(-1),    // 初始化未选中任何玩家精灵
    m_selectedAvailableCreatureIndex(-1), // 初始化未选中任何可用精灵
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

    setupUI(); // 初始化UI

    // 连接游戏引擎的信号到槽函数
    if (m_gameEngine) {
        connect(m_gameEngine, &GameEngine::playerTeamChanged, this, &PrepareScene::onPlayerTeamChanged);
    }

    refreshScene(); // 初始刷新场景内容
}

PrepareScene::~PrepareScene() {
    // UI组件的父子关系会自动处理删除，无需显式delete
}

void PrepareScene::setupUI() {
    // 设置背景图片
    QString bgImagePath = ":/images/background/preparation_bg.jpg"; // 修改为你的图片路径
    if (QFile::exists(bgImagePath)) {
        this->setStyleSheet(QString("QWidget#PrepareScene { border-image: url(%1) 0 0 0 0 stretch stretch; }").arg(bgImagePath));
        this->setObjectName("PrepareScene"); // 需要设置objectName才能让带ID的选择器生效
    } else {
        // 可以设置一个备用背景色或者打印警告
        qWarning() << "PrepareScene background image not found:" << bgImagePath;
        this->setStyleSheet("background-color: #e0e0e0;"); // 备用背景色
    }
    // 主垂直布局
    m_mainLayout = new QVBoxLayout(this);

    // 创建选项卡控件
    m_tabWidget = new QTabWidget(this);

    // --- 队伍标签页 ---
    m_teamTab = new QWidget(m_tabWidget);
    QVBoxLayout* teamLayout = new QVBoxLayout(m_teamTab); // 队伍页垂直布局

    QLabel* teamTitle = new QLabel("我的出战队伍 (最多6只):", m_teamTab);
    teamTitle->setStyleSheet("font-weight: bold; font-size: 18px; margin-bottom: 10px;");
    teamLayout->addWidget(teamTitle);

    QHBoxLayout* teamContentLayout = new QHBoxLayout(); // 水平布局，左侧列表，右侧详情

    // 玩家精灵列表
    m_playerCreaturesList = new QListWidget(m_teamTab);
    m_playerCreaturesList->setFixedWidth(220); // 固定宽度
    m_playerCreaturesList->setIconSize(QSize(32,32)); // 设置图标大小
    teamContentLayout->addWidget(m_playerCreaturesList, 1); // 占据1份伸缩空间

    // 玩家精灵详情显示区域
    m_playerCreatureDetail = new CreatureDetailWidget(m_teamTab);
    teamContentLayout->addWidget(m_playerCreatureDetail, 2); // 占据2份伸缩空间

    teamLayout->addLayout(teamContentLayout);

    // 队伍操作按钮布局
    QHBoxLayout* teamButtonLayout = new QHBoxLayout();
    teamButtonLayout->setSpacing(10);

    m_removeButton = new QPushButton("从队伍移除", m_teamTab);
    m_removeButton->setEnabled(false); // 初始禁用
    teamButtonLayout->addWidget(m_removeButton);

    m_startPvEButton = new QPushButton("挑战电脑 (PvE)", m_teamTab);
    m_startPvEButton->setEnabled(false); // 初始禁用，队伍非空时启用
    m_startPvEButton->setStyleSheet("background-color: #2196F3; color: white;");
    teamButtonLayout->addWidget(m_startPvEButton);

    m_startPvPButton = new QPushButton("玩家对战 (PvP)", m_teamTab);
    m_startPvPButton->setEnabled(false); // 初始禁用
    m_startPvPButton->setStyleSheet("background-color: #FF9800; color: white;");
    teamButtonLayout->addWidget(m_startPvPButton);

    teamButtonLayout->addStretch(); // 弹性空间

    QPushButton* backButton = new QPushButton("返回主菜单", m_teamTab);
    backButton->setStyleSheet("background-color: #757575; color: white;");
    teamButtonLayout->addWidget(backButton);

    teamLayout->addLayout(teamButtonLayout);
    m_teamTab->setLayout(teamLayout);

    // --- 精灵库标签页 ---
    m_creatureLibraryTab = new QWidget(m_tabWidget);
    QVBoxLayout* libraryLayout = new QVBoxLayout(m_creatureLibraryTab);

    QLabel* libraryTitle = new QLabel("所有可用精灵:", m_creatureLibraryTab);
    libraryTitle->setStyleSheet("font-weight: bold; font-size: 18px; margin-bottom: 10px;");
    libraryLayout->addWidget(libraryTitle);

    QHBoxLayout* libraryContentLayout = new QHBoxLayout();

    // 可用精灵列表
    m_availableCreaturesList = new QListWidget(m_creatureLibraryTab);
    m_availableCreaturesList->setFixedWidth(220);
    m_availableCreaturesList->setIconSize(QSize(32,32));
    libraryContentLayout->addWidget(m_availableCreaturesList, 1);

    // 可用精灵详情显示
    m_availableCreatureDetail = new CreatureDetailWidget(m_creatureLibraryTab);
    libraryContentLayout->addWidget(m_availableCreatureDetail, 2);

    libraryLayout->addLayout(libraryContentLayout);

    // 添加到队伍按钮
    m_addButton = new QPushButton("添加到队伍", m_creatureLibraryTab);
    m_addButton->setEnabled(false); // 初始禁用
    m_addButton->setStyleSheet("background-color: #4CAF50; color: white;");
    libraryLayout->addWidget(m_addButton, 0, Qt::AlignCenter); // 居中

    m_creatureLibraryTab->setLayout(libraryLayout);

    // --- 背包标签页 ---
    m_bagTab = new QWidget(m_tabWidget);
    QVBoxLayout* bagLayout = new QVBoxLayout(m_bagTab);

    QLabel* bagTitle = new QLabel("我的背包:", m_bagTab);
    bagTitle->setStyleSheet("font-weight: bold; font-size: 18px; margin-bottom: 10px;");
    bagLayout->addWidget(bagTitle);

    QLabel* bagComingSoon = new QLabel("背包功能及道具系统正在开发中...", m_bagTab);
    bagComingSoon->setAlignment(Qt::AlignCenter);
    bagComingSoon->setStyleSheet("font-size: 16px; color: gray;");
    bagLayout->addWidget(bagComingSoon);
    bagLayout->addStretch();

    m_bagTab->setLayout(bagLayout);

    // 将标签页添加到选项卡控件
    m_tabWidget->addTab(m_teamTab, "我的队伍");
    m_tabWidget->addTab(m_creatureLibraryTab, "精灵图鉴"); // 或叫精灵中心/精灵库
    m_tabWidget->addTab(m_bagTab, "背包");

    // 将选项卡控件添加到主布局
    m_mainLayout->addWidget(m_tabWidget);

    // 设置场景的布局
    setLayout(m_mainLayout);

    // 连接UI信号到槽函数
    connect(m_playerCreaturesList, &QListWidget::currentRowChanged, this, &PrepareScene::onPlayerCreatureSelected);
    connect(m_availableCreaturesList, &QListWidget::currentRowChanged, this, &PrepareScene::onAvailableCreatureSelected);
    connect(m_removeButton, &QPushButton::clicked, this, &PrepareScene::onRemoveCreatureClicked);
    connect(m_addButton, &QPushButton::clicked, this, &PrepareScene::onAddCreatureClicked);
    connect(m_startPvEButton, &QPushButton::clicked, this, &PrepareScene::onStartPvEBattleClicked);
    connect(m_startPvPButton, &QPushButton::clicked, this, &PrepareScene::onStartPvPBattleClicked);
    connect(backButton, &QPushButton::clicked, this, &PrepareScene::onBackToMainMenuClicked);
}

void PrepareScene::refreshScene() {
    if (!m_gameEngine) return;

    updatePlayerCreaturesList();    // 更新玩家队伍列表显示
    updateAvailableCreaturesList(); // 更新可用精灵列表显示

    // 重置选中状态和详情显示
    m_selectedPlayerCreatureIndex = -1;
    if (m_playerCreaturesList) m_playerCreaturesList->setCurrentRow(-1); // 清除列表选中
    if (m_playerCreatureDetail) m_playerCreatureDetail->updateCreatureInfo(nullptr); // 清空详情
    if (m_removeButton) m_removeButton->setEnabled(false); // 禁用移除按钮

    m_selectedAvailableCreatureIndex = -1;
    if (m_availableCreaturesList) m_availableCreaturesList->setCurrentRow(-1);
    if (m_availableCreatureDetail) m_availableCreatureDetail->updateCreatureInfo(nullptr);
    if (m_addButton) m_addButton->setEnabled(false);

    // 根据玩家队伍是否为空，更新开始战斗按钮的状态
    bool teamIsNotEmpty = !m_gameEngine->getPlayerTeam().isEmpty();
    if (m_startPvEButton) m_startPvEButton->setEnabled(teamIsNotEmpty);
    if (m_startPvPButton) m_startPvPButton->setEnabled(teamIsNotEmpty);
}

void PrepareScene::updatePlayerCreaturesList() {
    if (!m_playerCreaturesList || !m_gameEngine) return;
    m_playerCreaturesList->clear(); // 清空列表

    const QVector<Creature*>& playerTeam = m_gameEngine->getPlayerTeam(); // 获取当前玩家队伍

    for (Creature* creature : playerTeam) {
        if (creature) {
            QString itemText = QString("%1 (Lv.%2)").arg(creature->getName()).arg(creature->getLevel());
            QListWidgetItem* item = new QListWidgetItem(itemText);

            // 尝试加载精灵图标 (路径示例: :/sprites/精灵名小写_icon.png)
            QString iconPath = QString(":/sprites/%1_icon.png").arg(creature->getName().toLower().replace(' ', '_'));
            if (QFile::exists(iconPath)) { // 检查图标文件是否存在
                item->setIcon(QIcon(iconPath));
            } else {
                item->setIcon(QIcon(":/sprites/default_icon.png")); // 使用默认图标
            }
            m_playerCreaturesList->addItem(item);
        }
    }
}

void PrepareScene::updateAvailableCreaturesList() {
    if (!m_availableCreaturesList || !m_gameEngine) return;
    m_availableCreaturesList->clear();

    // GameEngine需要提供一个获取所有精灵模板或已解锁精灵的方法
    // 假设 GameEngine 有一个 getAvailableCreatures() 方法返回所有设计好的精灵（作为模板或实例）
    const QVector<Creature*>& availableCreatures = m_gameEngine->getAllCreatureTemplates();
    for (Creature* creatureTemplate : availableCreatures) {
        if (creatureTemplate) {
            // 这里我们显示的是精灵的“种类”，等级可以是默认的或者1级
            // 当玩家添加到队伍时，可以创建一个新的实例
            QString itemText = QString("%1 (Lv.%2)").arg(creatureTemplate->getName()).arg(creatureTemplate->getLevel()); // 显示模板等级
            QListWidgetItem* item = new QListWidgetItem(itemText);

            QString iconPath = QString(":/sprites/%1_icon.png").arg(creatureTemplate->getName().toLower().replace(' ', '_'));
            if (QFile::exists(iconPath)) {
                item->setIcon(QIcon(iconPath));
            } else {
                item->setIcon(QIcon(":/sprites/default_icon.png"));
            }
            // 将模板指针存入item，方便后续创建实例
            item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(creatureTemplate)));
            m_availableCreaturesList->addItem(item);
        }
    }
}

Creature* PrepareScene::getSelectedPlayerCreature() const {
    if (!m_gameEngine || m_selectedPlayerCreatureIndex < 0 || m_selectedPlayerCreatureIndex >= m_gameEngine->getPlayerTeam().size()) {
        return nullptr; // 无效索引或队伍为空
    }
    return m_gameEngine->getPlayerTeam()[m_selectedPlayerCreatureIndex];
}

// 获取选中的可用精灵 (这里是从模板列表获取，实际添加时应创建新实例)
Creature* PrepareScene::getSelectedAvailableCreatureTemplate() const {
    if (!m_availableCreaturesList || m_selectedAvailableCreatureIndex < 0) return nullptr;
    QListWidgetItem* item = m_availableCreaturesList->item(m_selectedAvailableCreatureIndex);
    if (item) {
        // 从item中取出之前存入的模板指针
        return static_cast<Creature*>(item->data(Qt::UserRole).value<void*>());
    }
    return nullptr;
}

void PrepareScene::onPlayerCreatureSelected(int index) {
    m_selectedPlayerCreatureIndex = index; // 更新选中索引
    if (m_removeButton) m_removeButton->setEnabled(index >= 0); // 如果选中了精灵，则启用移除按钮

    // 更新右侧的精灵详情显示
    if (m_playerCreatureDetail) m_playerCreatureDetail->updateCreatureInfo(getSelectedPlayerCreature());
}

void PrepareScene::onAvailableCreatureSelected(int index) {
    m_selectedAvailableCreatureIndex = index;
    if (m_addButton) m_addButton->setEnabled(index >= 0); // 启用添加到队伍按钮

    // 更新右侧的精灵详情显示 (显示模板信息)
    if (m_availableCreatureDetail) m_availableCreatureDetail->updateCreatureInfo(getSelectedAvailableCreatureTemplate());
}

void PrepareScene::onAddCreatureClicked() {
    if (!m_gameEngine) return;
    Creature* creatureTemplate = getSelectedAvailableCreatureTemplate(); // 获取选中的精灵模板

    if (creatureTemplate) {
        if (m_gameEngine->getPlayerTeam().size() < 6) { // 检查队伍数量是否已满 (最多6只)
            // 重要：不能直接添加模板，需要创建模板的一个新实例
            // GameEngine 应该提供一个方法根据模板名或类型创建新实例
            Creature* newCreatureInstance = m_gameEngine->createCreature(creatureTemplate->getName(), creatureTemplate->getLevel()); // 假设等级与模板一致
            if (newCreatureInstance) {
                m_gameEngine->addCreatureToPlayerTeam(newCreatureInstance);
                // refreshScene(); // playerTeamChanged信号会自动触发刷新
            } else {
                QMessageBox::critical(this, "错误", "无法创建精灵实例。");
            }
        } else {
            QMessageBox::information(this, "队伍已满", "你的队伍已经有6只精灵了，无法再添加。请先移除部分精灵。");
        }
    }
}

void PrepareScene::onRemoveCreatureClicked() {
    if (!m_gameEngine) return;
    if (m_selectedPlayerCreatureIndex >= 0 && m_selectedPlayerCreatureIndex < m_gameEngine->getPlayerTeam().size()) {
        // 确认是否移除
        Creature* toRemove = m_gameEngine->getPlayerTeam()[m_selectedPlayerCreatureIndex];
        if (toRemove) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "确认移除",
                                             QString("确定要从队伍中移除 '%1' 吗?").arg(toRemove->getName()),
                                             QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                 m_gameEngine->removeCreatureFromPlayerTeam(m_selectedPlayerCreatureIndex);
                // refreshScene(); // playerTeamChanged信号会自动触发刷新
            }
        }
    }
}

void PrepareScene::onPlayerTeamChanged() {
    refreshScene(); // 当玩家队伍发生变化时，刷新整个场景
}

void PrepareScene::onStartPvEBattleClicked() {
    if (!m_gameEngine || m_gameEngine->getPlayerTeam().isEmpty()) {
        QMessageBox::warning(this, "无法开始", "你的队伍中至少需要有一只精灵才能开始对战！");
        return;
    }
    m_gameEngine->startPvEBattle(); // 通知游戏引擎开始PvE战斗
    // 游戏状态的改变会由 GameEngine 发信号，MainWindow 接收后切换场景
}

void PrepareScene::onStartPvPBattleClicked() {
    if (!m_gameEngine || m_gameEngine->getPlayerTeam().isEmpty()) {
        QMessageBox::warning(this, "无法开始", "你的队伍中至少需要有一只精灵才能开始对战！");
        return;
    }
    // TODO: PvP 对战可能需要更复杂的设置，如选择对手或网络连接
    QMessageBox::information(this, "PvP对战", "PvP对战模式正在开发中，当前将使用AI作为对手进行模拟。");
    m_gameEngine->startPvPBattle(); // 通知游戏引擎开始PvP战斗 (目前可能也是打AI)
}

void PrepareScene::onBackToMainMenuClicked() {
    if (m_gameEngine) {
        // 在返回主菜单前，询问是否保存当前进度
        QMessageBox::StandardButton reply = QMessageBox::question(this, "返回主菜单",
                                         "确定要返回主菜单吗？未保存的进度将会丢失。",
                                         QMessageBox::Yes | QMessageBox::Save | QMessageBox::Cancel,
                                         QMessageBox::Cancel); // 默认焦点在Cancel

        if (reply == QMessageBox::Yes) {
            m_gameEngine->setGameState(GameState::MAIN_MENU); // 直接返回
        } else if (reply == QMessageBox::Save) {
            // TODO: 调用保存游戏的逻辑
            // MainWindow* mainWindow = qobject_cast<MainWindow*>(this->parentWidget()->parentWidget()); // 获取MainWindow实例
            // if (mainWindow) {
            //     mainWindow->saveGame(); // 假设MainWindow有saveGame方法
            // }
            SaveGameDialog dialog(this);
            if (dialog.exec() == QDialog::Accepted) {
                QString saveName = dialog.getSaveName();
                if (!saveName.isEmpty()) {
                    if(m_gameEngine->saveGame(saveName)) {
                         QMessageBox::information(this, "成功", "游戏已保存。");
                         m_gameEngine->setGameState(GameState::MAIN_MENU);
                    } else {
                        QMessageBox::critical(this, "错误", "保存失败。");
                    }
                } else {
                     QMessageBox::warning(this, "错误", "存档名不能为空。");
                }
            }
        }
        // 如果是Cancel，则什么都不做
    }
}