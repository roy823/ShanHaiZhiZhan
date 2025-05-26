// src/battle/battlesystem.h
#ifndef BATTLESYSTEM_H
#define BATTLESYSTEM_H

#include <QObject>
#include <QVector>
#include <QTimer> // 为AI思考延迟添加
#include "../core/creature.h"
// 其他头文件...

// BattleAction 和 BattleResult 枚举保持不变
// BattleLogEntry 结构体保持不变

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
    
    // 移除了 playerActionTaken，其功能已整合到新流程中

public slots:
    // 移除了 onPlayerActionSelected 和 onAIActionSelected，它们被直接的方法调用取代

signals:
    void battleStarted();
    void battleEnded(BattleResult result);
    // isPlayerTurn 现在更多指示玩家的输入窗口开始
    void turnStarted(int turn, bool isPlayerTurn); 
    void turnEnded(int turn); // 表示一个完整回合（双方行动后）的结束
    void skillUsed(Creature *user, Creature *target, Skill *skill, bool hit, int damage);
    void damageCaused(Creature *creature, int damage);
    void healingReceived(Creature *creature, int amount);
    void statusChanged(Creature *creature, StatusCondition oldStatus, StatusCondition newStatus);
    void statStageChanged(Creature *creature, StatType stat, int oldStage, int newStage);
    void creatureSwitched(Creature *oldCreature, Creature *newCreature, bool isPlayer);
    void battleLogUpdated(const QString &message);

    // 新增信号，用于UI反馈
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

    void triggerHealingReceived(Creature* creature, int amount);
    void triggerDamageCaused(Creature* creature, int amount);
    void triggerStatusChanged(Creature* target, StatusCondition oldCondition, StatusCondition newCondition);
    void triggerStatStageChanged(Creature* target, StatType stat, int oldStage, int newStage);
};

#endif // BATTLESYSTEM_H