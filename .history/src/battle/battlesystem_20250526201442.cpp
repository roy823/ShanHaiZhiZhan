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

bool BattleSystem::isPvPBattle() const { return m_isPvP; }

bool BattleSystem::isPvPBattle() const
{
    return m_isPvP;
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
// 玩家提交行动
void BattleSystem::playerSubmittedAction(BattleAction action, int param1, int param2) {
    if (m_playerActionSubmittedThisTurn || m_battleResult != BattleResult::ONGOING) {
        return; // 玩家已行动或战斗已结束
    }

    Creature* playerCreature = getPlayerActiveCreature();
    if (!playerCreature || playerCreature->isDead() || !playerCreature->canAct()) {
         if(playerCreature) addBattleLog(QString("%1 无法行动!").arg(playerCreature->getName()));
        // 如果玩家精灵无法行动，其行动在效果上是“跳过”
        // UI层面应该阻止提交行动，但这里做一层保护
        m_playerActionSubmittedThisTurn = true; // 标记为已提交，以便回合能继续（如果AI行动）
        emit playerActionConfirmed(); // 通知UI
        if (!m_isPvP) {
            // AI仍然可以行动
            QTimer::singleShot(100, this, &BattleSystem::decideAIAction); 
        } else {
            tryProcessTurnActions(); // PvP模式下检查对方是否已行动
        }
        return;
    }

    queuePlayerAction(action, param1, param2); // 将玩家行动加入队列
    m_playerActionSubmittedThisTurn = true;    // 标记玩家已提交行动
    if (playerCreature) {
        addBattleLog(QString("%1 选择了行动.").arg(playerCreature->getName()));
    }
    emit playerActionConfirmed(); // 发出信号，UI可以据此更新（例如禁用按钮）

    if (!m_isPvP) { // 如果是PVE模式
        // AI在玩家提交行动后进行决策 (可以加一点延迟模拟思考)
        QTimer::singleShot(500, this, &BattleSystem::decideAIAction); 
    } else {
        // 如果是PVP模式，等待另一个玩家的行动 (通过类似 opponentSubmittedAction 的方法)
        tryProcessTurnActions(); // 检查是否双方都已行动
    }
}

// AI决定并提交行动
void BattleSystem::decideAIAction() {
    if (m_opponentActionSubmittedThisTurn || m_battleResult != BattleResult::ONGOING) {
        return; // AI已行动或战斗已结束
    }

    Creature *aiCreature = getOpponentActiveCreature();
    bool actionTakenByAI = false; // 标记AI是否成功选择了一个行动

    if (!aiCreature || aiCreature->isDead()) { // 如果AI当前精灵濒死
        // 尝试切换精灵
        for (int i = 0; i < m_opponentTeam.size(); ++i) {
            if (m_opponentTeam[i] && !m_opponentTeam[i]->isDead()) {
                queueOpponentAction(BattleAction::SWITCH_CREATURE, i); // param1 是队伍中的索引
                addBattleLog(QString("对手换上了 %1.").arg(m_opponentTeam[i]->getName()));
                actionTakenByAI = true;
                break;
            }
        }
        if (!actionTakenByAI) {
            addBattleLog("对手没有其他可战斗的精灵了!");
        }
    } else if (!aiCreature->canAct()) { // 如果AI精灵因状态无法行动
        addBattleLog(QString("%1 因特殊状态无法行动!").arg(aiCreature->getName()));
        actionTakenByAI = true; // 视为“跳过”行动
    } else { // AI精灵可以行动，选择技能
        QVector<int> usableSkillIndices;
        // 检查普通技能
        if (aiCreature->getSkillCount() > 0) {
            for (int i = 0; i < aiCreature->getSkillCount(); ++i) {
                Skill* s = aiCreature->getSkill(i);
                if (s && aiCreature->getCurrentPP() >= s->getPPCost()) {
                    usableSkillIndices.append(i);
                }
            }
        }
        // 检查第五技能
        Skill* fifthSkill = aiCreature->getFifthSkill();
        if (fifthSkill && aiCreature->getCurrentPP() >= fifthSkill->getPPCost()) {
            usableSkillIndices.append(-1); // 用-1代表第五技能
        }

        if (!usableSkillIndices.isEmpty()) { // 如果有可用的技能
            int choice = QRandomGenerator::global()->bounded(usableSkillIndices.size());
            int skillIndexToUse = usableSkillIndices[choice];
            queueOpponentAction(BattleAction::USE_SKILL, skillIndexToUse);
            
            Skill* chosenSkill = (skillIndexToUse == -1) ? fifthSkill : aiCreature->getSkill(skillIndexToUse);
            if(chosenSkill) addBattleLog(QString("对手准备使用 %1.").arg(chosenSkill->getName()));
            actionTakenByAI = true;
        } else { // 没有可用的技能 (例如PP耗尽)
            // AI可以尝试恢复PP或使用“挣扎”（如果实现了）
            if (aiCreature->getCurrentPP() < aiCreature->getMaxPP()) {
                queueOpponentAction(BattleAction::RESTORE_PP);
                addBattleLog(QString("对手试图恢复PP."));
                actionTakenByAI = true;
            } else {
                 addBattleLog(QString("%1 无计可施!").arg(aiCreature->getName()));
                 actionTakenByAI = true; // 视为“跳过”
            }
        }
    }

    m_opponentActionSubmittedThisTurn = true; // 标记AI已提交行动
    emit opponentActionConfirmed();           // 发出信号 (UI可选地响应)
    tryProcessTurnActions();                  // 检查是否双方都已行动，以开始结算
}

// 尝试处理回合行动（当一方行动后调用，检查是否双方都已行动）
void BattleSystem::tryProcessTurnActions() {
    if (m_playerActionSubmittedThisTurn && m_opponentActionSubmittedThisTurn && m_battleResult == BattleResult::ONGOING) {
        // 双方都已提交行动，且战斗仍在进行，则进入执行阶段
        processTurnExecutePhase();
    }
}

// 新的回合输入阶段设置
void BattleSystem::processTurnInputPhase() {
    // 如果战斗非进行中状态，并且不是第一回合（m_currentTurn > 0），则不应发出 turnEnded
    if (m_battleResult != BattleResult::ONGOING && m_currentTurn > 0) { 
        return;
    }
    // 只有在非第一回合时，才触发上一回合的 turnEnded
    if (m_currentTurn > 0) { 
         emit turnEnded(m_currentTurn);
    }

    m_currentTurn++; // 回合数增加
    m_playerActionSubmittedThisTurn = false;   // 重置玩家行动提交状态
    m_opponentActionSubmittedThisTurn = false; // 重置对手行动提交状态
    m_actionQueue.clear();                     // 清空上一回合的行动队列

    // 发出新回合开始信号，true 表示现在是玩家的输入时机
    emit turnStarted(m_currentTurn, true); 
    addBattleLog(QString("--- 第 %1 回合 ---").arg(m_currentTurn));
}

// 新的回合执行阶段（替换旧的 processTurn）
void BattleSystem::processTurnExecutePhase() {
    if (m_battleResult != BattleResult::ONGOING) return; // 如果战斗已结束，则不处理

    addBattleLog("行动处理阶段...");

    processTurnStartEffects(); // 处理回合开始时的效果
    if (checkBattleEnd()) { // 检查回合开始效果是否导致战斗结束
        emit battleEnded(m_battleResult);
        addBattleLog(QString("战斗因回合开始效果结束. 结果代码: %1").arg((int)m_battleResult));
        return;
    }

    sortActionQueue();      // 根据优先级和速度排序行动队列
    executeActionQueue();   // 执行队列中的行动（此方法内部会检查战斗是否结束）

    // executeActionQueue 内部的 checkBattleEnd 会设置 m_battleResult
    // 如果战斗在行动执行期间结束，下面的代码不应再次发出 battleEnded
    if (m_battleResult != BattleResult::ONGOING) {
        // battleEnded 信号应该已在 executeActionQueue 或其调用的 checkBattleEnd 中发出
        return;
    }

    processTurnEndEffects(); // 处理回合结束时的效果
    if (checkBattleEnd()) { // 检查回合结束效果是否导致战斗结束
        emit battleEnded(m_battleResult);
        addBattleLog(QString("战斗因回合结束效果结束. 结果代码: %1").arg((int)m_battleResult));
        return;
    }

    // 如果战斗仍然在进行中，则准备下一回合的输入
    if (m_battleResult == BattleResult::ONGOING) {
        processTurnInputPhase();
    }
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