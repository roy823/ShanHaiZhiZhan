#ifndef EFFECT_H
#define EFFECT_H

#include <QString>
#include <functional>
#include "../core/ability.h"
#include "../core/type.h"
#include <QVariant>

// 前向声明
class Creature;
class BattleSystem;

// 效果类型枚举
enum class EffectType
{
    GENERIC,              // 通用效果
    TURN_BASED,           // 回合类效果
    STATUS_CONDITION,     // 施加异常状态效果
    STAT_CHANGE,          // 能力等级变化效果
    CLEAR_EFFECTS,        // 清除目标身上的效果
    IMMUNITY,             // 提供免疫效果
    HEALING,              // 回复HP效果
    FIXED_DAMAGE,         // 造成固定数值伤害效果
    CRITICAL_HIT_MOD,     // 影响暴击率或暴击伤害的效果
    FAILURE_COMPENSATION, // 当主要技能效果未触发时的补偿效果
    FIELD_EFFECT,         // 场地效果
    ENTRY_HAZARD          // 入场障碍效果
};

// 效果基类
class Effect
{
public:
    // 构造函数
    Effect(EffectType type, int chance = 100);

    // 虚析构函数
    virtual ~Effect();

    // 获取效果类型
    EffectType getType() const;

    // 获取效果触发的基础几率
    int getChance() const;

    // 设置效果触发的基础几率
    void setChance(int chance);

    // 目标类型相关方法
    TargetType getTargetType() const;
    void setTargetType(TargetType targetType);
    
    // 获取实际目标（统一接口）
    Creature* getActualTarget(Creature *source, Creature *target, BattleSystem *battle) const;

    // 应用效果的核心虚函数
    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) = 0;

    // 获取效果的文本描述
    virtual QString getDescription() const = 0;

    // 检查几率是否触发（公开接口）
    bool checkChanceTrigger() const;
    
protected:
    EffectType m_type;      // 效果类型
    int m_chance;           // 效果触发的基础几率
    TargetType m_targetType; // 效果目标类型
};

// 回合类效果
class TurnBasedEffect : public Effect
{
public:
    TurnBasedEffect(int duration,
                    std::function<void(Creature *affectedCreature, Creature *sourceCreature, BattleSystem *battle, TurnBasedEffect *self)> effectFunc,
                    bool onTurnStart = false,
                    int chance = 100);

    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;
    virtual QString getDescription() const override;

    int getDuration() const;
    void setDuration(int duration);
    bool decrementDuration();

    void executeTurnLogic(Creature *affectedCreature, Creature *sourceCreature, BattleSystem *battle);
    bool isOnTurnStart() const { return m_onTurnStart; }

    // 源精灵相关方法
    Creature *getOriginalSource() const;
    void setOriginalSource(Creature *source);
    
    // 获取效果的实际目标
    Creature* getEffectTarget(Creature *affectedCreature, BattleSystem *battle) const;
    
    void setDescription(const QString &desc);

private:
    int m_initialDuration;
    int m_currentDuration;
    std::function<void(Creature *affectedCreature, Creature *sourceCreature, BattleSystem *battle, TurnBasedEffect *self)> m_effectLogic;
    bool m_onTurnStart;
    QString m_description;
    Creature *m_originalSource;
};

// 施加异常状态效果
class StatusConditionEffect : public Effect
{
public:
    StatusConditionEffect(StatusCondition condition, int chance = 100);

    StatusCondition getCondition() const;

    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;
    virtual QString getDescription() const override;

private:
    StatusCondition m_condition;
};

// 能力等级变化效果
class StatChangeEffect : public Effect
{
public:
    StatChangeEffect(StatType stat, int stages, TargetType targetType = TargetType::OPPONENT, int chance = 100);

    StatType getStat() const;
    int getStages() const;

    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;
    virtual QString getDescription() const override;

private:
    StatType m_stat;
    int m_stages;
};

// 清除效果
class ClearEffectsEffect : public Effect
{
public:
    ClearEffectsEffect(bool clearPositiveStatChanges,
                       bool clearNegativeStatChanges,
                       bool clearStatusConditions,
                       bool clearTurnBasedEffects,
                       TargetType targetType = TargetType::OPPONENT,
                       int chance = 100);

    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;
    virtual QString getDescription() const override;

private:
    bool m_clearPositiveStatChanges;
    bool m_clearNegativeStatChanges;
    bool m_clearStatusConditions;
    bool m_clearTurnBasedEffects;
};

// 免疫效果
class ImmunityEffect : public Effect
{
public:
    ImmunityEffect(int duration, bool immuneToStatus, ElementType immuneToSpecificTypeDamage = ElementType::NONE, int chance = 100);

    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;
    virtual QString getDescription() const override;

private:
    int m_duration;
    bool m_immuneToStatus;
    ElementType m_immuneToTypeDamage;
};

// 回复HP效果
class HealingEffect : public Effect
{
public:
    HealingEffect(int amount, bool isPercentage, int chance = 100);

    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;
    virtual QString getDescription() const override;

private:
    int m_amount;
    bool m_isPercentage;
};

// 固定伤害效果
class FixedDamageEffect : public Effect
{
public:
    FixedDamageEffect(int damageAmount, int chance = 100);

    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;
    virtual QString getDescription() const override;

private:
    int m_damageAmount;
};

#endif // EFFECT_H
