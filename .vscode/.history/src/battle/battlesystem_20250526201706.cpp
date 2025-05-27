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

// 回合执行阶段
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

bool BattleSystem::checkBattleEnd() {
    // 如果战斗结果已经不是“进行中”，则直接返回true表示战斗已结束
    if (m_battleResult != BattleResult::ONGOING) return true;

    // 检查玩家队伍是否全部濒死
    bool playerAllFainted = true;
    for (Creature *creature : m_playerTeam) {
        if (creature && !creature->isDead()) {
            playerAllFainted = false;
            break;
        }
    }

    // 检查对手队伍是否全部濒死
    bool opponentAllFainted = true;
    for (Creature *creature : m_opponentTeam) {
        if (creature && !creature->isDead()) {
            opponentAllFainted = false;
            break;
        }
    }
    
    BattleResult newResultState = BattleResult::ONGOING;
    if (playerAllFainted && opponentAllFainted) {
        newResultState = BattleResult::DRAW;
    } else if (playerAllFainted) {
        newResultState = BattleResult::OPPONENT_WIN;
    } else if (opponentAllFainted) {
        newResultState = BattleResult::PLAYER_WIN;
    }

    // 如果战斗结果从“进行中”变为其他状态，则更新 m_battleResult
    if (newResultState != BattleResult::ONGOING) {
        m_battleResult = newResultState;
        return true; // 表示战斗刚刚结束或已结束
    }
    return false; // 战斗仍在进行
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

/*void BattleSystem::sortActionQueue()
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
}*/

void BattleSystem::executeActionQueue() {
    for (const ActionQueueItem &item : m_actionQueue) {
        if (m_battleResult != BattleResult::ONGOING) break; // 如果战斗中途结束，停止处理后续行动

        Creature* actor = item.actor;
        // 检查行动者状态
        if (!actor || actor->isDead()) {
            addBattleLog(QString("%1 无法行动 (已濒死)!").arg(actor ? actor->getName() : "一个精灵"));
            continue;
        }
        if (!actor->canAct()) {
            addBattleLog(QString("%1 因状态无法行动!").arg(actor->getName()));
            // 精灵的 onTurnStart() 或状态效果逻辑可能已经打印了更具体的信息
            continue;
        }

        Creature* playerActive = getPlayerActiveCreature();
        Creature* opponentActive = getOpponentActiveCreature();
        bool isActorPlayer = (actor == playerActive); // 判断行动者是玩家还是对手

        // 根据行动类型处理
        switch (item.action) {
            case BattleAction::USE_SKILL: {
                Skill *skillToUse = nullptr;
                // 获取技能，-1 代表第五技能
                if (item.param1 == -1) skillToUse = actor->getFifthSkill(); 
                else if (item.param1 >= 0 && item.param1 < actor->getSkillCount()) skillToUse = actor->getSkill(item.param1);

                if (!skillToUse) {
                    addBattleLog(QString("%1 试图使用未知技能!").arg(actor->getName()));
                    continue; // 跳过此无效行动
                }
                // 检查PP是否足够
                if (actor->getCurrentPP() < skillToUse->getPPCost()){
                    addBattleLog(QString("%1 的 %2 因PP不足使用失败!").arg(actor->getName()).arg(skillToUse->getName()));
                    continue;
                }
                
                // 确定技能目标
                Creature *target = isActorPlayer ? opponentActive : playerActive;
                // 特殊情况：如果技能是状态类，且其效果目标是自身
                if(skillToUse->getCategory() == SkillCategory::STATUS && 
                   !skillToUse->getEffects().isEmpty() && 
                   skillToUse->getEffects().first()->isTargetSelf()){ // 假设Effect有isTargetSelf()
                    target = actor; 
                }

                // 再次检查目标有效性（例如，如果对方在我方选择技能后恰好切换或濒死）
                if (!target || (target->isDead() && skillToUse->getCategory() != SkillCategory::STATUS)) { 
                    addBattleLog(QString("%1 的技能 %2 没有有效目标!").arg(actor->getName()).arg(skillToUse->getName()));
                    continue;
                }
                
                addBattleLog(QString("%1 使用了 %2!").arg(actor->getName()).arg(skillToUse->getName()));
                actor->consumePP(skillToUse->getPPCost()); // 消耗PP

                // Skill::use 现在主要负责命中判定和应用非伤害效果
                if (skillToUse->use(actor, target, this)) { 
                    // 对于物理或特殊攻击类技能，伤害计算和应用由BattleSystem处理
                    if (skillToUse->getCategory() == SkillCategory::PHYSICAL || skillToUse->getCategory() == SkillCategory::SPECIAL) {
                        if (checkSkillHit(actor, target, skillToUse)) { // BattleSystem进行最终命中判定
                            int damage = calculateDamage(actor, target, skillToUse); // BattleSystem计算伤害
                            target->takeDamage(damage); // 应用伤害

                            // 添加更详细的战斗日志
                            QString stabBonusText = actor->hasTypeAdvantage(skillToUse->getType()) ? "(属性一致加成) " : "";
                            double effectiveness = actor->getTypeEffectivenessAgainst(target, skillToUse->getType());
                            QString effectText = "";
                            if (effectiveness > 1.5) effectText = " 效果拔群!"; // 假设1.5倍算拔群
                            else if (effectiveness == 1.5) effectText = " 效果绝佳!";
                            else if (effectiveness < 1.0 && effectiveness > 0.0) effectText = " 效果不理想.";
                            else if (effectiveness == 0.0) effectText = " 没有效果.";
                            
                            addBattleLog(QString("%1的%2对%3造成了 %4%5点伤害!%6").arg(actor->getName()).arg(skillToUse->getName()).arg(target->getName()).arg(stabBonusText).arg(damage).arg(effectText));
                            emit damageCaused(target, damage); // 发出伤害信号

                            // 检查目标是否因此次伤害而濒死
                            if (target->isDead()) {
                                addBattleLog(QString("%1 倒下了!").arg(target->getName()));
                                if (checkBattleEnd()) { emit battleEnded(m_battleResult); return; } // 战斗结束则返回
                                // TODO: 如果目标是当前活跃精灵且濒死，应提示切换（如果是AI，则AI决策切换）
                            }
                        } else {
                            addBattleLog(QString("%1 的 %2 未能命中 %3!").arg(actor->getName()).arg(skillToUse->getName()).arg(target->getName()));
                        }
                    }
                    // 状态技能或攻击技能附带的非伤害效果已在 Skill::use -> Effect::apply 中处理
                    // 检查目标是否因技能效果（如中毒、诅咒等间接伤害）而濒死
                    if (target && target->isDead() && (skillToUse->getCategory() == SkillCategory::STATUS || skillToUse->getCategory() == SkillCategory::PHYSICAL || skillToUse->getCategory() == SkillCategory::SPECIAL) ) {
                         // 此检查可能在伤害判定后有些多余，但保留以防万一
                         if (checkBattleEnd()) { emit battleEnded(m_battleResult); return; }
                    }
                } else {
                    // Skill::use 返回 false 通常意味着未命中或不满足使用条件（已在内部log）
                    // addBattleLog(QString("%1的%2使用失败或未命中!").arg(actor->getName()).arg(skillToUse->getName())); // 此log可能重复
                }
                break;
            }
            case BattleAction::SWITCH_CREATURE: { // 处理精灵切换逻辑
                int switchToIndex = item.param1;
                Creature* oldCreature = actor; // 记录旧精灵
                if (isActorPlayer) { // 如果是玩家切换
                    if (switchToIndex >= 0 && switchToIndex < m_playerTeam.size() && 
                        m_playerTeam[switchToIndex] && !m_playerTeam[switchToIndex]->isDead() && 
                        m_playerActiveIndex != switchToIndex) {
                        m_playerActiveIndex = switchToIndex;
                        addBattleLog(QString("你换上了 %1!").arg(getPlayerActiveCreature()->getName()));
                        emit creatureSwitched(oldCreature, getPlayerActiveCreature(), true);
                        getPlayerActiveCreature()->resetStatStages(); // 新上场精灵重置能力等级
                    } else {
                        addBattleLog("切换精灵失败 (选择无效或精灵已濒死).");
                    }
                } else { // 如果是对手切换
                    if (switchToIndex >= 0 && switchToIndex < m_opponentTeam.size() && 
                        m_opponentTeam[switchToIndex] && !m_opponentTeam[switchToIndex]->isDead() && 
                        m_opponentActiveIndex != switchToIndex) {
                        m_opponentActiveIndex = switchToIndex;
                        addBattleLog(QString("对手换上了 %1!").arg(getOpponentActiveCreature()->getName()));
                        emit creatureSwitched(oldCreature, getOpponentActiveCreature(), false);
                        getOpponentActiveCreature()->resetStatStages(); // 新上场精灵重置能力等级
                    } else {
                         addBattleLog("对手切换精灵失败.");
                    }
                }
                break;
            }
            case BattleAction::USE_ITEM: { // 处理使用道具逻辑（当前为占位）
                addBattleLog(QString("%1 使用了道具 (功能待实现).").arg(actor->getName()));
                break;
            }
            case BattleAction::RESTORE_PP: { // 处理恢复PP逻辑
                if (actor && actor->canAct() && actor->getCurrentPP() < actor->getMaxPP()) {
                    actor->restorePP(4); // 实际恢复PP
                    addBattleLog(QString("<font color='blue'>%1 恢复了4点PP!</font>").arg(actor->getName()), actor);
                    // UI更新将通过 battleLogUpdated 或其他特定信号（如PP变化信号，如果添加的话）触发
                    // BattleScene的onBattleLogUpdated会调用updatePlayerUI和updateSkillButtons
                } else if (actor) {
                    addBattleLog(QString("%1 试图恢复PP但失败了(PP已满或无法行动)!").arg(actor->getName()), actor);
                }
                break;
            }
            case BattleAction::ESCAPE: { // 处理逃跑逻辑
                if (m_isPvP) {
                    addBattleLog("PvP战斗中无法逃跑!");
                } else {
                    // 逃跑成功率提高到75%（示例）
                    if (QRandomGenerator::global()->bounded(100) < 75) { 
                        m_battleResult = BattleResult::ESCAPE;
                        addBattleLog("成功逃脱!");
                        emit battleEnded(m_battleResult); // 逃跑成功，战斗立即结束
                        return; // 停止处理后续行动
                    } else {
                        addBattleLog("逃跑失败!");
                    }
                }
                break;
            }
        }
        // 每次行动执行完毕后，都检查战斗是否结束
        if (checkBattleEnd() && m_battleResult != BattleResult::ONGOING) { // checkBattleEnd 会设置 m_battleResult
             emit battleEnded(m_battleResult); // 如果战斗结束，发出信号
             return; // 战斗结束，不再处理队列中剩余的行动
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