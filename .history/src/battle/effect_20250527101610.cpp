#include "effect.h"
#include "../core/creature.h"
#include "battlesystem.h"
#include <QRandomGenerator>

// --- Effect基类实现 ---

Effect::Effect(EffectType type, int chance)
    : m_type(type),
      m_chance(chance),
      m_targetType(TargetType::OPPONENT) // 默认目标为对手
{
}

Effect::~Effect()
{
}

EffectType Effect::getType() const
{
    return m_type;
}

int Effect::getChance() const
{
    return m_chance;
}

void Effect::setChance(int chance)
{
    m_chance = qBound(0, chance, 100);
}

TargetType Effect::getTargetType() const
{
    return m_targetType;
}

void Effect::setTargetType(TargetType targetType)
{
    m_targetType = targetType;
}

Creature* Effect::getActualTarget(Creature *source, Creature *target, BattleSystem *battle) const
{
    if (!source || !battle) return nullptr;
    
    switch (m_targetType) {
        case TargetType::SELF:
            return source;
        case TargetType::OPPONENT:
            return target ? target : battle->getOpponentActiveCreature() == source ? 
                                     battle->getPlayerActiveCreature() : 
                                     battle->getOpponentActiveCreature();
        case TargetType::ALL_OPPONENTS:
            // 简化实现，目前只返回当前对手
            return battle->getOpponentActiveCreature() == source ? 
                   battle->getPlayerActiveCreature() : 
                   battle->getOpponentActiveCreature();
        case TargetType::ALL_CREATURES:
            // 简化实现，需要在具体效果中处理
            return target;
        default:
            return target;
    }
}

bool Effect::checkChanceTrigger() const
{
    return QRandomGenerator::global()->bounded(100) < m_chance;
}

// --- TurnBasedEffect实现 ---

TurnBasedEffect::TurnBasedEffect(int duration, 
                                 std::function<void(Creature*, Creature*, BattleSystem*, TurnBasedEffect*)> effectFunc,
                                 bool onTurnStart, 
                                 int chance)
    : Effect(EffectType::TURN_BASED, chance),
      m_initialDuration(duration),
      m_currentDuration(duration),
      m_effectLogic(effectFunc),
      m_onTurnStart(onTurnStart),
      m_description(""),
      m_originalSource(nullptr)
{
}

bool TurnBasedEffect::apply(Creature *source, Creature *target, BattleSystem *battle)
{
    if (!source || !battle) return false;
    
    // 获取实际目标
    Creature *actualTarget = getActualTarget(source, target, battle);
    if (!actualTarget) return false;
    
    // 检查触发几率
    if (!checkChanceTrigger()) return false;
    
    // 记录原始源
    setOriginalSource(source);
    
    // 将效果添加到目标的回合效果列表
    actualTarget->addTurnEffect(this);
    
    return true;
}

QString TurnBasedEffect::getDescription() const
{
    return m_description;
}

int TurnBasedEffect::getDuration() const
{
    return m_currentDuration;
}

void TurnBasedEffect::setDuration(int duration)
{
    m_currentDuration = duration;
}

bool TurnBasedEffect::decrementDuration()
{
    m_currentDuration--;
    return m_currentDuration <= 0;
}

void TurnBasedEffect::executeTurnLogic(Creature *affectedCreature, Creature *sourceCreature, BattleSystem *battle)
{
    if (m_effectLogic && affectedCreature && battle) {
        m_effectLogic(affectedCreature, sourceCreature, battle, this);
    }
}

Creature* TurnBasedEffect::getOriginalSource() const
{
    return m_originalSource;
}

void TurnBasedEffect::setOriginalSource(Creature *source)
{
    m_originalSource = source;
}

Creature* TurnBasedEffect::getEffectTarget(Creature *affectedCreature, BattleSystem *battle) const
{
    if (!affectedCreature || !battle) return nullptr;
    
    switch (m_targetType) {
        case TargetType::SELF:
            return affectedCreature;
        case TargetType::OPPONENT:
            return battle->getOpponentActiveCreature() == affectedCreature ? 
                   battle->getPlayerActiveCreature() : 
                   battle->getOpponentActiveCreature();
        case TargetType::ALL_OPPONENTS:
            // 简化实现，目前只返回当前对手
            return battle->getOpponentActiveCreature() == affectedCreature ? 
                   battle->getPlayerActiveCreature() : 
                   battle->getOpponentActiveCreature();
        case TargetType::ALL_CREATURES:
            // 简化实现，需要在具体效果中处理
            return affectedCreature;
        default:
            return affectedCreature;
    }
}

void TurnBasedEffect::setDescription(const QString &desc)
{
    m_description = desc;
}

// --- StatusConditionEffect实现 ---

StatusConditionEffect::StatusConditionEffect(StatusCondition condition, int chance)
    : Effect(EffectType::STATUS_CONDITION, chance),
      m_condition(condition)
{
    // 状态效果默认作用于对手
    setTargetType(TargetType::OPPONENT);
}

StatusCondition StatusConditionEffect::getCondition() const
{
    return m_condition;
}

bool StatusConditionEffect::apply(Creature *source, Creature *target, BattleSystem *battle)
{
    if (!source || !battle) return false;
    
    // 获取实际目标
    Creature *actualTarget = getActualTarget(source, target, battle);
    if (!actualTarget) return false;
    
    // 检查触发几率
    if (!checkChanceTrigger()) return false;
    
    // 应用状态条件
    actualTarget->setStatusCondition(m_condition);
    
    return true;
}

QString StatusConditionEffect::getDescription() const
{
    QString conditionName;
    switch (m_condition) {
        case StatusCondition::POISON: conditionName = "中毒"; break;
        case StatusCondition::PARALYZE: conditionName = "麻痹"; break;
        case StatusCondition::BURNED: conditionName = "烧伤"; break;
        case StatusCondition::FROZEN: conditionName = "冰冻"; break;
        case StatusCondition::ASLEEP: conditionName = "睡眠"; break;
        case StatusCondition::CONFUSED: conditionName = "混乱"; break;
        default: conditionName = "未知状态"; break;
    }
    
    return QString("造成%1状态").arg(conditionName);
}

// --- StatChangeEffect实现 ---

StatChangeEffect::StatChangeEffect(StatType stat, int stages, TargetType targetType, int chance)
    : Effect(EffectType::STAT_CHANGE, chance),
      m_stat(stat),
      m_stages(stages)
{
    setTargetType(targetType);
}

StatType StatChangeEffect::getStat() const
{
    return m_stat;
}

int StatChangeEffect::getStages() const
{
    return m_stages;
}

bool StatChangeEffect::apply(Creature *source, Creature *target, BattleSystem *battle)
{
    if (!source || !battle) return false;
    
    // 获取实际目标
    Creature *actualTarget = getActualTarget(source, target, battle);
    if (!actualTarget) return false;
    
    // 检查触发几率
    if (!checkChanceTrigger()) return false;
    
    // 应用能力变化
    actualTarget->modifyStatStage(m_stat, m_stages);
    
    return true;
}

QString StatChangeEffect::getDescription() const
{
    QString statName;
    switch (m_stat) {
        case StatType::ATTACK: statName = "物攻"; break;
        case StatType::DEFENSE: statName = "物防"; break;
        case StatType::SP_ATTACK: statName = "特攻"; break;
        case StatType::SP_DEFENSE: statName = "特防"; break;
        case StatType::SPEED: statName = "速度"; break;
        case StatType::ACCURACY: statName = "命中"; break;
        case StatType::EVASION: statName = "闪避"; break;
        default: statName = "未知属性"; break;
    }
    
    QString targetDesc = m_targetType == TargetType::SELF ? "自身" : "对方";
    QString changeDesc = m_stages > 0 ? "提升" : "降低";
    int absStages = abs(m_stages);
    
    return QString("%1%2%3%4级").arg(targetDesc).arg(statName).arg(changeDesc).arg(absStages);
}

// --- ClearEffectsEffect实现 ---

ClearEffectsEffect::ClearEffectsEffect(bool clearPositiveStatChanges,
                                       bool clearNegativeStatChanges,
                                       bool clearStatusConditions,
                                       bool clearTurnBasedEffects,
                                       TargetType targetType,
                                       int chance)
    : Effect(EffectType::CLEAR_EFFECTS, chance),
      m_clearPositiveStatChanges(clearPositiveStatChanges),
      m_clearNegativeStatChanges(clearNegativeStatChanges),
      m_clearStatusConditions(clearStatusConditions),
      m_clearTurnBasedEffects(clearTurnBasedEffects)
{
    setTargetType(targetType);
}

bool ClearEffectsEffect::apply(Creature *source, Creature *target, BattleSystem *battle)
{
    if (!source || !battle) return false;
    
    // 获取实际目标
    Creature *actualTarget = getActualTarget(source, target, battle);
    if (!actualTarget) return false;
    
    // 检查触发几率
    if (!checkChanceTrigger()) return false;
    
    // 清除状态条件
    if (m_clearStatusConditions) {
        actualTarget->clearStatusCondition();
    }
    
    // 清除能力变化
    if (m_clearPositiveStatChanges || m_clearNegativeStatChanges) {
        StatStages stages = actualTarget->getStatStages();
        for (int i = 0; i < static_cast<int>(StatType::COUNT); i++) {
            StatType stat = static_cast<StatType>(i);
            int stage = stages.getStage(stat);
            
            if ((stage > 0 && m_clearPositiveStatChanges) || 
                (stage < 0 && m_clearNegativeStatChanges)) {
                actualTarget->modifyStatStage(stat, -stage); // 重置为0
            }
        }
    }
    
    // 清除回合效果
    if (m_clearTurnBasedEffects) {
        actualTarget->clearAllTurnEffects();
    }
    
    return true;
}

QString ClearEffectsEffect::getDescription() const
{
    QStringList effects;
    
    if (m_clearPositiveStatChanges && m_clearNegativeStatChanges) {
        effects << "所有能力变化";
    } else if (m_clearPositiveStatChanges) {
        effects << "能力提升";
    } else if (m_clearNegativeStatChanges) {
        effects << "能力降低";
    }
    
    if (m_clearStatusConditions) {
        effects << "异常状态";
    }
    
    if (m_clearTurnBasedEffects) {
        effects << "回合效果";
    }
    
    QString targetDesc = m_targetType == TargetType::SELF ? "自身" : "对方";
    
    return QString("清除%1的%2").arg(targetDesc).arg(effects.join("、"));
}

// --- ImmunityEffect实现 ---

ImmunityEffect::ImmunityEffect(int duration, bool immuneToStatus, ElementType immuneToSpecificTypeDamage, int chance)
    : Effect(EffectType::IMMUNITY, chance),
      m_duration(duration),
      m_immuneToStatus(immuneToStatus),
      m_immuneToTypeDamage(immuneToSpecificTypeDamage)
{
    // 免疫效果默认作用于自身
    setTargetType(TargetType::SELF);
}

bool ImmunityEffect::apply(Creature *source, Creature *target, BattleSystem *battle)
{
    if (!source || !battle) return false;
    
    // 获取实际目标
    Creature *actualTarget = getActualTarget(source, target, battle);
    if (!actualTarget) return false;
    
    // 检查触发几率
    if (!checkChanceTrigger()) return false;
    
    // 创建并应用回合效果
    auto immunityLambda = [this](Creature *affected, Creature *source_unused, BattleSystem *battle_unused, TurnBasedEffect *self) {
        // 免疫逻辑在BattleSystem中处理
        // 这里只是占位，实际免疫检查在伤害计算和状态应用时进行
        if (self->getDuration() <= 0) {
            affected->removeTurnEffect(self);
        }
    };
    
    TurnBasedEffect *immunityEffect = new TurnBasedEffect(m_duration, immunityLambda, true);
    immunityEffect->setTargetType(m_targetType);
    immunityEffect->setOriginalSource(source);
    
    QString desc = "免疫";
    if (m_immuneToStatus) {
        desc += "异常状态";
    }
    if (m_immuneToTypeDamage != ElementType::NONE) {
        if (m_immuneToStatus) desc += "和";
        desc += Type::getElementTypeName(m_immuneToTypeDamage) + "属性伤害";
    }
    immunityEffect->setDescription(desc);
    
    actualTarget->addTurnEffect(immunityEffect);
    
    return true;
}

QString ImmunityEffect::getDescription() const
{
    QString desc = "获得";
    if (m_immuneToStatus) {
        desc += "异常状态";
    }
    if (m_immuneToTypeDamage != ElementType::NONE) {
        if (m_immuneToStatus) desc += "和";
        desc += Type::getElementTypeName(m_immuneToTypeDamage) + "属性伤害";
    }
    desc += QString("免疫，持续%1回合").arg(m_duration);
    
    return desc;
}

// --- HealingEffect实现 ---

HealingEffect::HealingEffect(int amount, bool isPercentage, int chance)
    : Effect(EffectType::HEALING, chance),
      m_amount(amount),
      m_isPercentage(isPercentage)
{
    // 治疗效果默认作用于自身
    setTargetType(TargetType::SELF);
}

bool HealingEffect::apply(Creature *source, Creature *target, BattleSystem *battle)
{
    if (!source || !battle) return false;
    
    // 获取实际目标
    Creature *actualTarget = getActualTarget(source, target, battle);
    if (!actualTarget) return false;
    
    // 检查触发几率
    if (!checkChanceTrigger()) return false;
    
    // 计算治疗量
    int healAmount = 0;
    if (m_isPercentage) {
        healAmount = actualTarget->getMaxHP() * m_amount / 100;
    } else {
        healAmount = m_amount;
    }
    
    // 应用治疗
    actualTarget->heal(healAmount);
    
    return true;
}

QString HealingEffect::getDescription() const
{
    QString targetDesc = m_targetType == TargetType::SELF ? "自身" : "目标";
    
    if (m_isPercentage) {
        return QString("恢复%1最大HP的%2%").arg(targetDesc).arg(m_amount);
    } else {
        return QString("恢复%1%2点HP").arg(targetDesc).arg(m_amount);
    }
}

// --- FixedDamageEffect实现 ---

FixedDamageEffect::FixedDamageEffect(int damageAmount, int chance)
    : Effect(EffectType::FIXED_DAMAGE, chance),
      m_damageAmount(damageAmount)
{
    // 伤害效果默认作用于对手
    setTargetType(TargetType::OPPONENT);
}

bool FixedDamageEffect::apply(Creature *source, Creature *target, BattleSystem *battle)
{
    if (!source || !battle) return false;
    
    // 获取实际目标
    Creature *actualTarget = getActualTarget(source, target, battle);
    if (!actualTarget) return false;
    
    // 检查触发几率
    if (!checkChanceTrigger()) return false;
    
    // 应用固定伤害
    actualTarget->takeDamage(m_damageAmount);
    
    return true;
}

QString FixedDamageEffect::getDescription() const
{
    return QString("造成%1点固定伤害").arg(m_damageAmount);
}
