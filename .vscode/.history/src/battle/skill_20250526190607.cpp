// src/battle/skill.cpp
#include "skill.h"
#include "../core/creature.h"     // 核心 - 精灵类
#include "battlesystem.h"         // 战斗 - 战斗系统类
#include "../battle/effect.h"     // 战斗 - 效果类 (如果技能直接创建效果)
#include <QRandomGenerator>       // Qt随机数生成器

// --- Skill基类实现 ---

// 构造函数
Skill::Skill(const QString &name, ElementType type, SkillCategory category,
             int power, int ppCost, int accuracy, int priority)
    : m_name(name),                  // 初始化技能名称
      m_type(type),                  // 初始化技能属性
      m_category(category),          // 初始化技能类别
      m_power(power),                // 初始化技能威力
      m_ppCost(ppCost),              // 初始化PP消耗 (注意：这里是消耗量，不是最大PP)
      m_accuracy(accuracy),          // 初始化技能命中率
      m_priority(priority)           // 初始化技能优先级
{
    
}

// 析构函数
Skill::~Skill()
{
    // 释放技能所持有的所有效果对象
    // 使用 qDeleteAll 可以方便地删除容器中的所有指针元素
    qDeleteAll(m_effects);
    m_effects.clear(); // 清空列表
}

// 获取技能名称
QString Skill::getName() const
{
    return m_name;
}

// 获取技能属性
ElementType Skill::getType() const
{
    return m_type;
}

// 获取技能类别
SkillCategory Skill::getCategory() const
{
    return m_category;
}

// 获取技能威力
int Skill::getPower() const
{
    // TODO: 某些技能的威力可能会根据特定条件动态变化 (例如：HP越低威力越高)
    // 可以在派生类中重写此方法以实现动态威力
    return m_power;
}

// 获取技能PP消耗
int Skill::getPPCost() const
{
    return m_ppCost;
}

// 获取技能命中率
int Skill::getAccuracy() const
{
    return m_accuracy;
}

// 获取技能优先级
int Skill::getPriority() const
{
    return m_priority;
}

// 使用技能 (通用逻辑)
bool Skill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    // 检查使用者和战斗系统是否存在
    if (!user || !battle)
    {
        // battle->addBattleLog("技能使用失败：参数无效。"); // 假设有战斗日志系统
        return false;
    }

    // 检查使用者是否有足够的PP (从全局PP池扣除)
    if (user->getCurrentPP() < m_ppCost)
    {
        // battle->addBattleLog(QString("%1 PP不足，无法使用 %2!").arg(user->getName()).arg(m_name));
        return false; // PP不足，无法使用技能
    }

    // 消耗PP
    user->consumePP(m_ppCost);
    // battle->addBattleLog(QString("%1 消耗了 %2点 PP。").arg(user->getName()).arg(m_ppCost));


    // 检查技能是否命中 (状态技能可能不需要目标，或者目标是自身)
    if (m_category == SkillCategory::STATUS && !target) { // 对于一些以自身为目标的状态技能
        // battle->addBattleLog(QString("%1 使用了 %2。").arg(user->getName()).arg(m_name));
        // 应用效果 (通常作用于自身)
        for (Effect *effect : m_effects)
        {
            if (effect) effect->apply(user, user, battle); // 状态技能效果可能作用于使用者
        }
        return true; // 状态技能命中通常为必中或有独立判定
    }

    // 对于需要目标或有命中判定的技能
    if (!target) { // 如果是攻击或指向性状态技能但无目标
         // battle->addBattleLog(QString("%1 的技能 %2 没有目标!").arg(user->getName()).arg(m_name));
        return false;
    }

    if (checkHit(user, target, battle))
    {
        // battle->addBattleLog(QString("%1 的 %2 命中了 %3!").arg(user->getName()).arg(m_name).arg(target->getName()));

        // 应用所有附加效果 (例如: 降低防御、施加中毒等)
        // 伤害计算和应用将在BattleSystem中处理，这里只应用非伤害性效果
        for (Effect *effect : m_effects)
        {
           if (effect) effect->apply(user, target, battle);
        }
        // 对于攻击型技能，实际伤害计算和应用由BattleSystem在执行队列时处理
        // 这里返回true表示技能“成功发动”（命中并通过了PP检查）
        return true;
    }
    else
    {
        // battle->addBattleLog(QString("%1 的 %2 未能命中 %3!").arg(user->getName()).arg(m_name).arg(target->getName()));
        return false; // 未命中
    }
}

// 获取技能描述
QString Skill::getDescription() const
{
    QString description = m_name + "\n";
    description += "系别: " + Type::getElementTypeName(m_type) + "\n"; // 使用静态方法获取系别名称

    switch (m_category)
    {
    case SkillCategory::PHYSICAL:
        description += "类别: 物理\n";
        break;
    case SkillCategory::SPECIAL:
        description += "类别: 特殊\n";
        break;
    case SkillCategory::STATUS:
        description += "类别: 属性\n";
        break;
    }

    if (m_category != SkillCategory::STATUS) { // 状态技能威力通常无意义
        description += "威力: " + QString::number(m_power) + "\n";
    }

    description += "PP消耗: " + QString::number(m_ppCost) + "\n"; // 显示PP消耗

    if (m_accuracy >= 101) // 特殊处理高精度作为必中 (例如宝可梦中设定 > 100 为必中)
    {
        description += "命中: 必中\n";
    }
    else if (m_accuracy == 0) { // 0命中率通常用于非攻击技能或有特殊规则的技能
        description += "命中: --\n";
    }
    else
    {
        description += "命中: " + QString::number(m_accuracy) + "%\n";
    }

    if (m_priority != 0)
    {
        description += "优先级: " + QString::number(m_priority) + "\n";
    }

    // 添加效果描述 (如果有)
    if (!m_effects.isEmpty())
    {
        description += "效果:\n";
        for (const Effect *effect : m_effects)
        {
            if (effect) description += "- " + effect->getDescription() + "\n";
        }
    }
    return description;
}

// 判断是否必中
bool Skill::isAlwaysHit() const
{
    return m_accuracy >= 101; // 假设命中率101或更高表示必中 (一些游戏用 >100 或特定标志)
}

// 添加效果到技能
void Skill::addEffect(Effect *effect)
{
    if (effect)
    {
        m_effects.append(effect);
    }
}

// 获取技能的所有效果
const QVector<Effect *> &Skill::getEffects() const
{
    return m_effects;
}

// 计算伤害 (基类实现，主要返回威力，实际复杂计算在BattleSystem)
int Skill::calculateDamage(Creature *user, Creature *target)
{
    // 对于非攻击技能，威力通常为0
    if (m_category == SkillCategory::STATUS) {
        return 0;
    }
    // 攻击技能返回其基础威力，详细计算由BattleSystem完成
    // 特殊技能（如固定伤害）会重写此方法
    return m_power;
}

// 检查命中 (基础版本，BattleSystem会加入更多修正)
bool Skill::checkHit(Creature *user, Creature *target, BattleSystem *battle)
{
    if (isAlwaysHit()) // 如果技能设定为必中
    {
        return true;
    }
    if (!user || !target || !battle) return false; // 基本参数检查

    // BattleSystem中会进行更复杂的命中计算，这里是一个非常基础的判定
    // battle->checkSkillHit(user, target, this) 应该在BattleSystem中被调用
    // 这里仅作一个占位或最基础的判定
    if (m_accuracy == 0) return true; // 命中为0的技能通常是状态类或必中，这里暂定为必中（需根据游戏设计调整）

    int chance = QRandomGenerator::global()->bounded(1, 101); // 生成1到100的随机数
    return chance <= m_accuracy;
}


// --- PhysicalSkill 实现 ---
PhysicalSkill::PhysicalSkill(const QString &name, ElementType type, int power,
                             int ppCost, int accuracy, int priority)
    : Skill(name, type, SkillCategory::PHYSICAL, power, ppCost, accuracy, priority)
{
    // 物理技能的特定初始化（如果需要）
}

// PhysicalSkill::calculateDamage现在依赖BattleSystem，基类实现已足够
// int PhysicalSkill::calculateDamage(Creature *user, Creature *target)
// {
//     // 伤害计算会非常复杂，通常放在BattleSystem中处理
//     // 这里仅返回技能威力作为基础，或一个简化的计算
//     // 实际伤害 = ( ( (等级 * 2 / 5 + 2) * 威力 * 攻击 / 防御 ) / 50 + 2 ) * 属性相性 * STAB * 随机数 等
//     return Skill::calculateDamage(user, target); // 调用基类实现，或进行特定物理伤害计算
// }


// --- SpecialSkill 实现 ---
SpecialSkill::SpecialSkill(const QString &name, ElementType type, int power,
                           int ppCost, int accuracy, int priority)
    : Skill(name, type, SkillCategory::SPECIAL, power, ppCost, accuracy, priority)
{
    // 特殊技能的特定初始化
}

// SpecialSkill::calculateDamage现在依赖BattleSystem
// int SpecialSkill::calculateDamage(Creature *user, Creature *target)
// {
//     return Skill::calculateDamage(user, target); // 调用基类实现，或进行特定特殊伤害计算
// }


// --- StatusSkill 实现 ---
StatusSkill::StatusSkill(const QString &name, ElementType type,
                         int ppCost, int accuracy, int priority)
    : Skill(name, type, SkillCategory::STATUS, 0, ppCost, accuracy, priority) // 状态技能威力通常为0
{
    // 状态技能的特定初始化
}

bool StatusSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    // 状态技能不造成直接伤害，主要应用效果
    if (!user || !battle) return false;

    if (user->getCurrentPP() < m_ppCost) return false;
    user->consumePP(m_ppCost);
    // battle->addBattleLog(QString("%1 消耗了 %2点 PP。").arg(user->getName()).arg(m_ppCost));


    // 状态技能通常对自己或对方使用，目标可能需要判断
    Creature* actualTarget = target; // 默认目标是传入的target
    // 许多状态技能可能以自身为目标，或需要根据效果判断目标
    // 例如: 硬化 -> target=user; 催眠粉 -> target=opponent

    if (checkHit(user, actualTarget, battle)) // 状态技能也需要命中判定
    {
        // battle->addBattleLog(QString("%1 使用了 %2!").arg(user->getName()).arg(m_name));
        for (Effect *effect : m_effects)
        {
            if (effect) {
                 // 需要确定效果是作用于自身还是对方
                 // 简单的做法是在Effect中指明目标，或StatChangeEffect那种有targetSelf标志
                 // 假设Effect::apply内部处理目标选择或有targetSelf机制
                effect->apply(user, actualTarget, battle);
            }
        }
        return true;
    }
    else
    {
        // battle->addBattleLog(QString("%1 的 %2 未能命中!").arg(user->getName()).arg(m_name));
        return false;
    }
}


// --- CompositeSkill 实现 ---
CompositeSkill::CompositeSkill(const QString &name, ElementType type, SkillCategory category,
                               int power, int ppCost, int accuracy, int priority)
    : Skill(name, type, category, power, ppCost, accuracy, priority),
      m_effectChance(100) // 默认附加效果100%触发，除非被setEffectChance修改
{
}

void CompositeSkill::setEffectChance(int chance)
{
    m_effectChance = qBound(0, chance, 100); // 保证几率在0-100之间
}

int CompositeSkill::getEffectChance() const
{
    return m_effectChance;
}

bool CompositeSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    // 首先执行基础技能的使用逻辑 (PP消耗、命中判断等)
    // 基类的use方法已经不再直接应用伤害，仅处理命中和PP
    bool hitSuccess = Skill::use(user, target, battle); // Skill::use现在不应用效果，只检查PP和命中

    if (hitSuccess) // 如果技能命中 (PP消耗已在Skill::use中处理)
    {
        // battle->addBattleLog(QString("%1 的 %2 命中了 %3!").arg(user->getName()).arg(m_name).arg(target->getName()));
        // 伤害计算和应用由BattleSystem处理

        // 检查是否触发附加效果
        if (QRandomGenerator::global()->bounded(100) < m_effectChance)
        {
            // battle->addBattleLog(QString("%1 的 %2 触发了附加效果!").arg(user->getName()).arg(m_name));
            for (Effect *effect : m_effects) // 应用所有定义的附加效果
            {
                if (effect) effect->apply(user, target, battle);
            }
        }
        return true; // 表示技能成功发动并命中
    }
    // battle->addBattleLog(QString("%1 的 %2 未能命中 %3!").arg(user->getName()).arg(m_name).arg(target->getName()));
    return false; // 技能未命中
}


// --- MultiHitSkill 实现 ---
MultiHitSkill::MultiHitSkill(const QString &name, ElementType type, SkillCategory category,
                             int power, int ppCost, int accuracy,
                             int minHits, int maxHits, int priority)
    : Skill(name, type, category, power, ppCost, accuracy, priority),
      m_minHits(qMax(1, minHits)),       // 确保最小攻击次数至少为1
      m_maxHits(qMax(m_minHits, maxHits)) // 确保最大攻击次数不小于最小次数
{
}

bool MultiHitSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    if (!user || !target || !battle) return false;

    if (user->getCurrentPP() < m_ppCost) {
        // battle->addBattleLog(QString("%1 PP不足!").arg(user->getName()));
        return false;
    }
    user->consumePP(m_ppCost); // 消耗一次PP
    // battle->addBattleLog(QString("%1 使用了 %2! (消耗 %3 PP)").arg(user->getName()).arg(m_name).arg(m_ppCost));


    int numberOfHits = QRandomGenerator::global()->bounded(m_minHits, m_maxHits + 1); // 随机决定攻击次数
    bool hitAtLeastOnce = false;
    int totalDamageDealt = 0; // 用于记录总伤害，方便日志

    // battle->addBattleLog(QString("%1 将攻击 %2 次!").arg(m_name).arg(numberOfHits));

    for (int i = 0; i < numberOfHits; ++i)
    {
        if (target->isDead()) break; // 如果目标在中途被打败，停止攻击

        if (checkHit(user, target, battle)) // 每次攻击独立计算命中
        {
            hitAtLeastOnce = true;
            // battle->addBattleLog(QString("第 %1 段攻击命中了!").arg(i + 1));

            // 伤害计算和应用由 BattleSystem 处理，这里仅示意
            // BattleSystem会调用 skill->calculateDamage() 然后 target->takeDamage()
            // BattleSystem::processTurn 中应该有逻辑处理多段攻击

            // 应用附加效果 (如果有多段攻击也触发效果的设计)
            // 这里的效果是技能本身附带的，可能每次命中都判定一次
            // 或者设计为只有一次判定，或最后一次命中判定
            // 当前逻辑：如果技能有效果，且每次命中都可能触发
            if (QRandomGenerator::global()->bounded(100) < m_effectChance) { // 假设CompositeMultiHitSkill有m_effectChance
                 for (Effect *effect : m_effects) {
                    if (effect) effect->apply(user, target, battle);
                }
            }
        }
        else
        {
            // battle->addBattleLog(QString("第 %1 段攻击未命中。").arg(i + 1));
            // 根据游戏规则，一次未命中是否终止后续攻击 (通常是继续完所有判定)
        }
    }

    // BattleSystem 会在处理这个技能时，循环调用 calculateDamage 和 takeDamage
    // 此处的返回值表示技能是否至少命中了一次（即PP消耗了且有行动）
    return hitAtLeastOnce;
}


// --- FixedDamageSkill 实现 ---
FixedDamageSkill::FixedDamageSkill(const QString &name, ElementType type, SkillCategory category,
                                   int fixedDamageAmount, int ppCost, int accuracy, int priority)
    : Skill(name, type, category, 0, ppCost, accuracy, priority), // 威力设为0，因伤害固定
      m_fixedDamage(fixedDamageAmount)
{
}

// 重写calculateDamage以返回固定伤害值
int FixedDamageSkill::calculateDamage(Creature *user, Creature *target)
{
    // 固定伤害技能忽略攻击方、防御方能力以及属性克制等大部分因素
    // 但可能仍受某些特殊特性或场地效果影响，这里简化处理
    if (!user || !target) return 0; // 基本检查

    // TODO: 检查目标是否有免疫固定伤害的特性或效果

    return m_fixedDamage; // 直接返回设定的固定伤害值
}


// --- HealingSkill 实现 ---
HealingSkill::HealingSkill(const QString &name, ElementType type,
                           int ppCost, int accuracy, int healAmount, bool isPercentage, int priority)
    : StatusSkill(name, type, ppCost, accuracy, priority), // 继承自StatusSkill
      m_healAmount(healAmount),
      m_isPercentage(isPercentage)
{
}

bool HealingSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    // 治疗技能通常作用于使用者自身，但也可以设计为治疗友方目标
    // 此处假设 target 参数是实际的治疗对象，如果target为nullptr，则治疗user

    Creature* healTarget = target ? target : user; // 确定治疗目标

    if (!user || !healTarget || !battle) return false;

    if (user->getCurrentPP() < m_ppCost) {
        // battle->addBattleLog(QString("%1 PP不足!").arg(user->getName()));
        return false;
    }
    user->consumePP(m_ppCost);
    // battle->addBattleLog(QString("%1 使用了 %2! (消耗 %3 PP)").arg(user->getName()).arg(m_name).arg(m_ppCost));


    // 治疗技能通常必中，或命中率很高，除非有特殊规则
    if (!checkHit(user, healTarget, battle)) { // 即使是治疗，也可能有命中判定 (例如对混乱的队友使用)
        // battle->addBattleLog(QString("%1 的 %2 未能成功施放!").arg(user->getName()).arg(m_name));
        return false;
    }

    int actualHealAmount = 0;
    if (m_isPercentage) // 如果是百分比治疗
    {
        actualHealAmount = healTarget->getMaxHP() * m_healAmount / 100;
    }
    else // 如果是固定值治疗
    {
        actualHealAmount = m_healAmount;
    }

    if (actualHealAmount > 0) {
        healTarget->heal(actualHealAmount);
        // battle->addBattleLog(QString("%1 恢复了 %2点 HP!").arg(healTarget->getName()).arg(actualHealAmount));
        if(battle) battle->emitHealingReceived(healTarget, actualHealAmount); // 发出治疗信号
    }

    // 应用其他附加效果 (如果有)
    for (Effect *effect : m_effects)
    {
        if (effect) effect->apply(user, healTarget, battle);
    }

    return true;
}


// --- StatChangeSkill 实现 ---
StatChangeSkill::StatChangeSkill(const QString &name, ElementType type,
                                 int ppCost, int accuracy, int priority)
    : StatusSkill(name, type, ppCost, accuracy, priority) // 继承自StatusSkill
{
}

void StatChangeSkill::addStatChange(StatType stat, int stages, bool targetSelf)
{
    m_statChanges.append({stat, stages, targetSelf}); // 添加能力变化到列表中
}

bool StatChangeSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    if (!user || !battle) return false; // target 可以为nullptr如果所有效果都对自己

    if (user->getCurrentPP() < m_ppCost) {
        // battle->addBattleLog(QString("%1 PP不足!").arg(user->getName()));
        return false;
    }
    user->consumePP(m_ppCost);
    // battle->addBattleLog(QString("%1 使用了 %2! (消耗 %3 PP)").arg(user->getName()).arg(m_name).arg(m_ppCost));


    // 能力变化技能通常需要命中判定，除非是对自身使用且必中
    // 假设目标是传入的target，如果效果是针对自身，则内部会处理
    bool overallHit = true; // 假设对自身的效果总是命中
    if (!m_statChanges.isEmpty() && !m_statChanges.first().targetSelf && target) { // 如果有对敌效果，则检查命中
        overallHit = checkHit(user, target, battle);
    }


    if (overallHit)
    {
        // battle->addBattleLog(QString("%1 的 %2 成功施放!").arg(user->getName()).arg(m_name));
        for (const auto &change : m_statChanges)
        {
            Creature *actualTarget = change.targetSelf ? user : target;
            if (actualTarget) // 确保目标有效
            {
                actualTarget->modifyStatStage(change.stat, change.stages);
                // battle->addBattleLog(QString("%1 的 %2 %3了 %4 等级。")
                //                      .arg(actualTarget->getName())
                //                      .arg(StatStages::getStatName(change.stat)) // 假设有此方法
                //                      .arg(change.stages > 0 ? "提升" : "降低")
                //                      .arg(std::abs(change.stages)));
                 if(battle) battle->emitStatStageChanged(actualTarget, change.stat, actualTarget->getStatStages().getStage(change.stat) - change.stages, actualTarget->getStatStages().getStage(change.stat)); // 发出信号
            }
        }

        // 应用其他非能力改变的附加效果 (如果有)
        for (Effect *effect : m_effects)
        {
            if (effect) {
                // 确定效果目标
                Creature* effectTarget = target; // 默认是传入的目标
                // TODO: Effect类本身可能需要一个 TargetType (SELF, OPPONENT, ALLY 等) 来决定作用对象
                effect->apply(user, effectTarget, battle);
            }
        }
        return true;
    }
    else
    {
        // battle->addBattleLog(QString("%1 的 %2 未能命中!").arg(user->getName()).arg(m_name));
        return false;
    }
}


// --- FifthSkill 实现 ---
FifthSkill::FifthSkill(const QString &name, ElementType type, SkillCategory category,
                       int power, int ppCost, int accuracy, int priority)
    : Skill(name, type, category, power, ppCost, accuracy, priority)
{
    // 第五技能的特定初始化
}

// 检查第五技能是否可以使用
bool FifthSkill::canUse(Creature *user, Creature *target, BattleSystem* battle) const
{
    if(!user) return false;
    // 基础检查：PP是否足够 (由Creature::useSkill或BattleSystem在选择时检查)
    // if (user->getCurrentPP() < m_ppCost) return false;

    // 默认第五技能总能使用，派生类可以重写此方法添加特定条件
    // 例如："不屈战魂" 需要HP低于1/2
    if (m_name == "不屈战魂") { // 示例：根据技能名称判断
        return user->getCurrentHP() < (user->getMaxHP() / 2);
    }
    // 其他第五技能的特定条件可以在这里添加或在各自的派生类中重写

    return true; // 默认无特殊条件
}