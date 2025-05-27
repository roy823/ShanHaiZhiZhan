#include "specialskills.h"
#include "../core/creature.h"

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