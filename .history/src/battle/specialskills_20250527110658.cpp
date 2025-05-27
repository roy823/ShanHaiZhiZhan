#include "specialskills.h"
#include "../core/creature.h" // 需要 Creature 类的完整定义，特别是派生类
#include "battlesystem.h"
#include "effect.h"         // 包括 StatChangeEffect, HealingEffect, TurnBasedEffect
#include <QRandomGenerator>
#include <QDebug>

// --- 木棍人技能 ---
IndomitableSpiritSkill::IndomitableSpiritSkill()
    : FifthSkill("不屈战魂", ElementType::NORMAL, SkillCategory::STATUS, 0, 3, 101, 0) // 101命中=必中, 0威力, 0优先级
{
    // 添加效果：提升物攻+2，速度+1，恢复33%HP
    addEffect(new StatChangeEffect(StatType::ATTACK, 2, true)); // true 表示作用于自身
    addEffect(new StatChangeEffect(StatType::SPEED, 1, true));  // true 表示作用于自身
    addEffect(new HealingEffect(33, true, true)); // 治疗量33, 是百分比, 作用于自身
    setDescription("HP低于一半时可用。提升自身物攻等级+2，速度等级+1，并恢复33%最大HP。");
}

// canUse 由 specific FifthSkill 实现
bool IndomitableSpiritSkill::canUse(Creature *user, Creature *target, BattleSystem *battle) const
{
    // Q_UNUSED(target); // 标记未使用
    // Q_UNUSED(battle); // 标记未使用
    if (!user) return false;
    // 检查HP是否低于50%
    return user->getCurrentHP() <= (user->getMaxHP() / 2);
}

QString IndomitableSpiritSkill::getDescription() const
{
    // m_description 已在构造函数中通过 setDescription 设置，或者可以直接返回
    return Skill::getDescription(); // 或者直接返回构造时设置的固定描述
}
Skill* IndomitableSpiritSkill::clone() const { return new IndomitableSpiritSkill(*this); }


// --- 鳄鱼轰炸机技能 ---
AirspaceSupremacySkill::AirspaceSupremacySkill()
    : FifthSkill("空域压制", ElementType::FLYING, SkillCategory::STATUS, 0, 3, 101, 0) // 必中状态技
{
    // Lambda 函数定义，用于回合效果：降低目标速度
    // 参数：效果拥有者(owner_of_effect), 战斗系统(battle_ctx), 当前回合的对手(opponent_ctx), 效果自身(self_effect)
    auto airspaceLambda = [](Creature* owner_of_effect, BattleSystem* battle_ctx, Creature* opponent_on_turn, TurnBasedEffect* self_effect) {
        // Q_UNUSED(owner_of_effect); // 此效果是施加给对方的，owner_of_effect 是对方
        // Q_UNUSED(battle_ctx);    // 此简单效果不需要战斗系统
        // Q_UNUSED(self_effect);   // 不需要效果自身指针

        // 这里的 owner_of_effect 是指“拥有这个回合Debuff的生物”，也就是被施加了技能的目标。
        Creature* actualTarget = owner_of_effect; // 因为这个effect是施加在对方身上的

        if (actualTarget && !actualTarget->isDead()) {
            // battle_ctx 可能是 nullptr，如果 Creature::onTurnEnd 没有传递它
            // if (battle_ctx) battle_ctx->addBattleLog(QString("%1 因空域压制，速度下降!").arg(actualTarget->getName()));
            qDebug() << actualTarget->getName() << "因空域压制，速度下降!";
            actualTarget->modifyStatStage(StatType::SPEED, -1);
        }
    };

    // 创建一个持续3回合的回合开始效果 (debuffEffect)
    TurnBasedEffect *debuffEffect = new TurnBasedEffect(
        "空域压制Debuff", // 效果名称/ID
        3,                  // 持续回合数
        true,               // 回合开始时执行
        airspaceLambda      // 执行的逻辑
    );
    debuffEffect->setTargetSelf(false); // 此效果是施加给对方的
    debuffEffect->setDescription("空域压制：速度持续下降。"); // TurnBasedEffect的描述
    addEffect(debuffEffect); // 将此 "施加回合效果" 的动作添加到技能中

    // 技能的主描述
    setDescription("施加给对手，持续3回合每回合开始降低目标速度等级-1。(场上己方飞行系和机械系攻击威力提升光环需BattleSystem支持)");
}

QString AirspaceSupremacySkill::getDescription() const
{
    return Skill::getDescription();
}
Skill* AirspaceSupremacySkill::clone() const { return new AirspaceSupremacySkill(*this); }

// --- 耐克鲨鱼技能 ---
BlitzPredatorSkill::BlitzPredatorSkill()
    : FifthSkill("极速掠食", ElementType::WATER, SkillCategory::PHYSICAL, 100, 4, 95, 0)
{
    // 主要效果1: 提升自身速度等级+1
    // 这个效果应该在技能命中后立即触发
    addEffect(new StatChangeEffect(StatType::SPEED, 1, true)); // true表示作用于自身
    setDescription("提升自身速度等级+1。若击败目标，则额外提升物攻等级+1。");
}

// 极速掠食的 use 方法
// 基类 Skill::use 会处理命中和主要效果（如上述的速度提升）
// 此处重写 use 是为了处理“若击败目标，额外提升物攻”的逻辑。
// 这个逻辑实际上更适合在 BattleSystem 的伤害结算后，检查目标是否被击败时触发。
bool BlitzPredatorSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    // 调用基类方法执行技能命中判定和基础效果 (如提速)
    if (!FifthSkill::use(user, target, battle)) { // FifthSkill::use 最终调用 Skill::use
        return false; // 未命中或无法使用
    }

    // 伤害计算和目标HP减少由 BattleSystem 处理。
    // BattleSystem 在伤害结算后，应检查 target 是否 isDead()。
    // 如果是，并且造成击败的技能是 BlitzPredatorSkill，则 BattleSystem 应用额外效果。
    // 此处 use 方法返回 true 表示技能成功发动。后续的击败判定在 BattleSystem。

    // 示例：如果BattleSystem提供了回调或状态来检查是否击败
    // if (battle && battle->wasTargetDefeatedByLastSkill(target) && user) {
    //     user->modifyStatStage(StatType::ATTACK, 1);
    //     battle->addBattleLog(QString("%1 击败了 %2，物攻提升了!").arg(user->getName()).arg(target->getName()));
    // }
    // 由于BattleSystem未知，这里仅作标记。技能本身成功发动。
    qDebug() << "BlitzPredatorSkill: 击败目标后提升物攻的逻辑需要BattleSystem在伤害结算后处理。";

    return true;
}

QString BlitzPredatorSkill::getDescription() const
{
    return Skill::getDescription();
}
Skill* BlitzPredatorSkill::clone() const { return new BlitzPredatorSkill(*this); }

// --- 仙人掌大象技能 ---
LifeSiphonFieldSkill::LifeSiphonFieldSkill()
    : FifthSkill("生命汲取领域", ElementType::GRASS, SkillCategory::STATUS, 0, 4, 101, 0) // 必中状态技
{
    // Lambda 定义：领域效果逻辑
    // 参数：效果拥有者(owner_of_effect), 战斗系统(battle_ctx), 当前回合的对手(opponent_ctx), 效果自身(self_effect)
    auto siphonLambda = [](Creature* owner_of_effect, BattleSystem* battle_ctx, Creature* opponent_on_turn, TurnBasedEffect* self_effect) {
        // Q_UNUSED(opponent_on_turn); // 对手信息可能从 battle_ctx 获取
        // Q_UNUSED(self_effect);

        if (!owner_of_effect || owner_of_effect->isDead()) return; // 效果拥有者必须存活

        // BattleSystem 指针非常重要，如果为null，则无法获取对手列表或场上其他精灵
        if (!battle_ctx) {
            qWarning() << "LifeSiphonFieldSkill: BattleSystem 为空，无法执行领域效果。";
            // 尝试只回复自身 HP (如果这是设计的一部分)
            int selfHealAmount = owner_of_effect->getMaxHP() / 8;
            owner_of_effect->heal(selfHealAmount);
            qDebug() << owner_of_effect->getName() << "因生命汲取领域回复了" << selfHealAmount << "HP (BattleSystem缺失，仅自身回复)。";
            return;
        }

        // 对场上所有非草系精灵造成伤害
        // 这需要BattleSystem提供获取场上所有精灵（特别是对手）的方法
        // QVector<Creature*> allOpponents = battle_ctx->getAllOpponentCreatures(owner_of_effect); // 假设有此方法
        // QVector<Creature*> allAllies = battle_ctx->getAllAlliedCreatures(owner_of_effect); // 假设有此方法
        // for (Creature* creature_on_field : allOpponents + allAllies) { // 伪代码
        //伪代码开始：实际需要BattleSystem支持
        Creature* example_opponent = battle_ctx->getOpponentOf(owner_of_effect); // 假设获取一个对手
        if (example_opponent && !example_opponent->isDead()) {
            bool isGrassType = example_opponent->getType().isType(ElementType::GRASS);
            if (!isGrassType) {
                int damage = example_opponent->getMaxHP() / 16;
                example_opponent->takeDamage(damage);
                // battle_ctx->addBattleLog(QString("%1受到了生命汲取领域的%2点伤害!").arg(example_opponent->getName()).arg(damage));
                qDebug() << example_opponent->getName() << "受到了生命汲取领域的" << damage << "点伤害!";
            }
        }
        //伪代码结束

        // 自身回复HP
        int healAmount = owner_of_effect->getMaxHP() / 8;
        owner_of_effect->heal(healAmount);
        // battle_ctx->addBattleLog(QString("%1回复了%2点生命!").arg(owner_of_effect->getName()).arg(healAmount));
        qDebug() << owner_of_effect->getName() << "因生命汲取领域回复了" << healAmount << "HP。";
    };

    TurnBasedEffect *siphonEffect = new TurnBasedEffect(
        "生命汲取领域效果",
        3,      // 持续3回合
        false,  // 回合结束时执行
        siphonLambda
    );
    siphonEffect->setTargetSelf(true); // 这个回合效果是施加在技能使用者身上的，标记其领域状态
    siphonEffect->setDescription("生命汲取领域激活中，每回合吸取生命。");
    addEffect(siphonEffect);

    setDescription("持续3回合，每回合结束时对场上所有非草系精灵造成1/16最大HP伤害，自身回复1/8最大HP。(此效果依赖BattleSystem正确传递上下文)");
}

QString LifeSiphonFieldSkill::getDescription() const
{
    return Skill::getDescription();
}
Skill* LifeSiphonFieldSkill::clone() const { return new LifeSiphonFieldSkill(*this); }


// --- 香蕉绿猩猩技能 ---
JungleKingStrikeSkill::JungleKingStrikeSkill()
    : FifthSkill("丛林之王强击", ElementType::GRASS, SkillCategory::PHYSICAL, 130, 5, 90, 0)
{
    setDescription("威力130的强力物理攻击。使用者处于狂暴形态时威力提升20%。");
}

// 重写 calculateDamage 以处理狂暴形态的威力加成
int JungleKingStrikeSkill::calculateDamage(Creature *user, Creature *target)
{
    // Q_UNUSED(target); // target 在此特定逻辑中未使用，但在通用伤害公式中会使用

    if (!user) return 0;

    // 先获取基础威力 (来自Skill基类的getPower，它返回m_power)
    int currentPower = Skill::getPower(user, target); // 等效于 m_power

    // 检查使用者是否为香蕉绿猩猩，并且处于狂暴形态
    // 需要 dynamic_cast 来安全地检查类型并访问派生类成员
    const ChimpanziniBananini *chimp = dynamic_cast<const ChimpanziniBananini *>(user);
    if (chimp && chimp->isInBerserkForm()) {
        // 若处于狂暴形态，则威力提升20%
        // battle->addBattleLog(QString("%1的狂暴形态增强了丛林之王强击的威力!").arg(user->getName())); // 日志应由BattleSystem处理
        qDebug() << user->getName() << "的狂暴形态增强了丛林之王强击的威力!";
        currentPower = qRound(currentPower * 1.20); // 威力增加20%
    }
    
    // 返回的是修正后的“威力”，最终伤害由BattleSystem根据此威力和其他因素（攻防、属性等）计算
    // 注意：Skill::calculateDamage 通常被 BattleSystem 用来获取技能的基础威力部分。
    // 如果 BattleSystem 使用 getPower() 而不是 calculateDamage() 来获取威力，
    // 那么应该重写 getPower()。当前框架下，重写 calculateDamage 更直接。
    return currentPower;
}

// bool JungleKingStrikeSkill::use(Creature *user, Creature *target, BattleSystem *battle)
// {
//     // 基类 FifthSkill::use (即 Skill::use) 已足够处理命中等。
//     // 威力加成在 calculateDamage 中处理。
//     // 如果有其他特殊发动逻辑，才需要重写 use。
//     bool result = FifthSkill::use(user, target, battle);
//     if (result && battle) {
//         ChimpanziniBananini *chimp = dynamic_cast<ChimpanziniBananini *>(user);
//         if (chimp && chimp->isInBerserkForm()) {
//             // battle->addBattleLog(QString("%1的狂暴形态增强了丛林之王强击的威力!").arg(user->getName()));
//         }
//     }
//     return result;
// }

QString JungleKingStrikeSkill::getDescription() const
{
    return Skill::getDescription();
}
Skill* JungleKingStrikeSkill::clone() const { return new JungleKingStrikeSkill(*this); }


// --- 鹿管鹿管鹿鹿时间到了技能 ---
TemporalParadoxSkill::TemporalParadoxSkill()
    : FifthSkill("时间悖论", ElementType::NORMAL, SkillCategory::STATUS, 0, 5, 101, 0) // 必中状态技
{
    setDescription("记录当前场上各精灵状态。3回合后，若此精灵仍在场上，有50%几率将所有精灵恢复到记录时的状态。若失败，则自身陷入疲惫1回合。(此技能实现非常复杂，依赖BattleSystem)");
}

bool TemporalParadoxSkill::use(Creature *user, Creature *target, BattleSystem *battle)
{
    // Q_UNUSED(target); // 此技能通常不直接作用于选定目标，而是全场

    // 先执行基类use，主要是参数检查
    if (!FifthSkill::use(user, target, battle)) { // target 可能为 nullptr
        return false;
    }
    
    if (user && battle) {
        // 安全进行动态类型转换
        Luguanluguanlulushijiandaole *deer = dynamic_cast<Luguanluguanlulushijiandaole *>(user);
        if (deer) {
            // 指示 BattleSystem (或精灵自身，如果它能访问全局状态) 记录当前场上状态
            // battle->recordGlobalBattleStateForTemporalParadox(deer);
            // deer->recordBattleState(battle); // 或者精灵自己记录，需要battle访问其他精灵
            qDebug() << deer->getName() << "使用了时间悖论，记录了战场状态 (具体实现需BattleSystem支持)。";
            // battle->addBattleLog(QString("%1记录了当前战场状态!").arg(deer->getName()));

            // TODO: 设置一个3回合后的触发器/效果，该效果有50%几率调用
            // battle->revertToRecordedState(deer) 或 deer->tryRevertBattleState(battle)
            // 若失败，则 user->setStatusCondition(StatusCondition::TIRED); battle->addLog(...)
            // 这通常通过一个特殊的 TurnBasedEffect 来实现。
        } else {
             qWarning("TemporalParadoxSkill used by a non-Luguanluguanlulushijiandaole creature!");
            return false;
        }
    } else {
        return false;
    }
    
    return true;
}

QString TemporalParadoxSkill::getDescription() const
{
    return Skill::getDescription();
}
Skill* TemporalParadoxSkill::clone() const { return new TemporalParadoxSkill(*this); }

// --- 卡布奇诺忍者技能 ---
PhantomAssassinateSkill::PhantomAssassinateSkill()
    : FifthSkill("绝影刺杀", ElementType::SHADOW, SkillCategory::PHYSICAL, 90, 4, 101, 1) // 101命中=必中, 优先级+1
{
    setDescription("先制+1。若目标HP高于75%，则此技能威力提升50%；若目标HP低于25%，则此技能必定暴击。");
}

// 判断是否应该强制暴击
bool PhantomAssassinateSkill::shouldForceCriticalHit(const Creature* user, const Creature* target) const
{
    // Q_UNUSED(user); // 使用者参数暂时未在此逻辑中用到
    if (!target) return false;
    
    // 如果目标HP低于25% (例如 MaxHP 100, CurrentHP < 25)
    return target->getCurrentHP() < (target->getMaxHP() * 0.25);
}

// 获取技能的实际威力 (重写基类方法)
// 基类 Skill::getPower(user, target) 已被声明为 virtual
int PhantomAssassinateSkill::getPower(const Creature* user, const Creature* target) const
{
    // Q_UNUSED(user); // 使用者参数暂时未在此逻辑中用到
    
    int currentPower = Skill::getPower(user, target); // 获取基础威力 (m_power)

    if (!target) return currentPower; // 没有目标则返回基础威力

    // 如果目标HP高于75% (例如 MaxHP 100, CurrentHP > 75)
    if (target->getCurrentHP() > (target->getMaxHP() * 0.75)) {
        // battle->addBattleLog("目标HP较高，绝影刺杀威力提升!"); // 日志应由BattleSystem处理
        qDebug() << "绝影刺杀：目标HP较高，威力提升!";
        return qRound(currentPower * 1.5); // 威力提升50%
    }
    return currentPower; // 其他情况返回基础威力
}

QString PhantomAssassinateSkill::getDescription() const
{
    return Skill::getDescription();
}
Skill* PhantomAssassinateSkill::clone() const { return new PhantomAssassinateSkill(*this); }