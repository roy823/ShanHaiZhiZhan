#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QScrollArea> // <-- 包括 QScrollArea
#include "../core/gameengine.h"
#include "../battle/skill.h" // <-- 包括 Skill 定义

// 前向声明
class BattleSystem;
// SkillButton 将在这里定义

// 技能按钮类实现
// 将 SkillButton 类定义移到这里
class SkillButton : public QPushButton
{
    Q_OBJECT

public:
    SkillButton(int index, Skill *skill, QWidget *parent = nullptr)
        : QPushButton(parent), m_index(index), m_skill(skill)
    {
        updateText(); // 更新按钮文本和样式
        // 根据技能类型设置不同颜色等可以在 updateText 中实现
        connect(this, &QPushButton::clicked, this, &SkillButton::onClicked);
    }

    // 设置与此按钮关联的技能
    void setSkill(Skill *skill)
    {
        m_skill = skill;
        updateText();
        // 启用/禁用按钮的逻辑也应在 updateText 或 BattleScene 中处理
    }

    // 获取技能索引
    int getIndex() const { return m_index; }
    // 获取技能指针 (可选，如果外部需要访问)
    Skill* getSkillPtr() const { return m_skill; }


signals:
    // 当按钮被点击时，发出技能索引信号
    void skillSelected(int index);

private slots:
    // 内部槽，处理按钮点击事件
    void onClicked()
    {
        if (m_skill && isEnabled()) // 确保技能存在且按钮可用
        {
            emit skillSelected(m_index);
        }
    }

private:
    int m_index;    // 技能在列表中的索引
    Skill *m_skill; // 指向技能对象的指针

    // 更新按钮的文本和外观
    void updateText()
    {
        if (m_skill)
        {
            QString skillCategoryText;
            switch (m_skill->getCategory())
            {
            case SkillCategory::PHYSICAL: skillCategoryText = "物理"; break;
            case SkillCategory::SPECIAL:  skillCategoryText = "特殊"; break;
            case SkillCategory::STATUS:   skillCategoryText = "状态"; break;
            default: skillCategoryText = "未知"; break;
            }

            // 格式化技能信息显示在按钮上
            // 注意: ElementType::getTypeName 在你的代码中似乎是静态方法 Type::getTypeName
            QString text = QString("%1\n<font size='-2'>%2 | %3 | PP: %4/%5</font>")
                               .arg(m_skill->getName())
                               .arg(Type::getTypeName(m_skill->getType())) // 修正调用方式
                               .arg(skillCategoryText)
                               .arg(m_skill->getCurrentPP())
                               .arg(m_skill->getPPCost() == 0 && m_skill->getMaxPP() != 0 ? m_skill->getMaxPP() : m_skill->getPPCost()); // 显示正确的最大PP或消耗

            setText(text);
            setToolTip(m_skill->getDescription().replace("\n", "<br>")); // Tooltip显示技能描述

            // 设置按钮样式，可以根据技能类型改变颜色
            // 注意: ElementType::getElementTypeColor 在你的代码中似乎是静态方法 Type::getElementTypeColor
            QString typeColor = Type::getElementTypeColor(m_skill->getType()); // 修正调用方式
            setStyleSheet(QString("QPushButton { background-color: %1; color: white; padding: 5px; border-radius: 5px; text-align: center;} QPushButton:disabled { background-color: gray; color: lightgray; }").arg(typeColor));
            setEnabled(m_skill->getCurrentPP() > 0); // 根据PP值设置是否可用
        }
        else
        {
            setText("--"); // 没有技能时显示占位符
            setToolTip("");
            setStyleSheet("QPushButton { background-color: #707070; color: #A0A0A0; padding: 5px; border-radius: 5px; }"); // 默认灰色禁用样式
            setEnabled(false);
        }
    }
};


class BattleScene : public QWidget
{
    Q_OBJECT

public:
    explicit BattleScene(GameEngine *gameEngine, QWidget *parent = nullptr);
    ~BattleScene();

    // 初始化场景
    void initScene();

private slots:
    // 技能按钮点击响应
    void onSkillButtonClicked(int skillIndex);
    // 第五技能按钮点击响应
    void onFifthSkillButtonClicked();

    // 切换精灵按钮点击响应
    void onSwitchButtonClicked();

    // 逃跑按钮点击响应
    void onEscapeButtonClicked();

    // 战斗相关信号响应
    void onBattleLogUpdated(const QString &message);
    void onTurnStarted(int turn, bool isPlayerTurn);
    void onTurnEnded(int turn);
    void onDamageCaused(Creature *creature, int damage);
    void onHealingReceived(Creature *creature, int amount);
    void onCreatureSwitched(Creature *oldCreature, Creature *newCreature, bool isPlayer);

private:
    // 游戏引擎和战斗系统
    GameEngine *m_gameEngine;
    BattleSystem *m_battleSystem;

    // UI组件
    QLabel *m_playerCreatureLabel;
    QLabel *m_opponentCreatureLabel;
    QProgressBar *m_playerHPBar;
    QProgressBar *m_playerPPBar;
    QProgressBar *m_opponentHPBar;
    QProgressBar *m_opponentPPBar; 
    QLabel *m_playerStatusLabel;
    QLabel *m_opponentStatusLabel;
    
    QScrollArea *m_logScrollArea; // <-- 新增日志滚动区域成员
    QLabel *m_battleLogLabel;
    QLabel *m_turnLabel;

    // 技能按钮
    QVector<SkillButton *> m_skillButtons; // 现在 SkillButton 类型是完整的
    QPushButton *m_fifthSkillButton; 
    QPushButton *m_switchButton;
    QPushButton *m_escapeButton;

    // 布局
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_battlefieldLayout;
    QVBoxLayout *m_playerLayout;
    QVBoxLayout *m_opponentLayout;
    QGridLayout *m_actionLayout;
    QVBoxLayout *m_logLayout;

    // 动画计时器
    QTimer *m_animationTimer;

    // 设置UI
    void setupUI();

    // 更新UI
    void updatePlayerUI();
    void updateOpponentUI();
    void updateSkillButtons();
    void updateFifthSkillButton(); 
    void updateBattleLog(const QString &message);

    // 动画效果
    void animateDamage(QLabel *label, int damage);
    void animateHealing(QLabel *label, int amount);

    // 获取精灵状态信息
    QString getStatusText(StatusCondition condition);
    QString getStatStageText(const StatStages &stages);
};

#endif // BATTLESCENE_H