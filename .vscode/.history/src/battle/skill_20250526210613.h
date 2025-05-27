// src/battle/skill.h
#ifndef SKILL_H
#define SKILL_H

#include <QString>
#include <QVector>
#include <functional>
#include "../core/type.h"     // 核心 - 类型系统
#include "../core/ability.h"  // 核心 - 能力系统

// 前向声明
class Creature;     // 精灵类
class Effect;       // 效果类
class BattleSystem; // 战斗系统类

// 技能类别枚举
enum class SkillCategory
{
    PHYSICAL, // 物理攻击技能
    SPECIAL,  // 特殊攻击技能
    STATUS    // 属性/状态技能 (例如：提升能力、施加异常状态)
};

// 技能基类
class Skill
{
public:
    // 构造函数
    // name: 技能名称
    // type: 技能的元素属性
    // category: 技能的类别 (物理, 特殊, 状态)
    // power: 技能威力 (对于状态技能通常为0)
    // ppCost: 技能消耗的PP值 (从精灵的全局PP池中扣除)
    // accuracy: 技能命中率 (0-100，100以上视为必中或有特殊规则)
    // priority: 技能优先级 (影响出招顺序)
    Skill(const QString &name, ElementType type, SkillCategory category,
          int power, int ppCost, int accuracy, int priority = 0);

    // 虚析构函数，确保派生类的析构函数能被正确调用
    virtual ~Skill();

    // --- 获取技能基本信息 ---
    QString getName() const;           // 获取技能名称
    ElementType getType() const;       // 获取技能的元素属性
    SkillCategory getCategory() const; // 获取技能类别
    int getPower() const;              // 获取技能威力 (一些技能可能会在特定条件下动态改变威力)
    int getPPCost() const;             // 获取技能的PP消耗
    int getAccuracy() const;           // 获取技能基础命中率
    int getPriority() const;           // 获取技能优先级

    // --- 技能核心逻辑 ---
    // 使用技能的虚函数，由派生类具体实现
    // user: 技能使用者
    // target: 技能目标
    // battle: 当前战斗系统实例，用于与战斗环境交互
    // 返回值: 通常表示技能是否成功执行了其主要意图（例如命中并应用效果，或状态技能成功发动）
    virtual bool use(Creature *user, Creature *target, BattleSystem *battle);

    // 获取技能的描述文本，用于UI显示
    virtual QString getDescription() const;
    // 设置技能描述
    void setDescription(const QString &description);

    // 判断技能是否为必中技能 (基于其accuracy值)
    bool isAlwaysHit() const;

    // --- 技能效果管理 ---
    void addEffect(Effect *effect);             // 为技能添加一个效果
    const QVector<Effect *> &getEffects() const; // 获取技能持有的所有效果

    // --- 伤害计算 (主要由BattleSystem调用或技能自身特殊情况处理) ---
    // user: 攻击方
    // target: 防御方
    // 返回值: 计算出的基础伤害值 (可能未应用所有修正，如STAB、暴击、随机数等，这些通常在BattleSystem中处理)
    // 对于大多数技能，这个方法可能只返回m_power。特定技能如FixedDamageSkill会重写它。
    virtual int calculateDamage(Creature *user, Creature *target);

protected:
    QString m_name;           // 技能名称
    ElementType m_type;       // 技能的元素属性
    SkillCategory m_category; // 技能类别
    int m_power;              // 技能威力
    int m_ppCost;             // PP消耗 (从精灵的全局PP池扣除)
    int m_accuracy;           // 命中率 (百分比, e.g., 90 表示 90%)
    int m_priority;           // 优先级 (正数优先，负数延后)

    QVector<Effect *> m_effects; // 技能附加的效果列表 (例如: 中毒、能力提升等)

    // 检查技能是否命中目标 (基础命中判定)
    // user: 技能使用者
    // target: 技能目标
    // 返回值: true 如果命中，false 如果未命中
    // 注意: BattleSystem中会有更完整的命中判定，包括能力等级修正等
    virtual bool checkHit(Creature *user, Creature *target, BattleSystem *battle);
};

// --- 具体技能类型 ---

// 物理攻击技能
class PhysicalSkill : public Skill
{
public:
    PhysicalSkill(const QString &name, ElementType type, int power,
                  int ppCost, int accuracy, int priority = 0);

    // 物理技能的伤害计算可能依赖物攻/物防，但具体计算通常在BattleSystem中完成
    // 此处的calculateDamage主要返回基础威力，或在有特殊计算逻辑时重写
    // virtual int calculateDamage(Creature *user, Creature *target) override;
};

// 特殊攻击技能
class SpecialSkill : public Skill
{
public:
    SpecialSkill(const QString &name, ElementType type, int power,
                 int ppCost, int accuracy, int priority = 0);

    // 特殊技能的伤害计算可能依赖特攻/特防
    // virtual int calculateDamage(Creature *user, Creature *target) override;
};

// 属性/状态技能 (不直接造成伤害，而是改变状态或能力)
class StatusSkill : public Skill
{
public:
    // 状态技能威力通常为0
    StatusSkill(const QString &name, ElementType type,
                int ppCost, int accuracy, int priority = 0);

    // 状态技能的use方法通常侧重于应用其效果
    virtual bool use(Creature *user, Creature *target, BattleSystem *battle) override;
};

// 复合技能 (攻击的同时附加效果，例如火焰拳：造成伤害 + 可能烧伤)
class CompositeSkill : public Skill // 可以继承自PhysicalSkill或SpecialSkill，或保持Skill基类
{
public:
    CompositeSkill(const QString &name, ElementType type, SkillCategory category, // 需要明确是物理还是特殊
                   int power, int ppCost, int accuracy, int priority = 0);

    void setEffectChance(int chance); // 设置附加效果的触发几率
    int getEffectChance() const;      // 获取附加效果的触发几率

    // use方法会处理伤害，并根据几率应用附加效果
    virtual bool use(Creature *user, Creature *target, BattleSystem *battle) override;

private:
    int m_effectChance; // 附加效果的触发几率 (百分比)
};

// 多段攻击技能 (例如：连环巴掌，攻击2-5次)
class MultiHitSkill : public Skill // 可以继承自PhysicalSkill或SpecialSkill
{
public:
    MultiHitSkill(const QString &name, ElementType type, SkillCategory category,
                  int power, int ppCost, int accuracy,
                  int minHits, int maxHits, int priority = 0);

    // use方法会处理多次攻击判定和伤害计算
    virtual bool use(Creature *user, Creature *target, BattleSystem *battle) override;
    int getMinHits() const { return m_minHits; } // 获取最小攻击次数
    int getMaxHits() const { return m_maxHits; } // 获取最大攻击次数

private:
    int m_minHits; // 最小攻击次数
    int m_maxHits; // 最大攻击次数
};


// 固定伤害技能 (造成固定数值的伤害，不受攻防影响)
class FixedDamageSkill : public Skill // 通常视为特殊或物理，取决于游戏设计
{
public:
    // 固定伤害技能的威力(power)字段可以设为0，实际伤害由m_fixedDamage决定
    FixedDamageSkill(const QString &name, ElementType type, SkillCategory category,
                     int fixedDamageAmount, int ppCost, int accuracy, int priority = 0);

    // 重写calculateDamage以返回固定伤害值
    virtual int calculateDamage(Creature *user, Creature *target) override;

private:
    int m_fixedDamage; // 造成的固定伤害值
};

// 回复技能 (为使用者或目标回复HP)
class HealingSkill : public StatusSkill // 通常是状态技能
{
public:
    // healAmount: 可以是固定值，也可以是最大HP的百分比
    // isPercentage: true表示healAmount是百分比，false表示是固定值
    HealingSkill(const QString &name, ElementType type,
                 int ppCost, int accuracy, int healAmount, bool isPercentage, int priority = 0);

    virtual bool use(Creature *user, Creature *target, BattleSystem *battle) override;

private:
    int m_healAmount;   // 治疗量 (数值或百分比)
    bool m_isPercentage; // 治疗量是否为百分比
};

// 能力变化技能 (提升或降低使用者或目标的能力等级)
class StatChangeSkill : public StatusSkill // 状态技能
{
public:
    StatChangeSkill(const QString &name, ElementType type,
                    int ppCost, int accuracy, int priority = 0);

    // 添加一个能力变化效果到此技能
    // stat: 要改变的能力类型
    // stages: 变化的等级数 (正数为提升，负数为降低)
    // targetSelf: true表示作用于使用者，false表示作用于目标
    void addStatChange(StatType stat, int stages, bool targetSelf);

    virtual bool use(Creature *user, Creature *target, BattleSystem *battle) override;

private:
    // 结构体，用于存储一个能力变化的效果
    struct StatChange
    {
        StatType stat;   // 能力类型
        int stages;      // 变化等级
        bool targetSelf; // 是否作用于自身
    };
    QString m_description; // 技能描述
    QVector<StatChange> m_statChanges; // 该技能包含的所有能力变化效果
};


// 第五技能基类 (可能有特殊的使用条件或效果)
class FifthSkill : public Skill
{
public:
    FifthSkill(const QString &name, ElementType type, SkillCategory category, // ElementType is correctly used here as a type
               int power, int ppCost, int accuracy, int priority = 0);

    virtual bool canUse(Creature *user, Creature *target, BattleSystem* battle) const;
};

#endif // SKILL_H