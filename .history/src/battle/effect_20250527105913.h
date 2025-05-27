#ifndef EFFECT_H
#define EFFECT_H

#include <QString>
#include <functional>
#include "../core/type.h"

// 前向声明
class Creature;
class BattleSystem;
class TurnBasedEffect;

// 效果基类
class Effect
{
public:
    Effect();
    virtual ~Effect();

    // 获取和设置目标类型
    TargetType getTargetType() const;
    void setTargetType(TargetType targetType);
    
    // 获取实际目标
    Creature* getActualTarget(Creature *user, Creature *target, BattleSystem *battle) const;

    // 应用效果
    virtual bool apply(Creature *user, Creature *target, BattleSystem *battle);

    // 获取和设置效果描述
    QString getDescription() const;
    void setDescription(const QString &description);

protected:
    TargetType m_targetType;
    QString m_description;
};

// 状态条件效果
class StatusConditionEffect : public Effect
{
public:
    StatusConditionEffect(StatusCondition condition, int chance = 100);

    virtual bool apply(Creature *user, Creature *target, BattleSystem *battle) override;

private:
    StatusCondition m_condition;
    int m_chance; // 触发几率（百分比）
};

// 属性变化效果
class StatChangeEffect : public Effect
{
public:
    StatChangeEffect(StatType stat, int stages, TargetType targetType = TargetType::OPPONENT);

    virtual bool apply(Creature *user, Creature *target, BattleSystem *battle) override;

private:
    StatType m_stat;
    int m_stages; // 正数表示提升，负数表示降低
};

// 回合效果
class TurnBasedEffect
{
public:
    TurnBasedEffect(int duration, 
                    std::function<void(Creature*, Creature*, BattleSystem*, TurnBasedEffect*)> effectLogic,
                    bool onTurnStart = false);

    void setDescription(const QString &description);
    QString getDescription() const;

    void setOriginalSource(Creature *source);
    Creature *getOriginalSource() const;

    bool isOnTurnStart() const;
    void executeTurnLogic(Creature *affected, Creature *source, BattleSystem *battle);
    bool decrementDuration();
    int getRemainingDuration() const;

private:
    int m_duration;
    std::function<void(Creature*, Creature*, BattleSystem*, TurnBasedEffect*)> m_effectLogic;
    bool m_onTurnStart;
    QString m_description;
    Creature *m_originalSource;
};

// 清除效果
class ClearEffectsEffect : public Effect
{
public:
    ClearEffectsEffect(bool clearStatChanges = false, 
                       bool clearTurnEffects = false, 
                       bool clearStatusCondition = false, 
                       bool clearAll = false,
                       TargetType targetType = TargetType::SELF);

    virtual bool apply(Creature *user, Creature *target, BattleSystem *battle) override;

private:
    bool m_clearStatChanges;
    bool m_clearTurnEffects;
    bool m_clearStatusCondition;
    bool m_clearAll;
};

// 免疫效果
class ImmunityEffect : public Effect
{
public:
    ImmunityEffect(ElementType type, int duration);
    virtual ~ImmunityEffect();

    virtual bool apply(Creature *user, Creature *target, BattleSystem *battle) override;

    ElementType getImmuneType() const;
    int getDuration() const;
    TurnBasedEffect *getTurnEffect() const;

private:
    ElementType m_type;
    int m_duration;
    TurnBasedEffect *m_turnEffect;
};

#endif // EFFECT_H
