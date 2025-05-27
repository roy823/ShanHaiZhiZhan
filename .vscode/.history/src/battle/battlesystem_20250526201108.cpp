#include "battlesystem.h"
#include <algorithm>
#include <QRandomGenerator>

// 构造函数
BattleSystem::BattleSystem(QObject *parent)
    : QObject(parent),
      m_battleResult(BattleResult::ONGOING),
      m_currentTurn(0),
      m_isPvP(false),
      m_playerActiveIndex(0),
      m_opponentActiveIndex(0),
      m_playerActionSubmittedThisTurn(false), 
      m_opponentActionSubmittedThisTurn(false)  
{
}

BattleSystem::~BattleSystem()
{
    // 注意：不要在这里删除精灵对象，因为它们可能在其他地方被使用
}

void BattleSystem::initBattle(QVector<Creature *> playerTeam, QVector<Creature *> opponentTeam, bool isPvP)
{
    m_battleResult = BattleResult::ONGOING;
    m_currentTurn = 0; // 在 processTurnInputPhase 中会自增到1
    m_isPvP = isPvP;

    m_playerTeam = playerTeam;
    m_opponentTeam = opponentTeam;
    m_playerActiveIndex = 0;
    m_opponentActiveIndex = 0;

    m_battleLog.clear();
    m_actionQueue.clear(); // 确保行动队列清空

    emit battleStarted();
    addBattleLog("战斗开始!");

    processTurnInputPhase(); // 开始第一个回合的输入阶段
}

BattleResult BattleSystem::getBattleResult() const
{
    return m_battleResult;
}

int BattleSystem::getCurrentTurn() const
{
    return m_currentTurn;
}

bool BattleSystem::isPlayerTurn() const
{
    return m_playerTurn;
}

bool BattleSystem::isPvPBattle() const
{
    return m_isPvP;
}

void BattleSystem::playerActionTaken() {
    // TODO: 实现玩家行动逻辑
}

void BattleSystem::decideAIActionAndProcess() {
    // TODO: 实现AI行动逻辑
}

Creature *BattleSystem::getPlayerActiveCreature() const
{
    if (m_playerActiveIndex >= 0 && m_playerActiveIndex < m_playerTeam.size())
    {
        return m_playerTeam[m_playerActiveIndex];
    }
    return nullptr;
}

Creature *BattleSystem::getOpponentActiveCreature() const
{
    if (m_opponentActiveIndex >= 0 && m_opponentActiveIndex < m_opponentTeam.size())
    {
        return m_opponentTeam[m_opponentActiveIndex];
    }
    return nullptr;
}

QVector<Creature *> BattleSystem::getPlayerTeam() const
{
    return m_playerTeam;
}

QVector<Creature *> BattleSystem::getOpponentTeam() const
{
    return m_opponentTeam;
}

bool BattleSystem::executeAction(BattleAction action, int param1, int param2)
{
    // 根据当前是谁的回合来队列相应的动作
    if (m_playerTurn) {
        queuePlayerAction(action, param1, param2);
        m_playerActionReady = true;
    } else {
        queueOpponentAction(action, param1, param2);
        m_opponentActionReady = true;
    }

    // 如果是PvE且AI还没行动，则让AI行动
    if (!m_isPvP && !m_opponentActionReady) {
        decideAIAction();
        m_opponentActionReady = true;
    }

    // 双方都准备好后，处理回合
    if (m_playerActionReady && m_opponentActionReady) {
        processTurn();
        m_playerActionReady = false;
        m_opponentActionReady = false;
        return true;
    }
    return false;
}

void BattleSystem::processTurn()
{
    // 处理回合开始效果
    processTurnStartEffects();

    // 排序行动队列
    sortActionQueue();

    // 执行行动
    executeActionQueue();

    // 处理回合结束效果
    processTurnEndEffects();

    // 检查战斗是否结束
    if (checkBattleEnd())
    {
        emit battleEnded(m_battleResult);
        addBattleLog("战斗结束！");
        return;
    }

    // 进入下一个回合
    m_currentTurn++;
    m_playerTurn = !m_playerTurn;

    // 清空行动队列，准备下一回合
    m_actionQueue.clear();

    // 发出回合结束信号
    emit turnEnded(m_currentTurn - 1);

    // 发出新回合开始信号
    emit turnStarted(m_currentTurn, m_playerTurn);
    addBattleLog(QString("第 %1 回合开始").arg(m_currentTurn));
}

bool BattleSystem::checkBattleEnd()
{
    // 检查玩家队伍是否全部失败
    bool playerLost = true;
    for (Creature *creature : m_playerTeam)
    {
        if (!creature->isDead())
        {
            playerLost = false;
            break;
        }
    }

    // 检查对手队伍是否全部失败
    bool opponentLost = true;
    for (Creature *creature : m_opponentTeam)
    {
        if (!creature->isDead())
        {
            opponentLost = false;
            break;
        }
    }

    // 设置战斗结果
    if (playerLost && opponentLost)
    {
        m_battleResult = BattleResult::DRAW;
        return true;
    }
    else if (playerLost)
    {
        m_battleResult = BattleResult::OPPONENT_WIN;
        return true;
    }
    else if (opponentLost)
    {
        m_battleResult = BattleResult::PLAYER_WIN;
        return true;
    }

    // 战斗仍在进行
    return false;
}

QVector<BattleLogEntry> BattleSystem::getBattleLog() const
{
    return m_battleLog;
}

int BattleSystem::calculateDamage(Creature *attacker, Creature *defender, Skill *skill)
{
    if (!attacker || !defender || !skill)
    {
        return 0;
    }

    // 基础伤害计算
    int damage = 0;
    double typeEffectiveness = 1.0;
    bool isCritical = false;

    // 根据技能类别选择攻击和防御数值
    int attackStat = 0;
    int defenseStat = 0;
    if (skill->getCategory() == SkillCategory::PHYSICAL)
    {
        attackStat = attacker->calculateAttack();
        defenseStat = defender->calculateDefense();
    }
    else if (skill->getCategory() == SkillCategory::SPECIAL)
    {
        attackStat = attacker->calculateSpecialAttack();
        defenseStat = defender->calculateSpecialDefense();
    }
    else
    {
        // 非攻击技能
        return 0;
    }

    // 计算基础伤害
    damage = ((2 * attacker->getLevel() / 5 + 2) * skill->getPower() * attackStat / defenseStat) / 50 + 2;

    // 计算STAB加成
    if (attacker->hasTypeAdvantage(skill->getType()))
    {
        damage *= 1.5;
    }

    // 计算属性相性
    typeEffectiveness = attacker->getTypeEffectivenessAgainst(defender, skill->getType());
    damage *= typeEffectiveness;

    // 计算暴击
    int critChance = QRandomGenerator::global()->bounded(100);
    if (critChance < 6)
    {                  // 6%的暴击率
        damage *= 1.8; // 暴击伤害为正常的1.8倍
        isCritical = true;
    }

    // 应用随机变化 (85%-100%)
    int randomFactor = QRandomGenerator::global()->bounded(85, 101);
    damage = damage * randomFactor / 100;

    return damage;
}

bool BattleSystem::checkSkillHit(Creature *attacker, Creature *defender, Skill *skill)
{
    if (!attacker || !defender || !skill)
    {
        return false;
    }

    // 必中技能
    if (skill->isAlwaysHit())
    {
        return true;
    }

    // 计算命中率
    int accuracy = skill->getAccuracy();

    // 应用攻击者的命中等级修正
    double accuracyMod = StatStages::calculateModifier(StatType::ACCURACY, attacker->getStatStages().getStage(StatType::ACCURACY));
    accuracy = static_cast<int>(accuracy * accuracyMod);

    // 应用防御者的闪避等级修正
    double evasionMod = StatStages::calculateModifier(StatType::EVASION, defender->getStatStages().getStage(StatType::EVASION));
    accuracy = static_cast<int>(accuracy / evasionMod);

    // 检查是否命中
    int hitChance = QRandomGenerator::global()->bounded(100);
    return hitChance < accuracy;
}

//处理玩家和AI的行动选择
void BattleSystem::onPlayerActionSelected(BattleAction action, int param1, int param2)
{
    if (m_playerTurn) {
        executeAction(action, param1, param2);
    }
}

void BattleSystem::onAIActionSelected()
{
    if (!m_playerTurn && !m_isPvP) {
        // AI行动已在 executeAction 中自动触发，无需再处理
    }
}
// ...existing code...



void BattleSystem::queueOpponentAction(BattleAction action, int param1, int param2)
{
    ActionQueueItem item;
    item.actor = getOpponentActiveCreature();
    item.action = action;
    item.param1 = param1;
    item.param2 = param2;

    // 设置优先级
    item.priority = 0;
    if (action == BattleAction::USE_SKILL && param1 >= 0 && param1 < item.actor->getSkillCount())
    {
        Skill *skill = item.actor->getSkill(param1);
        if (skill)
        {
            item.priority = skill->getPriority();
        }
    }

    m_actionQueue.append(item);
}

void BattleSystem::sortActionQueue()
{
    // 按优先级和速度排序行动队列
    std::sort(m_actionQueue.begin(), m_actionQueue.end(), [](const ActionQueueItem &a, const ActionQueueItem &b)
              {
        // 先按优先级排序（高优先级先行动）
        if (a.priority != b.priority) {
            return a.priority > b.priority;
        }
        
        // 优先级相同时，按速度排序
        return a.actor->calculateSpeed() > b.actor->calculateSpeed(); });
}

void BattleSystem::executeActionQueue()
{
    for (const ActionQueueItem &item : m_actionQueue)
    {
        // 检查精灵是否可以行动
        if (!item.actor || item.actor->isDead() || !item.actor->canAct())
        {
            continue;
        }

        // 执行动作
        switch (item.action)
        {
        case BattleAction::USE_SKILL:
        {
            // 使用技能
            if (item.param1 >= 0 && item.param1 < item.actor->getSkillCount())
            {
                Skill *skill = item.actor->getSkill(item.param1);
                Creature *target = nullptr;

                // 确定技能目标
                if (item.actor == getPlayerActiveCreature())
                {
                    target = getOpponentActiveCreature();
                }
                else
                {
                    target = getPlayerActiveCreature();
                }

                if (skill && target)
                {
                    // 尝试使用技能
                    bool success = item.actor->useSkill(item.param1, target, this);

                    if (success)
                    {
                        addBattleLog(QString("%1 使用了 %2").arg(item.actor->getName()).arg(skill->getName()), item.actor, target);

                        // 检查命中
                        bool hit = checkSkillHit(item.actor, target, skill);
                        if (hit)
                        {
                            // 计算伤害
                            if (skill->getCategory() == SkillCategory::PHYSICAL ||
                                skill->getCategory() == SkillCategory::SPECIAL)
                            {
                                int damage = calculateDamage(item.actor, target, skill);
                                target->takeDamage(damage);
                                emit damageCaused(target, damage);
                                addBattleLog(QString("造成了 %1 点伤害").arg(damage), item.actor, target);
                            }
                        }
                        else
                        {
                            addBattleLog("但是没有命中", item.actor, target);
                        }
                    }
                }
            }
            break;
        }
        case BattleAction::SWITCH_CREATURE:
        {
            // 切换精灵
            if (item.actor == getPlayerActiveCreature())
            {
                if (item.param1 >= 0 && item.param1 < m_playerTeam.size() && !m_playerTeam[item.param1]->isDead())
                {
                    Creature *oldCreature = getPlayerActiveCreature();
                    m_playerActiveIndex = item.param1;
                    Creature *newCreature = getPlayerActiveCreature();

                    emit creatureSwitched(oldCreature, newCreature, true);
                    addBattleLog(QString("切换到了 %1").arg(newCreature->getName()));
                }
            }
            else
            {
                if (item.param1 >= 0 && item.param1 < m_opponentTeam.size() && !m_opponentTeam[item.param1]->isDead())
                {
                    Creature *oldCreature = getOpponentActiveCreature();
                    m_opponentActiveIndex = item.param1;
                    Creature *newCreature = getOpponentActiveCreature();

                    emit creatureSwitched(oldCreature, newCreature, false);
                    addBattleLog(QString("切换到了 %1").arg(newCreature->getName()));
                }
            }
            break;
        }
        case BattleAction::USE_ITEM:
        {
            // 使用道具（待实现）
            addBattleLog("使用了道具");
            break;
        }
        case BattleAction::RESTORE_PP:
        {
            if (item.actor && item.actor->canAct()) {
                item.actor->restorePP(4); // 恢复4点PP
                addBattleLog(QString("%1 恢复了4点PP!").arg(item.actor->getName()), item.actor);
                // 恢复PP通常消耗一回合，后续的回合处理会自动进行
            }
            break;
        }
        case BattleAction::ESCAPE:
        {
            // 尝试逃跑
            if (m_isPvP)
            {
                // PvP模式下不能逃跑
                addBattleLog("无法从对战中逃跑！");
            }
            else
            {
                // 计算逃跑几率
                int escapeChance = QRandomGenerator::global()->bounded(100);
                if (escapeChance < 50)
                { // 50%的逃跑成功率
                    m_battleResult = BattleResult::ESCAPE;
                    addBattleLog("成功逃离了战斗！");
                    emit battleEnded(m_battleResult);
                }
                else
                {
                    addBattleLog("逃跑失败！");
                }
            }
            break;
        }
        }

        // 检查战斗是否结束
        if (checkBattleEnd())
        {
            break;
        }
    }
}

void BattleSystem::processTurnStartEffects()
{
    // 处理玩家精灵的回合开始效果
    if (getPlayerActiveCreature())
    {
        getPlayerActiveCreature()->onTurnStart();
    }

    // 处理对手精灵的回合开始效果
    if (getOpponentActiveCreature())
    {
        getOpponentActiveCreature()->onTurnStart();
    }
}

void BattleSystem::processTurnEndEffects()
{
    // 处理玩家精灵的回合结束效果
    if (getPlayerActiveCreature())
    {
        getPlayerActiveCreature()->onTurnEnd();
    }

    // 处理对手精灵的回合结束效果
    if (getOpponentActiveCreature())
    {
        getOpponentActiveCreature()->onTurnEnd();
    }
}

void BattleSystem::addBattleLog(const QString &message, Creature *source, Creature *target)
{
    BattleLogEntry entry;
    entry.message = message;
    entry.turn = m_currentTurn;

    if (source)
    {
        entry.sourceCreature = source->getName();
    }

    if (target)
    {
        entry.targetCreature = target->getName();
    }

    m_battleLog.append(entry);
    emit battleLogUpdated(message);
}

void BattleSystem::decideAIAction()
{
    Creature *aiCreature = getOpponentActiveCreature();
    if (!aiCreature || aiCreature->isDead() || !aiCreature->canAct())
    {
        // 尝试切换精灵
        for (int i = 0; i < m_opponentTeam.size(); ++i)
        {
            if (i != m_opponentActiveIndex && !m_opponentTeam[i]->isDead())
            {
                queueOpponentAction(BattleAction::SWITCH_CREATURE, i);
                return;
            }
        }
        return;
    }

    // 简单的AI策略：随机选择技能
    int skillCount = aiCreature->getSkillCount();
    if (skillCount > 0)
    {
        int skillIndex = QRandomGenerator::global()->bounded(skillCount);
        queueOpponentAction(BattleAction::USE_SKILL, skillIndex);
    }
}

// 触发技能使用信号

void BattleSystem::triggerHealingReceived(Creature* creature, int amount) {
    if (amount > 0) { // Optional: only emit if healing actually happened
        emit healingReceived(creature, amount);
    }
}

void BattleSystem::triggerDamageCaused(Creature* creature, int amount) {
    if (amount > 0) {
        emit damageCaused(creature, amount);
    }
}

void BattleSystem::triggerStatusChanged(Creature* target, StatusCondition oldCondition, StatusCondition newCondition) {
    // You might need to fetch the oldCondition before it's changed by the effect
    emit statusChanged(target, oldCondition, newCondition);
}

void BattleSystem::triggerStatStageChanged(Creature* target, StatType stat, int oldStage, int newStage) {
    // You might need to fetch the oldStage before it's changed by the effect
    emit statStageChanged(target, stat, oldStage, newStage);
}