#include "effect.h"
#include "../core/creature.h"
#include "battlesystem.h"
#include <QRandomGenerator>

// Effect基类实现
Effect::Effect(EffectType type, int chance)
    : m_type(type), m_chance(chance) {
}

Effect::~Effect() {
}

EffectType Effect::getType() const {
    return m_type;
}

int Effect::getChance() const {
    return m_chance;
}

void Effect::setChance(int chance) {
    m_chance = qBound(0, chance, 100);
}

bool Effect::checkChance() const {
    int random = QRandomGenerator::global()->bounded(100);
    return random < m_chance;
}

// TurnBasedEffect实现
TurnBasedEffect::TurnBasedEffect(int duration, int chance)
    : Effect(EffectType::TURN_BASED, chance), m_duration(duration) {
}

void TurnBasedEffect::setStartTurnEffect(std::function<void(Creature*, Creature*, BattleSystem*)> effect) {
    m_startTurnEffect = effect;
}

void TurnBasedEffect::setEndTurnEffect(std::function<void(Creature*, Creature*, BattleSystem*)> effect) {
    m_endTurnEffect = effect;
}

int TurnBasedEffect::getDuration() const {
    return m_duration;
}

void TurnBasedEffect::decreaseTurn() {
    if (m_duration > 0) {
        m_duration--;
    }
}

bool TurnBasedEffect::apply(Creature* source, Creature* target, BattleSystem* battle) {
    if (!source || !target || !checkChance()) {
        return false;
    }
    
    // 添加到目标的回合效果列表中
    target->addTurnEffect(this);
    return true;
}

void TurnBasedEffect::onTurnStart(Creature* source, Creature* target, BattleSystem* battle) {
    if (m_startTurnEffect) {
        m_startTurnEffect(source, target, battle);
    }
}

void TurnBasedEffect::onTurnEnd(Creature* source, Creature* target, BattleSystem* battle) {
    if (m_endTurnEffect) {
        m_endTurnEffect(source, target, battle);
    }
    
    // 减少回合计数
    decreaseTurn();
}

QString TurnBasedEffect::getDescription() const {
    return QString("持续 %1 回合").arg(m_duration);
}

// StatusConditionEffect实现
StatusConditionEffect::StatusConditionEffect(StatusCondition condition, int chance)
    : Effect(EffectType::STATUS_CONDITION, chance), m_condition(condition) {
}

StatusCondition StatusConditionEffect::getCondition() const {
    return m_condition;
}

bool StatusConditionEffect::apply(Creature* source, Creature* target, BattleSystem* battle) {
    if (!target || !checkChance()) {
        return false;
    }
    
    // 设置异常状态
    target->setStatusCondition(m_condition);
    return true;
}

QString StatusConditionEffect::getDescription() const {
    QString conditionText;
    
    switch (m_condition) {
        case StatusCondition::POISON:
            conditionText = "中毒";
            break;
        case StatusCondition::BURN:
            conditionText = "烧伤";
            break;
        case StatusCondition::FREEZE:
            conditionText = "冻伤";
            break;
        case StatusCondition::PARALYZE:
            conditionText = "麻痹";
            break;
        case StatusCondition::SLEEP:
            conditionText = "睡眠";
            break;
        case StatusCondition::FEAR:
            conditionText = "害怕";
            break;
        case StatusCondition::TIRED:
            conditionText = "疲惫";
            break;
        case StatusCondition::BLEED:
            conditionText = "流血";
            break;
        case StatusCondition::CONFUSION:
            conditionText = "混乱";
            break;
        case StatusCondition::NONE:
        default:
            conditionText = "正常";
            break;
    }
    
    return QString("%1% 几率造成%2").arg(m_chance).arg(conditionText);
}

// StatChangeEffect实现
StatChangeEffect::StatChangeEffect(StatType stat, int stages, bool targetSelf, int chance)
    : Effect(EffectType::STAT_CHANGE, chance), m_stat(stat), m_stages(stages), m_targetSelf(targetSelf) {
}

StatType StatChangeEffect::getStat() const {
    return m_stat;
}

int StatChangeEffect::getStages() const {
    return m_stages;
}

bool StatChangeEffect::isTargetSelf() const {
    return m_targetSelf;
}

bool StatChangeEffect::apply(Creature* source, Creature* target, BattleSystem* battle) {
    if (!source || !target || !checkChance()) {
        return false;
    }
    
    // 根据目标应用能力变化
    if (m_targetSelf) {
        source->modifyStatStage(m_stat, m_stages);
    } else {
        target->modifyStatStage(m_stat, m_stages);
    }
    
    return true;
}

QString StatChangeEffect::getDescription() const {
    QString statText;
    
    switch (m_stat) {
        case StatType::ATTACK:
            statText = "物攻";
            break;
        case StatType::SP_ATTACK:
            statText = "特攻";
            break;
        case StatType::DEFENSE:
            statText = "物防";
            break;
        case StatType::SP_DEFENSE:
            statText = "特防";
            break;
        case StatType::SPEED:
            statText = "速度";
            break;
        case StatType::ACCURACY:
            statText = "命中";
            break;
        case StatType::EVASION:
            statText = "闪避";
            break;
        default:
            statText = "未知";
            break;
    }
    
    QString changeText;
    if (m_stages > 0) {
        changeText = QString("提升%1等级+%2").arg(statText).arg(m_stages);
    } else {
        changeText = QString("降低%1等级%2").arg(statText).arg(m_stages);
    }
    
    if (m_chance < 100) {
        return QString("%1% 几率%2").arg(m_chance).arg(changeText);
    } else {
        return changeText;
    }
}

// ClearEffect实现
ClearEffect::ClearEffect(bool clearStatusConditions, bool clearStatChanges, bool clearTurnEffects, int chance)
    : Effect(EffectType::CLEAR_EFFECTS, chance),
      m_clearStatusConditions(clearStatusConditions),
      m_clearStatChanges(clearStatChanges),
      m_clearTurnEffects(clearTurnEffects) {
}

bool ClearEffect::apply(Creature* source, Creature* target, BattleSystem* battle) {
    if (!target || !checkChance()) {
        return false;
    }
    
    // 清除指定的效果
    if (m_clearStatusConditions) {
        target->clearStatusCondition();
    }
    
    if (m_clearStatChanges) {
        target->resetStatStages();
    }
    
    if (m_clearTurnEffects) {
        target->clearAllTurnEffects();
    }
    
    return true;
}

QString ClearEffect::getDescription() const {
    QStringList effects;
    
    if (m_clearStatusConditions) {
        effects.append("异常状态");
    }
    
    if (m_clearStatChanges) {
        effects.append("能力变化");
    }
    
    if (m_clearTurnEffects) {
        effects.append("回合效果");
    }
    
    QString effectsText = effects.join("、");
    
    if (m_chance < 100) {
        return QString("%1% 几率清除%2").arg(m_chance).arg(effectsText);
    } else {
        return QString("清除%1").arg(effectsText);
    }
}

// ImmunityEffect实现
ImmunityEffect::ImmunityEffect(int duration, bool immuneToStatus, bool immuneToDamage, 
                             bool immuneToStatChanges, int chance)
    : Effect(EffectType::IMMUNITY, chance),
      m_duration(duration),
      m_immuneToStatus(immuneToStatus),
      m_immuneToDamage(immuneToDamage),
      m_immuneToStatChanges(immuneToStatChanges) {
}

bool ImmunityEffect::apply(Creature* source, Creature* target, BattleSystem* battle) {
    // 创建一个回合效果，提供免疫能力
    if (!source || !target || !checkChance()) {
        return false;
    }
    
    // 创建免疫回合效果
    TurnBasedEffect* immunityEffect = new TurnBasedEffect(m_duration);
    
    // 设置回合开始效果
    immunityEffect->setStartTurnEffect([this](Creature* src, Creature* tgt, BattleSystem* btl) {
        // 回合开始时处理免疫逻辑
    });
    
    // 添加到目标的回合效果列表中
    target->addTurnEffect(immunityEffect);
    
    return true;
}

QString ImmunityEffect::getDescription() const {
    QStringList immunities;
    
    if (m_immuneToStatus) {
        immunities.append("异常状态");
    }
    
    if (m_immuneToDamage) {
        immunities.append("伤害");
    }
    
    if (m_immuneToStatChanges) {
        immunities.append("能力降低");
    }
    
    QString immunitiesText = immunities.join("、");
    
    if (m_chance < 100) {
        return QString("%1% 几率获得%2回合对%3的免疫").arg(m_chance).arg(m_duration).arg(immunitiesText);
    } else {
        return QString("获得%1回合对%2的免疫").arg(m_duration).arg(immunitiesText);
    }
}

// HealingEffect实现
HealingEffect::HealingEffect(int healPercent, int chance)
    : Effect(EffectType::HEALING, chance), m_healPercent(healPercent) {
}

int HealingEffect::getHealPercent() const {
    return m_healPercent;
}

bool HealingEffect::apply(Creature* source, Creature* target, BattleSystem* battle) {
    if (!source || !checkChance()) {
        return false;
    }
    
    // 计算治疗量
    int maxHP = source->getMaxHP();
    int healAmount = maxHP * m_healPercent / 100;
    
    // 治疗
    source->heal(healAmount);
    
    return true;
}

QString HealingEffect::getDescription() const {
    if (m_chance < 100) {
        return QString("%1% 几率回复最大体力的%2%").arg(m_chance).arg(m_healPercent);
    } else {
        return QString("回复最大体力的%1%").arg(m_healPercent);
    }
}

// FixedDamageEffect实现
FixedDamageEffect::FixedDamageEffect(int damage, int chance)
    : Effect(EffectType::FIXED_DAMAGE, chance), m_damage(damage) {
}

int FixedDamageEffect::getDamage() const {
    return m_damage;
}

bool FixedDamageEffect::apply(Creature* source, Creature* target, BattleSystem* battle) {
    if (!target || !checkChance()) {
        return false;
    }
    
    // 造成固定伤害
    target->takeDamage(m_damage);
    
    return true;
}

QString FixedDamageEffect::getDescription() const {
    if (m_chance < 100) {
        return QString("%1% 几率造成%2点固定伤害").arg(m_chance).arg(m_damage);
    } else {
        return QString("造成%1点固定伤害").arg(m_damage);
    }
}

// CriticalHitEffect实现
CriticalHitEffect::CriticalHitEffect(int criticalBoost, int chance)
    : Effect(EffectType::CRITICAL_HIT, chance), m_criticalBoost(criticalBoost) {
}

int CriticalHitEffect::getCriticalBoost() const {
    return m_criticalBoost;
}

bool CriticalHitEffect::apply(Creature* source, Creature* target, BattleSystem* battle) {
    // 此效果通常会在伤害计算中自动应用
    return checkChance();
}

QString CriticalHitEffect::getDescription() const {
    if (m_criticalBoost > 1) {
        if (m_chance < 100) {
            return QString("%1% 几率造成致命一击，暴击率提高%2级").arg(m_chance).arg(m_criticalBoost);
        } else {
            return QString("造成致命一击，暴击率提高%1级").arg(m_criticalBoost);
        }
    } else {
        if (m_chance < 100) {
            return QString("%1% 几率造成致命一击").arg(m_chance);
        } else {
            return "致命一击";
        }
    }
}

// FailureCompensationEffect实现
FailureCompensationEffect::FailureCompensationEffect(int chance)
    : Effect(EffectType::FAILURE_COMPENSATION, chance), m_compensationEffect(nullptr) {
}

void FailureCompensationEffect::setCompensationEffect(Effect* effect) {
    m_compensationEffect = effect;
}

bool FailureCompensationEffect::apply(Creature* source, Creature* target, BattleSystem* battle) {
    // 如果有补偿效果设置，使用它
    if (m_compensationEffect && source && target && checkChance()) {
        return m_compensationEffect->apply(source, target, battle);
    }
    
    return false;
}

QString FailureCompensationEffect::getDescription() const {
    if (m_compensationEffect) {
        return QString("技能效果未触发时: %1").arg(m_compensationEffect->getDescription());
    } else {
        return "技能效果未触发时有补偿";
    }
}
