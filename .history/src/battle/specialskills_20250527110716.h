#ifndef SPECIALSKILLS_H
#define SPECIALSKILLS_H

#include "skill.h"
#include "../core/creature.h" // ChimpanziniBananini, Luguanluguanlulushijiandaole 等定义需要

class BattleSystem;

/**
 * 木棍人的第五技能：不屈战魂
 * HP低于一半时可用。提升自身物攻等级+2，速度等级+1，并恢复33%最大HP。
 */
class IndomitableSpiritSkill : public FifthSkill {
public:
    IndomitableSpiritSkill();
    bool canUse(Creature *user, Creature *target, BattleSystem *battle) const override;
    QString getDescription() const override;
    Skill* clone() const override;
};

/**
 * 鳄鱼轰炸机的第五技能：空域压制
 * 施加给对手，持续3回合每回合开始降低目标速度等级-1。
 * (场上己方飞行系和机械系攻击威力+20% -> 这个效果需要BattleSystem支持，当前技能不直接实现)
 */
class AirspaceSupremacySkill : public FifthSkill {
public:
    AirspaceSupremacySkill();
    QString getDescription() const override;
    Skill* clone() const override;
    // 注意：威力提升20%的光环效果未在此技能的Effect中实现，需要BattleSystem配合
};

/**
 * 耐克鲨鱼的第五技能：极速掠食
 * 提升自身速度等级+1。若击败目标，则额外提升物攻等级+1。
 */
class BlitzPredatorSkill : public FifthSkill {
public:
    BlitzPredatorSkill();
    // use 方法需要检查击败目标后的逻辑，这通常由BattleSystem在伤害结算后回调或通知
    bool use(Creature *user, Creature *target, BattleSystem *battle) override;
    QString getDescription() const override;
    Skill* clone() const override;
};

/**
 * 仙人掌大象的第五技能：生命汲取领域
 * 持续3回合，每回合结束时对场上所有非草系精灵造成1/16最大HP伤害，自身回复1/8最大HP。
 * (此效果需要BattleSystem支持，因涉及全场和特定条件判断)
 */
class LifeSiphonFieldSkill : public FifthSkill {
public:
    LifeSiphonFieldSkill();
    QString getDescription() const override;
    Skill* clone() const override;
    // 注意：领域效果的实现依赖于TurnBasedEffect的lambda能够正确访问BattleSystem和对手信息
    // 如果BattleSystem不能提供这些上下文给Creature::onTurnEnd, 此效果可能无法完全按描述工作
};

/**
 * 香蕉绿猩猩的第五技能：丛林之王强击
 * 威力130的强力物理攻击。狂暴形态下威力提升20%。
 */
class JungleKingStrikeSkill : public FifthSkill {
public:
    JungleKingStrikeSkill();
    // calculateDamage 已在基类Skill中声明为虚函数，这里重写以处理狂暴形态的威力加成
    int calculateDamage(Creature *user, Creature *target) override;
    // bool use(Creature *user, Creature *target, BattleSystem *battle) override; // 基类use已足够，除非有特殊发动逻辑
    QString getDescription() const override;
    Skill* clone() const override;
};

/**
 * 鹿管鹿管鹿鹿时间到了的第五技能：时间悖论
 * 记录当前场上各精灵状态。3回合后，若此精灵仍在场上，有50%几率将所有精灵恢复到记录时的状态。
 * 若失败，则自身陷入疲惫1回合。(此技能非常复杂，需要BattleSystem深度支持状态快照和回溯)
 */
class TemporalParadoxSkill : public FifthSkill {
public:
    TemporalParadoxSkill();
    bool use(Creature *user, Creature *target, BattleSystem *battle) override;
    QString getDescription() const override;
    Skill* clone() const override;
    // 注意：此技能的实现复杂度非常高，远超普通技能效果。
};

/**
 * 卡布奇诺忍者的第五技能：绝影刺杀
 * 先制+1。若目标HP高于75%，则此技能威力提升50%；若目标HP低于25%，则此技能必定暴击。
 */
class PhantomAssassinateSkill : public FifthSkill {
public:
    PhantomAssassinateSkill();
    
    // 判断是否应该强制暴击
    bool shouldForceCriticalHit(const Creature* user, const Creature* target) const;
    
    // 获取技能的实际威力 (重写基类方法)
    int getPower(const Creature* user, const Creature* target) const override;
    
    QString getDescription() const override;
    Skill* clone() const override;
};

#endif // SPECIALSKILLS_H