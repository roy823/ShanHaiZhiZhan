#ifndef BATTLESYSTEM_H
#define BATTLESYSTEM_H

#include <QObject>
#include <QVector>
#include <QPair>
#include "../core/creature.h"

// 战斗操作枚举
enum class BattleAction
{
    USE_SKILL, SWITCH_CREATURE, USE_ITEM, ESCAPE, RESTORE_PP,
};

// 战斗结果枚举
enum class BattleResult
{
    ONGOING, PLAYER_WIN, OPPONENT_WIN, DRAW, ESCAPE
};

// 战斗日志条目结构
struct BattleLogEntry
{
    QString message;
    int turn;
    QString sourceCreature;
    QString targetCreature;
};

// 行动队列条目
struct ActionQueueEntry
{
    BattleAction action;
    int param1;
    int param2;
    Creature* actor;
    int priority;
    int speed;
};

class BattleSystem : public QObject
{
    Q_OBJECT
public:
    explicit BattleSystem(QObject *parent = nullptr);
    ~BattleSystem();

    void initBattle(QVector<Creature *> playerTeam, QVector<Creature *> opponentTeam, bool isPvP);

    BattleResult getBattleResult() const;
    int getCurrentTurn() const;
    bool isPlayerTurn() const;
    bool isPvPBattle() const;

    void playerActionTaken();
    void decideAIActionAndProcess();

    Creature *getPlayerActiveCreature() const;
    Creature *getOpponentActiveCreature() const;
    QVector<Creature *> getPlayerTeam() const;
    QVector<Creature *> getOpponentTeam() const;

    // 新版：只收集行动，双方都准备好后统一处理
    bool executeAction(BattleAction action, int param1, int param2);

    void processTurn();

    bool checkBattleEnd();

    QVector<BattleLogEntry> getBattleLog() const;

    int calculateDamage(Creature *attacker, Creature *defender, Skill *skill);
    bool checkSkillHit(Creature *attacker, Creature *defender, Skill *skill);

    void onPlayerActionSelected(BattleAction action, int param1, int param2);
    void onAIActionSelected();

    void queuePlayerAction(BattleAction action, int param1, int param2);
    void queueOpponentAction(BattleAction action, int param1, int param2);

    void sortActionQueue();

signals:
    void battleEnded(BattleResult result);
    void turnEnded(int turn);
    void turnStarted(int turn, bool isPlayerTurn);

private:
    BattleResult m_battleResult;
    int m_currentTurn;
    bool m_isPvP;
    int m_playerActiveIndex;
    int m_opponentActiveIndex;
    QVector<Creature*> m_playerTeam;
    QVector<Creature*> m_opponentTeam;

    // 行动队列
    QVector<ActionQueueEntry> m_actionQueue;
    bool m_playerActionReady = false;
    bool m_opponentActionReady = false;

    QVector<BattleLogEntry> m_battleLog;

    void addBattleLog(const QString& msg);

    // 你可以根据需要添加更多私有成员
};

#endif