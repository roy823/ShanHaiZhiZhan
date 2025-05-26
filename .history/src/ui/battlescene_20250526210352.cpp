// src/ui/battlescene.cpp
#include "battlescene.h"
#include "../battle/battlesystem.h" // 引入战斗系统
#include "../core/creature.h"     // 引入精灵类
#include "../battle/skill.h"      // 引入技能类
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QScrollArea>
#include <QscrollBar>
#include <QFont>
#include <QPixmap>

// 技能按钮类实现
class SkillButton : public QPushButton
{
    Q_OBJECT

public:
    // 构造函数，index为技能索引，skill为技能指针，ownerCreature为技能所属的精灵，parent为父控件
    SkillButton(int index, Skill *skill, Creature* ownerCreature, QWidget *parent = nullptr)
        : QPushButton(parent), m_index(index), m_skill(skill), m_ownerCreature(ownerCreature)
    {
        updateText(); // 初始化时更新按钮文本

        // 连接按钮点击信号到自定义槽函数
        connect(this, &QPushButton::clicked, this, &SkillButton::onClicked);
    }

    // 设置技能，并更新按钮状态
    void setSkill(Skill *skill, Creature* ownerCreature)
    {
        m_skill = skill;
        m_ownerCreature = ownerCreature;
        
        if (skill && ownerCreature)
        {
            // 设置按钮文字
            QString skillCategoryText;
            switch (skill->getCategory())
            {
            case SkillCategory::PHYSICAL: skillCategoryText = "物理"; break;
            case SkillCategory::SPECIAL: skillCategoryText = "特殊"; break;
            case SkillCategory::STATUS: skillCategoryText = "属性"; break;
            }

            // 显示技能名称、属性和当前PP
            setText(QString("%1\n%2 | %3")
                    .arg(skill->getName())
                    .arg(Type::getElementTypeName(skill->getType()))
                    .arg(skillCategoryText));
                    
            // 设置是否可用（根据PP是否足够）
            bool canUse = ownerCreature->getCurrentPP() >= skill->getPPCost() && ownerCreature->canAct();
            setEnabled(canUse);
            
            // 设置详细的技能提示信息
            QString tooltipText = QString(
                "<h3>%1</h3>"
                "<b>系别:</b> %2<br>"
                "<b>类别:</b> %3<br>"
                "<b>威力:</b> %4<br>"
                "<b>命中:</b> %5<br>"
                "<b>PP消耗:</b> %6/%7<br><br>"
                "<b>效果:</b> %8")
                .arg(skill->getName())
                .arg(Type::getElementTypeName(skill->getType()))
                .arg(skillCategoryText)
                .arg(skill->getPower())
                .arg(skill->isAlwaysHit() ? "必中" : QString::number(skill->getAccuracy()) + "%")
                .arg(skill->getPPCost())
                .arg(ownerCreature->getCurrentPP())
                .arg(skill->getDescription());
                
            setToolTip(tooltipText);
            
            // 根据PP是否足够设置按钮样式
            if(!canUse) {
                setStyleSheet("background-color: #ffcccc; color: #888888;");
            } else {
                setStyleSheet("");
            }
        }
        else
            {
                setText("--");
                setEnabled(false);
                setToolTip("");
                setStyleSheet("");
            }   
}

signals:
    // 技能被选择信号，传递技能索引
    void skillSelected(int index);

private slots:
    // 按钮点击槽函数
    void onClicked()
    {
        // 如果技能存在且按钮可用
        if (m_skill && isEnabled())
        {
            emit skillSelected(m_index); // 发出技能选择信号
        }
    }

private:
    int m_index;                // 技能索引 (0-3 for normal skills)
    Skill *m_skill;             // 指向技能对象的指针
    Creature *m_ownerCreature;  // 指向技能所属精灵的指针

    // 更新按钮文本和样式
    void updateText()
    {
        if (m_skill && m_ownerCreature) // 确保技能和所属精灵都存在
        {
            QString skillCategoryText; // 技能类型文本
            switch (m_skill->getCategory())
            {
            case SkillCategory::PHYSICAL:
                skillCategoryText = "物理";
                break;
            case SkillCategory::SPECIAL:
                skillCategoryText = "特殊";
                break;
            case SkillCategory::STATUS:
                skillCategoryText = "属性"; // 设计文档中为“状态”，统一为“属性”
                break;
            }

            // 格式化按钮文本，显示技能名、系别、类别和PP消耗
            QString text = QString("%1\n%2 | %3 PP Cost: %4")
                               .arg(m_skill->getName())
                               .arg(Type::getElementTypeName(m_skill->getType()))
                               .arg(skillCategoryText)
                               .arg(m_skill->getPPCost()); // 显示技能的PP消耗

            setText(text);

            // 根据技能系别设置按钮样式 (背景色)
            QString typeColor = Type::getElementTypeColor(m_skill->getType());
            setStyleSheet(QString("QPushButton { background-color: %1; color: white; padding: 5px; border-radius: 5px; }").arg(typeColor));
        }
        else
        {
            setText("--"); // 若无技能，显示"--"
            // 设置灰色背景
            setStyleSheet("QPushButton { background-color: gray; color: white; padding: 5px; border-radius: 5px; }");
        }
    }
};

// 战斗场景实现
BattleScene::BattleScene(GameEngine *gameEngine, QWidget *parent) : QWidget(parent),
                                                                    m_gameEngine(gameEngine),
                                                                    m_battleSystem(gameEngine->getBattleSystem()),
                                                                    m_playerCreatureLabel(nullptr),
                                                                    m_opponentCreatureLabel(nullptr),
                                                                    m_playerHPBar(nullptr),
                                                                    m_playerPPBar(nullptr),
                                                                    m_opponentHPBar(nullptr),
                                                                    m_opponentPPBar(nullptr),
                                                                    m_playerStatusLabel(nullptr),
                                                                    m_opponentStatusLabel(nullptr),
                                                                    m_battleLogLabel(nullptr),
                                                                    m_turnLabel(nullptr),
                                                                    m_switchButton(nullptr),
                                                                    m_escapeButton(nullptr),
                                                                    m_fifthSkillButton(nullptr),
                                                                    m_restorePPButton(nullptr),
                                                                    m_mainLayout(nullptr),
                                                                    m_battlefieldLayout(nullptr),
                                                                    m_playerLayout(nullptr),
                                                                    m_opponentLayout(nullptr),
                                                                    m_actionLayout(nullptr),
                                                                    m_logLayout(nullptr),
                                                                    m_animationTimer(nullptr)
{
    setupUI();  // 初始化UI元素

    // 连接战斗系统信号到场景的槽函数
    connect(m_battleSystem, &BattleSystem::battleStarted, this, &BattleScene::initScene);
    connect(m_battleSystem, &BattleSystem::battleEnded, this, [this](BattleResult result)
            {
        // 显示战斗结果
        QString resultMessage;
        // 根据战斗结果设置消息文本
        switch (result) {
            case BattleResult::PLAYER_WIN: resultMessage = "你赢了!"; break;
            case BattleResult::OPPONENT_WIN: resultMessage = "你输了!"; break;
            case BattleResult::DRAW: resultMessage = "平局!"; break;
            case BattleResult::ESCAPE: resultMessage = "成功逃脱!"; break;
            default: resultMessage = "战斗结束";
        }
        updateBattleLog(resultMessage); // 在日志中显示结果
        disableAllActionButtons();      // 战斗结束，禁用所有行动按钮
    });

    connect(m_battleSystem, &BattleSystem::turnStarted, this, &BattleScene::onTurnStarted);
    connect(m_battleSystem, &BattleSystem::turnEnded, this, &BattleScene::onTurnEnded);
    connect(m_battleSystem, &BattleSystem::damageCaused, this, &BattleScene::onDamageCaused);
    connect(m_battleSystem, &BattleSystem::healingReceived, this, &BattleScene::onHealingReceived);
    connect(m_battleSystem, &BattleSystem::creatureSwitched, this, &BattleScene::onCreatureSwitched);
    connect(m_battleSystem, &BattleSystem::battleLogUpdated, this, &BattleScene::onBattleLogUpdated);
    connect(m_battleSystem, &BattleSystem::playerActionConfirmed, this, &BattleScene::onPlayerActionConfirmed);
    connect(m_battleSystem, &BattleSystem::opponentActionConfirmed, this, &BattleScene::onOpponentActionConfirmed);

    // 设置动画计时器
    m_animationTimer = new QTimer(this);
}

// 辅助函数：禁用所有玩家行动按钮
void BattleScene::disableAllActionButtons() {
    for (auto* btn : m_skillButtons) { if(btn) btn->setEnabled(false); }
    if(m_fifthSkillButton) m_fifthSkillButton->setEnabled(false);
    if(m_switchButton) m_switchButton->setEnabled(false);
    if(m_escapeButton) m_escapeButton->setEnabled(false);
    if(m_restorePPButton) m_restorePPButton->setEnabled(false);
    // 可以在这里更新一个状态提示，例如 "等待行动处理..."
    // updateBattleLog("<i>请等待行动处理...</i>"); // BattleSystem 会记录更具体的日志
}
// 辅助函数：启用玩家行动按钮
void BattleScene::enablePlayerActionButtons() {
    Creature* playerCreature = nullptr;
    if (m_battleSystem) playerCreature = m_battleSystem->getPlayerActiveCreature();

    // 玩家精灵必须存在、未濒死且能够行动，才能启用按钮
    bool canPlayerReallyAct = playerCreature && !playerCreature->isDead() && playerCreature->canAct();

    // updateSkillButtons 会根据精灵状态和PP更新各个技能按钮的可用性
    updateSkillButtons(); 

    if(m_switchButton) {
        bool canSwitch = canPlayerReallyAct && m_gameEngine && m_gameEngine->getPlayerTeam().size() > 1;
        if (canSwitch) { // 进一步检查是否有其他可切换的非濒死精灵
            bool hasSwitchableOption = false;
            for(Creature* c : m_gameEngine->getPlayerTeam()){
                // 必须是队伍中其他精灵，且该精灵存在且未濒死
                if(c != playerCreature && c && !c->isDead()){
                    hasSwitchableOption = true;
                    break;
                }
            }
            m_switchButton->setEnabled(hasSwitchableOption);
        } else {
            m_switchButton->setEnabled(false);
        }
    }
    if(m_escapeButton && m_battleSystem) {
        // 逃跑按钮的可用性：玩家能行动，且不是PvP战斗
        m_escapeButton->setEnabled(canPlayerReallyAct && !m_battleSystem->isPvPBattle());
    }
    if(m_restorePPButton) {
        // 恢复PP按钮的可用性：玩家能行动，且精灵PP未满
        bool canRestore = canPlayerReallyAct && playerCreature && playerCreature->getCurrentPP() < playerCreature->getMaxPP();
        m_restorePPButton->setEnabled(canRestore);
    }
    // 普通技能和第五技能按钮的可用性由 updateSkillButtons() 内部逻辑处理，
    // 它会检查技能是否存在、PP是否足够以及精灵是否能行动。
}

BattleScene::~BattleScene()
{
    // 资源清理：删除动态创建的技能按钮
    // QLayouts会自动删除它们管理的widgets，所以不需要手动删除布局中的QLabel, QProgressBar等
    // SkillButton是指针向量，需要手动删除
    for (auto *btn : m_skillButtons)
    {
        delete btn;
    }
    m_skillButtons.clear();
    // m_animationTimer 会被 Qt的父子对象机制自动删除
}

void BattleScene::setupUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);

    // 创建战场布局 (精灵显示区)
    m_battlefieldLayout = new QHBoxLayout();

    // 创建玩家和对手的垂直布局
    m_playerLayout = new QVBoxLayout();
    m_opponentLayout = new QVBoxLayout();

    // --- 对手方 UI ---
    m_opponentCreatureLabel = new QLabel(this); // 对手精灵图片
    m_opponentCreatureLabel->setMinimumSize(200, 200);
    m_opponentCreatureLabel->setAlignment(Qt::AlignCenter);
    m_opponentLayout->addWidget(m_opponentCreatureLabel);

    QHBoxLayout *oppStatusLayout = new QHBoxLayout(); // 对手状态信息的水平布局

    m_opponentStatusLabel = new QLabel(this); // 对手精灵名称、等级、类型、状态等
    m_opponentStatusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    oppStatusLayout->addWidget(m_opponentStatusLabel);

    m_opponentHPBar = new QProgressBar(this); // 对手HP条
    m_opponentHPBar->setMinimum(0);
    m_opponentHPBar->setTextVisible(true);
    m_opponentHPBar->setFormat("HP: %v/%m");
    oppStatusLayout->addWidget(m_opponentHPBar);

    m_opponentPPBar = new QProgressBar(this); // 对手PP条
    m_opponentPPBar->setMinimum(0);
    m_opponentPPBar->setTextVisible(true);
    m_opponentPPBar->setFormat("PP: %v/%m");
    oppStatusLayout->addWidget(m_opponentPPBar);

    m_opponentLayout->addLayout(oppStatusLayout);

    // --- 玩家方 UI ---
    m_playerCreatureLabel = new QLabel(this); // 玩家精灵图片
    m_playerCreatureLabel->setMinimumSize(200, 200);
    m_playerCreatureLabel->setAlignment(Qt::AlignCenter);
    m_playerLayout->addWidget(m_playerCreatureLabel);

    QHBoxLayout *playerStatusLayout = new QHBoxLayout(); // 玩家状态信息的水平布局

    m_playerStatusLabel = new QLabel(this); // 玩家精灵名称、等级、类型、状态等
    m_playerStatusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    playerStatusLayout->addWidget(m_playerStatusLabel);

    m_playerHPBar = new QProgressBar(this); // 玩家HP条
    m_playerHPBar->setMinimum(0);
    m_playerHPBar->setTextVisible(true);
    m_playerHPBar->setFormat("HP: %v/%m");
    playerStatusLayout->addWidget(m_playerHPBar);

    m_playerPPBar = new QProgressBar(this); // 玩家PP条
    m_playerPPBar->setMinimum(0);
    m_playerPPBar->setTextVisible(true);
    m_playerPPBar->setFormat("PP: %v/%m");
    playerStatusLayout->addWidget(m_playerPPBar);

    m_playerLayout->addLayout(playerStatusLayout);

    // 将玩家和对手布局添加到战场布局
    m_battlefieldLayout->addLayout(m_opponentLayout); // 对手在左
    m_battlefieldLayout->addLayout(m_playerLayout);   // 玩家在右 (或根据实际视觉调整顺序)

    // --- 回合和日志布局 ---
    m_logLayout = new QVBoxLayout();

    m_turnLabel = new QLabel("回合: 1", this); // 回合数显示
    m_turnLabel->setAlignment(Qt::AlignCenter);
    m_turnLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    m_logLayout->addWidget(m_turnLabel);

    QScrollArea *scrollArea = new QScrollArea(this); // 战斗日志滚动区域
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumHeight(100);

    QWidget *scrollContent = new QWidget(scrollArea); // 滚动区域的内容Widget
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);

    m_battleLogLabel = new QLabel(scrollContent); // 战斗日志文本显示
    m_battleLogLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_battleLogLabel->setWordWrap(true);
    m_battleLogLabel->setTextFormat(Qt::RichText); // 支持富文本以显示颜色等

    scrollLayout->addWidget(m_battleLogLabel);
    scrollLayout->addStretch(); // 添加弹性空间，使日志内容置顶

    scrollContent->setLayout(scrollLayout);
    scrollArea->setWidget(scrollContent);

    m_logLayout->addWidget(scrollArea);

    // --- 操作按钮布局 ---
    m_actionLayout = new QGridLayout(); // 使用网格布局

    // 创建4个普通技能按钮
    // 初始化 SkillButton 时需要 Creature 指针，但此时可能还没有激活的 Creature
    // 暂时传入 nullptr，在 initScene 或 updateSkillButtons 中更新
    Creature* initialCreature = nullptr; // 或者从 m_battleSystem 获取初始精灵
    if (m_battleSystem && m_battleSystem->getPlayerActiveCreature()){
        initialCreature = m_battleSystem->getPlayerActiveCreature();
    }

    for (int i = 0; i < 4; ++i)
    {
        SkillButton *skillBtn = new SkillButton(i, nullptr, initialCreature, this);
        connect(skillBtn, &SkillButton::skillSelected, this, &BattleScene::onSkillButtonClicked);
        m_skillButtons.append(skillBtn);
        m_actionLayout->addWidget(skillBtn, i / 2, i % 2); // 2x2 网格排列
    }

    // 创建第五技能按钮
    m_fifthSkillButton = new QPushButton("第五技能", this);
    connect(m_fifthSkillButton, &QPushButton::clicked, this, &BattleScene::onFifthSkillButtonClicked);
    m_actionLayout->addWidget(m_fifthSkillButton, 0, 2, 1, 1); // 放置在技能旁边，可以调整行跨度和列跨度 (row, col, rowSpan, colSpan)

    // 创建恢复PP按钮
    m_restorePPButton = new QPushButton("恢复PP", this);
    connect(m_restorePPButton, &QPushButton::clicked, this, &BattleScene::onRestorePPButtonClicked);
    m_actionLayout->addWidget(m_restorePPButton, 1, 2, 1, 1); // 放置在第五技能下方

    // 创建切换和逃跑按钮
    m_switchButton = new QPushButton("切换精灵", this);
    connect(m_switchButton, &QPushButton::clicked, this, &BattleScene::onSwitchButtonClicked);
    m_actionLayout->addWidget(m_switchButton, 2, 0); // 放置在技能按钮下方

    m_escapeButton = new QPushButton("逃跑", this);
    connect(m_escapeButton, &QPushButton::clicked, this, &BattleScene::onEscapeButtonClicked);
    m_actionLayout->addWidget(m_escapeButton, 2, 1); // 放置在技能按钮下方

    // 将各部分布局添加到主布局
    m_mainLayout->addLayout(m_battlefieldLayout, 3); // 战场占3份伸缩因子
    m_mainLayout->addLayout(m_logLayout, 1);         // 日志占1份
    m_mainLayout->addLayout(m_actionLayout, 1);      // 操作按钮占1份

    // 设置场景的主布局
    setLayout(m_mainLayout);
}

void BattleScene::initScene()
{
    // 更新UI显示 (玩家、对手、技能按钮)
    updatePlayerUI();
    updateOpponentUI();
    updateSkillButtons();

    // 清空战斗日志
    if(m_battleLogLabel) m_battleLogLabel->setText("");

    // 设置初始回合标签
    if(m_turnLabel && m_battleSystem) m_turnLabel->setText(QString("回合: %1").arg(m_battleSystem->getCurrentTurn()));

    // 启用相关按钮
    if(m_switchButton) m_switchButton->setEnabled(true);
    if(m_escapeButton && m_battleSystem) m_escapeButton->setEnabled(!m_battleSystem->isPvPBattle()); // PvP中通常不能逃跑
    if(m_restorePPButton) m_restorePPButton->setEnabled(true); // 初始时允许恢复PP

    // 添加初始战斗日志
    updateBattleLog("<b>战斗开始!</b>");
}

void BattleScene::updatePlayerUI()
{
    if (!m_battleSystem) return;
    Creature *playerCreature = m_battleSystem->getPlayerActiveCreature();
    if (!playerCreature) return; // 如果没有玩家精灵，则不更新

    // 更新精灵图像 (假设图片资源路径为 :/sprites/精灵名小写_back.png)
    QPixmap creaturePixmap(QString(":/sprites/%1_back.png").arg(playerCreature->getName().toLower().replace(' ', '_')));
    if (creaturePixmap.isNull()) // 如果找不到特定精灵的图片，使用默认图片
    {
        creaturePixmap = QPixmap(":/sprites/default_back.png"); // 默认背面图
    }
    if(m_playerCreatureLabel) m_playerCreatureLabel->setPixmap(creaturePixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 更新HP条
    if(m_playerHPBar) {
        m_playerHPBar->setMaximum(playerCreature->getMaxHP());
        m_playerHPBar->setValue(playerCreature->getCurrentHP());
        // 根据HP百分比设置HP条颜色
        double hpRatio = static_cast<double>(playerCreature->getCurrentHP()) / playerCreature->getMaxHP();
        QString hpColor;
        if (hpRatio > 0.5) hpColor = "green";
        else if (hpRatio > 0.25) hpColor = "yellow";
        else hpColor = "red";
        m_playerHPBar->setStyleSheet(QString("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: %1; }").arg(hpColor));
    }

    // 更新PP条 (显示全局PP)
    if(m_playerPPBar) {
        m_playerPPBar->setMaximum(playerCreature->getMaxPP());
        m_playerPPBar->setValue(playerCreature->getCurrentPP());
        m_playerPPBar->setStyleSheet("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: blue; }"); // PP条通常为蓝色
    }

    // 更新状态标签 (名称、等级、类型、能力变化、异常状态)
    if(m_playerStatusLabel) {
        QString statusText = playerCreature->getName() + " Lv." + QString::number(playerCreature->getLevel()) + "\n";
        statusText += "类型: " + Type::getElementTypeName(playerCreature->getType().getPrimaryType());
        if (playerCreature->getType().getSecondaryType() != ElementType::NONE)
        {
            statusText += "/" + Type::getElementTypeName(playerCreature->getType().getSecondaryType());
        }
        QString statStage = getStatStageText(playerCreature->getStatStages()); // 获取能力等级变化文本
        if (!statStage.isEmpty())
        {
            statusText += "\n能力: " + statStage;
        }
        if (playerCreature->getStatusCondition() != StatusCondition::NONE) // 获取异常状态文本
        {
            statusText += "\n状态: " + getStatusText(playerCreature->getStatusCondition());
        }
        m_playerStatusLabel->setText(statusText);
    }
}

void BattleScene::updateOpponentUI()
{
    if (!m_battleSystem) return;
    Creature *opponentCreature = m_battleSystem->getOpponentActiveCreature();
    if (!opponentCreature) return; // 如果没有对手精灵，则不更新

    // 更新精灵图像 (假设图片资源路径为 :/sprites/精灵名小写_front.png)
    QPixmap creaturePixmap(QString(":/sprites/%1_front.png").arg(opponentCreature->getName().toLower().replace(' ', '_')));
    if (creaturePixmap.isNull()) // 如果找不到特定精灵的图片，使用默认图片
    {
        creaturePixmap = QPixmap(":/sprites/default_front.png"); // 默认正面图
    }
    if(m_opponentCreatureLabel) m_opponentCreatureLabel->setPixmap(creaturePixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 更新HP条
    if(m_opponentHPBar) {
        m_opponentHPBar->setMaximum(opponentCreature->getMaxHP());
        m_opponentHPBar->setValue(opponentCreature->getCurrentHP());
        // 根据HP百分比设置HP条颜色
        double hpRatio = static_cast<double>(opponentCreature->getCurrentHP()) / opponentCreature->getMaxHP();
        QString hpColor;
        if (hpRatio > 0.5) hpColor = "green";
        else if (hpRatio > 0.25) hpColor = "yellow";
        else hpColor = "red";
        m_opponentHPBar->setStyleSheet(QString("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: %1; }").arg(hpColor));
    }

    // 更新PP条 (显示全局PP)
    if(m_opponentPPBar) {
        m_opponentPPBar->setMaximum(opponentCreature->getMaxPP());
        m_opponentPPBar->setValue(opponentCreature->getCurrentPP());
        m_opponentPPBar->setStyleSheet("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: blue; }");
    }

    // 更新状态标签
    if(m_opponentStatusLabel) {
        QString statusText = opponentCreature->getName() + " Lv." + QString::number(opponentCreature->getLevel()) + "\n";
        statusText += "类型: " + Type::getElementTypeName(opponentCreature->getType().getPrimaryType());
        if (opponentCreature->getType().getSecondaryType() != ElementType::NONE)
        {
            statusText += "/" + Type::getElementTypeName(opponentCreature->getType().getSecondaryType());
        }
        QString statStage = getStatStageText(opponentCreature->getStatStages());
        if (!statStage.isEmpty())
        {
            statusText += "\n能力: " + statStage;
        }
        if (opponentCreature->getStatusCondition() != StatusCondition::NONE)
        {
            statusText += "\n状态: " + getStatusText(opponentCreature->getStatusCondition());
        }
        m_opponentStatusLabel->setText(statusText);
    }
}

void BattleScene::updateSkillButtons()
{
    if (!m_battleSystem) return;
    Creature *playerCreature = m_battleSystem->getPlayerActiveCreature();
    if (!playerCreature) { 
        // 如果没有玩家精灵，禁用所有技能按钮
        for (int i = 0; i < 4; ++i) {
            if (i < m_skillButtons.size() && m_skillButtons[i]) {
                m_skillButtons[i]->setSkill(nullptr, nullptr);
            }
        }
        if(m_fifthSkillButton) {
            m_fifthSkillButton->setText("第五技能\n--");
            m_fifthSkillButton->setEnabled(false);
        }
        if(m_restorePPButton) {
            m_restorePPButton->setEnabled(false);
        }
        return;
    }

    // 更新4个普通技能按钮
    for (int i = 0; i < 4; ++i)
    {
        if (i < m_skillButtons.size() && m_skillButtons[i]) {
            Skill *skill = nullptr;
            if (i < playerCreature->getSkillCount()) // 检查技能索引是否有效
            {
                skill = playerCreature->getSkill(i);
            }
            m_skillButtons[i]->setSkill(skill, playerCreature); // 传递精灵指针以检查PP
        }
    }

    // 更新第五技能按钮
    if (m_fifthSkillButton) {
        Skill *fifthSkill = playerCreature->getFifthSkill();
        if (fifthSkill)
        {
            QString skillCategoryText;
            switch (fifthSkill->getCategory())
            {
            case SkillCategory::PHYSICAL: skillCategoryText = "物理"; break;
            case SkillCategory::SPECIAL: skillCategoryText = "特殊"; break;
            case SkillCategory::STATUS: skillCategoryText = "属性"; break;
            }
            
            m_fifthSkillButton->setText(QString("%1\n%2 | %3")
                                            .arg(fifthSkill->getName())
                                            .arg(Type::getElementTypeName(fifthSkill->getType()))
                                            .arg(skillCategoryText));
                                            
            bool canUse = playerCreature->getCurrentPP() >= fifthSkill->getPPCost() && playerCreature->canAct();
            m_fifthSkillButton->setEnabled(canUse);
            
            // 设置详细tooltip
            QString tooltipText = QString(
                "<h3>%1</h3>"
                "<b>系别:</b> %2<br>"
                "<b>类别:</b> %3<br>"
                "<b>威力:</b> %4<br>"
                "<b>命中:</b> %5<br>"
                "<b>PP消耗:</b> %6/%7<br><br>"
                "<b>效果:</b> %8")
                .arg(fifthSkill->getName())
                .arg(Type::getElementTypeName(fifthSkill->getType()))
                .arg(skillCategoryText)
                .arg(fifthSkill->getPower())
                .arg(fifthSkill->isAlwaysHit() ? "必中" : QString::number(fifthSkill->getAccuracy()) + "%")
                .arg(fifthSkill->getPPCost())
                .arg(playerCreature->getCurrentPP())
                .arg(fifthSkill->getDescription());
                
            m_fifthSkillButton->setToolTip(tooltipText);
            
            // 根据PP是否足够设置按钮样式
            if(!canUse) {
                m_fifthSkillButton->setStyleSheet("background-color: #ffcccc; color: #888888;");
            } else {
                m_fifthSkillButton->setStyleSheet("");
            }
        }
        else
        {
            m_fifthSkillButton->setText("第五技能\n--");
            m_fifthSkillButton->setEnabled(false);
            m_fifthSkillButton->setToolTip("");
            m_fifthSkillButton->setStyleSheet("");
        }
    }

    // 更新恢复PP按钮的状态
    if (m_restorePPButton) {
        // 允许恢复PP的条件：当前PP未满，且精灵可以行动
        bool canRestore = playerCreature->getCurrentPP() < playerCreature->getMaxPP() && playerCreature->canAct();
        m_restorePPButton->setEnabled(canRestore);
    }
}

void BattleScene::updateBattleLog(const QString &message)
{
    if(!m_battleLogLabel) return;
    QString currentText = m_battleLogLabel->text();
    if (!currentText.isEmpty())
    {
        currentText += "<br>"; // HTML换行
    }
    currentText += message;
    m_battleLogLabel->setText(currentText);

    // 自动滚动到底部 (需要 QScrollArea 和其内部的 QScrollBar)
    QScrollArea* scrollArea = m_battleLogLabel->parentWidget()->parentWidget()->findChild<QScrollArea*>();
    if (scrollArea) {
        scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
    }
}

// --- 按钮点击处理函数 ---
// 玩家点击技能按钮（普通或第五技能）
void BattleScene::onSkillButtonClicked(int skillIndex) {
    // 检查玩家精灵是否能行动
    if(!m_battleSystem || !m_battleSystem->getPlayerActiveCreature() || !m_battleSystem->getPlayerActiveCreature()->canAct()) return;
    // 提交行动给BattleSystem，不再在此处禁用按钮（由onPlayerActionConfirmed处理）
    m_battleSystem->playerSubmittedAction(BattleAction::USE_SKILL, skillIndex);
}

void BattleScene::onFifthSkillButtonClicked() {
    if(!m_battleSystem || !m_battleSystem->getPlayerActiveCreature() || !m_battleSystem->getPlayerActiveCreature()->canAct()) return;
    m_battleSystem->playerSubmittedAction(BattleAction::USE_SKILL, -1); // -1 代表第五技能
}
// 玩家点击恢复PP按钮
void BattleScene::onRestorePPButtonClicked() {
    if (!m_battleSystem || !m_battleSystem->getPlayerActiveCreature() || !m_battleSystem->getPlayerActiveCreature()->canAct()) return;
    
    Creature *playerCreature = m_battleSystem->getPlayerActiveCreature();
    // 确保精灵PP未满才提交恢复PP的行动
    if (playerCreature && playerCreature->getCurrentPP() < playerCreature->getMaxPP()) { 
        m_battleSystem->playerSubmittedAction(BattleAction::RESTORE_PP);
    } else {
        // 可以在UI上提示PP已满或无法行动，但通常按钮的enable状态会阻止无效点击
        updateBattleLog("PP已满或当前无法恢复PP.");
    }
}

// 玩家点击切换精灵按钮
void BattleScene::onSwitchButtonClicked() {
    if (!m_battleSystem || !m_battleSystem->getPlayerActiveCreature() || !m_battleSystem->getPlayerActiveCreature()->canAct()) {
        updateBattleLog("现在无法切换精灵.");
        return;
    }
    // TODO: 此处应弹出一个对话框，让玩家选择要切换到哪只精灵。
    // 作为临时占位实现：尝试切换到队伍中第一个可用的、非当前且未濒死的精灵。
    int targetSwitchIndex = -1;
    QVector<Creature*> playerTeam = m_battleSystem->getPlayerTeam(); // 获取玩家队伍
    Creature* activeCreature = m_battleSystem->getPlayerActiveCreature(); // 获取当前活跃精灵

    for (int i = 0; i < playerTeam.size(); ++i) {
        if (playerTeam[i] != activeCreature && playerTeam[i] && !playerTeam[i]->isDead()) {
            targetSwitchIndex = i; // 找到一个可切换的目标
            break;
        }
    }

    if (targetSwitchIndex != -1) { // 如果找到了可切换的精灵
        m_battleSystem->playerSubmittedAction(BattleAction::SWITCH_CREATURE, targetSwitchIndex);
    } else {
        updateBattleLog("没有其他可切换的精灵!");
    }
}

// 玩家点击逃跑按钮
void BattleScene::onEscapeButtonClicked() {
    if(!m_battleSystem || !m_battleSystem->getPlayerActiveCreature() || !m_battleSystem->getPlayerActiveCreature()->canAct()) return;
    m_battleSystem->playerSubmittedAction(BattleAction::ESCAPE);
}

// --- 响应行动确认 ---
void BattleScene::onPlayerActionConfirmed() {
    disableAllActionButtons(); // 玩家提交行动后，禁用所有行动按钮
    // BattleSystem 会记录更具体的日志，例如 "玩家选择了XX"
    // updateBattleLog("<i>等待对手行动...</i>"); 
}

void BattleScene::onOpponentActionConfirmed() {
    // 可选：如果需要，可以在UI上给出对手已行动的反馈
    // 例如，一个短暂的提示或动画。BattleSystem会记录AI的选择。
    // updateBattleLog("<i>对手已决定行动.</i>");
}

// --- 回合管理槽函数 ---
void BattleScene::onBattleLogUpdated(const QString &message) // 使用传入的 message
{
    if(!m_battleLogLabel) return;
    QString currentText = m_battleLogLabel->text();
    if (!currentText.isEmpty() && !message.isEmpty()) // 只有在旧文本和新消息都不为空时才加换行
    {
        currentText += "<br>"; // HTML换行
    }
    currentText += message; // 追加新的消息
    m_battleLogLabel->setText(currentText);

    // 自动滚动到底部
    QScrollArea* scrollArea = nullptr;
    if (m_battleLogLabel && m_battleLogLabel->parentWidget() && m_battleLogLabel->parentWidget()->parentWidget()) {
        scrollArea = qobject_cast<QScrollArea*>(m_battleLogLabel->parentWidget()->parentWidget());
    }
    if (scrollArea) {
        scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
    }

    // 日志更新可能意味着精灵状态改变，刷新UI和技能按钮
    updatePlayerUI();
    updateOpponentUI();
    updateSkillButtons();
}

// 当新回合的输入阶段开始时调用
void BattleScene::onTurnStarted(int turn, bool isPlayerTurn_unused /* 此参数现在意义不大，因为总是玩家先输入 */) {
    if(m_turnLabel) m_turnLabel->setText(QString("回合: %1").arg(turn));

    // 刷新双方UI状态
    updatePlayerUI();
    updateOpponentUI();
    
    // 如果战斗仍在进行，则为玩家启用行动按钮
    if (m_battleSystem && m_battleSystem->getBattleResult() == BattleResult::ONGOING) {
        enablePlayerActionButtons(); 
        // BattleSystem 的 processTurnInputPhase 会记录 "--- 第 X 回合 ---"
        // UI层面可以额外提示 "轮到你行动了"
        // updateBattleLog(QString("<b>轮到你行动了! (回合 %1)</b>").arg(turn)); 
    } else {
        disableAllActionButtons(); // 如果战斗已结束或系统出错，确保按钮禁用
    }
}

// 当一个完整回合的执行阶段结束后调用
void BattleScene::onTurnEnded(int turn) {
    // 刷新UI，以反映回合结束效果（如中毒掉血、PP恢复等）
    updatePlayerUI();
    updateOpponentUI();
    updateSkillButtons(); // 技能按钮的可用状态可能因PP变化而改变
    // BattleSystem 的 processTurnInputPhase 会记录下一回合的开始
    updateBattleLog(QString("<i>第 %1 回合行动结算完毕.</i>").arg(turn));
}

void BattleScene::onDamageCaused(Creature *creature, int damage)
{
    QLabel *targetLabel = nullptr; // 受到伤害的精灵的图片标签

    if (m_battleSystem && creature == m_battleSystem->getPlayerActiveCreature())
    {
        targetLabel = m_playerCreatureLabel;
    }
    else if (m_battleSystem && creature == m_battleSystem->getOpponentActiveCreature())
    {
        targetLabel = m_opponentCreatureLabel;
    }
    else
    {
        return; // 无效目标
    }

    animateDamage(targetLabel, damage); // 执行伤害动画

    // 更新UI (HP条等会变化)
    updatePlayerUI();
    updateOpponentUI();
}

void BattleScene::onHealingReceived(Creature *creature, int amount)
{
    QLabel *targetLabel = nullptr; // 接受治疗的精灵的图片标签

    if (m_battleSystem && creature == m_battleSystem->getPlayerActiveCreature())
    {
        targetLabel = m_playerCreatureLabel;
    }
    else if (m_battleSystem && creature == m_battleSystem->getOpponentActiveCreature())
    {
        targetLabel = m_opponentCreatureLabel;
    }
    else
    {
        return;
    }

    animateHealing(targetLabel, amount); // 执行治疗动画

    // 更新UI
    updatePlayerUI();
    updateOpponentUI();
}

void BattleScene::onCreatureSwitched(Creature *oldCreature, Creature *newCreature, bool isPlayer)
{
    // 添加日志："[玩家/对手] 切换到了 [新精灵名]!"
    QString message = QString("%1 切换到了 %2!")
                          .arg(isPlayer ? "你" : "对手")
                          .arg(newCreature ? newCreature->getName() : "未知精灵");
    updateBattleLog(message);

    // 更新UI以反映新上场的精灵
    updatePlayerUI();
    updateOpponentUI();
    updateSkillButtons(); // 新精灵的技能和PP需要更新按钮
}

void BattleScene::animateDamage(QLabel *label, int damage)
{
    if (!label) return;

    // 创建伤害数值标签
    QLabel *damageLabel = new QLabel(QString("-%1").arg(damage), this); // `this`作为父对象，确保能显示在BattleScene上
    damageLabel->setAlignment(Qt::AlignCenter);
    damageLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: red; background-color: transparent;"); // 加大字号，透明背景
    damageLabel->adjustSize(); // 根据内容调整大小

    // 定位在受击精灵图像的中心靠上位置
    QPoint spawnPos = label->mapTo(this, label->rect().center()); // 获取label中心点相对于BattleScene的坐标
    damageLabel->move(spawnPos.x() - damageLabel->width() / 2, spawnPos.y() - label->height() / 2 - damageLabel->height()); // 调整使其在精灵图上方
    damageLabel->show();

    // 创建向上飘动动画
    QPropertyAnimation *animation = new QPropertyAnimation(damageLabel, "pos", this); // `this`作为父对象，方便管理
    animation->setDuration(1000); // 动画时长1秒
    animation->setStartValue(damageLabel->pos());
    animation->setEndValue(damageLabel->pos() - QPoint(0, 60)); // 向上移动60像素
    animation->setEasingCurve(QEasingCurve::OutQuad); // 缓动曲线

    // 创建淡出效果
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(damageLabel); // `damageLabel`作为父对象
    damageLabel->setGraphicsEffect(opacityEffect);
    QPropertyAnimation *fadeOut = new QPropertyAnimation(opacityEffect, "opacity", this); // `this`作为父对象
    fadeOut->setDuration(1000);
    fadeOut->setStartValue(1.0); // 完全不透明
    fadeOut->setEndValue(0.0);   // 完全透明
    fadeOut->setEasingCurve(QEasingCurve::Linear);

    // 动画完成后删除伤害标签
    connect(fadeOut, &QPropertyAnimation::finished, damageLabel, &QLabel::deleteLater);

    // 播放动画 (动画对象会在播放完毕后自动删除)
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);

    // 创建精灵受击闪烁效果 (可选)
    QTimer *flashTimer = new QTimer(this);
    connect(flashTimer, &QTimer::timeout, [label, flashTimer, count = 0]() mutable {
        if (!label) { // 安全检查
             if(flashTimer) flashTimer->stop();
             if(flashTimer) flashTimer->deleteLater();
            return;
        }
        label->setVisible(!label->isVisible()); // 切换可见性
        count++;
        if (count >= 6) { // 闪烁3次 (6次切换)
            label->setVisible(true); // 确保最后是可见的
            flashTimer->stop();
            flashTimer->deleteLater(); // 删除计时器
        }
    });
    flashTimer->start(100); // 每100毫秒切换一次
}

void BattleScene::animateHealing(QLabel *label, int amount)
{
    if (!label) return;

    // 创建治疗数值标签
    QLabel *healLabel = new QLabel(QString("+%1").arg(amount), this);
    healLabel->setAlignment(Qt::AlignCenter);
    healLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: green; background-color: transparent;");
    healLabel->adjustSize();

    QPoint spawnPos = label->mapTo(this, label->rect().center());
    healLabel->move(spawnPos.x() - healLabel->width() / 2, spawnPos.y() - label->height() / 2 - healLabel->height());
    healLabel->show();

    // 动画效果 (与伤害动画类似)
    QPropertyAnimation *animation = new QPropertyAnimation(healLabel, "pos", this);
    animation->setDuration(1000);
    animation->setStartValue(healLabel->pos());
    animation->setEndValue(healLabel->pos() - QPoint(0, 60));
    animation->setEasingCurve(QEasingCurve::OutQuad);

    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(healLabel);
    healLabel->setGraphicsEffect(opacityEffect);
    QPropertyAnimation *fadeOut = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeOut->setDuration(1000);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    connect(fadeOut, &QPropertyAnimation::finished, healLabel, &QLabel::deleteLater);

    animation->start(QAbstractAnimation::DeleteWhenStopped);
    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);

    // 精灵治疗时的视觉效果 (例如短暂发绿光，可选)
    // 这里用简单的闪烁代替
    QTimer *flashTimer = new QTimer(this);
    connect(flashTimer, &QTimer::timeout, [label, flashTimer, count = 0]() mutable {
        if (!label) {
             if(flashTimer) flashTimer->stop();
             if(flashTimer) flashTimer->deleteLater();
            return;
        }
        label->setVisible(!label->isVisible());
        count++;
        if (count >= 4) { // 闪烁2次
            label->setVisible(true);
            flashTimer->stop();
            flashTimer->deleteLater();
        }
    });
    flashTimer->start(120);
}

QString BattleScene::getStatusText(StatusCondition condition)
{
    // 将异常状态枚举转换为中文文本
    switch (condition)
    {
    case StatusCondition::POISON: return "中毒";
    case StatusCondition::PARALYZE: return "麻痹";
    case StatusCondition::BURN: return "烧伤";
    case StatusCondition::FREEZE: return "冻伤"; // 设计文档中是“冻伤”
    case StatusCondition::SLEEP: return "睡眠";
    case StatusCondition::CONFUSION: return "混乱";
    case StatusCondition::FEAR: return "害怕"; // 设计文档中是“害怕”
    case StatusCondition::TIRED: return "疲惫"; // 设计文档中是“疲惫”
    case StatusCondition::BLEED: return "流血";
    case StatusCondition::NONE:
    default: return ""; // 无状态则不显示
    }
}

QString BattleScene::getStatStageText(const StatStages &stages)
{
    // 将能力等级变化转换为文本，例如 "攻击+1, 速度-2"
    QStringList changes;
    auto checkAndAppend = [&](StatType type, const QString& name) {
        int stage = stages.getStage(type);
        if (stage != 0) {
            changes << QString("%1%2%3").arg(name).arg(stage > 0 ? "+" : "").arg(stage);
        }
    };

    checkAndAppend(StatType::ATTACK, "物攻");
    checkAndAppend(StatType::DEFENSE, "物防");
    checkAndAppend(StatType::SP_ATTACK, "特攻");
    checkAndAppend(StatType::SP_DEFENSE, "特防");
    checkAndAppend(StatType::SPEED, "速度");
    checkAndAppend(StatType::ACCURACY, "命中");
    checkAndAppend(StatType::EVASION, "闪避");

    return changes.join(", ");
}

// 为了编译Q_OBJECT元对象代码，包含moc文件
#include "battlescene.moc"