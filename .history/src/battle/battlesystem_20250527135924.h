#ifndef BATTLESYSTEM_H
#define BATTLESYSTEM_H

#include <QObject>
#include <QVector>
#include <QPair>
#include <QTimer> 
#include "../core/creature.h"

// 战斗操作枚举
enum class BattleAction
{
    USE_SKILL,       // 使用技能
    SWITCH_CREATURE, // 切换精灵
    USE_ITEM,        // 使用道具
    ESCAPE ,         // 逃跑
    RESTORE_PP,      // 恢复PP
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

    void initBattle(QVector<Creature *> playerTeam, QVector<Creature *> opponentTeam, bool isPvP = false);

    BattleResult getBattleResult() const;
    int getCurrentTurn() const;
    bool isPvPBattle() const;

    Creature *getPlayerActiveCreature() const;
    Creature *getOpponentActiveCreature() const;
    QVector<Creature *> getPlayerTeam() const;
    QVector<Creature *> getOpponentTeam() const;

    // 新的行动提交流程
    void playerSubmittedAction(BattleAction action, int param1 = -1, int param2 = -1);
    // PvP模式下，你会有类似的 opponentSubmittedAction。对于PvE：
    void decideAIAction(); // AI决定并提交其行动

    bool checkBattleEnd(); // 改为 public，方便外部潜在检查，但主要还是内部使用
    QVector<BattleLogEntry> getBattleLog() const;

    int calculateDamage(Creature *attacker, Creature *defender, Skill *skill);
    bool checkSkillHit(Creature *attacker, Creature *defender, Skill *skill);
    
    // 触发战斗事件
    void triggerHealingReceived(Creature* creature, int amount);
    void triggerDamageCaused(Creature* creature, int amount);
    void triggerStatusChanged(Creature* target, StatusCondition oldCondition, StatusCondition newCondition);
    void triggerStatStageChanged(Creature* target, StatType stat, int oldStage, int newStage);
    QString getStatTypeName(StatType stat) const;


public slots:

signals:
    void battleStarted();
    void battleEnded(BattleResult result);
    void turnStarted(int turn, bool isPlayerTurn); 
    void turnEnded(int turn); // 表示一个完整回合（双方行动后）的结束
    void skillUsed(Creature *user, Creature *target, Skill *skill, bool hit, int damage);
    void damageCaused(Creature *creature, int damage);
    void healingReceived(Creature *creature, int amount);
    void statusChanged(Creature *creature, StatusCondition oldStatus, StatusCondition newStatus);
    void statStageChanged(Creature *creature, StatType stat, int oldStage, int newStage);
    void creatureSwitched(Creature *oldCreature, Creature *newCreature, bool isPlayer);
    void battleLogUpdated(const QString &message);

    void playerActionConfirmed();   // 玩家已提交本回合行动
    void opponentActionConfirmed(); // 对手/AI已提交本回合行动

private:
    BattleResult m_battleResult;
    int m_currentTurn;
    bool m_isPvP;
    QVector<Creature *> m_playerTeam;
    QVector<Creature *> m_opponentTeam;
    int m_playerActiveIndex;
    int m_opponentActiveIndex;

    struct ActionQueueItem { // 保持不变
        Creature *actor;
        BattleAction action;
        int param1; 
        int param2; 
        int priority;
    };
    QVector<ActionQueueItem> m_actionQueue;
    QVector<BattleLogEntry> m_battleLog;

    // 新增状态标志，用于回合流程控制
    bool m_playerActionSubmittedThisTurn;
    bool m_opponentActionSubmittedThisTurn;

    void queuePlayerAction(BattleAction action, int param1 = -1, int param2 = -1);
    void queueOpponentAction(BattleAction action, int param1 = -1, int param2 = -1);
    void sortActionQueue();
    void executeActionQueue();
    void processTurnStartEffects();
    void processTurnEndEffects();
    void addBattleLog(const QString &message, Creature *source = nullptr, Creature *target = nullptr);

    // 精细化的回合处理方法
    void tryProcessTurnActions();     // 检查是否双方都已行动，如果是则开始结算
    void processTurnInputPhase();     // 设置进入行动输入阶段
    void processTurnExecutePhase();   // 执行已提交的行动并结束当前回合的结算

};

#endif // BATTLESYSTEM_H