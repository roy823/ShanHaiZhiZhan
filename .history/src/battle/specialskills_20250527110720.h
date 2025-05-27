#ifndef SPECIALSKILLS_H
#define SPECIALSKILLS_H

#include "skill.h"

// 绝影刺杀技能类
class PhantomAssassinateSkill : public FifthSkill {
public:
    PhantomAssassinateSkill();
    
    // 检查是否应该强制暴击
    bool shouldForceCriticalHit(const Creature* user, const Creature* target) const;
    
    // 获取技能的实际威力
    int getPower(const Creature* user, const Creature* target) const;
    
    // 重写描述
    QString getDescription() const override;
};

#endif // SPECIALSKILLS_H