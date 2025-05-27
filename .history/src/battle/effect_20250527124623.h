// src/battle/effect.h
#ifndef EFFECT_H
#define EFFECT_H

#include <QString>
#include <functional>        // 用于std::function，实现可回调的效果逻辑
#include "../core/ability.h" // 核心 - 能力系统 (StatType, StatusCondition)
#include "../core/type.h"    // 核心 - 类型系统 (ElementType)
#include <QVariant>          // 用于在std::function中传递通用数据

// 前向声明
class Creature;     // 精灵类
class BattleSystem; // 战斗系统类

// 效果类型枚举 (用于区分不同效果，方便管理和序列化)
enum class EffectType
{
    GENERIC,              // 通用效果 (未指定类型)
    TURN_BASED,           // 回合类效果 (持续数回合)
    STATUS_CONDITION,     // 施加异常状态效果
    STAT_CHANGE,          // 能力等级变化效果
    CLEAR_EFFECTS,        // 清除目标身上的效果 (如异常状态、能力等级变化)
    IMMUNITY,             // 提供免疫效果 (如免疫异常、免疫特定属性伤害)
    HEALING,              // 回复HP效果
    FIXED_DAMAGE,         // 造成固定数值伤害效果
    CRITICAL_HIT_MOD,     // 影响暴击率或暴击伤害的效果
    FAILURE_COMPENSATION, // 当主要技能效果未触发时的补偿效果
    FIELD_EFFECT,         // 场地效果 (影响整个战场)
    ENTRY_HAZARD          // 入场障碍效果 (如钉子、毒钉)
    // ... 可根据需要扩展更多效果类型
};

// 效果基类
class Effect
{
public:
    // 构造函数
    // type: 效果的类型
    // chance: 效果触发的几率 (0-100)
    Effect(EffectType type, int chance = 100);

    // 虚析构函数
    virtual ~Effect();

    // 获取效果类型
    EffectType getType() const;

    // 获取效果触发的基础几率
    int getChance() const;

    // 设置效果触发的基础几率
    void setChance(int chance);

    // 应用效果的核心虚函数，由派生类实现具体逻辑
    // source: 效果的来源 (通常是技能的使用者)
    // target: 效果作用的目标
    // battle: 当前战斗系统实例，用于与战斗环境交互或触发战斗日志
    // 返回值: true 表示效果成功应用，false 表示失败或未触发
    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) = 0;

    // 获取效果的文本描述，用于UI显示或战斗日志
    virtual QString getDescription() const = 0;

    // 设置效果的目标是使用者自身还是对方
    // (一些效果可能固定目标，一些可能由技能设定)
    void setTargetSelf(bool self);
    bool isTargetSelf() const;
    bool publicCheckChance() const { return checkChance(); }
    Creature* determineActualTarget(Creature* source, Creature* defaultTarget, BattleSystem* battle) const {
        if (isTargetSelf()) {
            return source; // 如果效果作用于自身，返回源精灵
        } else {
            return defaultTarget; // 否则返回默认目标（通常是对手）
        }
    }
protected:
    EffectType m_type; // 效果类型
    int m_chance;      // 效果触发的基础几率 (0-100)
    bool m_targetSelf; // 效果是否作用于使用者自身 (默认为false，作用于对方)

    // 辅助函数：根据m_chance检查几率是否触发
    bool checkChance() const;
};

// --- 具体效果类 ---

// 回合类效果 (例如：中毒每回合扣血，能力提升持续数回合)
class TurnBasedEffect : public Effect
{
public:
    // duration: 效果持续的回合数
    // effectFunc: 在每回合特定阶段（如开始或结束）执行的逻辑
    // onTurnStart: true则在回合开始时触发，false则在回合结束时触发
    TurnBasedEffect(int duration,
                    std::function<void(Creature *affectedCreature, Creature *sourceCreature, BattleSystem *battle, TurnBasedEffect *self)> effectFunc,
                    bool onTurnStart = false, // true则在回合开始时触发，false则在回合结束时触发
                    int chance = 100);

    // 应用效果 (通常是将此效果添加到目标精灵的持续效果列表中)
    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;

    // 获取效果描述
    virtual QString getDescription() const override;

    // 获取剩余持续回合数
    int getDuration() const;
    // 设置剩余持续回合数
    void setDuration(int duration);
    // 减少一回合持续时间，如果减至0或以下则返回true表示效果结束
    bool decrementDuration();

    // 执行回合效果的逻辑
    void executeTurnLogic(Creature *affectedCreature, Creature *sourceCreature, BattleSystem *battle);

    bool isOnTurnStart() const { return m_onTurnStart; } // 效果是在回合开始还是结束时触发

private:
    int m_initialDuration; // 初始设定的持续回合数
    int m_currentDuration; // 当前剩余的持续回合数
    // 使用std::function存储效果的具体逻辑，使其更灵活
    // 参数：受影响的精灵，效果来源精灵(可能为nullptr)，战斗系统，效果自身(用于自我移除等)
    std::function<void(Creature *affectedCreature, Creature *sourceCreature, BattleSystem *battle, TurnBasedEffect *self)> m_effectLogic;
    bool m_onTurnStart;    // 标记效果是在回合开始还是结束时执行
    QString m_description; // 效果的文本描述
public:
    void setDescription(const QString &desc) { m_description = desc; } // 允许外部设置描述
    Creature *getOriginalSource() const { return m_originalSource; }   // 获取最初施加此效果的源
    void setOriginalSource(Creature *source) { m_originalSource = source; }

private:
    Creature *m_originalSource = nullptr; // 记录最初施加此效果的源，用于一些需要追溯来源的逻辑
};

// 施加异常状态效果
class StatusConditionEffect : public Effect
{
public:
    StatusConditionEffect(StatusCondition condition, int chance = 100);

    StatusCondition getCondition() const; // 获取要施加的异常状态

    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;
    virtual QString getDescription() const override;

private:
    StatusCondition m_condition; // 要施加的异常状态类型
};

// 能力等级变化效果
class StatChangeEffect : public Effect
{
public:
    // stat: 要改变的能力类型
    // stages: 变化的等级 (+1, -2 等)
    // targetSelf: true表示作用于使用者自身，false表示作用于对方
    StatChangeEffect(StatType stat, int stages, bool targetSelf, int chance = 100);

    StatType getStat() const; // 获取目标能力类型
    int getStages() const;    // 获取变化等级
    // isTargetSelf() 在基类中已有

    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;
    virtual QString getDescription() const override;

private:
    StatType m_stat; // 要改变的能力类型
    int m_stages;    // 变化等级 (+/-)
    // m_targetSelf 在基类 Effect 中
};

// 清除效果 (例如：清除对方强化、解除我方异常)
class ClearEffectsEffect : public Effect
{ // 修改类名以避免与EffectType冲突
public:
    // flags 指示要清除哪些类型的效果
    ClearEffectsEffect(bool clearPositiveStatChanges, // 清除能力提升
                       bool clearNegativeStatChanges, // 清除能力下降
                       bool clearStatusConditions,    // 清除异常状态
                       bool clearTurnBasedEffects,    // 清除所有回合性效果
                       bool targetSelf,               // 是清除自己还是对方
                       int chance = 100);

    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;
    virtual QString getDescription() const override;

private:
    bool m_clearPositiveStatChanges; // 是否清除正向能力等级变化
    bool m_clearNegativeStatChanges; // 是否清除负向能力等级变化
    bool m_clearStatusConditions;    // 是否清除异常状态
    bool m_clearTurnBasedEffects;    // 是否清除回合制效果
    // m_targetSelf 在基类 Effect 中
};

// 免疫效果 (例如：数回合内免疫异常状态)
class ImmunityEffect : public Effect
{
public:
    // duration: 免疫效果的持续回合数
    // immuneToStatus: 是否免疫所有异常状态
    // immuneToSpecificTypeDamage: 免疫特定属性的攻击 (ElementType::NONE 表示不免疫任何特定属性)
    ImmunityEffect(int duration, bool immuneToStatus, ElementType immuneToSpecificTypeDamage = ElementType::NONE, int chance = 100); // 修正: 默认参数在声明中

    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;
    virtual QString getDescription() const override;

private:
    int m_duration;                   // 免疫效果的持续回合数
    bool m_immuneToStatus;            // 是否免疫所有异常状态
    ElementType m_immuneToTypeDamage; // 免疫哪个属性的伤害
};

// 回复HP效果
class HealingEffect : public Effect
{
public:
    // healAmount: 治疗量 (如果是固定值)
    // healPercentage: 治疗百分比 (基于目标最大HP，0-100)
    // isPercentage: true 表示使用百分比治疗，false 表示使用固定值治疗
    HealingEffect(int amount, bool isPercentage, int chance = 100);

    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;
    virtual QString getDescription() const override;

private:
    int m_amount;        // 治疗的数值或百分比
    bool m_isPercentage; // 标记是数值还是百分比
};

// 固定伤害效果
class FixedDamageEffect : public Effect
{
public:
    FixedDamageEffect(int damageAmount, int chance = 100);

    virtual bool apply(Creature *source, Creature *target, BattleSystem *battle) override;
    virtual QString getDescription() const override;

private:
    int m_damageAmount; // 造成的固定伤害值
};

// TODO: 根据游戏设计文档，实现更多具体效果类
// 例如:
// - CriticalHitModEffect (提升暴击率)
// - FieldEffect (改变场地状态，如天气、空间)
// - EntryHazardEffect (撒钉子等)
// - FailureCompensationEffect (主要效果失败时的补偿效果)

#endif // EFFECT_H