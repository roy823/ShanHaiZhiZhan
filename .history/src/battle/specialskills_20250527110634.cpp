#include "specialskills.h"
#include "../core/creature.h"
#include "battlesystem.h"  
#include "effect.h"
#include <QRandomGenerator>

// --- 木棍人技能 ---
IndomitableSpiritSkill::IndomitableSpiritSkill()
    : FifthSkill("不屈战魂", ElementType::NORMAL, SkillCategory::STATUS, 0, 3, 100)
{
    addEffect(new StatChangeEffect(StatType::ATTACK, 2, true));
    addEffect(new StatChangeEffect(StatType::SPEED, 1, true));
    addEffect(new HealingEffect(33, true)); // 恢复33%最大HP
}

bool IndomitableSpiritSkill::canUse(Creature *user, Creature *target, BattleSystem *battle) const
{
    // 忽略未使用的参数
    Q_UNUSED(target);
    Q_UNUSED(battle);
    
    // 检查HP是否低于50%
    return user && user->getCurrentHP() <= user->getMaxHP() / 2;
}

QString IndomitableSpiritSkill::getDescription() const
{
    return "HP低于一半时可用。提升自身物攻等级+2，速度等级+1，并恢复33%最大HP。";
}

// --- 鳄鱼轰炸机技能 ---
AirspaceSupremacySkill::AirspaceSupremacySkill()
    : FifthSkill("空域压制", ElementType::FLYING, SkillCategory::STATUS, 0, 3, 100)
{
    // 优化Lambda函数，移除未使用的参数名
    auto airspaceLambda = [](Creature *, Creature *target, BattleSystem *, TurnBasedEffect *)
    {
        if (target)
        {
            target->modifyStatStage(StatType::SPEED, -1);
        }
    };
    
    TurnBasedEffect *debuffEffect = new TurnBasedEffect(3, airspaceLambda, true);
    debuffEffect->setTargetSelf(false);
    debuffEffect->setDescription("空域压制：速度下降");
    addEffect(debuffEffect);
}

QString AirspaceSupremacySkill::getDescription() const
{
    return "施加给对手，持续3回合每回合开始降低目标速度等级-1。场上己方飞行系和机械系攻击威力+20%。";
}

// --- 耐克鲨鱼技能 ---
BlitzPredatorSkill::BlitzPredatorSkill()
    : FifthSkill("极速掠食", ElementType::WATER, SkillCategory::PHYSICAL, 100, 4, 95)
{
    addEffect(new StatChangeEffect(StatType::SPEED, 1, true));
}

bool BlitzPredatorSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    // 首先调用基类方法执行技能效果
    bool result = FifthSkill::use(user, target, battle);
    
    // 安全检查
    if (result && user && target && battle && target->isDead())
    {
        // 若击败目标，额外提升物攻等级+1
        user->modifyStatStage(StatType::ATTACK, 1);
        //battle->addBattleLog(QString("%1的物攻等级提升了!").arg(user->getName()), user);
    }
    
    return result;
}

QString BlitzPredatorSkill::getDescription() const
{
    return "提升自身速度等级+1。若击败目标，则额外提升物攻等级+1。";
}

// --- 仙人掌大象技能 ---
LifeSiphonFieldSkill::LifeSiphonFieldSkill()
    : FifthSkill("生命汲取领域", ElementType::GRASS, SkillCategory::STATUS, 0, 4, 100)
{
    auto siphonLambda = [](Creature *source, Creature *, BattleSystem *battle, TurnBasedEffect *)
    {
        if (!battle || !source) return;
        
        // 获取当前对手
        Creature *opponent = battle->getOpponentActiveCreature() == source ? 
                            battle->getPlayerActiveCreature() : battle->getOpponentActiveCreature();
                              
        if (opponent && !opponent->isDead())
        {
            // 检查对手是否为草系精灵
            bool isGrassType = 
                opponent->getType().getPrimaryType() == ElementType::GRASS ||
                opponent->getType().getSecondaryType() == ElementType::GRASS;
                
            // 非草系精灵受到伤害
            if (!isGrassType) {
                int damage = opponent->getMaxHP() / 16;
                opponent->takeDamage(damage);
                //battle->addLogEntry(QString("%1受到了生命汲取领域的%2点伤害!").arg(
                //opponent->getName().arg(damage);
            }
        }
        
        // 自身回复HP
        int healAmount = source->getMaxHP() / 8;
        source->heal(healAmount);
        //battle->addLogEntry(QString("%1回复了%2点生命!").arg(source->getName()).arg(healAmount));
    };
    
    TurnBasedEffect *siphonEffect = new TurnBasedEffect(3, siphonLambda, false);
    siphonEffect->setDescription("生命汲取领域激活中");
    siphonEffect->setTargetSelf(true);
    addEffect(siphonEffect);
}

QString LifeSiphonFieldSkill::getDescription() const
{
    return "持续3回合，每回合结束时对场上所有非草系精灵造成1/16最大HP伤害，自身回复1/8最大HP。";
}

// --- 香蕉绿猩猩技能 ---
JungleKingStrikeSkill::JungleKingStrikeSkill()
    : FifthSkill("丛林之王强击", ElementType::GRASS, SkillCategory::PHYSICAL, 130, 5, 90)
{
    // 构造函数无需额外操作
}

int JungleKingStrikeSkill::calculateDamage(Creature *user, Creature *target)
{
    // 安全检查
    if (!user || !target) {
        return 0;  // 如果用户或目标为空，返回0伤害
    }
    
    // 使用正确的基类方法计算基础伤害
    int baseDamage = Skill::calculateDamage(user, target);
    
    // 安全地处理动态类型转换
    ChimpanziniBananini *chimp = dynamic_cast<ChimpanziniBananini *>(user);
    if (chimp && chimp->isInBerserkForm())
    {
        // 若处于狂暴形态，则威力提升20%
        baseDamage = baseDamage * 6 / 5; // 增加20%
    }
    
    return baseDamage;
}

bool JungleKingStrikeSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    // 先执行基类方法
    bool result = FifthSkill::use(user, target, battle);
    
    if (result && battle)
    {
        // 检查是否是香蕉绿猩猩在使用技能
        ChimpanziniBananini *chimp = dynamic_cast<ChimpanziniBananini *>(user);
        if (chimp && chimp->isInBerserkForm() && battle)
        {
            // 可以添加一条战斗日志，表明狂暴形态增强了技能威力
            //battle->addBattleLog(QString("%1的狂暴形态增强了丛林之王强击的威力!").arg(user->getName()), user);
        }
    }
    
    return result;
}

QString JungleKingStrikeSkill::getDescription() const
{
    return "威力130的强力物理攻击。狂暴形态下威力提升20%。";
}

// --- 鹿管鹿管鹿鹿时间到了技能 ---
TemporalParadoxSkill::TemporalParadoxSkill()
    : FifthSkill("时间悖论", ElementType::NORMAL, SkillCategory::STATUS, 0, 5, 100)
{
    // 构造函数无需额外操作
}

bool TemporalParadoxSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    // 先执行基类方法
    bool result = FifthSkill::use(user, target, battle);
    
    // 安全检查和记录战场状态
    if (result && user && battle)
    {
        // 安全进行动态类型转换
        Luguanluguanlulushijiandaole *deer = 
                dynamic_cast<Luguanluguanlulushijiandaole *>(user);
        if (deer)
        {
            // 记录当前场上状态
            deer->recordBattleState();
            //battle->addLogEntry(QString("%1记录了当前战场状态!").arg(deer->getName()));
        }
    }
    
    return result;
}

QString TemporalParadoxSkill::getDescription() const
{
    return "记录当前场上各精灵状态。3回合后，若此精灵仍在场上，有50%几率将所有精灵恢复到记录时的状态。若失败，则自身陷入疲惫1回合。";
}

// --- 卡布奇诺忍者技能 ---
/**
 * 构造函数初始化绝影刺杀技能
 * 设置为物理技能，威力90，优先级+1，必中
 */
PhantomAssassinateSkill::PhantomAssassinateSkill()
    : FifthSkill("绝影刺杀", ElementType::SHADOW, SkillCategory::PHYSICAL, 90, 4, 101, 1) // 101命中=必中, 优先级+1
{
    // 构造函数无需额外逻辑
}

/**
 * 判断是否应该强制暴击
 * 
 * @param user 技能使用者
 * @param target 技能目标
 * @return 当目标HP低于25%时返回true，否则返回false
 */
bool PhantomAssassinateSkill::shouldForceCriticalHit(const Creature* user, const Creature* target) const
{
    Q_UNUSED(user); // 暂时用不到使用者参数
    
    // 安全检查
    if (!target) return false;
    
    // 如果目标HP低于25%，必定暴击
    if (target->getCurrentHP() < target->getMaxHP() * 25 / 100) {
        return true;
    }
    return false;
}

/**
 * 获取技能的实际威力，根据目标HP计算
 * 
 * @param user 技能使用者
 * @param target 技能目标
 * @return 基础威力或提升50%后的威力值
 */
int PhantomAssassinateSkill::getPower(const Creature* user, const Creature* target) const
{
    Q_UNUSED(user); // 暂时用不到使用者参数
    
    // 安全检查
    if (!target) return m_power;
    
    // 如果目标HP高于75%，威力提升50%
    if (target->getCurrentHP() > target->getMaxHP() * 75 / 100) {
        return m_power * 3 / 2; // 威力提升50%
    }
    return m_power;
}

/**
 * 返回技能的详细描述
 */
QString PhantomAssassinateSkill::getDescription() const
{
    return "先制+1。若目标HP高于75%，则此技能威力提升50%；若目标HP低于25%，则此技能必定暴击。";
}