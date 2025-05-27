#ifndef EFFECT_H
#define EFFECT_H

#include <QString>
#include <functional>
#include "../core/ability.h"

// 前向声明
class Creature;
class BattleSystem;

// 效果类型枚举
enum class EffectType {
    TURN_BASED,            // 回合类效果
    STATUS_CONDITION,      // 异常状态效果
    STAT_CHANGE,           // 能力变化效果
    CLEAR_EFFECTS,         // 清除效果
    IMMUNITY,              // 免疫效果
    HEALING,               // 回复效果
    FIXED_DAMAGE,          // 固定伤害效果
    CRITICAL_HIT,          // 致命一击效果
    FAILURE_COMPENSATION   // 效果未触发补偿
};

// 效果基类
class Effect {
public:
    Effect(EffectType type, int chance = 100);
    virtual ~Effect();
    
    // 获取效果类型
    EffectType getType() const;
    
    // 获取触发几率
    int getChance() const;
    
    // 设置触发几率
    void setChance(int chance);
    
    // 应用效果
    virtual bool apply(Creature* source, Creature* target, BattleSystem* battle) = 0;
    
    // 描述效果
    virtual QString getDescription() const = 0;
    
protected:
    EffectType m_type;     // 效果类型
    int m_chance;          // 触发几率（百分比）
    
    // 检查几率是否触发
    bool checkChance() const;
};

// 回合类效果
class TurnBasedEffect : public Effect {
public:
    TurnBasedEffect(int duration, int chance = 100);
    
    // 设置回合开始/结束时的效果
    void setStartTurnEffect(std::function<void(Creature*, Creature*, BattleSystem*)> effect);
    void setEndTurnEffect(std::function<void(Creature*, Creature*, BattleSystem*)> effect);
    
    // 获取持续回合数
    int getDuration() const;
    
    // 减少回合计数
    void decreaseTurn();
    
    // 应用效果
    virtual bool apply(Creature* source, Creature* target, BattleSystem* battle) override;
    
    // 回合开始时触发
    void onTurnStart(Creature* source, Creature* target, BattleSystem* battle);
    
    // 回合结束时触发
    void onTurnEnd(Creature* source, Creature* target, BattleSystem* battle);
    
    // 描述效果
    virtual QString getDescription() const override;
    
private:
    int m_duration;    // 持续回合数
    std::function<void(Creature*, Creature*, BattleSystem*)> m_startTurnEffect;  // 回合开始时的效果
    std::function<void(Creature*, Creature*, BattleSystem*)> m_endTurnEffect;    // 回合结束时的效果
};

// 异常状态效果
class StatusConditionEffect : public Effect {
public:
    StatusConditionEffect(StatusCondition condition, int chance = 100);
    
    // 获取异常状态
    StatusCondition getCondition() const;
    
    // 应用效果
    virtual bool apply(Creature* source, Creature* target, BattleSystem* battle) override;
    
    // 描述效果
    virtual QString getDescription() const override;
    
private:
    StatusCondition m_condition;   // 异常状态类型
};

// 能力变化效果
class StatChangeEffect : public Effect {
public:
    StatChangeEffect(StatType stat, int stages, bool targetSelf, int chance = 100);
    
    // 获取能力变化信息
    StatType getStat() const;
    int getStages() const;
    bool isTargetSelf() const;
    
    // 应用效果
    virtual bool apply(Creature* source, Creature* target, BattleSystem* battle) override;
    
    // 描述效果
    virtual QString getDescription() const override;
    
private:
    StatType m_stat;       // 能力类型
    int m_stages;          // 变化等级
    bool m_targetSelf;     // 是否针对自己
};

// 清除效果
class ClearEffect : public Effect {
public:
    ClearEffect(bool clearStatusConditions, bool clearStatChanges, 
                bool clearTurnEffects, int chance = 100);
    
    // 应用效果
    virtual bool apply(Creature* source, Creature* target, BattleSystem* battle) override;
    
    // 描述效果
    virtual QString getDescription() const override;
    
private:
    bool m_clearStatusConditions;  // 清除异常状态
    bool m_clearStatChanges;       // 清除能力变化
    bool m_clearTurnEffects;       // 清除回合效果
};

// 免疫效果
class ImmunityEffect : public Effect {
public:
    ImmunityEffect(int duration, bool immuneToStatus, bool immuneToDamage, 
                  bool immuneToStatChanges, int chance = 100);
    
    // 应用效果
    virtual bool apply(Creature* source, Creature* target, BattleSystem* battle) override;
    
    // 描述效果
    virtual QString getDescription() const override;
    
private:
    int m_duration;            // 持续回合数
    bool m_immuneToStatus;     // 免疫异常状态
    bool m_immuneToDamage;     // 免疫伤害
    bool m_immuneToStatChanges; // 免疫能力变化
};

// 回复效果
class HealingEffect : public Effect {
public:
    HealingEffect(int healPercent, int chance = 100);
    
    // 获取回复百分比
    int getHealPercent() const;
    
    // 应用效果
    virtual bool apply(Creature* source, Creature* target, BattleSystem* battle) override;
    
    // 描述效果
    virtual QString getDescription() const override;
    
private:
    int m_healPercent;     // 回复百分比
};

// 固定伤害效果
class FixedDamageEffect : public Effect {
public:
    FixedDamageEffect(int damage, int chance = 100);
    
    // 获取伤害值
    int getDamage() const;
    
    // 应用效果
    virtual bool apply(Creature* source, Creature* target, BattleSystem* battle) override;
    
    // 描述效果
    virtual QString getDescription() const override;
    
private:
    int m_damage;      // 伤害值
};

// 致命一击效果
class CriticalHitEffect : public Effect {
public:
    CriticalHitEffect(int criticalBoost = 1, int chance = 100);
    
    // 获取暴击增益级别
    int getCriticalBoost() const;
    
    // 应用效果
    virtual bool apply(Creature* source, Creature* target, BattleSystem* battle) override;
    
    // 描述效果
    virtual QString getDescription() const override;
    
private:
    int m_criticalBoost;   // 暴击增益级别
};

// 效果未触发补偿
class FailureCompensationEffect : public Effect {
public:
    FailureCompensationEffect(int chance = 100);
    
    // 设置补偿效果
    void setCompensationEffect(Effect* effect);
    
    // 应用效果
    virtual bool apply(Creature* source, Creature* target, BattleSystem* battle) override;
    
    // 描述效果
    virtual QString getDescription() const override;
    
private:
    Effect* m_compensationEffect;  // 补偿效果
};

#endif // EFFECT_H
