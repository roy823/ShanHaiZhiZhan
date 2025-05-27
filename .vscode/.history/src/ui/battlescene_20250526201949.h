// src/ui/battlescene.h
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

#include "../core/gameengine.h" // 引入游戏引擎核心

// 前向声明
class BattleSystem; // 战斗系统类
class SkillButton;  // 技能按钮类
class Creature;     // 精灵类

class BattleScene : public QWidget
{
    Q_OBJECT

public:
    // 构造函数，传入游戏引擎指针和父控件
    explicit BattleScene(GameEngine *gameEngine, QWidget *parent = nullptr);
    // 析构函数
    ~BattleScene();

    // 初始化场景
    void initScene();

private slots:
    // 技能按钮点击响应
    void onSkillButtonClicked(int skillIndex);

    // 切换精灵按钮点击响应
    void onSwitchButtonClicked();

    // 逃跑按钮点击响应
    void onEscapeButtonClicked();

    // 战斗相关信号响应
    void onBattleLogUpdated(const QString &message); // 战斗日志更新
    void onTurnStarted(int turn, bool isPlayerTurn); // 回合开始
    void onTurnEnded(int turn);                      // 回合结束
    void onDamageCaused(Creature *creature, int damage); // 造成伤害
    void onHealingReceived(Creature *creature, int amount); // 受到治疗
    void onCreatureSwitched(Creature *oldCreature, Creature *newCreature, bool isPlayer); // 精灵切换
    void onPlayerActionConfirmed();    // 响应玩家行动已确认的信号
    void onOpponentActionConfirmed();  // 响应对手行动已确认的信号（可选，用于UI反馈）
    
    // 第五技能按钮点击响应
    void onFifthSkillButtonClicked();

    // 恢复PP按钮点击响应
    void onRestorePPButtonClicked();

private:
    // 游戏引擎和战斗系统
    GameEngine *m_gameEngine;       // 游戏引擎实例指针
    BattleSystem *m_battleSystem;   // 战斗系统实例指针

    // UI组件
    QLabel *m_playerCreatureLabel;      // 玩家精灵图片标签
    QLabel *m_opponentCreatureLabel;    // 对手精灵图片标签
    QProgressBar *m_playerHPBar;        // 玩家HP条
    QProgressBar *m_playerPPBar;        // 玩家PP条 (全局PP)
    QProgressBar *m_opponentHPBar;      // 对手HP条
    QProgressBar *m_opponentPPBar;      // 对手PP条 (全局PP)
    QLabel *m_playerStatusLabel;      // 玩家状态标签
    QLabel *m_opponentStatusLabel;    // 对手状态标签
    QLabel *m_battleLogLabel;         // 战斗日志标签
    QLabel *m_turnLabel;              // 回合数标签

    // 技能按钮
    QVector<SkillButton *> m_skillButtons; // 存储普通技能按钮的向量
    QPushButton *m_switchButton;        // 切换精灵按钮
    QPushButton *m_escapeButton;        // 逃跑按钮

    // 第五技能按钮
    QPushButton *m_fifthSkillButton;    // 第五技能按钮

    // 恢复PP按钮
    QPushButton *m_restorePPButton;     // 恢复PP按钮

    // 布局
    QVBoxLayout *m_mainLayout;          // 主垂直布局
    QHBoxLayout *m_battlefieldLayout;   // 战场水平布局 (精灵显示区域)
    QVBoxLayout *m_playerLayout;        // 玩家区域垂直布局
    QVBoxLayout *m_opponentLayout;      // 对手区域垂直布局
    QGridLayout *m_actionLayout;        // 行动按钮网格布局
    QVBoxLayout *m_logLayout;           // 日志和回合信息垂直布局

    // 动画计时器 (目前未使用，可用于更复杂的动画)
    QTimer *m_animationTimer;

    // 设置UI界面元素
    void setupUI();

    // 更新UI显示
    void updatePlayerUI();      // 更新玩家侧UI
    void updateOpponentUI();    // 更新对手侧UI
    void updateSkillButtons();  // 更新技能按钮状态和文本
    void updateBattleLog(const QString &message); // 更新战斗日志显示
    
    // 动画效果 (伤害和治疗的数字跳动)
    void animateDamage(QLabel *label, int damage);  // 伤害动画
    void animateHealing(QLabel *label, int amount); // 治疗动画

    // 获取精灵状态文本
    QString getStatusText(StatusCondition condition); // 获取异常状态的文本描述
    QString getStatStageText(const StatStages &stages); // 获取能力等级变化的文本描述
};

#endif // BATTLESCENE_H