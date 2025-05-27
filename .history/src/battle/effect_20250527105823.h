#include "effect.h"
#include "skill.h"
#include "../core/creature.h"
#include "../core/gameengine.h"
#include "../core/type.h"
#include <QRandomGenerator>
#include <QDebug>

// 基础效果构造函数
Effect::Effect()
    : m_targetType(TargetType::OPPONENT)
{
}

// 虚析构函数
Effect::~Effect()
{
}

// 获取目标类型
TargetType Effect::getTargetType() const
{
    return m_targetType;
}

// 设置目标类型
void Effect::setTargetType(TargetType targetType)
{
    m_targetType = targetType;
}

// 获取实际目标
Creature* Effect::getActualTarget(Creature *user, Creature *target, BattleSystem *battle) const
{
    if (!user || !battle) {
        return nullptr;
    }
    
    switch (m_targetType) {
        case TargetType::SELF:
            return user;
        case TargetType::OPPONENT:
            return target;
        case TargetType::RANDOM:
            // 随机选择一个目标
            if (QRandomGenerator::global()->bounded(2) == 0) {
                return user;
            } else {
                return target;
            }
        case TargetType::BOTH:
            // 对双方都有效果，但这里只能返回一个，由调用者处理
            return target;
        default:
            return target;
    }
}

// 应用效果
bool Effect::apply(Creature *user, Creature *target, BattleSystem *battle)
{
    // 获取实际目标
    Creature *actualTarget = getActualTarget(user, target, battle);
    if (!actualTarget) {
        return false;
    }
    
    // 基类不做任何实际效果，由子类实现
    return true;
}

// 获取效果描述
QString Effect::getDescription() const
{
    return m_description;
}

// 设置效果描述
void Effect::setDescription(const QString &description)
{
    m_description = description;
}

// 状态条件效果构造函数
StatusConditionEffect::StatusConditionEffect(StatusCondition condition, int chance)
    : m_condition(condition), m_chance(chance)
{
    // 设置默认描述
    QString conditionName;
    switch (condition) {
        case StatusCondition::POISON: conditionName = "中毒"; break;
        case StatusCondition::BURN: conditionName = "烧伤"; break;
        case StatusCondition::FREEZE: conditionName = "冻伤"; break;
        case StatusCondition::PARALYZE: conditionName = "麻痹"; break;
        case StatusCondition::SLEEP: conditionName = "睡眠"; break;
        case StatusCondition::FEAR: conditionName = "害怕"; break;
        case StatusCondition::TIRED: conditionName = "疲惫"; break;
        case StatusCondition::BLEED: conditionName = "流血"; break;
        case StatusCondition::CONFUSION: conditionName = "混乱"; break;
        default: conditionName = "未知状态"; break;
    }
    
    setDescription(QString("%1%2几率造成%3").arg(m_targetType == TargetType::SELF ? "自身" : "目标").arg(m_chance).arg(conditionName));
}

// 应用状态条件效果
bool StatusConditionEffect::apply(Creature *user, Creature *target, BattleSystem *battle)
{
    // 获取实际目标
    Creature *actualTarget = getActualTarget(user, target, battle);
    if (!actualTarget) {
        return false;
    }
    
    // 检查几率
    if (QRandomGenerator::global()->bounded(100) >= m_chance) {
        return false; // 几率未命中
    }
    
    // 设置状态条件
    actualTarget->setStatusCondition(m_condition);
    
    // 添加战斗日志
    if (battle) {
        QString conditionName;
        switch (m_condition) {
            case StatusCondition::POISON: conditionName = "中毒"; break;
            case StatusCondition::BURN: conditionName = "烧伤"; break;
            case StatusCondition::FREEZE: conditionName = "冻伤"; break;
            case StatusCondition::PARALYZE: conditionName = "麻痹"; break;
            case StatusCondition::SLEEP: conditionName = "睡眠"; break;
            case StatusCondition::FEAR: conditionName = "害怕"; break;
            case StatusCondition::TIRED: conditionName = "疲惫"; break;
            case StatusCondition::BLEED: conditionName = "流血"; break;
            case StatusCondition::CONFUSION: conditionName = "混乱"; break;
            default: conditionName = "未知状态"; break;
        }
        
        battle->addBattleLog(QString("%1陷入了%2状态！").arg(actualTarget->getName()).arg(conditionName));
    }
    
    return true;
}

// 属性变化效果构造函数
StatChangeEffect::StatChangeEffect(StatType stat, int stages, TargetType targetType)
    : m_stat(stat), m_stages(stages)
{
    setTargetType(targetType);
    
    // 设置默认描述
    QString statName;
    switch (stat) {
        case StatType::ATTACK: statName = "攻击"; break;
        case StatType::SP_ATTACK: statName = "特攻"; break;
        case StatType::DEFENSE: statName = "防御"; break;
        case StatType::SP_DEFENSE: statName = "特防"; break;
        case StatType::SPEED: statName = "速度"; break;
        case StatType::ACCURACY: statName = "命中"; break;
        case StatType::EVASION: statName = "闪避"; break;
        default: statName = "未知属性"; break;
    }
    
    setDescription(QString("%1%2%3%4级").
                  arg(m_targetType == TargetType::SELF ? "自身" : "目标").
                  arg(statName).
                  arg(m_stages > 0 ? "提升" : "降低").
                  arg(qAbs(m_stages)));
}

// 应用属性变化效果
bool StatChangeEffect::apply(Creature *user, Creature *target, BattleSystem *battle)
{
    // 获取实际目标
    Creature *actualTarget = getActualTarget(user, target, battle);
    if (!actualTarget) {
        return false;
    }
    
    // 修改属性等级
    bool success = actualTarget->modifyStatStage(m_stat, m_stages);
    
    // 添加战斗日志
    if (success && battle) {
        QString statName;
        switch (m_stat) {
            case StatType::ATTACK: statName = "攻击"; break;
            case StatType::SP_ATTACK: statName = "特攻"; break;
            case StatType::DEFENSE: statName = "防御"; break;
            case StatType::SP_DEFENSE: statName = "特防"; break;
            case StatType::SPEED: statName = "速度"; break;
            case StatType::ACCURACY: statName = "命中"; break;
            case StatType::EVASION: statName = "闪避"; break;
            default: statName = "未知属性"; break;
        }
        
        battle->addBattleLog(QString("%1的%2%3了%4级！").
                           arg(actualTarget->getName()).
                           arg(statName).
                           arg(m_stages > 0 ? "提升" : "降低").
                           arg(qAbs(m_stages)));
    }
    
    return success;
}

// 回合效果构造函数
TurnBasedEffect::TurnBasedEffect(int duration, std::function<void(Creature*, Creature*, BattleSystem*, TurnBasedEffect*)> effectLogic, bool onTurnStart)
    : m_duration(duration), m_effectLogic(effectLogic), m_onTurnStart(onTurnStart), m_originalSource(nullptr)
{
}

// 设置效果描述
void TurnBasedEffect::setDescription(const QString &description)
{
    m_description = description;
}

// 获取效果描述
QString TurnBasedEffect::getDescription() const
{
    return m_description;
}

// 设置原始来源
void TurnBasedEffect::setOriginalSource(Creature *source)
{
    m_originalSource = source;
}

// 获取原始来源
Creature *TurnBasedEffect::getOriginalSource() const
{
    return m_originalSource;
}

// 是否在回合开始时触发
bool TurnBasedEffect::isOnTurnStart() const
{
    return m_onTurnStart;
}

// 执行回合逻辑
void TurnBasedEffect::executeTurnLogic(Creature *affected, Creature *source, BattleSystem *battle)
{
    if (m_effectLogic) {
        m_effectLogic(affected, source, battle, this);
    }
}

// 减少持续时间并检查是否结束
bool TurnBasedEffect::decrementDuration()
{
    m_duration--;
    return m_duration <= 0;
}

// 获取剩余持续时间
int TurnBasedEffect::getRemainingDuration() const
{
    return m_duration;
}

// 清除效果构造函数
ClearEffectsEffect::ClearEffectsEffect(bool clearStatChanges, bool clearTurnEffects, bool clearStatusCondition, bool clearAll, TargetType targetType)
    : m_clearStatChanges(clearStatChanges), m_clearTurnEffects(clearTurnEffects), 
      m_clearStatusCondition(clearStatusCondition), m_clearAll(clearAll)
{
    setTargetType(targetType);
    
    // 设置默认描述
    QStringList effects;
    if (m_clearAll) {
        effects << "所有效果";
    } else {
        if (m_clearStatChanges) effects << "能力变化";
        if (m_clearTurnEffects) effects << "回合效果";
        if (m_clearStatusCondition) effects << "状态异常";
    }
    
    setDescription(QString("%1清除%2").
                  arg(m_targetType == TargetType::SELF ? "自身" : "目标").
                  arg(effects.join("、")));
}

// 应用清除效果
bool ClearEffectsEffect::apply(Creature *user, Creature *target, BattleSystem *battle)
{
    // 获取实际目标
    Creature *actualTarget = getActualTarget(user, target, battle);
    if (!actualTarget) {
        return false;
    }
    
    bool anyEffectCleared = false;
    
    // 清除所有效果
    if (m_clearAll) {
        actualTarget->resetStatStages();
        actualTarget->clearTurnEffects();
        actualTarget->clearStatusCondition();
        anyEffectCleared = true;
    } else {
        // 清除特定效果
        if (m_clearStatChanges) {
            actualTarget->resetStatStages();
            anyEffectCleared = true;
        }
        
        if (m_clearTurnEffects) {
            actualTarget->clearTurnEffects();
            anyEffectCleared = true;
        }
        
        if (m_clearStatusCondition) {
            actualTarget->clearStatusCondition();
            anyEffectCleared = true;
        }
    }
    
    // 添加战斗日志
    if (anyEffectCleared && battle) {
        battle->addBattleLog(QString("%1的效果被清除了！").arg(actualTarget->getName()));
    }
    
    return anyEffectCleared;
}

// 免疫效果构造函数
ImmunityEffect::ImmunityEffect(ElementType type, int duration)
    : m_type(type), m_duration(duration)
{
    // 设置默认描述
    setDescription(QString("免疫%1属性攻击，持续%2回合").
                  arg(Type::getElementTypeName(m_type)).
                  arg(m_duration));
    
    // 创建回合效果
    auto immunityLambda = [this](Creature *affected, Creature * /* source_unused */, BattleSystem * /* battle_unused */, TurnBasedEffect *self) {
        // 标记精灵为免疫特定属性
        // 实际免疫逻辑需要在BattleSystem中处理
        if (affected) {
            // 可以在这里添加视觉效果或其他提示
        }
    };
    
    m_turnEffect = new TurnBasedEffect(m_duration, immunityLambda, true); // 回合开始时生效
    m_turnEffect->setDescription(QString("免疫%1属性").arg(Type::getElementTypeName(m_type)));
}

// 析构函数
ImmunityEffect::~ImmunityEffect()
{
    // 不要在这里删除m_turnEffect，它会被添加到精灵的效果列表中
    // 并由精灵负责清理
}

// 应用免疫效果
bool ImmunityEffect::apply(Creature *user, Creature *target, BattleSystem *battle)
{
    // 获取实际目标
    Creature *actualTarget = getActualTarget(user, target, battle);
    if (!actualTarget) {
        return false;
    }
    
    // 设置效果的原始来源
    m_turnEffect->setOriginalSource(user);
    
    // 添加回合效果
    actualTarget->addTurnEffect(m_turnEffect);
    
    // 添加战斗日志
    if (battle) {
        battle->addBattleLog(QString("%1获得了对%2属性的免疫，持续%3回合！").
                           arg(actualTarget->getName()).
                           arg(Type::getElementTypeName(m_type)).
                           arg(m_duration));
    }
    
    return true;
}

// 获取免疫的属性类型
ElementType ImmunityEffect::getImmuneType() const
{
    return m_type;
}

// 获取持续时间
int ImmunityEffect::getDuration() const
{
    return m_duration;
}

// 获取回合效果
TurnBasedEffect *ImmunityEffect::getTurnEffect() const
{
    return m_turnEffect;
}
