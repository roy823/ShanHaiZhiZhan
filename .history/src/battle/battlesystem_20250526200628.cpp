#include "battlesystem.h"
#include <algorithm>
#include <QRandomGenerator>

BattleSystem::BattleSystem(QObject *parent)
    : QObject(parent),
      m_battleResult(BattleResult::ONGOING),
      m_currentTurn(1),
      m_isPvP(false),
      m_playerActiveIndex(0),
      m_opponentActiveIndex(0),
      m_playerActionReady(false),
      m_opponentActionReady(false)
{
}

BattleSystem::~BattleSystem()
{
}

void BattleSystem::initBattle(QVector<Creature *> playerTeam, QVector<Creature *> opponentTeam, bool isPvP)
{
    m_playerTeam = playerTeam;
    m_opponentTeam = opponentTeam;
    m_isPvP = isPvP;
    m_currentTurn = 1;
    m_battleResult = BattleResult::ONGOING;
    m_playerActiveIndex = 0;
    m_opponentActiveIndex = 0;
    m_actionQueue.clear();
    m_battleLog.clear();
    m_playerActionReady = false;
    m_opponentActionReady = false;
    emit turnStarted(m_currentTurn, true);
}

BattleResult BattleSystem::getBattleResult() const { return m_battleResult; }
int BattleSystem::getCurrentTurn() const { return m_currentTurn; }
bool BattleSystem::isPlayerTurn() const { return !m_playerActionReady; }
bool BattleSystem::isPvPBattle() const { return m_isPvP; }

void BattleSystem::playerActionTaken() { m_playerActionReady = true; }
void BattleSystem::decideAIActionAndProcess()
{
    // 简单AI：总是用第一个技能
    if (!m_opponentActionReady) {
        queueOpponentAction(BattleAction::USE_SKILL, 0, 0);
        m_opponentActionReady = true;
    }
    // 如果玩家也已准备好，则处理回合
    if (m_playerActionReady && m_opponentActionReady) {
        processTurn();
        m_playerActionReady = false;
        m_opponentActionReady = false;
    }
}

Creature *BattleSystem::getPlayerActiveCreature() const
{
    if (m_playerActiveIndex >= 0 && m_playerActiveIndex < m_playerTeam.size())
        return m_playerTeam[m_playerActiveIndex];
    return nullptr;
}
Creature *BattleSystem::getOpponentActiveCreature() const
{
    if (m_opponentActiveIndex >= 0 && m_opponentActiveIndex < m_opponentTeam.size())
        return m_opponentTeam[m_opponentActiveIndex];
    return nullptr;
}
QVector<Creature *> BattleSystem::getPlayerTeam() const { return m_playerTeam; }
QVector<Creature *> BattleSystem::getOpponentTeam() const { return m_opponentTeam; }

bool BattleSystem::executeAction(BattleAction action, int param1, int param2)
{
    // 玩家行动
    if (!m_playerActionReady) {
        queuePlayerAction(action, param1, param2);
        m_playerActionReady = true;
    }
    // AI行动（PvE自动，PvP需等待对方）
    else if (!m_opponentActionReady) {
        queueOpponentAction(action, param1, param2);
        m_opponentActionReady = true;
    }

    // PvE下AI自动行动
    if (!m_isPvP && !m_opponentActionReady) {
        decideAIActionAndProcess();
        return false;
    }

    // 双方都准备好，处理回合
    if (m_playerActionReady && m_opponentActionReady) {
        processTurn();
        m_playerActionReady = false;
        m_opponentActionReady = false;
        return true;
    }
    return false;
}

void BattleSystem::onPlayerActionSelected(BattleAction action, int param1, int param2)
{
    executeAction(action, param1, param2);
}

void BattleSystem::onAIActionSelected()
{
    // PvE下AI自动行动，无需手动调用
}

void BattleSystem::queuePlayerAction(BattleAction action, int param1, int param2)
{
    Creature* actor = getPlayerActiveCreature();
    int priority = 0;
    int speed = actor ? actor->getSpeed() : 0;
    if (actor && action == BattleAction::USE_SKILL) {
        Skill* skill = actor->getSkill(param1);
        if (skill) {
            priority = skill->getPriority();
        }
    }
    m_actionQueue.append({action, param1, param2, actor, priority, speed});
}

void BattleSystem::queueOpponentAction(BattleAction action, int param1, int param2)
{
    Creature* actor = getOpponentActiveCreature();
    int priority = 0;
    int speed = actor ? actor->getSpeed() : 0;
    if (actor && action == BattleAction::USE_SKILL) {
        Skill* skill = actor->getSkill(param1);
        if (skill) {
            priority = skill->getPriority();
        }
    }
    m_actionQueue.append({action, param1, param2, actor, priority, speed});
}

void BattleSystem::sortActionQueue()
{
    std::sort(m_actionQueue.begin(), m_actionQueue.end(), [](const ActionQueueEntry& a, const ActionQueueEntry& b) {
        if (a.priority != b.priority)
            return a.priority > b.priority;
        if (a.speed != b.speed)
            return a.speed > b.speed;
        // 随机决定
        return QRandomGenerator::global()->bounded(2) == 0;
    });
}

void BattleSystem::processTurn()
{
    // 排序行动队列
    sortActionQueue();

    // 执行所有行动
    for (const auto& entry : m_actionQueue) {
        // 这里只做简单示例，实际应根据 action 类型分别处理
        if (entry.action == BattleAction::USE_SKILL && entry.actor) {
            Creature* target = (entry.actor == getPlayerActiveCreature()) ? getOpponentActiveCreature() : getPlayerActiveCreature();
            Skill* skill = entry.actor->getSkill(entry.param1);
            if (skill && target) {
                skill->use(entry.actor, target, this);
                addBattleLog(QString("%1 使用了 %2").arg(entry.actor->getName(), skill->getName()));
            }
        }
        // 其他行动类型可自行扩展
    }

    m_actionQueue.clear();

    // 检查战斗是否结束
    if (checkBattleEnd()) {
        emit battleEnded(m_battleResult);
        addBattleLog("战斗结束！");
        return;
    }

    m_currentTurn++;
    emit turnEnded(m_currentTurn - 1);
    emit turnStarted(m_currentTurn, true); // 新回合，等待玩家操作
    addBattleLog(QString("第 %1 回合开始").arg(m_currentTurn));
}

bool BattleSystem::checkBattleEnd()
{
    // 简单判定：任一方全灭
    bool playerAllDead = std::all_of(m_playerTeam.begin(), m_playerTeam.end(), [](Creature* c) { return c->isDead(); });
    bool opponentAllDead = std::all_of(m_opponentTeam.begin(), m_opponentTeam.end(), [](Creature* c) { return c->isDead(); });
    if (playerAllDead && opponentAllDead) {
        m_battleResult = BattleResult::DRAW;
        return true;
    }
    if (playerAllDead) {
        m_battleResult = BattleResult::OPPONENT_WIN;
        return true;
    }
    if (opponentAllDead) {
        m_battleResult = BattleResult::PLAYER_WIN;
        return true;
    }
    return false;
}

QVector<BattleLogEntry> BattleSystem::getBattleLog() const { return m_battleLog; }

int BattleSystem::calculateDamage(Creature *attacker, Creature *defender, Skill *skill)
{
    // 示例：返回技能威力
    if (skill)
        return skill->getPower();
    return 0;
}

bool BattleSystem::checkSkillHit(Creature *attacker, Creature *defender, Skill *skill)
{
    if (!skill) return false;
    int acc = skill->getAccuracy();
    if (acc >= 100) return true;
    int roll = QRandomGenerator::global()->bounded(100);
    return roll < acc;
}

void BattleSystem::addBattleLog(const QString& msg)
{
    m_battleLog.append({msg, m_currentTurn, "", ""});
}