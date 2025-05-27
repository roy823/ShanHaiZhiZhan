#include "battlescene.h"
#include "../battle/battlesystem.h"
// #include "../battle/skill.h" // 已在 battlescene.h 中包含
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
// #include <QScrollArea> // 已在 battlescene.h 中包含
#include <QFont>
#include <QPixmap>
#include <QScrollBar> 
#include <QDebug>     

// SkillButton 类的定义已移至 battlescene.h

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
                                                                    m_logScrollArea(nullptr), // 初始化新增成员
                                                                    m_battleLogLabel(nullptr),
                                                                    m_turnLabel(nullptr),
                                                                    m_skillButtons(),
                                                                    m_fifthSkillButton(nullptr), 
                                                                    m_switchButton(nullptr),
                                                                    m_escapeButton(nullptr),
                                                                    m_mainLayout(nullptr),
                                                                    m_battlefieldLayout(nullptr),
                                                                    m_playerLayout(nullptr),
                                                                    m_opponentLayout(nullptr),
                                                                    m_actionLayout(nullptr),
                                                                    m_logLayout(nullptr),
                                                                    m_animationTimer(nullptr)
{
    // setupUI 和信号连接部分与之前的回复相同，此处省略以避免重复
    // 仅展示关键修改点
    // ... (原有的构造函数代码) ...
    setupUI(); // 调用UI设置
    initScene(); // 初始化场景状态

    // 连接战斗系统信号
    connect(m_battleSystem, &BattleSystem::battleStarted, this, &BattleScene::initScene);
    connect(m_battleSystem, &BattleSystem::battleEnded, this, [this](BattleResult result)
            {
        QString resultMessage;
        switch (result) { /* ... */ } // 省略 case 语句
        updateBattleLog(QString("<b>%1</b>").arg(resultMessage));
        for (auto* btn : m_skillButtons) btn->setEnabled(false);
        if (m_fifthSkillButton) m_fifthSkillButton->setEnabled(false);
        m_switchButton->setEnabled(false);
        m_escapeButton->setEnabled(false); 
    });
    connect(m_battleSystem, &BattleSystem::turnStarted, this, &BattleScene::onTurnStarted);
    connect(m_battleSystem, &BattleSystem::turnEnded, this, &BattleScene::onTurnEnded);
    connect(m_battleSystem, &BattleSystem::damageCaused, this, &BattleScene::onDamageCaused);
    connect(m_battleSystem, &BattleSystem::healingReceived, this, &BattleScene::onHealingReceived);
    connect(m_battleSystem, &BattleSystem::creatureSwitched, this, &BattleScene::onCreatureSwitched);
    connect(m_battleSystem, &BattleSystem::battleLogUpdated, this, &BattleScene::onBattleLogUpdated); // 直接连接
    m_animationTimer = new QTimer(this);
    m_animationTimer->setSingleShot(true); 
}


BattleScene::~BattleScene()
{
    // 资源清理
    // m_skillButtons 中的指针由 QVector 管理，但指向的对象需要 delete
    qDeleteAll(m_skillButtons); // 正确释放 QVector<T*> 中的对象
    m_skillButtons.clear();
    // 其他 QObject 子对象（如 m_fifthSkillButton, m_logScrollArea 等）会被 Qt 自动清理
}

void BattleScene::setupUI()
{
    // ... (大部分 setupUI 代码与上次回复相同，此处省略) ...
    // 仅展示与 m_logScrollArea 和 m_battleLogLabel 相关的修改

    QString panelColor = "#34495E";    // 面板颜色 (略浅)
    QString backgroundColor = "#2C3E50"; // 深蓝灰色背景
    QString defaultFont = "Arial"; 
    this->setStyleSheet(QString("QWidget { background-color: %1; color: white; font-family: %2; }").arg(backgroundColor).arg(defaultFont));
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(12); 
    m_mainLayout->setContentsMargins(10, 10, 10, 10); 

    // ... (战场布局 m_battlefieldLayout, 玩家/对手布局 m_playerLayout, m_opponentLayout 的创建) ...
    // ... (对手精灵UI m_opponentCreatureLabel, m_opponentStatusLabel, m_opponentHPBar 的创建) ...
    // ... (玩家精灵UI m_playerCreatureLabel, m_playerStatusLabel, m_playerHPBar, m_playerPPBar 的创建) ...

    m_battlefieldLayout = new QHBoxLayout();
    m_battlefieldLayout->setSpacing(20);
    // (此处省略了玩家和对手UI面板的详细创建过程，假设它们已正确创建并添加到 m_playerLayout 和 m_opponentLayout)
    m_playerLayout = new QVBoxLayout(); // 示例
    m_opponentLayout = new QVBoxLayout(); // 示例
    // 假设 m_playerCreatureLabel, m_playerHPBar 等已添加到 m_playerLayout
    // 假设 m_opponentCreatureLabel, m_opponentHPBar 等已添加到 m_opponentLayout
    m_battlefieldLayout->addLayout(m_opponentLayout); 
    m_battlefieldLayout->addStretch(1); 
    m_battlefieldLayout->addLayout(m_playerLayout);   


    // --- 回合和日志布局 ---
    m_logLayout = new QVBoxLayout();
    m_logLayout->setSpacing(8);

    m_turnLabel = new QLabel("回合: 1", this);
    m_turnLabel->setAlignment(Qt::AlignCenter);
    m_turnLabel->setStyleSheet("font-weight: bold; font-size: 16px; padding: 6px; background-color: #4A6A8A; border-radius: 5px;");
    m_logLayout->addWidget(m_turnLabel);

    // 创建并赋值给成员变量 m_logScrollArea
    m_logScrollArea = new QScrollArea(this); // 修改这里
    m_logScrollArea->setWidgetResizable(true);
    m_logScrollArea->setMinimumHeight(100); 
    m_logScrollArea->setStyleSheet(QString("QScrollArea { border: 1px solid #4A5A6A; border-radius: 5px; background-color: %1; } QScrollBar:vertical { border: none; background: %2; width: 12px; margin: 0px 0px 0px 0px; border-radius: 6px;} QScrollBar::handle:vertical { background: #5D6D7E; min-height: 25px; border-radius: 5px; } QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; border: none; background: none; }").arg(panelColor).arg(backgroundColor));

    QWidget *scrollContent = new QWidget(m_logScrollArea); // 父对象是 m_logScrollArea
    scrollContent->setStyleSheet(QString("background-color: transparent;")); 
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(5,5,5,5);

    m_battleLogLabel = new QLabel(scrollContent); // 父对象是 scrollContent
    m_battleLogLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_battleLogLabel->setWordWrap(true);
    m_battleLogLabel->setTextFormat(Qt::RichText); 
    m_battleLogLabel->setStyleSheet("padding: 5px; font-size: 12px; background-color: transparent;"); 

    scrollLayout->addWidget(m_battleLogLabel);
    scrollLayout->addStretch();
    scrollContent->setLayout(scrollLayout);
    m_logScrollArea->setWidget(scrollContent); // 设置 m_logScrollArea 的内容
    m_logLayout->addWidget(m_logScrollArea);   // 将 m_logScrollArea 添加到布局


    // --- 创建操作布局 (使用QGridLayout) ---
    // ... (技能按钮, 第五技能按钮, 切换和逃跑按钮的创建和添加到 m_actionLayout) ...
    // ... (与上次回复相同) ...
    QString buttonStyle = "QPushButton { background-color: #5D6D7E; color: white; padding: 8px; border-radius: 5px; font-size: 13px; border: 1px solid #4A5A6A;} QPushButton:hover { background-color: #85929E; } QPushButton:disabled { background-color: #4A5A6A; color: #BDC3C7; }";
    QString fifthSkillButtonStyle = "QPushButton { background-color: #F39C12; color: white; padding: 10px; border-radius: 5px; font-size: 14px; font-weight: bold; border: 1px solid #D35400;} QPushButton:hover { background-color: #F5B041; } QPushButton:disabled { background-color: #B9770E; color: #FAD7A0; }";
    
    m_actionLayout = new QGridLayout();
    m_actionLayout->setSpacing(8); 

    for (int i = 0; i < 4; ++i)
    {
        // SkillButton 的构造函数已在 .h 文件中定义
        SkillButton *skillBtn = new SkillButton(i, nullptr, this);
        skillBtn->setMinimumHeight(55); 
        skillBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        connect(skillBtn, &SkillButton::skillSelected, this, &BattleScene::onSkillButtonClicked);
        m_skillButtons.append(skillBtn);
        m_actionLayout->addWidget(skillBtn, i / 2, i % 2); 
    }

    m_fifthSkillButton = new QPushButton("第五技能", this);
    m_fifthSkillButton->setStyleSheet(fifthSkillButtonStyle); 
    m_fifthSkillButton->setMinimumHeight(45);
    m_fifthSkillButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(m_fifthSkillButton, &QPushButton::clicked, this, &BattleScene::onFifthSkillButtonClicked);
    m_actionLayout->addWidget(m_fifthSkillButton, 2, 0, 1, 2);

    m_switchButton = new QPushButton("切换", this);
    m_switchButton->setStyleSheet(buttonStyle);
    m_switchButton->setMinimumHeight(40);
    m_switchButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(m_switchButton, &QPushButton::clicked, this, &BattleScene::onSwitchButtonClicked);
    m_actionLayout->addWidget(m_switchButton, 3, 0); 

    m_escapeButton = new QPushButton("逃跑", this);
    m_escapeButton->setStyleSheet(buttonStyle);
    m_escapeButton->setMinimumHeight(40);
    m_escapeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(m_escapeButton, &QPushButton::clicked, this, &BattleScene::onEscapeButtonClicked);
    m_actionLayout->addWidget(m_escapeButton, 3, 1); 


    // 将布局添加到主布局
    m_mainLayout->addLayout(m_battlefieldLayout, 5); 
    m_mainLayout->addLayout(m_logLayout, 2);       
    m_mainLayout->addLayout(m_actionLayout, 3);    

    setLayout(m_mainLayout);
}

void BattleScene::initScene()
{
    updatePlayerUI();
    updateOpponentUI();
    updateSkillButtons(); 
    updateFifthSkillButton(); 

    m_battleLogLabel->setText("");
    // 正确滚动日志区域
    if (m_logScrollArea && m_logScrollArea->verticalScrollBar()) { // 修改这里
        m_logScrollArea->verticalScrollBar()->setValue(0); 
    }

    m_turnLabel->setText(QString("回合: %1").arg(m_battleSystem->getCurrentTurn()));

    Creature* playerCreature = m_battleSystem->getPlayerActiveCreature();
    bool canPlayerAct = playerCreature && playerCreature->canAct();
    
    for(SkillButton* btn : m_skillButtons) {
        // SkillButton 内部的 setSkill 和 updateText 会处理 enabled 状态
        // 这里确保基于 canPlayerAct 再做一次判断
        if (btn->getSkillPtr()) { // 检查按钮是否关联了有效技能
             btn->setEnabled(canPlayerAct && btn->getSkillPtr()->getCurrentPP() > 0 && playerCreature->getCurrentPP() >= btn->getSkillPtr()->getPPCost());
        } else {
             btn->setEnabled(false);
        }
    }
    if (m_fifthSkillButton) { 
        updateFifthSkillButton(); // updateFifthSkillButton 会考虑 canPlayerAct
    }
    
    m_switchButton->setEnabled(true); 
    m_escapeButton->setEnabled(!m_battleSystem->isPvPBattle());

    updateBattleLog("<b><font color='#2ECC71'>战斗开始!</font></b>");
}

// ... (updatePlayerUI, updateOpponentUI 与上次回复相同，可省略，但要确保 getStatusText, getStatStageText 的颜色更新应用了)
void BattleScene::updatePlayerUI()
{
    Creature *playerCreature = m_battleSystem->getPlayerActiveCreature();
    if (!playerCreature) return;

    QPixmap creaturePixmap(QString(":/sprites/%1_back.png").arg(playerCreature->getName().toLower().replace(' ', '_')));
    if (creaturePixmap.isNull()) creaturePixmap = QPixmap(":/sprites/default_back.png");
    m_playerCreatureLabel->setPixmap(creaturePixmap.scaled(m_playerCreatureLabel->size() * 0.95, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_playerHPBar->setMaximum(playerCreature->getMaxHP());
    m_playerHPBar->setValue(playerCreature->getCurrentHP());
    double hpRatio = static_cast<double>(playerCreature->getCurrentHP()) / playerCreature->getMaxHP();
    QString hpColor;
    if (hpRatio > 0.5) hpColor = "#2ECC71"; // 绿色
    else if (hpRatio > 0.25) hpColor = "#F1C40F"; // 黄色
    else hpColor = "#E74C3C"; // 红色
    m_playerHPBar->setStyleSheet(QString("QProgressBar { text-align: center; border-radius: 4px; height: 18px; background-color: #78909C; color: white;} QProgressBar::chunk { background-color: %1; border-radius: 3px; margin: 1px;}").arg(hpColor));

    m_playerPPBar->setMaximum(playerCreature->getMaxPP()); 
    m_playerPPBar->setValue(playerCreature->getCurrentPP());
    m_playerPPBar->setStyleSheet(QString("QProgressBar { text-align: center; border-radius: 4px; height: 16px; background-color: #78909C; color: white;} QProgressBar::chunk { background-color: #3498DB; border-radius: 3px; margin: 1px;}"));

    QString statusText = QString("<b>%1</b> <font color='#ECF0F1'>Lv.%2</font>").arg(playerCreature->getName()).arg(playerCreature->getLevel());
    statusText += "<br>属性: " + Type::getTypeName(playerCreature->getType().getPrimaryType()); // 使用 Type::getTypeName
    if (playerCreature->getType().getSecondaryType() != ElementType::NONE)
    {
        statusText += "/" + Type::getTypeName(playerCreature->getType().getSecondaryType()); // 使用 Type::getTypeName
    }
    if (playerCreature->getStatusCondition() != StatusCondition::NONE)
    {
        statusText += QString("<br>状态: %1").arg(getStatusText(playerCreature->getStatusCondition()));
    }
    QString statStageStr = getStatStageText(playerCreature->getStatStages());
    if (!statStageStr.isEmpty()) {
        statusText += "<br><font color='#AED6F1'>变化: " + statStageStr + "</font>";
    }
    m_playerStatusLabel->setText(statusText);
}

void BattleScene::updateOpponentUI()
{
    Creature *opponentCreature = m_battleSystem->getOpponentActiveCreature();
    if (!opponentCreature) return;

    QPixmap creaturePixmap(QString(":/sprites/%1_front.png").arg(opponentCreature->getName().toLower().replace(' ', '_')));
    if (creaturePixmap.isNull()) creaturePixmap = QPixmap(":/sprites/default_front.png");
    m_opponentCreatureLabel->setPixmap(creaturePixmap.scaled(m_opponentCreatureLabel->size() * 0.95, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_opponentHPBar->setMaximum(opponentCreature->getMaxHP());
    m_opponentHPBar->setValue(opponentCreature->getCurrentHP());
    double hpRatio = static_cast<double>(opponentCreature->getCurrentHP()) / opponentCreature->getMaxHP();
    QString hpColor;
    if (hpRatio > 0.5) hpColor = "#2ECC71"; 
    else if (hpRatio > 0.25) hpColor = "#F1C40F"; 
    else hpColor = "#E74C3C"; 
    m_opponentHPBar->setStyleSheet(QString("QProgressBar { text-align: center; border-radius: 4px; height: 18px; background-color: #78909C; color: white;} QProgressBar::chunk { background-color: %1; border-radius: 3px; margin: 1px;}").arg(hpColor));

    QString statusText = QString("<b>%1</b> <font color='#ECF0F1'>Lv.%2</font>").arg(opponentCreature->getName()).arg(opponentCreature->getLevel());
    statusText += "<br>属性: " + Type::getTypeName(opponentCreature->getType().getPrimaryType()); // 使用 Type::getTypeName
    if (opponentCreature->getType().getSecondaryType() != ElementType::NONE)
    {
        statusText += "/" + Type::getTypeName(opponentCreature->getType().getSecondaryType()); // 使用 Type::getTypeName
    }
    if (opponentCreature->getStatusCondition() != StatusCondition::NONE)
    {
        statusText += QString("<br>状态: %1").arg(getStatusText(opponentCreature->getStatusCondition()));
    }
    QString statStageStr = getStatStageText(opponentCreature->getStatStages());
    if (!statStageStr.isEmpty()) {
        statusText += "<br><font color='#AED6F1'>变化: " + statStageStr + "</font>";
    }
    m_opponentStatusLabel->setText(statusText);
}
// ...

void BattleScene::updateSkillButtons()
{
    Creature *playerCreature = m_battleSystem->getPlayerActiveCreature();
    bool canPlayerActGlobal = playerCreature && playerCreature->canAct();

    if (!playerCreature) {
        for(SkillButton* btn : m_skillButtons) {
            btn->setSkill(nullptr); 
            btn->setEnabled(false);
        }
        return;
    }

    for (int i = 0; i < 4; ++i)
    {
        Skill *skill = nullptr;
        if (i < playerCreature->getSkillCount())
        {
            skill = playerCreature->getSkill(i);
        }
        m_skillButtons[i]->setSkill(skill); // setSkill 内部会更新文本和基础的 enabled 状态(基于PP)
        // 额外根据行动能力和精灵总PP判断
        if (skill) {
             m_skillButtons[i]->setEnabled(canPlayerActGlobal && skill->getCurrentPP() > 0 && playerCreature->getCurrentPP() >= skill->getPPCost());
        } else {
             m_skillButtons[i]->setEnabled(false);
        }
    }
}

void BattleScene::updateFifthSkillButton() {
    if (!m_fifthSkillButton) return;

    Creature *playerCreature = m_battleSystem->getPlayerActiveCreature();
    bool canPlayerActGlobal = playerCreature && playerCreature->canAct();

    if (!playerCreature) {
        m_fifthSkillButton->setText("第五技能");
        m_fifthSkillButton->setEnabled(false);
        m_fifthSkillButton->setToolTip("没有可操作的精灵。");
        return;
    }

    Skill *fifthSkill = playerCreature->getFifthSkill();

    if (fifthSkill) {
        m_fifthSkillButton->setText(fifthSkill->getName());
        bool skillHasPP = fifthSkill->getCurrentPP() > 0; 
        bool creatureHasEnoughPP = playerCreature->getCurrentPP() >= fifthSkill->getPPCost(); 
        
        m_fifthSkillButton->setEnabled(canPlayerActGlobal && skillHasPP && creatureHasEnoughPP);

        QString categoryText;
        switch(fifthSkill->getCategory()){
            case SkillCategory::PHYSICAL: categoryText = "物理"; break;
            case SkillCategory::SPECIAL: categoryText = "特殊"; break;
            case SkillCategory::STATUS: categoryText = "状态"; break;
            default: categoryText = "未知"; break;
        }

        QString tooltipText = QString("<b>%1</b><br><br><b>属性:</b> %2<br><b>类别:</b> %3<br><b>威力:</b> %4 | <b>消耗精灵PP:</b> %5<br><b>命中:</b> %6<br><b>技能PP:</b> %7/%8<br><hr><b>描述:</b><br>%9")
                                  .arg(fifthSkill->getName())
                                  .arg(Type::getTypeName(fifthSkill->getType())) // 修正调用方式
                                  .arg(categoryText)
                                  .arg(fifthSkill->getPower())
                                  .arg(fifthSkill->getPPCost())
                                  .arg(fifthSkill->getAccuracy() >= 100 ? "必中" : QString::number(fifthSkill->getAccuracy()) + "%")
                                  .arg(fifthSkill->getCurrentPP()) 
                                  .arg(fifthSkill->getMaxPP())     
                                  .arg(fifthSkill->getDescription().replace("\n", "<br>"));
        m_fifthSkillButton->setToolTip(tooltipText);
    } else {
        m_fifthSkillButton->setText("无第五技能");
        m_fifthSkillButton->setEnabled(false);
        m_fifthSkillButton->setToolTip("此精灵没有第五技能。");
    }
}


void BattleScene::updateBattleLog(const QString &message)
{
    QString currentText = m_battleLogLabel->text();
    if (!currentText.isEmpty())
    {
        currentText += "<br>"; 
    }
    m_battleLogLabel->setText(currentText + message);

    QTimer::singleShot(0, [this]() { 
        // 正确访问 QScrollArea 的滚动条
        if (m_logScrollArea && m_logScrollArea->verticalScrollBar()) { // 修改这里
            m_logScrollArea->verticalScrollBar()->setValue(m_logScrollArea->verticalScrollBar()->maximum());
        }
    });
}

// onSkillButtonClicked, onFifthSkillButtonClicked, onSwitchButtonClicked, onEscapeButtonClicked
// onBattleLogUpdated, onTurnStarted, onTurnEnded
// onDamageCaused, onHealingReceived, onCreatureSwitched
// animateDamage, animateHealing, getStatusText, getStatStageText
// 这些函数的实现与上次回复基本一致，主要是确保在合适的时机调用 UI 更新函数，
// 并且在 onTurnStarted 中正确设置所有按钮的 enbabled 状态。

// 示例: onTurnStarted 的关键部分
void BattleScene::onTurnStarted(int turn, bool isPlayerTurn)
{
    updatePlayerUI(); 
    updateOpponentUI();

    m_turnLabel->setText(QString("<b>回合 %1</b> - %2行动").arg(turn).arg(isPlayerTurn ? "<font color='#2ECC71'>我方</font>" : "<font color='#E74C3C'>敌方</font>"));

    Creature* playerCreature = m_battleSystem->getPlayerActiveCreature();
    bool canPlayerAct = playerCreature && playerCreature->canAct();

    if (isPlayerTurn)
    {
        updateSkillButtons(); 
        updateFifthSkillButton();
        
        // 确保如果玩家不能行动，所有按钮都禁用
        if (!canPlayerAct) {
            for (auto *btn : m_skillButtons) btn->setEnabled(false);
            if (m_fifthSkillButton) m_fifthSkillButton->setEnabled(false);
        }
        // 即使玩家可以行动，各个技能按钮的可用性仍由其自身PP和精灵总PP决定（这在update函数中处理）
        
        m_switchButton->setEnabled(canPlayerAct); // 切换也应在可行动时
        m_escapeButton->setEnabled(canPlayerAct && !m_battleSystem->isPvPBattle());

        updateBattleLog(QString("<b>轮到你了! (回合 %1)</b>").arg(turn));
    }
    else
    {
        // 对手回合，禁用所有玩家操作按钮
        for (auto *btn : m_skillButtons) btn->setEnabled(false);
        if (m_fifthSkillButton) m_fifthSkillButton->setEnabled(false);
        m_switchButton->setEnabled(false);
        m_escapeButton->setEnabled(false);

        updateBattleLog(QString("<b>对手的回合! (回合 %1)</b>").arg(turn));
    }
}


// 其余函数的实现与上次回复相同，此处不再重复。
void BattleScene::onSwitchButtonClicked()
{
    // 实际的切换逻辑会更复杂，可能需要弹出一个对话框让玩家选择队伍中的精灵
    // 目前，我们只记录一个日志消息，并假设如果实现了选择，会调用 executeAction
    updateBattleLog("<font color='#F1C40F'>切换精灵功能：请先选择要切换的精灵（此功能待UI交互完善）。</font>");
    
    // 示例：如果玩家通过某种方式选择了 newCreatureIndex
    // Creature* playerCreature = m_battleSystem->getPlayerActiveCreature();
    // if (playerCreature && m_gameEngine->getPlayerTeam().size() > 1) { // 确保有其他精灵可以切换
    //     // 假设 newCreatureIndex 是玩家选择的队伍中其他精灵的索引
    //     // int newCreatureIndex = getPlayerChoiceForSwitch(); // 这需要一个UI元素来选择
    //     // if (newCreatureIndex != -1 && newCreatureIndex < m_gameEngine->getPlayerTeam().size() &&
    //     //     m_gameEngine->getPlayerTeam().at(newCreatureIndex) != playerCreature &&
    //     //     !m_gameEngine->getPlayerTeam().at(newCreatureIndex)->isDead())
    //     // {
    //     //     for (auto *btn : m_skillButtons) btn->setEnabled(false);
    //     //     if (m_fifthSkillButton) m_fifthSkillButton->setEnabled(false);
    //     //     m_switchButton->setEnabled(false);
    //     //     m_escapeButton->setEnabled(false);
    //     //     m_battleSystem->executeAction(BattleAction::SWITCH_CREATURE, newCreatureIndex);
    //     // } else {
    //     //     updateBattleLog("<font color='orange'>无法切换到该精灵。</font>");
    //     // }
    // } else {
    //    updateBattleLog("<font color='orange'>没有其他可切换的精灵。</font>");
    // }
}

void BattleScene::onEscapeButtonClicked()
{
    // 禁用所有按钮，防止重复点击
    for (auto *btn : m_skillButtons)
    {
        btn->setEnabled(false);
    }
    if (m_fifthSkillButton) m_fifthSkillButton->setEnabled(false);
    m_switchButton->setEnabled(false);
    m_escapeButton->setEnabled(false);

    updateBattleLog("<i>尝试逃跑...</i>");
    m_battleSystem->executeAction(BattleAction::ESCAPE);
    // 逃跑的结果将由 BattleSystem 的 battleEnded 信号，或特定的日志更新来通知
    // 如果逃跑失败，按钮应该在下一回合开始时根据玩家状态重新启用
}

void BattleScene::onBattleLogUpdated(const QString &message)
{
    // 这个槽函数由 BattleSystem 的 battleLogUpdated 信号触发
    // 它的作用就是调用我们已有的 updateBattleLog 方法来更新界面上的日志显示
    updateBattleLog(message);
}

void BattleScene::onTurnEnded(int turn) // turn 参数是已结束的回合编号
{
    // 回合结束时，主要更新双方UI以反映回合末效果（如中毒、烧伤伤害，或buff/debuff持续时间减少）
    updatePlayerUI();
    updateOpponentUI();

    // 战斗日志可能会有回合结束的总结信息，由 BattleSystem 发出
    // updateBattleLog(QString("<b>回合 %1 结束。</b>").arg(turn)); // 这条信息通常由BattleSystem发出

    // 下一回合的按钮状态（启用/禁用）将在 onTurnStarted 中处理
}

void BattleScene::onDamageCaused(Creature *creature, int damage)
{
    QLabel *targetLabel = nullptr;
    bool isPlayerTarget = (creature == m_battleSystem->getPlayerActiveCreature());

    if (isPlayerTarget) {
        targetLabel = m_playerCreatureLabel;
    } else if (creature == m_battleSystem->getOpponentActiveCreature()) {
        targetLabel = m_opponentCreatureLabel;
    } else {
        // 可能攻击了非场上精灵（例如，AOE技能，如果支持的话），或者精灵已经切换
        // 为简化，我们假设总是攻击场上精灵
        qWarning() << "onDamageCaused: Damaged creature is not active on field.";
        // 可能需要根据 creature 的指针找到它属于哪一方，然后更新对应的UI列表（如果显示了整个队伍）
        // 但对于战斗场景，通常只更新活跃精灵
        // 如果找不到对应的UI元素，就只更新HP条（如果HP条是直接从BattleSystem获取数据的话）
        if(isPlayerTarget) updatePlayerUI(); else updateOpponentUI();
        return;
    }

    animateDamage(targetLabel, damage); // 执行伤害动画

    // HP条的更新会在动画之后，或者在回合状态刷新时（onTurnStart/onTurnEnd）进行
    // 为了更即时的反馈，可以在动画播放的同时或稍后更新UI
    // QTimer::singleShot(100, this, [this, isPlayerTarget](){ // 延迟一点，让动画数字先显示
    //     if (isPlayerTarget) updatePlayerUI();
    //     else updateOpponentUI();
    // });
    // 不过，通常更稳妥的做法是依赖 onTurnStart/End 或特定状态改变信号来刷新UI，避免多次刷新或时序问题。
    // BattleSystem 应该在造成伤害后更新 Creature 对象的状态，UI刷新时会读取这些新状态。
}

void BattleScene::onHealingReceived(Creature *creature, int amount)
{
    QLabel *targetLabel = nullptr;
    bool isPlayerTarget = (creature == m_battleSystem->getPlayerActiveCreature());

    if (isPlayerTarget) {
        targetLabel = m_playerCreatureLabel;
    } else if (creature == m_battleSystem->getOpponentActiveCreature()) {
        targetLabel = m_opponentCreatureLabel;
    } else {
        qWarning() << "onHealingReceived: Healed creature is not active on field.";
        if(isPlayerTarget) updatePlayerUI(); else updateOpponentUI();
        return;
    }

    animateHealing(targetLabel, amount); // 执行治疗动画

    // 同 onDamageCaused，HP条的更新通常依赖后续的UI刷新调用
    // QTimer::singleShot(100, this, [this, isPlayerTarget](){
    //     if (isPlayerTarget) updatePlayerUI();
    //     else updateOpponentUI();
    // });
}
// 确保 Type::getTypeName 和 Type::getElementTypeColor 的调用是正确的静态方法调用。
// 例如，在 SkillButton::updateText 和 BattleScene::updateFifthSkillButton/updatePlayerUI/updateOpponentUI 中。
// ... (其他函数实现，如 onSkillButtonClicked, onFifthSkillButtonClicked, animateDamage 等) ...
// ... (getStatusText, getStatStageText 的颜色更新版本) ...
QString BattleScene::getStatusText(StatusCondition condition)
{
    switch (condition)
    {
    case StatusCondition::POISON:   return "<font color='#9B59B6'>中毒</font>";
    case StatusCondition::PARALYZE: return "<font color='#F1C40F'>麻痹</font>";
    case StatusCondition::BURN:     return "<font color='#E67E22'>烧伤</font>";
    case StatusCondition::FREEZE:   return "<font color='#3498DB'>冻结</font>";
    case StatusCondition::SLEEP:    return "<font color='#95A5A6'>睡眠</font>";
    case StatusCondition::CONFUSION:return "<font color='#E74C3C'>混乱</font>";
    case StatusCondition::FEAR:     return "<font color='#BDC3C7'>恐惧</font>";
    case StatusCondition::TIRED:    return "<font color='#7F8C8D'>疲惫</font>";
    case StatusCondition::BLEED:    return "<font color='#C0392B'>流血</font>";
    default: return "";
    }
}

QString BattleScene::getStatStageText(const StatStages &stages)
{
    QStringList changes;
    auto addChange = [&](StatType type, const QString& name) {
        int stage = stages.getStage(type);
        if (stage != 0) {
            changes << QString("%1: <font color='%2'>%3%4</font>")
                           .arg(name)
                           .arg(stage > 0 ? "#2ECC71" : "#E74C3C") 
                           .arg(stage > 0 ? "↑" : "↓") 
                           .arg(std::abs(stage));
        }
    };

    addChange(StatType::ATTACK, "攻击");
    addChange(StatType::DEFENSE, "防御");
    addChange(StatType::SP_ATTACK, "特攻");
    addChange(StatType::SP_DEFENSE, "特防");
    addChange(StatType::SPEED, "速度");
    addChange(StatType::ACCURACY, "命中");
    addChange(StatType::EVASION, "闪避");

    return changes.join("&nbsp;&nbsp;");
}

// animateDamage, animateHealing, onSkillButtonClicked, onFifthSkillButtonClicked 等函数的实现与之前类似，
void BattleScene::onSkillButtonClicked(int skillIndex)
{
    Creature* playerCreature = m_battleSystem->getPlayerActiveCreature();
    if (!playerCreature) return; 

    if (skillIndex >= 0 && skillIndex < m_skillButtons.size() && m_skillButtons[skillIndex]->getSkillPtr()) {
        Skill* selectedSkill = m_skillButtons[skillIndex]->getSkillPtr();
        if (playerCreature->getCurrentPP() < selectedSkill->getPPCost()) {
            updateBattleLog(QString("<font color='orange'>精灵PP不足以使用技能: %1!</font>").arg(selectedSkill->getName()));
            return; 
        }
        if (selectedSkill->getCurrentPP() <= 0) {
            updateBattleLog(QString("<font color='orange'>技能 %1 的PP耗尽!</font>").arg(selectedSkill->getName()));
            return;
        }
    } else {
        return; // 无效的技能索引或按钮未关联技能
    }

    for (auto *btn : m_skillButtons) btn->setEnabled(false);
    if (m_fifthSkillButton) m_fifthSkillButton->setEnabled(false);
    m_switchButton->setEnabled(false);
    m_escapeButton->setEnabled(false);

    m_battleSystem->executeAction(BattleAction::USE_SKILL, skillIndex);
}


void BattleScene::onFifthSkillButtonClicked() {
    Creature* playerCreature = m_battleSystem->getPlayerActiveCreature();
    if (!playerCreature || !playerCreature->getFifthSkill()) {
        updateBattleLog("<font color='orange'>无法使用第五技能 (精灵不存在或无第五技能)!</font>");
        return;
    }
    Skill* fifthSkill = playerCreature->getFifthSkill();
    if (playerCreature->getCurrentPP() < fifthSkill->getPPCost()) {
        updateBattleLog(QString("<font color='orange'>精灵PP不足以使用第五技能: %1!</font>").arg(fifthSkill->getName()));
        return;
    }
    if (fifthSkill->getCurrentPP() <= 0) {
         updateBattleLog(QString("<font color='orange'>第五技能 %1 的PP耗尽!</font>").arg(fifthSkill->getName()));
        return;
    }

    for (auto *btn : m_skillButtons) btn->setEnabled(false);
    if (m_fifthSkillButton) m_fifthSkillButton->setEnabled(false);
    m_switchButton->setEnabled(false);
    m_escapeButton->setEnabled(false);

    m_battleSystem->executeAction(BattleAction::USE_SKILL, -1); // -1 代表第五技能
}

void BattleScene::onCreatureSwitched(Creature *oldCreature, Creature *newCreature, bool isPlayer)
{
    QString switcher = isPlayer ? "你" : "对手";
    QString oldNameText = oldCreature ? oldCreature->getName() : "一个精灵";
    QString newNameText = newCreature ? newCreature->getName() : "另一个精灵";

    updateBattleLog(QString("<b>%1收回了 <font color='#AED6F1'>%2</font>，派出了 <font color='#AED6F1'>%3</font>!</b>").arg(switcher).arg(oldNameText).arg(newNameText));

    if (isPlayer) {
        updatePlayerUI();
        updateSkillButtons();       
        updateFifthSkillButton();   
    } else {
        updateOpponentUI();
    }
     // 确保在切换后，如果轮到玩家行动，按钮状态能正确更新
    if(m_battleSystem->isPlayerTurn()){
        onTurnStarted(m_battleSystem->getCurrentTurn(), true);
    }
}


void BattleScene::animateDamage(QLabel *label, int damage)
{
    if (!label) return;

    QLabel *damageLabel = new QLabel(QString("-%1").arg(damage), label); 
    damageLabel->setAlignment(Qt::AlignCenter);
    damageLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #E74C3C; background-color: transparent; border: none;"); 
    damageLabel->adjustSize(); 
    damageLabel->setMinimumWidth(damageLabel->width() + 10); 

    damageLabel->move(label->width() / 2 - damageLabel->width() / 2, label->height() / 2 - damageLabel->height() / 2 - 20); 
    damageLabel->show();
    damageLabel->raise(); 

    QPropertyAnimation *moveAnimation = new QPropertyAnimation(damageLabel, "pos", this);
    moveAnimation->setDuration(1000); 
    moveAnimation->setStartValue(damageLabel->pos());
    moveAnimation->setEndValue(damageLabel->pos() - QPoint(0, 60)); 
    moveAnimation->setEasingCurve(QEasingCurve::OutCubic); 

    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(damageLabel);
    damageLabel->setGraphicsEffect(opacityEffect);
    QPropertyAnimation *fadeOutAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeOutAnimation->setDuration(1000);
    fadeOutAnimation->setStartValue(1.0);
    fadeOutAnimation->setEndValue(0.0);
    fadeOutAnimation->setEasingCurve(QEasingCurve::InCubic);

    connect(fadeOutAnimation, &QPropertyAnimation::finished, damageLabel, &QLabel::deleteLater);

    moveAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    fadeOutAnimation->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer *timer = new QTimer(this);
    timer->setProperty("counter", 0);
    connect(timer, &QTimer::timeout, [label, timer]() mutable {
        int count = timer->property("counter").toInt();
        label->setVisible(count % 2 == 0); 
        timer->setProperty("counter", count + 1);
        if (count >= 5) { 
            label->setVisible(true); 
            timer->stop();
            timer->deleteLater();
        }
    });
    timer->start(80); 
}

void BattleScene::animateHealing(QLabel *label, int amount)
{
    if (!label) return;

    QLabel *healLabel = new QLabel(QString("+%1").arg(amount), label);
    healLabel->setAlignment(Qt::AlignCenter);
    healLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #2ECC71; background-color: transparent; border: none;"); 
    healLabel->adjustSize();
    healLabel->setMinimumWidth(healLabel->width() + 10);

    healLabel->move(label->width() / 2 - healLabel->width() / 2, label->height() / 2 - healLabel->height() / 2 - 20);
    healLabel->show();
    healLabel->raise();

    QPropertyAnimation *moveAnimation = new QPropertyAnimation(healLabel, "pos", this);
    moveAnimation->setDuration(1000);
    moveAnimation->setStartValue(healLabel->pos());
    moveAnimation->setEndValue(healLabel->pos() - QPoint(0, 60));
    moveAnimation->setEasingCurve(QEasingCurve::OutCubic);

    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(healLabel);
    healLabel->setGraphicsEffect(opacityEffect);
    QPropertyAnimation *fadeOutAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeOutAnimation->setDuration(1000);
    fadeOutAnimation->setStartValue(1.0);
    fadeOutAnimation->setEndValue(0.0);
    fadeOutAnimation->setEasingCurve(QEasingCurve::InCubic);

    connect(fadeOutAnimation, &QPropertyAnimation::finished, healLabel, &QLabel::deleteLater);

    moveAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    fadeOutAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}


// #include "battlescene.moc" // 通常由qmake/cmake自动处理，但有时显式提及无害