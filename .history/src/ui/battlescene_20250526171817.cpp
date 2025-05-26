#include "battlescene.h"
#include "../battle/battlesystem.h"
#include "../battle/skill.h" // 确保 Skill 类定义被包含，特别是 FifthSkill
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
#include <QFont>
#include <QPixmap>
#include <QScrollBar> // 确保包含
#include <QDebug>     // 用于调试

// 技能按钮类实现 (SkillButton)
// SkillButton类的实现保持不变，此处省略以保持简洁。
// class SkillButton : public QPushButton { ... }; // 原有代码

// 战斗场景实现
BattleScene::BattleScene(GameEngine *gameEngine, QWidget *parent) : QWidget(parent),
                                                                    m_gameEngine(gameEngine),
                                                                    m_battleSystem(gameEngine->getBattleSystem()),
                                                                    m_playerCreatureLabel(nullptr),
                                                                    m_opponentCreatureLabel(nullptr),
                                                                    m_playerHPBar(nullptr),
                                                                    m_playerPPBar(nullptr),
                                                                    m_opponentHPBar(nullptr),
                                                                    m_opponentPPBar(nullptr), // 如果不显示对手PP，可以移除
                                                                    m_playerStatusLabel(nullptr),
                                                                    m_opponentStatusLabel(nullptr),
                                                                    m_battleLogLabel(nullptr),
                                                                    m_turnLabel(nullptr),
                                                                    m_skillButtons(),
                                                                    m_fifthSkillButton(nullptr), // 初始化新增成员
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

    setupUI();
    initScene(); // initScene 会调用 updateSkillButtons 和 updateFifthSkillButton

    // 连接战斗系统信号
    connect(m_battleSystem, &BattleSystem::battleStarted, this, &BattleScene::initScene);
    connect(m_battleSystem, &BattleSystem::battleEnded, this, [this](BattleResult result)
            {
        // 显示战斗结果
        QString resultMessage;
        switch (result) {
            case BattleResult::PLAYER_WIN:
                resultMessage = "你赢了!";
                break;
            case BattleResult::OPPONENT_WIN:
                resultMessage = "你输了!";
                break;
            case BattleResult::DRAW:
                resultMessage = "平局!";
                break;
            case BattleResult::ESCAPE:
                resultMessage = "成功逃脱!";
                break;
            default:
                resultMessage = "战斗结束";
        }
        
        updateBattleLog(QString("<b>%1</b>").arg(resultMessage)); // 加粗结果
        
        // 禁用所有战斗按钮
        for (auto* btn : m_skillButtons) {
            btn->setEnabled(false);
        }
        if (m_fifthSkillButton) m_fifthSkillButton->setEnabled(false); // 禁用第五技能按钮
        m_switchButton->setEnabled(false);
        m_escapeButton->setEnabled(false); 
    });

    connect(m_battleSystem, &BattleSystem::turnStarted, this, &BattleScene::onTurnStarted);
    connect(m_battleSystem, &BattleSystem::turnEnded, this, &BattleScene::onTurnEnded);
    connect(m_battleSystem, &BattleSystem::damageCaused, this, &BattleScene::onDamageCaused);
    connect(m_battleSystem, &BattleSystem::healingReceived, this, &BattleScene::onHealingReceived);
    connect(m_battleSystem, &BattleSystem::creatureSwitched, this, &BattleScene::onCreatureSwitched);
    connect(m_battleSystem, &BattleSystem::battleLogUpdated, this, &BattleScene::onBattleLogUpdated);

    // 设置动画计时器
    m_animationTimer = new QTimer(this);
    m_animationTimer->setSingleShot(true); 
}

BattleScene::~BattleScene()
{
    // 资源清理
    for (auto *btn : m_skillButtons)
    {
        delete btn;
    }
    m_skillButtons.clear();
    // 其他 QPushButton 成员 (m_fifthSkillButton, m_switchButton, m_escapeButton) 
    // 以及布局和标签等作为此QWidget的子对象，会被Qt自动管理和删除。
}

void BattleScene::setupUI()
{
    // --- 主题颜色和字体 ---
    QString backgroundColor = "#2C3E50"; // 深蓝灰色背景
    QString panelColor = "#34495E";    // 面板颜色 (略浅)
    QString textColor = "white";
    QString progressBarChunkColor = "QProgressBar::chunk { background-color: %1; border-radius: 3px; margin: 1px;}"; // 添加边距使chunk看起来更清晰
    QString buttonStyle = "QPushButton { background-color: #5D6D7E; color: white; padding: 8px; border-radius: 5px; font-size: 13px; border: 1px solid #4A5A6A;} QPushButton:hover { background-color: #85929E; } QPushButton:disabled { background-color: #4A5A6A; color: #BDC3C7; }";
    QString fifthSkillButtonStyle = "QPushButton { background-color: #F39C12; color: white; padding: 10px; border-radius: 5px; font-size: 14px; font-weight: bold; border: 1px solid #D35400;} QPushButton:hover { background-color: #F5B041; } QPushButton:disabled { background-color: #B9770E; color: #FAD7A0; }"; // 橙色/金色按钮
    QString defaultFont = "Arial"; // 可以替换为更美观的字体名，如 "Noto Sans"

    this->setStyleSheet(QString("QWidget { background-color: %1; color: %2; font-family: %3; }").arg(backgroundColor).arg(textColor).arg(defaultFont));

    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(12); // 增加布局间距
    m_mainLayout->setContentsMargins(10, 10, 10, 10); // 设置外边距

    // 创建战场布局 (左右分布玩家和对手)
    m_battlefieldLayout = new QHBoxLayout();
    m_battlefieldLayout->setSpacing(20); // 精灵间的间距

    // 创建玩家和对手的垂直布局 (用于放置精灵图和状态信息)
    m_playerLayout = new QVBoxLayout();
    m_playerLayout->setSpacing(8);
    m_opponentLayout = new QVBoxLayout();
    m_opponentLayout->setSpacing(8);

    // --- 对手精灵UI ---
    m_opponentCreatureLabel = new QLabel(this);
    m_opponentCreatureLabel->setMinimumSize(220, 220); 
    m_opponentCreatureLabel->setAlignment(Qt::AlignCenter);
    m_opponentCreatureLabel->setStyleSheet(QString("background-color: %1; border-radius: 8px; border: 1px solid #4A5A6A; padding: 5px;").arg(panelColor));
    m_opponentLayout->addWidget(m_opponentCreatureLabel);
    m_opponentLayout->addStretch(1); // 使得状态信息部分在底部

    QWidget* opponentStatusWidget = new QWidget(this);
    opponentStatusWidget->setStyleSheet(QString("background-color: %1; border-radius: 6px; padding: 8px; border: 1px solid #4A5A6A;").arg(panelColor));
    QVBoxLayout* opponentStatusVLayout = new QVBoxLayout(opponentStatusWidget);
    opponentStatusVLayout->setSpacing(5);

    m_opponentStatusLabel = new QLabel("对手", this);
    m_opponentStatusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_opponentStatusLabel->setStyleSheet("font-size: 13px; font-weight: bold;");
    opponentStatusVLayout->addWidget(m_opponentStatusLabel);

    m_opponentHPBar = new QProgressBar(this);
    m_opponentHPBar->setMinimum(0);
    m_opponentHPBar->setTextVisible(true);
    m_opponentHPBar->setFormat("%v/%m HP"); // 格式化文本
    m_opponentHPBar->setStyleSheet(QString("QProgressBar { text-align: center; border-radius: 4px; height: 18px; background-color: #78909C; color: white;} ") + QString(progressBarChunkColor).arg("#E74C3C")); // 初始为红色
    opponentStatusVLayout->addWidget(m_opponentHPBar);
    m_opponentLayout->addWidget(opponentStatusWidget);


    // --- 玩家精灵UI ---
    m_playerCreatureLabel = new QLabel(this);
    m_playerCreatureLabel->setMinimumSize(220, 220); 
    m_playerCreatureLabel->setAlignment(Qt::AlignCenter);
    m_playerCreatureLabel->setStyleSheet(QString("background-color: %1; border-radius: 8px; border: 1px solid #4A5A6A; padding: 5px;").arg(panelColor));
    m_playerLayout->addWidget(m_playerCreatureLabel);
    m_playerLayout->addStretch(1);

    QWidget* playerStatusWidget = new QWidget(this);
    playerStatusWidget->setStyleSheet(QString("background-color: %1; border-radius: 6px; padding: 8px; border: 1px solid #4A5A6A;").arg(panelColor));
    QVBoxLayout* playerStatusVLayout = new QVBoxLayout(playerStatusWidget);
    playerStatusVLayout->setSpacing(5);

    m_playerStatusLabel = new QLabel("玩家", this); 
    m_playerStatusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_playerStatusLabel->setStyleSheet("font-size: 13px; font-weight: bold;");
    playerStatusVLayout->addWidget(m_playerStatusLabel);

    m_playerHPBar = new QProgressBar(this);
    m_playerHPBar->setMinimum(0);
    m_playerHPBar->setTextVisible(true);
    m_playerHPBar->setFormat("%v/%m HP");
    m_playerHPBar->setStyleSheet(QString("QProgressBar { text-align: center; border-radius: 4px; height: 18px; background-color: #78909C; color: white;} ") + QString(progressBarChunkColor).arg("#2ECC71")); // 初始为绿色
    playerStatusVLayout->addWidget(m_playerHPBar);

    m_playerPPBar = new QProgressBar(this);
    m_playerPPBar->setMinimum(0);
    m_playerPPBar->setTextVisible(true);
    m_playerPPBar->setFormat("%v/%m PP"); // 显示精灵总PP
    m_playerPPBar->setStyleSheet(QString("QProgressBar { text-align: center; border-radius: 4px; height: 16px; background-color: #78909C; color: white;} ") + QString(progressBarChunkColor).arg("#3498DB")); // 蓝色
    playerStatusVLayout->addWidget(m_playerPPBar);
    m_playerLayout->addWidget(playerStatusWidget);

    // 将玩家和对手布局添加到战场布局
    m_battlefieldLayout->addLayout(m_opponentLayout); 
    m_battlefieldLayout->addStretch(1); // 弹性空间，使双方精灵分布在战场两侧
    m_battlefieldLayout->addLayout(m_playerLayout);   

    // --- 回合和日志布局 ---
    m_logLayout = new QVBoxLayout();
    m_logLayout->setSpacing(8);

    m_turnLabel = new QLabel("回合: 1", this);
    m_turnLabel->setAlignment(Qt::AlignCenter);
    m_turnLabel->setStyleSheet("font-weight: bold; font-size: 16px; padding: 6px; background-color: #4A6A8A; border-radius: 5px;");
    m_logLayout->addWidget(m_turnLabel);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumHeight(100); // 减少高度以给操作区更多空间
    scrollArea->setStyleSheet(QString("QScrollArea { border: 1px solid #4A5A6A; border-radius: 5px; background-color: %1; } QScrollBar:vertical { border: none; background: %2; width: 12px; margin: 0px 0px 0px 0px; border-radius: 6px;} QScrollBar::handle:vertical { background: #5D6D7E; min-height: 25px; border-radius: 5px; } QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; border: none; background: none; }").arg(panelColor).arg(backgroundColor));

    QWidget *scrollContent = new QWidget(scrollArea);
    scrollContent->setStyleSheet(QString("background-color: transparent;")); // 日志内容区域背景透明，依赖父级ScrollArea
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(5,5,5,5);

    m_battleLogLabel = new QLabel(scrollContent);
    m_battleLogLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_battleLogLabel->setWordWrap(true);
    m_battleLogLabel->setTextFormat(Qt::RichText); 
    m_battleLogLabel->setStyleSheet("padding: 5px; font-size: 12px; background-color: transparent;"); 

    scrollLayout->addWidget(m_battleLogLabel);
    scrollLayout->addStretch();
    scrollContent->setLayout(scrollLayout);
    scrollArea->setWidget(scrollContent);
    m_logLayout->addWidget(scrollArea);

    // --- 创建操作布局 (使用QGridLayout) ---
    m_actionLayout = new QGridLayout();
    m_actionLayout->setSpacing(8); 

    for (int i = 0; i < 4; ++i)
    {
        SkillButton *skillBtn = new SkillButton(i, nullptr, this);
        skillBtn->setMinimumHeight(55); 
        skillBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // 使按钮填充单元格
        connect(skillBtn, &SkillButton::skillSelected, this, &BattleScene::onSkillButtonClicked);
        m_skillButtons.append(skillBtn);
        m_actionLayout->addWidget(skillBtn, i / 2, i % 2); 
    }

    m_fifthSkillButton = new QPushButton("第五技能", this);
    m_fifthSkillButton->setStyleSheet(fifthSkillButtonStyle); 
    m_fifthSkillButton->setMinimumHeight(45);
    m_fifthSkillButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(m_fifthSkillButton, &QPushButton::clicked, this, &BattleScene::onFifthSkillButtonClicked);
    m_actionLayout->addWidget(m_fifthSkillButton, 2, 0, 1, 2); // 横跨两列

    m_switchButton = new QPushButton("切换", this); // 简化文本
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
    if (m_battleLogLabel->verticalScrollBar()) {
        m_battleLogLabel->verticalScrollBar()->setValue(0); 
    }

    m_turnLabel->setText(QString("回合: %1").arg(m_battleSystem->getCurrentTurn()));

    Creature* playerCreature = m_battleSystem->getPlayerActiveCreature();
    bool canPlayerAct = playerCreature && playerCreature->canAct();
    
    for(SkillButton* btn : m_skillButtons) {
        // SkillButton的setSkill内部会根据skill是否为nullptr和PP来设置enabled状态
        // 此处我们确保基于canPlayerAct进行父级控制
        if(btn->property("skillValid").toBool()){ // 假设SkillButton内部设置一个属性来判断技能是否真的可选
             btn->setEnabled(canPlayerAct);
        } else {
             btn->setEnabled(false);
        }
    }
    if (m_fifthSkillButton) { // updateFifthSkillButton会处理具体逻辑
        updateFifthSkillButton(); // 再次调用以确保基于 canPlayerAct
         if (!canPlayerAct) m_fifthSkillButton->setEnabled(false);
    }
    
    m_switchButton->setEnabled(true); 
    m_escapeButton->setEnabled(!m_battleSystem->isPvPBattle());

    updateBattleLog("<b><font color='#2ECC71'>战斗开始!</font></b>");
}

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

    m_playerPPBar->setMaximum(playerCreature->getMaxPP()); // 显示精灵总PP
    m_playerPPBar->setValue(playerCreature->getCurrentPP());
    m_playerPPBar->setStyleSheet(QString("QProgressBar { text-align: center; border-radius: 4px; height: 16px; background-color: #78909C; color: white;} QProgressBar::chunk { background-color: #3498DB; border-radius: 3px; margin: 1px;}"));

    QString statusText = QString("<b>%1</b> <font color='#ECF0F1'>Lv.%2</font>").arg(playerCreature->getName()).arg(playerCreature->getLevel());
    statusText += "<br>属性: " + Type::getElementTypeName(playerCreature->getType().getPrimaryType());
    if (playerCreature->getType().getSecondaryType() != ElementType::NONE)
    {
        statusText += "/" + Type::getElementTypeName(playerCreature->getType().getSecondaryType());
    }
    if (playerCreature->getStatusCondition() != StatusCondition::NONE)
    {
        statusText += QString("<br>状态: %1").arg(getStatusText(playerCreature->getStatusCondition())); // getStatusText内部处理颜色
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
    statusText += "<br>属性: " + Type::getElementTypeName(opponentCreature->getType().getPrimaryType());
    if (opponentCreature->getType().getSecondaryType() != ElementType::NONE)
    {
        statusText += "/" + Type::getElementTypeName(opponentCreature->getType().getSecondaryType());
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

void BattleScene::updateSkillButtons()
{
    Creature *playerCreature = m_battleSystem->getPlayerActiveCreature();
    bool canPlayerActGlobal = playerCreature && playerCreature->canAct();

    if (!playerCreature) {
        for(SkillButton* btn : m_skillButtons) {
            btn->setSkill(nullptr); 
            btn->setEnabled(false);
            btn->setProperty("skillValid", false);
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
        m_skillButtons[i]->setSkill(skill); 
        bool skillHasPP = skill != nullptr && skill->getCurrentPP() > 0;
        bool creatureHasEnoughPP = skill != nullptr && playerCreature->getCurrentPP() >= skill->getPPCost(); // 检查精灵总PP
        m_skillButtons[i]->setEnabled(canPlayerActGlobal && skillHasPP && creatureHasEnoughPP);
        m_skillButtons[i]->setProperty("skillValid", skill != nullptr);
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
        bool skillHasPP = fifthSkill->getCurrentPP() > 0; // 检查第五技能自身的PP (如果它有独立PP)
        bool creatureHasEnoughPP = playerCreature->getCurrentPP() >= fifthSkill->getPPCost(); // 检查精灵总PP是否足够消耗
        
        m_fifthSkillButton->setEnabled(canPlayerActGlobal && skillHasPP && creatureHasEnoughPP);

        QString tooltipText = QString("<b>%1</b><br><br><b>属性:</b> %2<br><b>类别:</b> %3<br><b>威力:</b> %4 | <b>消耗PP:</b> %5<br><b>命中:</b> %6<br><b>当前PP:</b> %7/%8<br><hr><b>描述:</b><br>%9")
                                  .arg(fifthSkill->getName())
                                  .arg(Type::getElementTypeName(fifthSkill->getType()))
                                  .arg(fifthSkill->getCategory() == SkillCategory::PHYSICAL ? "物理" : fifthSkill->getCategory() == SkillCategory::SPECIAL ? "特殊" : "状态")
                                  .arg(fifthSkill->getPower())
                                  .arg(fifthSkill->getPPCost())
                                  .arg(fifthSkill->getAccuracy() >= 100 ? "必中" : QString::number(fifthSkill->getAccuracy()) + "%")
                                  .arg(fifthSkill->getCurrentPP()) 
                                  .arg(fifthSkill->getMaxPP())     
                                  .arg(fifthSkill->getDescription().replace("\n", "<br>")); // 替换换行符
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
        if (m_battleLogLabel && m_battleLogLabel->parentWidget()) { // 检查父控件是否存在，以获取滚动条
             QScrollArea* scrollArea = qobject_cast<QScrollArea*>(m_battleLogLabel->parentWidget()->parentWidget());
             if(scrollArea && scrollArea->verticalScrollBar()){
                 scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
             } else if (m_battleLogLabel->verticalScrollBar()) { // 如果Label自身有滚动条 (不太可能，通常在ScrollArea内)
                 m_battleLogLabel->verticalScrollBar()->setValue(m_battleLogLabel->verticalScrollBar()->maximum());
             }
        }
    });
}

void BattleScene::onSkillButtonClicked(int skillIndex)
{
    Creature* playerCreature = m_battleSystem->getPlayerActiveCreature();
    if (playerCreature && skillIndex >= 0 && skillIndex < playerCreature->getSkillCount()) {
        Skill* selectedSkill = playerCreature->getSkill(skillIndex);
        if (selectedSkill && playerCreature->getCurrentPP() < selectedSkill->getPPCost()) {
            updateBattleLog(QString("<font color='orange'>精灵PP不足以使用技能: %1!</font>").arg(selectedSkill->getName()));
            return; // 不执行动作
        }
    } else if (!playerCreature) {
        return; // 没有精灵
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
        updateBattleLog("<font color='orange'>无法使用第五技能!</font>");
        return;
    }
    if (playerCreature->getCurrentPP() < playerCreature->getFifthSkill()->getPPCost()) {
        updateBattleLog(QString("<font color='orange'>精灵PP不足以使用第五技能: %1!</font>").arg(playerCreature->getFifthSkill()->getName()));
        return;
    }
    if (playerCreature->getFifthSkill()->getCurrentPP() <= 0) {
         updateBattleLog(QString("<font color='orange'>第五技能 %1 的PP耗尽!</font>").arg(playerCreature->getFifthSkill()->getName()));
        return;
    }


    for (auto *btn : m_skillButtons) btn->setEnabled(false);
    if (m_fifthSkillButton) m_fifthSkillButton->setEnabled(false);
    m_switchButton->setEnabled(false);
    m_escapeButton->setEnabled(false);

    m_battleSystem->executeAction(BattleAction::USE_SKILL, -1); // -1 代表第五技能
}


void BattleScene::onSwitchButtonClicked()
{
    updateBattleLog("<font color='#F1C40F'>切换精灵功能暂未完全实现。请在游戏逻辑中完成精灵选择界面。</font>");
    // 假设玩家通过某种方式选择了队伍中的精灵，获得索引 `selectedCreatureIndex`
    // if (canSwitch) {
    //     for (auto *btn : m_skillButtons) btn->setEnabled(false);
    //     if (m_fifthSkillButton) m_fifthSkillButton->setEnabled(false);
    //     m_switchButton->setEnabled(false);
    //     m_escapeButton->setEnabled(false);
    //     m_battleSystem->executeAction(BattleAction::SWITCH_CREATURE, selectedCreatureIndex);
    // }
}

void BattleScene::onEscapeButtonClicked()
{
    for (auto *btn : m_skillButtons) btn->setEnabled(false);
    if (m_fifthSkillButton) m_fifthSkillButton->setEnabled(false);
    m_switchButton->setEnabled(false);
    m_escapeButton->setEnabled(false);

    updateBattleLog("<i>尝试逃跑...</i>");
    m_battleSystem->executeAction(BattleAction::ESCAPE);
    // 逃跑结果将由 BattleSystem 的 battleEnded 信号处理
}

void BattleScene::onBattleLogUpdated(const QString &message)
{
    // 此槽函数由BattleSystem发出，直接调用updateBattleLog即可
    updateBattleLog(message); 
}

void BattleScene::onTurnStarted(int turn, bool isPlayerTurn)
{
    updatePlayerUI(); // 确保回合开始时UI是最新的（例如中毒伤害后）
    updateOpponentUI();

    m_turnLabel->setText(QString("<b>回合 %1</b> - %2行动").arg(turn).arg(isPlayerTurn ? "<font color='#2ECC71'>我方</font>" : "<font color='#E74C3C'>敌方</font>"));

    Creature* playerCreature = m_battleSystem->getPlayerActiveCreature();
    bool canPlayerAct = playerCreature && playerCreature->canAct();

    if (isPlayerTurn)
    {
        updateSkillButtons(); // SkillButton的enable状态由内部setSkill和这里的canPlayerAct共同决定
        updateFifthSkillButton();
        
        // 确保即使技能按钮因为PP等原因不可用，如果玩家不能行动，所有按钮都禁用
        if (!canPlayerAct) {
            for (auto *btn : m_skillButtons) btn->setEnabled(false);
            if (m_fifthSkillButton) m_fifthSkillButton->setEnabled(false);
        }
        
        m_switchButton->setEnabled(true); 
        m_escapeButton->setEnabled(!m_battleSystem->isPvPBattle());

        updateBattleLog(QString("<b>轮到你了! (回合 %1)</b>").arg(turn));
    }
    else
    {
        for (auto *btn : m_skillButtons) btn->setEnabled(false);
        if (m_fifthSkillButton) m_fifthSkillButton->setEnabled(false);
        m_switchButton->setEnabled(false);
        m_escapeButton->setEnabled(false);

        updateBattleLog(QString("<b>对手的回合! (回合 %1)</b>").arg(turn));
    }
}

void BattleScene::onTurnEnded(int turn) 
{
    updatePlayerUI();
    updateOpponentUI();
    // 按钮状态的更新主要在 onTurnStarted 中进行
}

void BattleScene::onDamageCaused(Creature *creature, int damage)
{
    QLabel *creatureLabel = nullptr;
    bool isPlayer = (creature == m_battleSystem->getPlayerActiveCreature());

    if (isPlayer) creatureLabel = m_playerCreatureLabel;
    else if (creature == m_battleSystem->getOpponentActiveCreature()) creatureLabel = m_opponentCreatureLabel;
    else return; 

    animateDamage(creatureLabel, damage); 

    // UI的HP条更新会由随后的 updatePlayerUI/updateOpponentUI (通常在onTurnStart/End) 处理
    // 如果需要立即更新，可以取消以下注释：
    // if (isPlayer) QTimer::singleShot(100, this, &BattleScene::updatePlayerUI); // 延迟一点以配合动画
    // else QTimer::singleShot(100, this, &BattleScene::updateOpponentUI);
}

void BattleScene::onHealingReceived(Creature *creature, int amount)
{
    QLabel *creatureLabel = nullptr;
    bool isPlayer = (creature == m_battleSystem->getPlayerActiveCreature());

    if (isPlayer) creatureLabel = m_playerCreatureLabel;
    else if (creature == m_battleSystem->getOpponentActiveCreature()) creatureLabel = m_opponentCreatureLabel;
    else return;

    animateHealing(creatureLabel, amount); 
    
    // if (isPlayer) QTimer::singleShot(100, this, &BattleScene::updatePlayerUI);
    // else QTimer::singleShot(100, this, &BattleScene::updateOpponentUI);
}


void BattleScene::onCreatureSwitched(Creature *oldCreature, Creature *newCreature, bool isPlayer)
{
    QString switcher = isPlayer ? "你" : "对手";
    QString oldName = oldCreature ? oldCreature->getName() : "场上精灵"; // 更通用的默认名
    QString newName = newCreature ? newCreature->getName() : "新精灵";

    updateBattleLog(QString("<b>%1收回了 <font color='#AED6F1'>%2</font>，派出了 <font color='#AED6F1'>%3</font>!</b>").arg(switcher).arg(oldName).arg(newName));

    if (isPlayer) {
        updatePlayerUI();
        updateSkillButtons();       
        updateFifthSkillButton();   
    } else {
        updateOpponentUI();
    }
}

void BattleScene::animateDamage(QLabel *label, int damage)
{
    if (!label) return;

    QLabel *damageLabel = new QLabel(QString("-%1").arg(damage), label); 
    damageLabel->setAlignment(Qt::AlignCenter);
    damageLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #E74C3C; background-color: transparent; border: none;"); 
    damageLabel->adjustSize(); 
    damageLabel->setMinimumWidth(damageLabel->width() + 10); // 确保数字显示完整

    damageLabel->move(label->width() / 2 - damageLabel->width() / 2, label->height() / 2 - damageLabel->height() / 2 - 20); // 初始位置偏上
    damageLabel->show();
    damageLabel->raise(); 

    QPropertyAnimation *moveAnimation = new QPropertyAnimation(damageLabel, "pos", this);
    moveAnimation->setDuration(1000); 
    moveAnimation->setStartValue(damageLabel->pos());
    moveAnimation->setEndValue(damageLabel->pos() - QPoint(0, 60)); // 向上飘动
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

    // 精灵受击闪烁 (简单版：快速显隐)
    QTimer *timer = new QTimer(this);
    timer->setProperty("counter", 0);
    connect(timer, &QTimer::timeout, [label, timer]() mutable {
        int count = timer->property("counter").toInt();
        label->setVisible(count % 2 == 0); // 偶数次显示，奇数次隐藏
        timer->setProperty("counter", count + 1);
        if (count >= 5) { // 闪烁3次 (0-vis, 1-invis, 2-vis, 3-invis, 4-vis, 5-stop)
            label->setVisible(true); // 确保最后是可见的
            timer->stop();
            timer->deleteLater();
        }
    });
    timer->start(80); // 闪烁间隔
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

QString BattleScene::getStatusText(StatusCondition condition)
{
    switch (condition)
    {
    case StatusCondition::POISON:   return "<font color='#9B59B6'>中毒</font>"; // 紫色
    case StatusCondition::PARALYZE: return "<font color='#F1C40F'>麻痹</font>"; // 黄色
    case StatusCondition::BURN:     return "<font color='#E67E22'>烧伤</font>"; // 橙色
    case StatusCondition::FREEZE:   return "<font color='#3498DB'>冻结</font>"; // 蓝色
    case StatusCondition::SLEEP:    return "<font color='#95A5A6'>睡眠</font>"; // 灰蓝色
    case StatusCondition::CONFUSION:return "<font color='#E74C3C'>混乱</font>"; // 红色
    case StatusCondition::FEAR:     return "<font color='#BDC3C7'>恐惧</font>"; // 浅灰
    case StatusCondition::TIRED:    return "<font color='#7F8C8D'>疲惫</font>"; // 深灰
    case StatusCondition::BLEED:    return "<font color='#C0392B'>流血</font>"; // 深红
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
                           .arg(stage > 0 ? "#2ECC71" : "#E74C3C") // 绿色增益，红色减益
                           .arg(stage > 0 ? "↑" : "↓") // 使用箭头表示升降
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

    return changes.join("&nbsp;&nbsp;"); // 使用HTML空格分隔
}

#include "battlescene.moc" //