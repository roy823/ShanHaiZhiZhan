#ifndef SKILL_H
#define SKILL_H

#include <QString>
#include <QVector>
#include <functional>
#include "../core/type.h"

// 前向声明
class Creature;
class Effect;
class BattleSystem;

// 技能类型枚举
enum class SkillCategory {
    PHYSICAL,   // 物理攻击技能
    SPECIAL,    // 特殊攻击技能
    STATUS      // 属性技能
};

// 技能基类
class Skill {
public:
    Skill(const QString& name, ElementType type, SkillCategory category, 
          int power, int ppCost, int accuracy, int priority = 0);
    virtual ~Skill();
    
    // 获取技能信息
    QString getName() const;
    ElementType getType() const;
    SkillCategory getCategory() const;
    int getPower() const;
    int getPPCost() const;
    int getAccuracy() const;
    int getPriority() const;
    
    // 使用技能
    virtual bool use(Creature* user, Creature* target, BattleSystem* battle);
    
    // 描述技能效果
    virtual QString getDescription() const;
    
    // 判断技能是否必中
    bool isAlwaysHit() const;
    
    // 获取当前PP值
    int getCurrentPP() const;
    
    // 设置当前PP值
    void setCurrentPP(int value);
    
    // 消耗PP
    void consumePP(int amount = 1);
    
    // 恢复PP
    void restorePP(int amount = 1);

    // 获取最大PP值
    int getMaxPP() const { return m_maxPP; }
    
    // 添加效果
    void addEffect(Effect* effect);
    
    // 获取所有效果
    const QVector<Effect*>& getEffects() const;
    
protected:
    QString m_name;             // 技能名称
    ElementType m_type;         // 技能系别
    SkillCategory m_category;   // 技能类别
    int m_power;                // 技能威力
    int m_ppCost;               // PP消耗
    int m_accuracy;             // 命中率（百分比）
    int m_priority;             // 优先级
    int m_currentPP;            // 当前PP值
    int m_maxPP;                // 最大PP值
    QVector<Effect*> m_effects; // 技能效果列表
    
    // 计算技能伤害
    virtual int calculateDamage(Creature* user, Creature* target);
    
    // 检查命中
    virtual bool checkHit(Creature* user, Creature* target);
};

// 物理攻击技能
class PhysicalSkill : public Skill {
public:
    PhysicalSkill(const QString& name, ElementType type, int power, 
                  int ppCost, int accuracy, int priority = 0);
    
    virtual int calculateDamage(Creature* user, Creature* target) override;
};

// 特殊攻击技能
class SpecialSkill : public Skill {
public:
    SpecialSkill(const QString& name, ElementType type, int power, 
                 int ppCost, int accuracy, int priority = 0);
    
    virtual int calculateDamage(Creature* user, Creature* target) override;
};

// 状态技能
class StatusSkill : public Skill {
public:
    StatusSkill(const QString& name, ElementType type, 
                int ppCost, int accuracy, int priority = 0);
    
    virtual bool use(Creature* user, Creature* target, BattleSystem* battle) override;
};

// 复合攻击技能（攻击同时附加效果）
class CompositeSkill : public Skill {
public:
    CompositeSkill(const QString& name, ElementType type, SkillCategory category, 
                  int power, int ppCost, int accuracy, int priority = 0);
    
    // 设置和获取效果触发几率
    void setEffectChance(int chance);
    int getEffectChance() const;
    
    virtual bool use(Creature* user, Creature* target, BattleSystem* battle) override;
    
private:
    int m_effectChance; // 效果触发几率（百分比）
};

// 连续攻击技能
class MultiHitSkill : public Skill {
public:
    MultiHitSkill(const QString& name, ElementType type, SkillCategory category, 
                 int power, int ppCost, int accuracy, 
                 int minHits, int maxHits, int priority = 0);
    
    virtual bool use(Creature* user, Creature* target, BattleSystem* battle) override;
    
private:
    int m_minHits; // 最小攻击次数
    int m_maxHits; // 最大攻击次数
};

// 先制技能
class PrioritySkill : public Skill {
public:
    PrioritySkill(const QString& name, ElementType type, SkillCategory category, 
                 int power, int ppCost, int accuracy, int priority);
};

// 固定伤害技能
class FixedDamageSkill : public Skill {
public:
    FixedDamageSkill(const QString& name, ElementType type, 
                    int damage, int ppCost, int accuracy, int priority = 0);
    
    virtual int calculateDamage(Creature* user, Creature* target) override;
    
private:
    int m_fixedDamage; // 固定伤害值
};

// 回复技能
class HealingSkill : public StatusSkill {
public:
    HealingSkill(const QString& name, ElementType type, 
                int ppCost, int accuracy, int healPercent, int priority = 0);
    
    virtual bool use(Creature* user, Creature* target, BattleSystem* battle) override;
    
private:
    int m_healPercent; // 回复百分比
};

// 能力变化技能
class StatChangeSkill : public StatusSkill {
public:
    StatChangeSkill(const QString& name, ElementType type, 
                   int ppCost, int accuracy, int priority = 0);
    
    // 添加能力变化
    void addStatChange(StatType stat, int stages, bool targetSelf);
    
    virtual bool use(Creature* user, Creature* target, BattleSystem* battle) override;
    
private:
    struct StatChange {
        StatType stat;      // 能力类型
        int stages;         // 变化等级
        bool targetSelf;    // 是否针对自己
    };
    
    QVector<StatChange> m_statChanges; // 能力变化列表
};

// 第五技能基类
class FifthSkill : public Skill {
public:
    FifthSkill(const QString& name, ElementType type, SkillCategory category, 
              int power, int ppCost, int accuracy, int priority = 0);
    
    // 检查是否可以使用（第五技能可能有特殊条件）
    virtual bool canUse(Creature* user, Creature* target) const;
};

#endif // SKILL_H
