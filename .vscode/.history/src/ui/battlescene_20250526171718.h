#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton> // 确保 QPushButton 被包含
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimer>

#include "../core/gameengine.h"

// 前向声明
class BattleSystem;
class SkillButton;
// QPushButton 已包含

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
    // 第五技能按钮点击响应 (新添加)
    void onFifthSkillButtonClicked(); // 新增

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
    QProgressBar *m_opponentPPBar; // 如果需要显示对手PP，则保留
    QLabel *m_playerStatusLabel;
    QLabel *m_opponentStatusLabel;
    QLabel *m_battleLogLabel;
    QLabel *m_turnLabel;

    // 技能按钮
    QVector<SkillButton *> m_skillButtons;
    QPushButton *m_fifthSkillButton; // 新增第五技能按钮
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
    void updateFifthSkillButton(); // 新增，专门更新第五技能按钮状态和文本
    void updateBattleLog(const QString &message);

    // 动画效果
    void animateDamage(QLabel *label, int damage);
    void animateHealing(QLabel *label, int amount);

    // 获取精灵状态信息
    QString getStatusText(StatusCondition condition);
    QString getStatStageText(const StatStages &stages);
};

#endif // BATTLESCENE_H