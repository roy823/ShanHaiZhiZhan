#ifndef SPECIALSKILLS_H
#define SPECIALSKILLS_H

#include "skill.h"

// 第五技能基类
class FifthSkill : public Skill {
public:
    FifthSkill(const QString &name, ElementType type, SkillCategory category,
               int power, int ppCost, int accuracy, int priority = 0);
    
    // 检查第五技能是否可以使用
    virtual bool canUse(Creature *user, Creature *target, BattleSystem* battle) const;
    
    // 获取技能的实际威力（可被子类重写以实现特殊效果）
    virtual int getPower(const Creature* user, const Creature* target) const;
    
    // 检查是否应该强制暴击（可被子类重写）
    virtual bool shouldForceCriticalHit(const Creature* user, const Creature* target) const;
};

// 绝影刺杀技能类
class PhantomAssassinateSkill : public FifthSkill {
public:
    PhantomAssassinateSkill();
    
    // 重写基类方法
    bool shouldForceCriticalHit(const Creature* user, const Creature* target) const override;
    int getPower(const Creature* user, const Creature* target) const override;
    QString getDescription() const override;
};

// 不屈战魂技能类
class IndomitableSpiritSkill : public FifthSkill {
public:
    IndomitableSpiritSkill();
    
    bool canUse(Creature *user, Creature *target, BattleSystem* battle) const override;
    QString getDescription() const override;
};

// 空域压制技能类
class AirspaceSupremacySkill : public FifthSkill {
public:
    AirspaceSupremacySkill();
    
    QString getDescription() const override;
};

// 极速掠食技能类
class BlitzPredatorSkill : public FifthSkill {
public:
    BlitzPredatorSkill();
    
    QString getDescription() const override;
};

// 生命汲取领域技能类
class LifeSiphonFieldSkill : public FifthSkill {
public:
    LifeSiphonFieldSkill();
    
    QString getDescription() const override;
};

// 丛林之王强击技能类
class JungleKingStrikeSkill : public FifthSkill {
public:
    JungleKingStrikeSkill();
    
    QString getDescription() const override;
};

// 时间悖论技能类
class TemporalParadoxSkill : public FifthSkill {
public:
    TemporalParadoxSkill();
    
    QString getDescription() const override;
};

#endif // SPECIALSKILLS_H