#include "battlescene.h"
#include "../battle/battlesystem.h"
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

// 技能按钮类实现
class SkillButton : public QPushButton {
    Q_OBJECT
    
public:
    SkillButton(int index, Skill* skill, QWidget* parent = nullptr) : 
        QPushButton(parent), m_index(index), m_skill(skill) {
        updateText();
        
        connect(this, &QPushButton::clicked, this, &SkillButton::onClicked);
    }
    
    void setSkill(Skill* skill) {
        m_skill = skill;
        updateText();
        setEnabled(m_skill != nullptr && m_skill->getCurrentPP() > 0);
    }
    
signals:
    void skillSelected(int index);
    
private slots:
    void onClicked() {
        if (m_skill && isEnabled()) {
            emit skillSelected(m_index);
        }
    }
    
private:
    int m_index;
    Skill* m_skill;
    
    void updateText() {
        if (m_skill) {
            QString skillType;
            switch (m_skill->getCategory()) {
                case SkillCategory::PHYSICAL: skillType = "物理"; break;
                case SkillCategory::SPECIAL: skillType = "特殊"; break;
                case SkillCategory::STATUS: skillType = "状态"; break;
            }
            
            QString text = QString("%1\n%2 | %3 PP: %4/%5")
                .arg(m_skill->getName())
                .arg(Type::getElementTypeName(m_skill->getElementType()))
                .arg(skillType)
                .arg(m_skill->getCurrentPP())
                .arg(m_skill->getMaxPP());
            
            setText(text);
            
            // 设置按钮样式
            QString typeColor = Type::getElementTypeColor(m_skill->getElementType());
            setStyleSheet(QString("QPushButton { background-color: %1; color: white; padding: 5px; border-radius: 5px; }").arg(typeColor));
        } else {
            setText("--");
            setStyleSheet("QPushButton { background-color: gray; color: white; padding: 5px; border-radius: 5px; }");
        }
    }
};

// 战斗场景实现
BattleScene::BattleScene(GameEngine* gameEngine, QWidget *parent) :
    QWidget(parent),
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
    m_skillButtons(),
    m_switchButton(nullptr),
    m_escapeButton(nullptr),
    m_mainLayout(nullptr),
    m_battlefieldLayout(nullptr),
    m_playerLayout(nullptr),
    m_opponentLayout(nullptr),
    m_actionLayout(nullptr),
    m_logLayout(nullptr),
    m_animationTimer(nullptr) {
    
    setupUI();
    initScene();
    
    // 连接战斗系统信号
    connect(m_battleSystem, &BattleSystem::battleStarted, this, &BattleScene::initScene);
    connect(m_battleSystem, &BattleSystem::battleEnded, this, [this](BattleResult result) {
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
        
        updateBattleLog(resultMessage);
        
        // 禁用所有战斗按钮
        for (auto* btn : m_skillButtons) {
            btn->setEnabled(false);
        }
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
}

BattleScene::~BattleScene() {
    // 资源清理
    for (auto* btn : m_skillButtons) {
        delete btn;
    }
    m_skillButtons.clear();
}

void BattleScene::setupUI() {
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    
    // 创建战场布局
    m_battlefieldLayout = new QHBoxLayout();
    
    // 创建玩家和对手布局
    m_playerLayout = new QVBoxLayout();
    m_opponentLayout = new QVBoxLayout();
    
    // 创建对手精灵UI
    m_opponentCreatureLabel = new QLabel(this);
    m_opponentCreatureLabel->setMinimumSize(200, 200);
    m_opponentCreatureLabel->setAlignment(Qt::AlignCenter);
    m_opponentLayout->addWidget(m_opponentCreatureLabel);
    
    // 对手精灵状态
    QHBoxLayout* oppStatusLayout = new QHBoxLayout();
    
    m_opponentStatusLabel = new QLabel(this);
    m_opponentStatusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    oppStatusLayout->addWidget(m_opponentStatusLabel);
    
    m_opponentHPBar = new QProgressBar(this);
    m_opponentHPBar->setMinimum(0);
    m_opponentHPBar->setTextVisible(true);
    m_opponentHPBar->setFormat("HP: %v/%m");
    oppStatusLayout->addWidget(m_opponentHPBar);
    
    m_opponentPPBar = new QProgressBar(this);
    m_opponentPPBar->setMinimum(0);
    m_opponentPPBar->setTextVisible(true);
    m_opponentPPBar->setFormat("PP: %v/%m");
    oppStatusLayout->addWidget(m_opponentPPBar);
    
    m_opponentLayout->addLayout(oppStatusLayout);
    
    // 创建玩家精灵UI
    m_playerCreatureLabel = new QLabel(this);
    m_playerCreatureLabel->setMinimumSize(200, 200);
    m_playerCreatureLabel->setAlignment(Qt::AlignCenter);
    m_playerLayout->addWidget(m_playerCreatureLabel);
    
    // 玩家精灵状态
    QHBoxLayout* playerStatusLayout = new QHBoxLayout();
    
    m_playerStatusLabel = new QLabel(this);
    m_playerStatusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    playerStatusLayout->addWidget(m_playerStatusLabel);
    
    m_playerHPBar = new QProgressBar(this);
    m_playerHPBar->setMinimum(0);
    m_playerHPBar->setTextVisible(true);
    m_playerHPBar->setFormat("HP: %v/%m");
    playerStatusLayout->addWidget(m_playerHPBar);
    
    m_playerPPBar = new QProgressBar(this);
    m_playerPPBar->setMinimum(0);
    m_playerPPBar->setTextVisible(true);
    m_playerPPBar->setFormat("PP: %v/%m");
    playerStatusLayout->addWidget(m_playerPPBar);
    
    m_playerLayout->addLayout(playerStatusLayout);
    
    // 将玩家和对手布局添加到战场布局
    m_battlefieldLayout->addLayout(m_opponentLayout);
    m_battlefieldLayout->addLayout(m_playerLayout);
    
    // 创建回合和日志布局
    m_logLayout = new QVBoxLayout();
    
    // 回合标签
    m_turnLabel = new QLabel("回合: 1", this);
    m_turnLabel->setAlignment(Qt::AlignCenter);
    m_turnLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    m_logLayout->addWidget(m_turnLabel);
    
    // 战斗日志
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumHeight(100);
    
    QWidget* scrollContent = new QWidget(scrollArea);
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    
    m_battleLogLabel = new QLabel(scrollContent);
    m_battleLogLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_battleLogLabel->setWordWrap(true);
    m_battleLogLabel->setTextFormat(Qt::RichText);
    
    scrollLayout->addWidget(m_battleLogLabel);
    scrollLayout->addStretch();
    
    scrollContent->setLayout(scrollLayout);
    scrollArea->setWidget(scrollContent);
    
    m_logLayout->addWidget(scrollArea);
    
    // 创建操作布局
    m_actionLayout = new QGridLayout();
    
    // 创建技能按钮
    for (int i = 0; i < 4; ++i) {
        SkillButton* skillBtn = new SkillButton(i, nullptr, this);
        connect(skillBtn, &SkillButton::skillSelected, this, &BattleScene::onSkillButtonClicked);
        m_skillButtons.append(skillBtn);
        m_actionLayout->addWidget(skillBtn, i / 2, i % 2);
    }
    
    // 创建切换和逃跑按钮
    m_switchButton = new QPushButton("切换精灵", this);
    connect(m_switchButton, &QPushButton::clicked, this, &BattleScene::onSwitchButtonClicked);
    m_actionLayout->addWidget(m_switchButton, 2, 0);
    
    m_escapeButton = new QPushButton("逃跑", this);
    connect(m_escapeButton, &QPushButton::clicked, this, &BattleScene::onEscapeButtonClicked);
    m_actionLayout->addWidget(m_escapeButton, 2, 1);
    
    // 将布局添加到主布局
    m_mainLayout->addLayout(m_battlefieldLayout, 3);
    m_mainLayout->addLayout(m_logLayout, 1);
    m_mainLayout->addLayout(m_actionLayout, 1);
    
    // 设置布局
    setLayout(m_mainLayout);
}

void BattleScene::initScene() {
    // 更新UI显示
    updatePlayerUI();
    updateOpponentUI();
    updateSkillButtons();
    
    // 清空战斗日志
    m_battleLogLabel->setText("");
    
    // 设置回合标签
    m_turnLabel->setText(QString("回合: %1").arg(m_battleSystem->getCurrentTurn()));
    
    // 启用所有按钮
    m_switchButton->setEnabled(true);
    m_escapeButton->setEnabled(!m_battleSystem->isPvPBattle());
    
    // 添加初始战斗日志
    updateBattleLog("<b>战斗开始!</b>");
}

void BattleScene::updatePlayerUI() {
    Creature* playerCreature = m_battleSystem->getPlayerActiveCreature();
    if (!playerCreature) return;
    
    // 更新精灵图像
    QPixmap creaturePixmap(QString(":/sprites/%1_back.png").arg(playerCreature->getName().toLower().replace(' ', '_')));
    if (creaturePixmap.isNull()) {
        creaturePixmap = QPixmap(":/sprites/default_back.png");
    }
    m_playerCreatureLabel->setPixmap(creaturePixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    // 更新HP条
    m_playerHPBar->setMaximum(playerCreature->getMaxHP());
    m_playerHPBar->setValue(playerCreature->getCurrentHP());
    
    // 设置HP条颜色
    double hpRatio = (double)playerCreature->getCurrentHP() / playerCreature->getMaxHP();
    QString hpColor;
    if (hpRatio > 0.5) {
        hpColor = "green";
    } else if (hpRatio > 0.25) {
        hpColor = "yellow";
    } else {
        hpColor = "red";
    }
    m_playerHPBar->setStyleSheet(QString("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: %1; }").arg(hpColor));
    
    // 更新PP条
    m_playerPPBar->setMaximum(playerCreature->getMaxPP());
    m_playerPPBar->setValue(playerCreature->getCurrentPP());
    m_playerPPBar->setStyleSheet("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: blue; }");
    
    // 更新状态标签
    QString statusText = playerCreature->getName() + " Lv." + QString::number(playerCreature->getLevel()) + "\n";
    statusText += "类型: " + Type::getElementTypeName(playerCreature->getType().getPrimaryType());
    if (playerCreature->getType().getSecondaryType() != ElementType::NONE) {
        statusText += "/" + Type::getElementTypeName(playerCreature->getType().getSecondaryType());
    }
    if (playerCreature->getStatusCondition() != StatusCondition::NONE) {
        statusText += "\n状态: " + getStatusText(playerCreature->getStatusCondition());
    }
    m_playerStatusLabel->setText(statusText);
}

void BattleScene::updateOpponentUI() {
    Creature* opponentCreature = m_battleSystem->getOpponentActiveCreature();
    if (!opponentCreature) return;
    
    // 更新精灵图像
    QPixmap creaturePixmap(QString(":/sprites/%1_front.png").arg(opponentCreature->getName().toLower().replace(' ', '_')));
    if (creaturePixmap.isNull()) {
        creaturePixmap = QPixmap(":/sprites/default_front.png");
    }
    m_opponentCreatureLabel->setPixmap(creaturePixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    // 更新HP条
    m_opponentHPBar->setMaximum(opponentCreature->getMaxHP());
    m_opponentHPBar->setValue(opponentCreature->getCurrentHP());
    
    // 设置HP条颜色
    double hpRatio = (double)opponentCreature->getCurrentHP() / opponentCreature->getMaxHP();
    QString hpColor;
    if (hpRatio > 0.5) {
        hpColor = "green";
    } else if (hpRatio > 0.25) {
        hpColor = "yellow";
    } else {
        hpColor = "red";
    }
    m_opponentHPBar->setStyleSheet(QString("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: %1; }").arg(hpColor));
    
    // 更新PP条
    m_opponentPPBar->setMaximum(opponentCreature->getMaxPP());
    m_opponentPPBar->setValue(opponentCreature->getCurrentPP());
    m_opponentPPBar->setStyleSheet("QProgressBar { text-align: center; } QProgressBar::chunk { background-color: blue; }");
    
    // 更新状态标签
    QString statusText = opponentCreature->getName() + " Lv." + QString::number(opponentCreature->getLevel()) + "\n";
    statusText += "类型: " + Type::getElementTypeName(opponentCreature->getType().getPrimaryType());
    if (opponentCreature->getType().getSecondaryType() != ElementType::NONE) {
        statusText += "/" + Type::getElementTypeName(opponentCreature->getType().getSecondaryType());
    }
    if (opponentCreature->getStatusCondition() != StatusCondition::NONE) {
        statusText += "\n状态: " + getStatusText(opponentCreature->getStatusCondition());
    }
    m_opponentStatusLabel->setText(statusText);
}

void BattleScene::updateSkillButtons() {
    Creature* playerCreature = m_battleSystem->getPlayerActiveCreature();
    if (!playerCreature) return;
    
    // 获取精灵的技能
    for (int i = 0; i < 4; ++i) {
        Skill* skill = nullptr;
        if (i < playerCreature->getSkillCount()) {
            skill = playerCreature->getSkill(i);
        }
        
        // 更新按钮
        m_skillButtons[i]->setSkill(skill);
        m_skillButtons[i]->setEnabled(skill != nullptr && skill->getCurrentPP() > 0 && playerCreature->canAct());
    }
}

void BattleScene::updateBattleLog(const QString& message) {
    QString currentText = m_battleLogLabel->text();
    if (!currentText.isEmpty()) {
        currentText += "<br>";
    }
    m_battleLogLabel->setText(currentText + message);
}

void BattleScene::onSkillButtonClicked(int skillIndex) {
    // 禁用所有按钮，防止重复点击
    for (auto* btn : m_skillButtons) {
        btn->setEnabled(false);
    }
    m_switchButton->setEnabled(false);
    m_escapeButton->setEnabled(false);
    
    // 执行使用技能动作
    m_battleSystem->executeAction(BattleAction::USE_SKILL, skillIndex);
    
    // 更新UI
    updatePlayerUI();
    updateOpponentUI();
    
    // 处理回合
    QTimer::singleShot(500, m_battleSystem, &BattleSystem::processTurn);
}

void BattleScene::onSwitchButtonClicked() {
    // TODO: 实现切换精灵功能
    updateBattleLog("切换精灵功能尚未实现");
}

void BattleScene::onEscapeButtonClicked() {
    // 禁用所有按钮，防止重复点击
    for (auto* btn : m_skillButtons) {
        btn->setEnabled(false);
    }
    m_switchButton->setEnabled(false);
    m_escapeButton->setEnabled(false);
    
    // 执行逃跑动作
    if (m_battleSystem->executeAction(BattleAction::ESCAPE)) {
        updateBattleLog("尝试逃跑...");
    } else {
        updateBattleLog("<font color='red'>无法逃跑!</font>");
        
        // 重新启用按钮
        updateSkillButtons();
        m_switchButton->setEnabled(true);
        m_escapeButton->setEnabled(!m_battleSystem->isPvPBattle());
    }
}

void BattleScene::onBattleLogUpdated(const QString& message) {
    updateBattleLog(message);
}

void BattleScene::onTurnStarted(int turn, bool isPlayerTurn) {
    // 更新回合标签
    m_turnLabel->setText(QString("回合: %1 - %2的回合").arg(turn).arg(isPlayerTurn ? "玩家" : "对手"));
    
    if (isPlayerTurn) {
        // 启用玩家操作按钮
        updateSkillButtons();
        m_switchButton->setEnabled(true);
        m_escapeButton->setEnabled(!m_battleSystem->isPvPBattle());
        
        updateBattleLog("<b>你的回合</b>");
    } else {
        // 禁用玩家操作按钮
        for (auto* btn : m_skillButtons) {
            btn->setEnabled(false);
        }
        m_switchButton->setEnabled(false);
        m_escapeButton->setEnabled(false);
        
        updateBattleLog("<b>对手的回合</b>");
    }
}

void BattleScene::onTurnEnded(int turn) {
    // 更新回合标签
    m_turnLabel->setText(QString("回合: %1").arg(turn + 1));
    
    // 更新UI
    updatePlayerUI();
    updateOpponentUI();
    updateSkillButtons();
}

void BattleScene::onDamageCaused(Creature* creature, int damage) {
    // 根据是玩家还是对手的精灵，来执行相应动画
    QLabel* creatureLabel = nullptr;
    
    if (creature == m_battleSystem->getPlayerActiveCreature()) {
        creatureLabel = m_playerCreatureLabel;
    } else if (creature == m_battleSystem->getOpponentActiveCreature()) {
        creatureLabel = m_opponentCreatureLabel;
    } else {
        return;
    }
    
    // 执行伤害动画
    animateDamage(creatureLabel, damage);
    
    // 更新UI
    updatePlayerUI();
    updateOpponentUI();
}

void BattleScene::onHealingReceived(Creature* creature, int amount) {
    // 根据是玩家还是对手的精灵，来执行相应动画
    QLabel* creatureLabel = nullptr;
    
    if (creature == m_battleSystem->getPlayerActiveCreature()) {
        creatureLabel = m_playerCreatureLabel;
    } else if (creature == m_battleSystem->getOpponentActiveCreature()) {
        creatureLabel = m_opponentCreatureLabel;
    } else {
        return;
    }
    
    // 执行治疗动画
    animateHealing(creatureLabel, amount);
    
    // 更新UI
    updatePlayerUI();
    updateOpponentUI();
}

void BattleScene::onCreatureSwitched(Creature* oldCreature, Creature* newCreature, bool isPlayer) {
    QString message = QString("%1 切换到了 %2!")
        .arg(isPlayer ? "你" : "对手")
        .arg(newCreature->getName());
    updateBattleLog(message);
    
    // 更新UI
    updatePlayerUI();
    updateOpponentUI();
    updateSkillButtons();
}

void BattleScene::animateDamage(QLabel* label, int damage) {
    if (!label) return;
    
    // 创建伤害标签
    QLabel* damageLabel = new QLabel(QString("-%1").arg(damage), this);
    damageLabel->setAlignment(Qt::AlignCenter);
    damageLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: red;");
    
    // 定位在精灵图像上
    damageLabel->move(label->pos() + QPoint(label->width() / 2 - 20, label->height() / 2 - 20));
    damageLabel->show();
    
    // 创建动画
    QPropertyAnimation* animation = new QPropertyAnimation(damageLabel, "pos");
    animation->setDuration(1000);
    animation->setStartValue(damageLabel->pos());
    animation->setEndValue(damageLabel->pos() - QPoint(0, 50));
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    // 创建淡出效果
    QGraphicsOpacityEffect* opacity = new QGraphicsOpacityEffect(damageLabel);
    damageLabel->setGraphicsEffect(opacity);
    
    QPropertyAnimation* fadeOut = new QPropertyAnimation(opacity, "opacity");
    fadeOut->setDuration(1000);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::OutCubic);
    
    // 连接动画完成信号
    connect(fadeOut, &QPropertyAnimation::finished, damageLabel, &QLabel::deleteLater);
    
    // 播放动画
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    
    // 创建精灵的闪烁效果
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this, label, timer, count = 0]() mutable {
        label->setVisible(!label->isVisible());
        count++;
        if (count >= 6) { // 闪烁3次
            label->setVisible(true);
            timer->stop();
            timer->deleteLater();
        }
    });
    timer->start(100);
}

void BattleScene::animateHealing(QLabel* label, int amount) {
    if (!label) return;
    
    // 创建治疗标签
    QLabel* healLabel = new QLabel(QString("+%1").arg(amount), this);
    healLabel->setAlignment(Qt::AlignCenter);
    healLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: green;");
    
    // 定位在精灵图像上
    healLabel->move(label->pos() + QPoint(label->width() / 2 - 20, label->height() / 2 - 20));
    healLabel->show();
    
    // 创建动画
    QPropertyAnimation* animation = new QPropertyAnimation(healLabel, "pos");
    animation->setDuration(1000);
    animation->setStartValue(healLabel->pos());
    animation->setEndValue(healLabel->pos() - QPoint(0, 50));
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    // 创建淡出效果
    QGraphicsOpacityEffect* opacity = new QGraphicsOpacityEffect(healLabel);
    healLabel->setGraphicsEffect(opacity);
    
    QPropertyAnimation* fadeOut = new QPropertyAnimation(opacity, "opacity");
    fadeOut->setDuration(1000);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::OutCubic);
    
    // 连接动画完成信号
    connect(fadeOut, &QPropertyAnimation::finished, healLabel, &QLabel::deleteLater);
    
    // 播放动画
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    
    // 创建精灵的闪烁效果
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this, label, timer, count = 0]() mutable {
        label->setVisible(!label->isVisible());
        count++;
        if (count >= 6) { // 闪烁3次
            label->setVisible(true);
            timer->stop();
            timer->deleteLater();
        }
    });
    timer->start(100);
}

QString BattleScene::getStatusText(StatusCondition condition) {
    switch (condition) {
        case StatusCondition::POISONED: return "中毒";
        case StatusCondition::PARALYZED: return "麻痹";
        case StatusCondition::BURNED: return "烧伤";
        case StatusCondition::FROZEN: return "冻结";
        case StatusCondition::ASLEEP: return "睡眠";
        case StatusCondition::CONFUSED: return "混乱";
        case StatusCondition::FRIGHTENED: return "恐惧";
        default: return "";
    }
}

QString BattleScene::getStatStageText(const StatStages& stages) {
    QStringList changes;
    
    if (stages.attack != 0) {
        changes << QString("攻击: %1%2").arg(stages.attack > 0 ? "+" : "").arg(stages.attack);
    }
    if (stages.defense != 0) {
        changes << QString("防御: %1%2").arg(stages.defense > 0 ? "+" : "").arg(stages.defense);
    }
    if (stages.specialAttack != 0) {
        changes << QString("特攻: %1%2").arg(stages.specialAttack > 0 ? "+" : "").arg(stages.specialAttack);
    }
    if (stages.specialDefense != 0) {
        changes << QString("特防: %1%2").arg(stages.specialDefense > 0 ? "+" : "").arg(stages.specialDefense);
    }
    if (stages.speed != 0) {
        changes << QString("速度: %1%2").arg(stages.speed > 0 ? "+" : "").arg(stages.speed);
    }
    if (stages.accuracy != 0) {
        changes << QString("命中: %1%2").arg(stages.accuracy > 0 ? "+" : "").arg(stages.accuracy);
    }
    if (stages.evasion != 0) {
        changes << QString("闪避: %1%2").arg(stages.evasion > 0 ? "+" : "").arg(stages.evasion);
    }
    
    return changes.join(", ");
}

// 为了编译Q_OBJECT
#include "battlescene.moc"
