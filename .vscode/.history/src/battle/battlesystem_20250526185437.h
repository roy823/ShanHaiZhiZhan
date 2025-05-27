#ifndef BATTLESYSTEM_H
#define BATTLESYSTEM_H

#include <QObject>
#include <QVector>
#include <QPair>
#include "../core/creature.h"

// 战斗操作枚举
enum class BattleAction
{
    USE_SKILL,       // 使用技能
    SWITCH_CREATURE, // 切换精灵
    USE_ITEM,        // 使用道具
    ESCAPE ,          // 逃跑
    
};

// 战斗结果枚举
enum class BattleResult
{
    ONGOING,      // 战斗进行中
    PLAYER_WIN,   // 玩家胜利
    OPPONENT_WIN, // 对手胜利
    DRAW,         // 平局
    ESCAPE        // 逃跑成功
};

// 战斗日志条目结构
struct BattleLogEntry
{
    QString message;        // 日志消息
    int turn;               // 回合数
    QString sourceCreature; // 源精灵名称
    QString targetCreature; // 目标精灵名称
};

// 战斗系统类
class BattleSystem : public QObject
{
    Q_OBJECT

public:
    explicit BattleSystem(QObject *parent = nullptr);
    ~BattleSystem();

    // 初始化战斗
    void initBattle(QVector<Creature *> playerTeam, QVector<Creature *> opponentTeam, bool isPvP = false);

    // 获取战斗状态
    BattleResult getBattleResult() const;
    int getCurrentTurn() const;
    bool isPlayerTurn() const;
    bool isPvPBattle() const;

    // 获取当前场上精灵
    Creature *getPlayerActiveCreature() const;
    Creature *getOpponentActiveCreature() const;

    // 获取所有精灵
    QVector<Creature *> getPlayerTeam() const;
    QVector<Creature *> getOpponentTeam() const;

    // 执行战斗动作
    bool executeAction(BattleAction action, int param1 = -1, int param2 = -1);

    // 处理回合
    void processTurn();
    void playerActionTaken();

    // 检查战斗结束
    bool checkBattleEnd();

    // 获取战斗日志
    QVector<BattleLogEntry> getBattleLog() const;

    // 技能伤害计算相关辅助方法
    int calculateDamage(Creature *attacker, Creature *defender, Skill *skill);
    bool checkSkillHit(Creature *attacker, Creature *defender, Skill *skill);

    // AI决策
    void decideAIAction();
    void decideAIActionAndProcess();

public slots:
    // 玩家行动选择响应
    void onPlayerActionSelected(BattleAction action, int param1 = -1, int param2 = -1);

    // AI对手行动响应
    void onAIActionSelected();

signals:
    // 战斗开始信号
    void battleStarted();

    // 战斗结束信号
    void battleEnded(BattleResult result);

    // 回合开始信号
    void turnStarted(int turn, bool isPlayerTurn);

    // 回合结束信号
    void turnEnded(int turn);

    // 技能使用信号
    void skillUsed(Creature *user, Creature *target, Skill *skill, bool hit, int damage);

    // 伤害造成信号
    void damageCaused(Creature *creature, int damage);

    // 治疗信号
    void healingReceived(Creature *creature, int amount);

    // 状态改变信号
    void statusChanged(Creature *creature, StatusCondition oldStatus, StatusCondition newStatus);

    // 能力等级改变信号
    void statStageChanged(Creature *creature, StatType stat, int oldStage, int newStage);

    // 精灵切换信号
    void creatureSwitched(Creature *oldCreature, Creature *newCreature, bool isPlayer);

    // 战斗日志信号
    void battleLogUpdated(const QString &message);

private:
    // 战斗状态
    BattleResult m_battleResult;
    int m_currentTurn;
    bool m_playerTurn;
    bool m_isPvP;

    // 精灵队伍
    QVector<Creature *> m_playerTeam;
    QVector<Creature *> m_opponentTeam;

    // 当前场上的精灵
    int m_playerActiveIndex;
    int m_opponentActiveIndex;

    // 当前回合的行动队列
    struct ActionQueueItem
    {
        Creature *actor;
        BattleAction action;
        int param1;   // 技能索引或目标精灵索引
        int param2;   // 附加参数
        int priority; // 优先级
    };
    QVector<ActionQueueItem> m_actionQueue;

    // 战斗日志
    QVector<BattleLogEntry> m_battleLog;

    // 队列玩家和对手的行动
    void queuePlayerAction(BattleAction action, int param1 = -1, int param2 = -1);
    void queueOpponentAction(BattleAction action, int param1 = -1, int param2 = -1);

    // 排序行动队列（基于优先级和速度）
    void sortActionQueue();

    // 执行行动队列中的动作
    void executeActionQueue();

    // 处理回合开始和结束效果
    void processTurnStartEffects();
    void processTurnEndEffects();

    // 触发信号
    void triggerHealingReceived(Creature* creature, int amount);
    void triggerDamageCaused(Creature* creature, int amount);
    void triggerStatusChanged(Creature* target, StatusCondition oldCondition, StatusCondition newCondition); 
    void triggerStatStageChanged(Creature* target, StatType stat, int oldStage, int newStage); 

    // 添加战斗日志
    void addBattleLog(const QString &message, Creature *source = nullptr, Creature *target = nullptr);
};

#endif // BATTLESYSTEM_H
