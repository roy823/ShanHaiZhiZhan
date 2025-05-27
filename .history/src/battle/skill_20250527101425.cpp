#include "skill.h"
#include "../core/creature.h"
#include "battlesystem.h"
#include "../battle/effect.h"
#include <QRandomGenerator>

// --- Skill基类实现 ---

Skill::Skill(const QString &name, ElementType type, SkillCategory category,
             int power, int ppCost, int accuracy, int priority)
    : m_name(name),
      m_type(type),
      m_category(category),
      m_power(power),
      m_ppCost(ppCost),
      m_accuracy(accuracy),
      m_priority(priority),
      m_targetType(category == SkillCategory::STATUS ? TargetType::SELF : TargetType::OPPONENT) // 默认目标类型
{
    
}

Skill::~Skill()
{
    qDeleteAll(m_effects);
    m_effects.clear();
}

QString Skill::getName() const { return m_name; }
QString Skill::getDescription() const { return m_description; }
void Skill::setDescription(const QString &description) { m_description = description; }
ElementType Skill::getType() const { return m_type; }
SkillCategory Skill::getCategory() const { return m_category; }
int Skill::getPower() const { return m_power; }
int Skill::getPPCost() const { return m_ppCost; }
int Skill::getAccuracy() const { return m_accuracy; }
int Skill::getPriority() const { return m_priority; }

TargetType Skill::getTargetType() const
{
    return m_targetType;
}

void Skill::setTargetType(TargetType targetType)
{
    m_targetType = targetType;
}

Creature* Skill::getActualTarget(Creature *user, Creature *target, BattleSystem *battle) const
{
    if (!user || !battle) return nullptr;
    
    switch (m_targetType) {
        case TargetType::SELF:
            return user;
        case TargetType::OPPONENT:
            return target ? target : battle->getOpponentActiveCreature() == user ? 
                                     battle->getPlayerActiveCreature() : 
                                     battle->getOpponentActiveCreature();
        case TargetType::ALL_OPPONENTS:
            // 简化实现，目前只返回当前对手
            return battle->getOpponentActiveCreature() == user ? 
                   battle->getPlayerActiveCreature() : 
                   battle->getOpponentActiveCreature();
        case TargetType::ALL_CREATURES:
            // 简化实现，需要在具体技能中处理
            return target;
        default:
            return target;
    }
}

bool Skill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    if (!user || !battle) return false;

    // 获取实际目标
    Creature *actualTarget = getActualTarget(user, target, battle);
    
    // 对于需要目标的技能，确保目标存在
    if (m_targetType != TargetType::SELF && m_targetType != TargetType::NONE && !actualTarget) {
        return false;
    }

    // 检查技能是否命中
    if (checkHit(user, actualTarget, battle))
    {
        // 应用所有附加效果
        for (Effect *effect : m_effects)
        {
            if (effect) {
                // 使用效果自己的目标判定逻辑
                effect->apply(user, actualTarget, battle);
            }
        }
        return true;
    }
    else
    {
        return false; // 未命中
    }
}

QString Skill::getDetailedDescription() const
{
    QString description = m_name + "\n";
    description += "系别: " + Type::getElementTypeName(m_type) + "\n";

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

    if (m_category != SkillCategory::STATUS) {
        description += "威力: " + QString::number(m_power) + "\n";
    }

    description += "PP消耗: " + QString::number(m_ppCost) + "\n";

    if (m_accuracy >= 101)
    {
        description += "命中: 必中\n";
    }
    else if (m_accuracy == 0) {
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

    // 添加目标类型描述
    QString targetDesc;
    switch (m_targetType) {
        case TargetType::SELF: targetDesc = "自身"; break;
        case TargetType::OPPONENT: targetDesc = "对手"; break;
        case TargetType::ALLY: targetDesc = "友方"; break;
        case TargetType::ALL_OPPONENTS: targetDesc = "所有对手"; break;
        case TargetType::ALL_ALLIES: targetDesc = "所有友方"; break;
        case TargetType::ALL_CREATURES: targetDesc = "场上所有精灵"; break;
        default: targetDesc = "无"; break;
    }
    description += "目标: " + targetDesc + "\n";

    // 添加效果描述
    if (!m_effects.isEmpty())
    {
        description += "效果:\n";
        for (const Effect *effect : m_effects)
        {
            if (effect) description += "- " + effect->getDescription() + "\n";
        }
    }
    
    // 如果有自定义描述，也添加上
    if (!m_description.isEmpty()) {
        description += "\n" + m_description;
    }
    
    return description;
}

bool Skill::isAlwaysHit() const
{
    return m_accuracy >= 101;
}

void Skill::addEffect(Effect *effect)
{
    if (effect)
    {
        m_effects.append(effect);
    }
}

const QVector<Effect *> &Skill::getEffects() const
{
    return m_effects;
}

int Skill::calculateDamage(Creature *user, Creature *target)
{
    if (m_category == SkillCategory::STATUS) {
        return 0;
    }
    return m_power;
}

bool Skill::checkHit(Creature *user, Creature *target, BattleSystem *battle)
{
    if (isAlwaysHit())
    {
        return true;
    }
    if (!user || !target || !battle) return false;

    if (m_accuracy == 0) return true;

    int chance = QRandomGenerator::global()->bounded(1, 101);
    return chance <= m_accuracy;
}

// --- PhysicalSkill 实现 ---
PhysicalSkill::PhysicalSkill(const QString &name, ElementType type, int power,
                             int ppCost, int accuracy, int priority)
    : Skill(name, type, SkillCategory::PHYSICAL, power, ppCost, accuracy, priority)
{
    setTargetType(TargetType::OPPONENT);
}

// --- SpecialSkill 实现 ---
SpecialSkill::SpecialSkill(const QString &name, ElementType type, int power,
                           int ppCost, int accuracy, int priority)
    : Skill(name, type, SkillCategory::SPECIAL, power, ppCost, accuracy, priority)
{
    setTargetType(TargetType::OPPONENT);
}

// --- StatusSkill 实现 ---
StatusSkill::StatusSkill(const QString &name, ElementType type,
                         int ppCost, int accuracy, int priority)
    : Skill(name, type, SkillCategory::STATUS, 0, ppCost, accuracy, priority)
{
    setTargetType(TargetType::SELF); // 默认状态技能作用于自身
}

bool StatusSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    if (!user || !battle) return false;

    // 获取实际目标
    Creature *actualTarget = getActualTarget(user, target, battle);
    
    // 对于需要目标的技能，确保目标存在
    if (m_targetType != TargetType::SELF && m_targetType != TargetType::NONE && !actualTarget) {
        return false;
    }

    if (checkHit(user, actualTarget, battle))
    {
        for (Effect *effect : m_effects)
        {
            if (effect) {
                // 使用效果自己的目标判定逻辑
                effect->apply(user, actualTarget, battle);
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

// --- CompositeSkill 实现 ---
CompositeSkill::CompositeSkill(const QString &name, ElementType type, SkillCategory category,
                               int power, int ppCost, int accuracy, int priority)
    : Skill(name, type, category, power, ppCost, accuracy, priority),
      m_effectChance(100)
{
    setTargetType(TargetType::OPPONENT);
}

void CompositeSkill::setEffectChance(int chance)
{
    m_effectChance = qBound(0, chance, 100);
}

int CompositeSkill::getEffectChance() const
{
    return m_effectChance;
}

bool CompositeSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    if (!user || !battle) return false;

    // 获取实际目标
    Creature *actualTarget = getActualTarget(user, target, battle);
    
    // 对于需要目标的技能，确保目标存在
    if (m_targetType != TargetType::SELF && m_targetType != TargetType::NONE && !actualTarget) {
        return false;
    }

    // 检查技能是否命中
    if (checkHit(user, actualTarget, battle))
    {
        // 检查是否触发附加效果
        if (QRandomGenerator::global()->bounded(100) < m_effectChance)
        {
            for (Effect *effect : m_effects)
            {
                if (effect) {
                    // 使用效果自己的目标判定逻辑
                    effect->apply(user, actualTarget, battle);
                }
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

// --- MultiHitSkill 实现 ---
MultiHitSkill::MultiHitSkill(const QString &name, ElementType type, SkillCategory category,
                             int power, int ppCost, int accuracy,
                             int minHits, int maxHits, int priority)
    : Skill(name, type, category, power, ppCost, accuracy, priority),
      m_minHits(qMax(1, minHits)),
      m_maxHits(qMax(m_minHits, maxHits))
{
    setTargetType(TargetType::OPPONENT);
}

bool MultiHitSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    if (!user || !battle) return false;

    // 获取实际目标
    Creature *actualTarget = getActualTarget(user, target, battle);
    
    // 对于需要目标的技能，确保目标存在
    if (!actualTarget) return false;

    int numberOfHits = QRandomGenerator::global()->bounded(m_minHits, m_maxHits + 1);
    bool hitAtLeastOnce = false;

    for (int i = 0; i < numberOfHits; ++i) {
        if (checkHit(user, actualTarget, battle)) {
            hitAtLeastOnce = true;
            
            // 对于有附加效果的多段攻击，每次命中都尝试触发效果
            for (Effect *effect : m_effects) {
                if (effect && effect->checkChanceTrigger()) {
                    effect->apply(user, actualTarget, battle);
                }
            }
            
            if (i == 0) break; // 简化：只要命中一次就算成功，BattleSystem处理后续
        } else {
            if (i == 0) return false; // 第一次未命中则技能失败
            break; // 一旦失手，后续攻击也停止
        }
        
        if (actualTarget->isDead()) break; // 目标倒下则停止
    }
    
    return hitAtLeastOnce;
}

int MultiHitSkill::getMinHits() const
{
    return m_minHits;
}

int MultiHitSkill::getMaxHits() const
{
    return m_maxHits;
}

// --- FixedDamageSkill 实现 ---
FixedDamageSkill::FixedDamageSkill(const QString &name, ElementType type, SkillCategory category,
                                   int fixedDamageAmount, int ppCost, int accuracy, int priority)
    : Skill(name, type, category, 0, ppCost, accuracy, priority),
      m_fixedDamage(fixedDamageAmount)
{
    setTargetType(TargetType::OPPONENT);
}

int FixedDamageSkill::calculateDamage(Creature *user, Creature *target)
{
    if (!user || !target) return 0;
    return m_fixedDamage;
}

// --- HealingSkill 实现 ---
HealingSkill::HealingSkill(const QString &name, ElementType type,
                           int ppCost, int accuracy, int healAmount, bool isPercentage, int priority)
    : StatusSkill(name, type, ppCost, accuracy, priority),
      m_healAmount(healAmount),
      m_isPercentage(isPercentage)
{
    setTargetType(TargetType::SELF); // 默认治疗自身
}

bool HealingSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    if (!user || !battle) return false;

    // 获取实际目标
    Creature *actualTarget = getActualTarget(user, target, battle);
    if (!actualTarget) return false;

    if (checkHit(user, actualTarget, battle))
    {
        int actualHealAmount = 0;
        if (m_isPercentage)
        {
            actualHealAmount = actualTarget->getMaxHP() * m_healAmount / 100;
        }
        else
        {
            actualHealAmount = m_healAmount;
        }

        if (actualHealAmount > 0) {
            actualTarget->heal(actualHealAmount);
        }

        // 应用其他附加效果
        for (Effect *effect : m_effects)
        {
            if (effect) {
                effect->apply(user, actualTarget, battle);
            }
        }

        return true;
    }
    
    return false;
}

// --- StatChangeSkill 实现 ---
StatChangeSkill::StatChangeSkill(const QString &name, ElementType type,
                                 int ppCost, int accuracy, int priority)
    : StatusSkill(name, type, ppCost, accuracy, priority)
{
    // 默认目标类型由具体的能力变化决定
}

void StatChangeSkill::addStatChange(StatType stat, int stages, TargetType targetType)
{
    m_statChanges.append({stat, stages, targetType});
    
    // 如果是第一个能力变化，设置技能的默认目标类型
    if (m_statChanges.size() == 1) {
        setTargetType(targetType);
    }
}

bool StatChangeSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    if (!user || !battle) return false;

    // 检查命中
    bool overallHit = true;
    if (m_targetType == TargetType::OPPONENT && target) {
        overallHit = checkHit(user, target, battle);
    }

    if (overallHit)
    {
        // 应用所有能力变化
        for (const auto &change : m_statChanges)
        {
            Creature *actualTarget = nullptr;
            
            // 根据目标类型获取实际目标
            switch (change.targetType) {
                case TargetType::SELF:
                    actualTarget = user;
                    break;
                case TargetType::OPPONENT:
                    actualTarget = target ? target : 
                                   battle->getOpponentActiveCreature() == user ? 
                                   battle->getPlayerActiveCreature() : 
                                   battle->getOpponentActiveCreature();
                    break;
                default:
                    actualTarget = target;
                    break;
            }
            
            if (actualTarget)
            {
                actualTarget->modifyStatStage(change.stat, change.stages);
            }
        }

        // 应用其他效果
        for (Effect *effect : m_effects)
        {
            if (effect) {
                Creature* effectTarget = effect->getActualTarget(user, target, battle);
                if (effectTarget) {
                    effect->apply(user, effectTarget, battle);
                }
            }
        }
        
        return true;
    }
    
    return false;
}
