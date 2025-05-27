#ifndef SKILL_H
#define SKILL_H

#include <QString>
#include <QVector>
#include <functional>
#include "../core/type.h"
#include "../core/ability.h"

// 前向声明
class Creature;
class Effect;
class BattleSystem;

// 技能类别枚举
enum class SkillCategory
{
    PHYSICAL, // 物理攻击技能
    SPECIAL,  // 特殊攻击技能
    STATUS    // 属性/状态技能
};

// 技能基类
class Skill
{
public:
    // 构造函数
    Skill(const QString &name, ElementType type, SkillCategory category,
          int power, int ppCost, int accuracy, int priority = 0);

    // 虚析构函数
    virtual ~Skill();

    // --- 获取技能基本信息 ---
    QString getName() const;
    ElementType getType() const;
    SkillCategory getCategory() const;
    int getPower() const;
    int getPPCost() const;
    int getAccuracy() const;
    int getPriority() const;

    // --- 目标类型相关方法 ---
    TargetType getTargetType() const;
    void setTargetType(TargetType targetType);
    
    // 获取实际目标（统一接口）
    Creature* getActualTarget(Creature *user, Creature *target, BattleSystem *battle) const;

    // --- 技能核心逻辑 ---
    virtual bool use(Creature *user, Creature *target, BattleSystem *battle);

    // 获取技能的描述文本
    virtual QString getDescription() const;
    void setDescription(const QString &description);
    QString getDetailedDescription() const;
    bool isAlwaysHit() const;

    // --- 技能效果管理 ---
    void addEffect(Effect *effect);
    const QVector<Effect *> &getEffects() const;

    // --- 伤害计算 ---
    virtual int calculateDamage(Creature *user, Creature *target);

protected:
    QString m_name;
    ElementType m_type;
    SkillCategory m_category;
    int m_power;
    int m_ppCost;
    int m_accuracy;
    int m_priority;
    QString m_description;
    QVector<Effect *> m_effects;
    TargetType m_targetType;

    // 检查技能是否命中目标
    virtual bool checkHit(Creature *user, Creature *target, BattleSystem *battle);
};

// 物理攻击技能
class PhysicalSkill : public Skill
{
public:
    PhysicalSkill(const QString &name, ElementType type, int power,
                  int ppCost, int accuracy, int priority = 0);
};

// 特殊攻击技能
class SpecialSkill : public Skill
{
public:
    SpecialSkill(const QString &name, ElementType type, int power,
                 int ppCost, int accuracy, int priority = 0);
};

// 属性/状态技能
class StatusSkill : public Skill
{
public:
    StatusSkill(const QString &name, ElementType type,
                int ppCost, int accuracy, int priority = 0);

    virtual bool use(Creature *user, Creature *target, BattleSystem *battle) override;
};

// 复合技能
class CompositeSkill : public Skill
{
public:
    CompositeSkill(const QString &name, ElementType type, SkillCategory category,
                   int power, int ppCost, int accuracy, int priority = 0);

    void setEffectChance(int chance);
    int getEffectChance() const;

    virtual bool use(Creature *user, Creature *target, BattleSystem *battle) override;

private:
    int m_effectChance;
};

// 多段攻击技能
class MultiHitSkill : public Skill
{
public:
    MultiHitSkill(const QString &name, ElementType type, SkillCategory category,
                  int power, int ppCost, int accuracy,
                  int minHits, int maxHits, int priority = 0);

    virtual bool use(Creature *user, Creature *target, BattleSystem *battle) override;
    int getMinHits() const;
    int getMaxHits() const;

private:
    int m_minHits;
    int m_maxHits;
};

// 固定伤害技能
class FixedDamageSkill : public Skill
{
public:
    FixedDamageSkill(const QString &name, ElementType type, SkillCategory category,
                     int fixedDamageAmount, int ppCost, int accuracy, int priority = 0);

    virtual int calculateDamage(Creature *user, Creature *target) override;

private:
    int m_fixedDamage;
};

// 回复技能
class HealingSkill : public StatusSkill
{
public:
    HealingSkill(const QString &name, ElementType type,
                 int ppCost, int accuracy, int healAmount, bool isPercentage, int priority = 0);

    virtual bool use(Creature *user, Creature *target, BattleSystem *battle) override;

private:
    int m_healAmount;
    bool m_isPercentage;
};

// 能力变化技能
class StatChangeSkill : public StatusSkill
{
public:
    StatChangeSkill(const QString &name, ElementType type,
                    int ppCost, int accuracy, int priority = 0);

    // 添加一个能力变化效果到此技能
    void addStatChange(StatType stat, int stages, TargetType targetType = TargetType::SELF);

    virtual bool use(Creature *user, Creature *target, BattleSystem *battle) override;

private:
    struct StatChange
    {
        StatType stat;
        int stages;
        TargetType targetType;
    };
    QVector<StatChange> m_statChanges;
};

#endif // SKILL_H
