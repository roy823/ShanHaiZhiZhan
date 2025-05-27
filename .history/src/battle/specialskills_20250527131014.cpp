#include "specialskills.h"
#include "../core/creature.h"
#include "battlesystem.h"

// FifthSkill 基类实现
FifthSkill::FifthSkill(const QString &name, ElementType type, SkillCategory category,
                       int power, int ppCost, int accuracy, int priority)
    : Skill(name, type, category, power, ppCost, accuracy, priority)
{
    // 第五技能的共同初始化
}

bool FifthSkill::canUse(Creature *user, Creature *target, BattleSystem* battle) const
{
    return user != nullptr; // 基础检查
}

int FifthSkill::getPower(const Creature* user, const Creature* target) const
{
    return m_power; // 默认返回基础威力
}

bool FifthSkill::shouldForceCriticalHit(const Creature* user, const Creature* target) const
{
    return false; // 默认不强制暴击
}

// PhantomAssassinateSkill 实现
PhantomAssassinateSkill::PhantomAssassinateSkill()
    : FifthSkill("绝影刺杀", ElementType::SHADOW, SkillCategory::PHYSICAL, 90, 4, 101, 1) // 101命中=必中, 优先级+1
{
}

bool PhantomAssassinateSkill::shouldForceCriticalHit(const Creature* user, const Creature* target) const
{
    if (!target) return false;
    
    // 如果目标HP低于25%，必定暴击
    if (target->getCurrentHP() < target->getMaxHP() * 25 / 100) {
        return true;
    }
    return false;
}

int PhantomAssassinateSkill::getPower(const Creature* user, const Creature* target) const
{
    if (!target) return m_power;
    
    // 如果目标HP高于75%，威力提升50%
    if (target->getCurrentHP() > target->getMaxHP() * 75 / 100) {
        return m_power * 3 / 2; // 威力提升50%
    }
    return m_power;
}

QString PhantomAssassinateSkill::getDescription() const
{
    return "先制+1。若目标HP高于75%，则此技能威力提升50%；若目标HP低于25%，则此技能必定暴击。";
}

// IndomitableSpiritSkill 实现
IndomitableSpiritSkill::IndomitableSpiritSkill()
    : FifthSkill("不屈战魂", ElementType::NORMAL, SkillCategory::STATUS, 0, 3, 100)
{
    addEffect(new StatChangeEffect(StatType::ATTACK, 2, true));
    addEffect(new StatChangeEffect(StatType::SPEED, 1, true));
    addEffect(new HealingEffect(33, true)); // 恢复33%最大HP (true表示百分比)
}

bool IndomitableSpiritSkill::canUse(Creature *user, Creature *target, BattleSystem* battle) const
{
    return user && user->getCurrentHP() < (user->getMaxHP() / 2);
}

QString IndomitableSpiritSkill::getDescription() const
{
    return "当HP低于50%时可使用。提升物攻+2，速度+1，并恢复33%最大HP。";
}

// AirspaceSupremacySkill 实现
AirspaceSupremacySkill::AirspaceSupremacySkill()
    : FifthSkill("空域压制", ElementType::FLYING, SkillCategory::STATUS, 0, 3, 100)
{
    auto airspaceLambda = [](Creature *source, Creature *affectedCreature, BattleSystem *battle, TurnBasedEffect *effect)
    {
        // affectedCreature 是效果附着的精灵，这里是对手(因为setTargetSelf(false))
        if (!affectedCreature) return;
        
        // 获取修改前的速度阶段
        int oldSpeedStage = affectedCreature->getStatStages().getStage(StatType::SPEED);
        
        // 降低对手速度
        affectedCreature->modifyStatStage(StatType::SPEED, -1);
        
        // 使用触发器方法生成统一的日志
        battle->triggerStatStageChanged(affectedCreature, StatType::SPEED, oldSpeedStage, 
                                       affectedCreature->getStatStages().getStage(StatType::SPEED));
        
        // TODO: 这里可以添加对己方飞行系和机械系精灵攻击威力提升的效果
        // (根据描述文本，这部分功能似乎尚未实现)
    };
    
    TurnBasedEffect *debuffEffect = new TurnBasedEffect(3, airspaceLambda, true);
    debuffEffect->setTargetSelf(false);  // 效果附着在对手身上
    debuffEffect->setDescription("空域压制：速度下降");
    debuffEffect->setOriginalSource(nullptr);  // 会在apply时自动设置
    addEffect(debuffEffect);
}

QString AirspaceSupremacySkill::getDescription() const
{
    return "施加一个持续3回合的领域效果，使对方速度下降，己方飞行系和机械系精灵攻击威力提升20%。";
}

// BlitzPredatorSkill 实现
BlitzPredatorSkill::BlitzPredatorSkill()
    : FifthSkill("极速掠食", ElementType::WATER, SkillCategory::PHYSICAL, 100, 4, 95)
{
    addEffect(new StatChangeEffect(StatType::SPEED, 1, true));
}

QString BlitzPredatorSkill::getDescription() const
{
    return "威力100的物理攻击，提升自身速度+1。若击败目标，则额外提升物攻+1。";
}

// LifeSiphonFieldSkill 实现
LifeSiphonFieldSkill::LifeSiphonFieldSkill()
    : FifthSkill("生命汲取领域", ElementType::GRASS, SkillCategory::STATUS, 0, 4, 100)
{
    auto siphonLambda = [](Creature *source, Creature *affectedCreature, BattleSystem *battle, TurnBasedEffect *effect)
    {
        if (!battle || !source || !affectedCreature)
            return;
        
        // 正确获取对手
        Creature *opponent = battle->getOpponentActiveCreature() == source ? 
                         battle->getPlayerActiveCreature() : 
                         battle->getOpponentActiveCreature();
                         
        if (opponent && !opponent->isDead())
        {
            // 只对非草系精灵造成伤害
            if (opponent->getType().getPrimaryType() != ElementType::GRASS &&
                (opponent->getType().getSecondaryType() == ElementType::NONE || 
                 opponent->getType().getSecondaryType() != ElementType::GRASS))
            {
                opponent->takeDamage(opponent->getMaxHP() / 16);
            }
        }
        
        // 回复使用者生命值
        if (!affectedCreature->isDead()) {
            int healAmount = affectedCreature->getMaxHP() / 8;
            affectedCreature->heal(healAmount);
        }
    };
    
    TurnBasedEffect *siphonEffect = new TurnBasedEffect(3, siphonLambda, false);
    siphonEffect->setDescription("生命汲取领域激活中");
    siphonEffect->setTargetSelf(true);  // 效果附着在使用者身上
    siphonEffect->setOriginalSource(nullptr);  // 会在apply时自动设置
    addEffect(siphonEffect);
}

QString LifeSiphonFieldSkill::getDescription() const
{
    return "持续3回合，对场上所有非草系精灵造成最大HP的1/16伤害，自身每回合恢复最大HP的1/8。";
}

// JungleKingStrikeSkill 实现
JungleKingStrikeSkill::JungleKingStrikeSkill()
    : FifthSkill("丛林之王强击", ElementType::GRASS, SkillCategory::PHYSICAL, 130, 5, 90)
{
}

QString JungleKingStrikeSkill::getDescription() const
{
    return "威力130的强力物理攻击。";
}

// TemporalParadoxSkill 实现
TemporalParadoxSkill::TemporalParadoxSkill()
    : FifthSkill("时间悖论", ElementType::NORMAL, SkillCategory::STATUS, 0, 5, 100)
{
}

QString TemporalParadoxSkill::getDescription() const
{
    return "记录当前场上双方精灵的状态。3回合后，若此精灵仍在场，则有50%几率将场上所有精灵的状态恢复到记录时的状态。若发动失败，则自身陷入疲惫1回合。";
}