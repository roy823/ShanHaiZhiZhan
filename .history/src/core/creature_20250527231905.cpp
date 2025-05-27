// src/core/creature.cpp
#include "creature.h"
#include "../battle/battlesystem.h" // 战斗系统，主要用于效果函数签名
#include "../battle/effect.h"       // 效果类，用于创建具体效果实例
#include "../battle/skill.h"        // 技能类，用于技能相关操作
#include "../battle/specialskills.h" // 特殊技能类，包含第五技能的实现
#include <QRandomGenerator>         // Qt随机数
#include <QDateTime>                // Qt日期时间 (如果需要)
#include <QtMath>                   // Qt数学函数 (例如 qMax, qMin)

// --- Creature基类实现 ---

// 构造函数
Creature::Creature(const QString &name, const Type &type, int level)
    : m_name(name),                         // 初始化精灵名称
      m_type(type),                         // 初始化精灵属性
      m_level(qBound(1, level, MAX_LEVEL)), // 初始化等级，并确保在1到MAX_LEVEL之间
      m_experience(0),                      // 初始经验值为0
      // m_baseStats 默认构造 (如果需要，可以在这里或之后设置)
      // m_statStages 默认构造 (全为0)
      // m_talent 默认构造 (全为1)
      m_fifthSkill(nullptr), // 初始没有第五技能
      m_currentHP(1),        // HP相关会在setBaseStats或updateStatsOnLevelUp后正确设置
      m_maxHP(1),
      m_currentPP(8),                          // 初始PP值，根据设计文档设为8
      m_maxPP(8),                              // 最大PP值，根据设计文档设为8
      m_statusCondition(StatusCondition::NONE) // 初始无异常状态
{
    // 中文注释：精灵基类构造函数
    // name: 精灵名称
    // type: 精灵的属性 (Type对象)
    // level: 精灵的初始等级

    // 通常在构造函数后会立即调用 setBaseStats 和 setTalent
    // 以及学习初始技能
}

// 析构函数
Creature::~Creature()
{
    // 中文注释：精灵基类析构函数
    // 释放动态分配的技能对象
    qDeleteAll(m_skills); // QVector<Skill*> m_skills;
    m_skills.clear();

    // 释放第五技能对象 (如果存在)
    if (m_fifthSkill)
    {
        delete m_fifthSkill;
        m_fifthSkill = nullptr;
    }

    // 释放所有回合类效果对象
    qDeleteAll(m_turnEffects); // QVector<TurnBasedEffect*> m_turnEffects;
    m_turnEffects.clear();
}

// 获取精灵名称
QString Creature::getName() const
{
    return m_name;
}

QString Creature::getResourceName() const {
    return m_name.toLower().replace(' ', '_'); // 默认行为：将名称转小写并替换空格
}

// 获取精灵属性 (Type对象)
Type Creature::getType() const
{
    return m_type;
}

// 获取精灵当前等级
int Creature::getLevel() const
{
    return m_level;
}

// 获取精灵当前经验值
int Creature::getExperience() const
{
    return m_experience;
}

// 获取到下一级所需经验值
int Creature::getExperienceToNextLevel() const
{
    return calculateExperienceToNextLevel(); // 调用私有辅助函数计算
}

// 获取精灵的基础属性 (不受战斗中能力等级变化影响)
BaseStats Creature::getBaseStats() const
{
    return m_baseStats;
}

// 获取精灵当前战斗中的实际属性 (应用了能力等级修正)
BaseStats Creature::getCurrentStats() const
{
    BaseStats currentStats = m_baseStats; // 从基础属性开始

    // 应用各项能力等级的修正
    // StatType::HP 通常不参与能力等级修正
    currentStats.modifyStat(StatType::ATTACK, qRound(m_baseStats.getStat(StatType::ATTACK) * (StatStages::calculateModifier(StatType::ATTACK, m_statStages.getStage(StatType::ATTACK)) - 1.0)));
    currentStats.modifyStat(StatType::DEFENSE, qRound(m_baseStats.getStat(StatType::DEFENSE) * (StatStages::calculateModifier(StatType::DEFENSE, m_statStages.getStage(StatType::DEFENSE)) - 1.0)));
    currentStats.modifyStat(StatType::SP_ATTACK, qRound(m_baseStats.getStat(StatType::SP_ATTACK) * (StatStages::calculateModifier(StatType::SP_ATTACK, m_statStages.getStage(StatType::SP_ATTACK)) - 1.0)));
    currentStats.modifyStat(StatType::SP_DEFENSE, qRound(m_baseStats.getStat(StatType::SP_DEFENSE) * (StatStages::calculateModifier(StatType::SP_DEFENSE, m_statStages.getStage(StatType::SP_DEFENSE)) - 1.0)));
    currentStats.modifyStat(StatType::SPEED, qRound(m_baseStats.getStat(StatType::SPEED) * (StatStages::calculateModifier(StatType::SPEED, m_statStages.getStage(StatType::SPEED)) - 1.0)));
    // 命中和闪避的修正在战斗系统判定时应用，不直接修改属性值

    return currentStats;
}

// 获取精灵当前的能力等级阶段 (如物攻+1, 速度-2等)
StatStages Creature::getStatStages() const
{
    return m_statStages;
}

// 获取精灵的天赋信息
Talent Creature::getTalent() const
{
    return m_talent;
}

// 获取精灵当前的异常状态
StatusCondition Creature::getStatusCondition() const
{
    return m_statusCondition;
}

// 设置精灵的基础属性
void Creature::setBaseStats(const BaseStats &stats)
{
    m_baseStats = stats;
    // 当基础属性设定后，需要更新最大HP，并确保当前HP不超过最大HP
    m_maxHP = m_baseStats.getStat(StatType::HP);
    m_currentHP = qMin(m_currentHP, m_maxHP); // 如果之前HP高于新的maxHP，则调整
    if (m_currentHP <= 0 && m_maxHP > 0)
        m_currentHP = 1; // 如果设置后精灵濒死，至少保留1HP（除非maxHP为0）
    if (m_maxHP == 0)
        m_currentHP = 0;
}

// 设置精灵的天赋
void Creature::setTalent(const Talent &talent)
{
    m_talent = talent;
}

// 设置精灵的最大PP值
void Creature::setMaxPP(int maxPP)
{
    m_maxPP = qMax(0, maxPP);                 // PP值不应为负
    m_currentPP = qMin(m_currentPP, m_maxPP); // 确保当前PP不超过新的最大PP
}

// 获取当前HP
int Creature::getCurrentHP() const
{
    return m_currentHP;
}

// 获取最大HP
int Creature::getMaxHP() const
{
    return m_maxHP;
}

// 获取当前PP (全局PP池)
int Creature::getCurrentPP() const
{
    return m_currentPP;
}

// 获取最大PP (全局PP池)
int Creature::getMaxPP() const
{
    return m_maxPP;
}

// 判断精灵是否已濒死 (HP <= 0)
bool Creature::isDead() const
{
    return m_currentHP <= 0;
}

// 判断精灵当前回合是否能够行动
bool Creature::canAct() const
{
    if (isDead())
        return false; // 濒死状态无法行动

    // 根据异常状态判断
    switch (m_statusCondition)
    {
    case StatusCondition::PARALYZE: // 麻痹状态
        // 设计文档：麻痹状态无法行动
        return false;
    case StatusCondition::SLEEP: // 睡眠状态
        // 设计文档：睡眠状态无法行动
        return false;
    case StatusCondition::FREEZE: // 冻伤/冰冻状态
        // 设计文档：冻伤每回合扣血，但未明确是否无法行动。假设可以行动。
        // 若冰冻无法行动，则: return false;
        return true;            // 假设冻伤仍可行动，仅扣血
    case StatusCondition::FEAR: // 害怕状态
        // 设计文档：害怕状态无法行动
        return false;
    case StatusCondition::TIRED: // 疲惫状态 (通常由特定技能副作用导致)
        // 设计文档：疲惫状态无法行动
        return false;
    case StatusCondition::CONFUSION: // 混乱状态
        // 设计文档：混乱时有概率攻击自己或无法行动，此处简化为可以尝试行动，具体由BattleSystem判定
        return true; // 允许尝试行动，具体行为由战斗系统判定
    default:
        return true; // 无特殊限制行动的状态
    }
}

// 精灵获得经验值
void Creature::gainExperience(int exp)
{
    if (m_level >= MAX_LEVEL || exp <= 0) // 如果已满级或获得经验为0或负，则不处理
    {
        return;
    }
    m_experience += exp;
    // 尝试升级 (可能会连续升级)
    while (tryLevelUp())
    {
        // 循环直到不再满足升级条件
    }
}

// 尝试升级 (私有辅助函数)
bool Creature::tryLevelUp()
{
    if (m_level >= MAX_LEVEL)
        return false; // 已达最高等级

    int expToNext = calculateExperienceToNextLevel(); // 计算到下一级所需经验
    if (m_experience >= expToNext)                    // 如果当前经验足够
    {
        m_level++;                 // 等级提升
        m_experience -= expToNext; // 扣除升级所需经验
        updateStatsOnLevelUp();    // 升级后更新能力值
        // battle->addBattleLog(QString("%1 升到了 %2级!").arg(m_name).arg(m_level)); // 日志应由GameEngine或BattleSystem处理
        // TODO: 触发学习新技能的逻辑 (如果达到特定等级可以学新技能)
        return true; // 成功升级
    }
    return false; // 经验不足，未升级
}

// 精灵受到伤害
void Creature::takeDamage(int damage)
{
    if (damage < 0)
        damage = 0; // 伤害不应为负
    m_currentHP -= damage;
    if (m_currentHP < 0)
    {
        m_currentHP = 0; // HP不应为负
    }
    if (isDead())
    {
        // battle->addBattleLog(QString("%1 倒下了!").arg(m_name)); // 日志应由BattleSystem处理
        clearAllTurnEffects(); // 濒死时清除所有回合效果
        resetStatStages();     // 重置能力等级
        // clearStatusCondition(); // 濒死时是否清除异常状态，根据游戏设计决定
    }
}

// 精灵恢复HP
void Creature::heal(int amount)
{
    if (isDead() || amount <= 0)
        return; // 濒死状态不能恢复，无效治疗量也不处理
    m_currentHP += amount;
    if (m_currentHP > m_maxHP)
    {
        m_currentHP = m_maxHP; // HP不超过最大值
    }
}

// 消耗精灵的全局PP
void Creature::consumePP(int amount)
{
    if (amount < 0)
        return; // 消耗量不应为负
    m_currentPP -= amount;
    if (m_currentPP < 0)
    {
        m_currentPP = 0; // PP不应为负
    }
}

// 恢复精灵的全局PP
void Creature::restorePP(int amount)
{
    if (amount < 0)
        return; // 恢复量不应为负
    m_currentPP += amount;
    if (m_currentPP > m_maxPP)
    {
        m_currentPP = m_maxPP; // PP不超过最大值
    }
}

// 设置精灵的异常状态
void Creature::setStatusCondition(StatusCondition condition)
{
    // TODO: 添加逻辑，例如某些状态不能覆盖其他状态，或某些属性免疫特定状态
    m_statusCondition = condition;
}

// 清除精灵的异常状态
void Creature::clearStatusCondition()
{
    m_statusCondition = StatusCondition::NONE;
}

// 修改精灵的能力等级
void Creature::modifyStatStage(StatType stat, int delta)
{
    m_statStages.modifyStage(stat, delta);
}

// 重置精灵所有能力等级变化为0
void Creature::resetStatStages()
{
    m_statStages.reset();
}

// 精灵学习新技能 (最多4个普通技能)
void Creature::learnSkill(Skill *skill)
{
    if (skill)
    {
        if (m_skills.size() < 4) // 如果技能槽未满
        {
            m_skills.append(skill);
        }
        else
        {
            // 技能槽已满，处理替换逻辑或提示 (通常在UI层面处理选择替换哪个)
            // qWarning() << m_name << "的技能槽已满，无法学习" << skill->getName();
            delete skill; // 如果不学习，释放传入的技能对象内存
        }
    }
}

// 精灵忘记技能
void Creature::forgetSkill(int index)
{
    if (index >= 0 && index < m_skills.size())
    {
        delete m_skills.takeAt(index); // 从列表中移除并删除对象
    }
}

// 检查精灵是否已学会某个名称的技能
bool Creature::hasSkill(const QString &skillName) const
{
    for (const Skill *skill : m_skills)
    {
        if (skill && skill->getName() == skillName)
            return true;
    }
    if (m_fifthSkill && m_fifthSkill->getName() == skillName)
        return true;
    return false;
}

// 获取指定索引的技能
Skill *Creature::getSkill(int index) const
{
    if (index >= 0 && index < m_skills.size())
    {
        return m_skills[index];
    }
    return nullptr;
}

// 获取所有普通技能的列表
QVector<Skill *> Creature::getSkills() const
{
    return m_skills;
}

// 获取已学会的普通技能数量
int Creature::getSkillCount() const
{
    return m_skills.size();
}

// 设置第五技能
void Creature::setFifthSkill(Skill *skill)
{
    if (m_fifthSkill) // 如果已有第五技能，则替换并删除旧的
    {
        delete m_fifthSkill;
    }
    m_fifthSkill = skill; // 设置新的第五技能
}

// 获取第五技能
Skill *Creature::getFifthSkill() const
{
    return m_fifthSkill;
}

// 添加回合类效果到精灵身上
void Creature::addTurnEffect(TurnBasedEffect *effect)
{
    if (effect)
    {
        // TODO: 检查是否已有同类效果，是否可叠加等逻辑
        m_turnEffects.append(effect);
    }
}

// 移除一个特定的回合类效果
void Creature::removeTurnEffect(TurnBasedEffect *effect)
{
    if (m_turnEffects.removeAll(effect) > 0)
    {                  // 从列表中移除所有匹配的指针
        delete effect; // 释放效果对象内存
    }
}

// 清除所有回合类效果
void Creature::clearAllTurnEffects()
{
    qDeleteAll(m_turnEffects); // 删除所有效果对象
    m_turnEffects.clear();     // 清空列表
}

// 获取精灵当前所有的回合类效果
QVector<TurnBasedEffect *> Creature::getTurnEffects() const
{
    return m_turnEffects;
}

// 精灵使用技能 (核心逻辑委托给Skill对象和BattleSystem)
bool Creature::useSkill(int skillIndex, Creature *target, BattleSystem *battle)
{
    Skill *selectedSkill = nullptr;

    if (skillIndex >= 0 && skillIndex < m_skills.size()) // 选择普通技能
    {
        selectedSkill = m_skills[skillIndex];
    }
    else if (skillIndex == -1 && m_fifthSkill) // 选择第五技能 (约定-1为第五技能)
    {
        selectedSkill = m_fifthSkill;
    }

    if (selectedSkill && target && battle) // 确保技能、目标和战斗系统都有效
    {
        // 技能使用的具体逻辑（PP消耗、命中判定、效果应用）在Skill::use中处理
        // BattleSystem会在此之后处理伤害结算等
        return selectedSkill->use(this, target, battle);
    }
    // battle->addBattleLog(QString("%1 试图使用一个无效的技能或目标。").arg(m_name));
    return false; // 技能使用失败
}

// 回合开始时调用的处理函数
void Creature::onTurnStart(BattleSystem* battle)
{
    // 中文注释：精灵回合开始时的逻辑处理

    // 1. 处理回合类效果的开始阶段逻辑
    // 创建一个副本进行迭代，防止在迭代过程中修改m_turnEffects导致问题
    QVector<TurnBasedEffect *> effectsToProcess = m_turnEffects;
    for (TurnBasedEffect *effect : effectsToProcess)
    {
        if (effect && effect->isOnTurnStart()) // 如果效果是在回合开始时触发
        {
            effect->executeTurnLogic(this, nullptr, nullptr); // 传入当前精灵和战斗系统 (源精灵和战斗系统可根据需要传递)
        }
    }

    // 2. 处理部分异常状态在回合开始时的效果 (例如：睡眠苏醒判定)
    switch (m_statusCondition)
    {
    case StatusCondition::SLEEP:
        // 睡眠状态有几率苏醒，或持续固定回合
        // 此处简化：假设睡眠有25%几率当回合苏醒
        if (QRandomGenerator::global()->bounded(100) < 25)
        {
            // battle->addBattleLog(QString("%1 从睡眠中苏醒了!").arg(m_name));
            clearStatusCondition();
        }
        else
        {
            // battle->addBattleLog(QString("%1 仍在睡眠中...").arg(m_name));
        }
        break;
    default:
        break;
    }
}

// 回合结束时调用的处理函数
void Creature::onTurnEnd(BattleSystem* battle)
{
    // 处理回合结束时的状态效果
    if (battle) { // 确保battle不为空
        int damage = 0;
        switch (m_statusCondition) {
            case StatusCondition::BURN:
                damage = m_maxHP / 16; // 烧伤每回合损失1/16最大生命值
                takeDamage(damage);
                // 使用触发器记录伤害，而不是直接添加日志
                battle->triggerDamageCaused(this, damage);
                break;
            case StatusCondition::POISON:
                damage = m_maxHP / 8; 
                takeDamage(damage);
                battle->triggerDamageCaused(this, damage);
                break;
            case StatusCondition::BLEED:
                damage = m_maxHP / 10;
                takeDamage(damage);
                battle->triggerDamageCaused(this, damage);
                break;
            // 其他状态效果...
        }
    }    
    // 执行回合结束时的持续效果
    QVector<TurnBasedEffect*> effectsToProcess = m_turnEffects;
    for (TurnBasedEffect* effect : effectsToProcess) {
        if (effect && !effect->isOnTurnStart()) {
            effect->executeTurnLogic(this, effect->getOriginalSource(), battle);
            if (effect->decrementDuration()) {
                // 效果已结束
                if (battle) {
                    // 使用triggerEffectCleared而不是直接添加日志
                    battle->triggerEffectCleared(this, effect->getDescription());
                }
                removeTurnEffect(effect); // 使用安全删除方法
            }
        }
    }
    
    // 检查濒死状态
    if (m_currentHP <= 0 && battle) {
        battle->addBattleLog(QString("<font color='red'><b>%1 倒下了!</b></font>").arg(m_name));
    }
}

// --- 计算类方法 ---

// 计算当前实际物理攻击力
int Creature::calculateAttack() const
{
    // 基础物攻 * 能力等级修正 * 其他修正(如烧伤)
    double baseAtk = static_cast<double>(m_baseStats.getStat(StatType::ATTACK));
    double modifier = StatStages::calculateModifier(StatType::ATTACK, m_statStages.getStage(StatType::ATTACK));
    int finalAtk = qRound(baseAtk * modifier);

    // 烧伤状态会使物理攻击减半
    if (m_statusCondition == StatusCondition::BURN)
    {
        finalAtk /= 2;
    }
    return qMax(1, finalAtk); // 至少为1
}

// 计算当前实际特殊攻击力
int Creature::calculateSpecialAttack() const
{
    double baseSpAtk = static_cast<double>(m_baseStats.getStat(StatType::SP_ATTACK));
    double modifier = StatStages::calculateModifier(StatType::SP_ATTACK, m_statStages.getStage(StatType::SP_ATTACK));
    int finalSpAtk = qRound(baseSpAtk * modifier);
    return qMax(1, finalSpAtk);
}

// 计算当前实际物理防御力
int Creature::calculateDefense() const
{
    double baseDef = static_cast<double>(m_baseStats.getStat(StatType::DEFENSE));
    double modifier = StatStages::calculateModifier(StatType::DEFENSE, m_statStages.getStage(StatType::DEFENSE));
    int finalDef = qRound(baseDef * modifier);
    return qMax(1, finalDef);
}

// 计算当前实际特殊防御力
int Creature::calculateSpecialDefense() const
{
    double baseSpDef = static_cast<double>(m_baseStats.getStat(StatType::SP_DEFENSE));
    double modifier = StatStages::calculateModifier(StatType::SP_DEFENSE, m_statStages.getStage(StatType::SP_DEFENSE));
    int finalSpDef = qRound(baseSpDef * modifier);
    return qMax(1, finalSpDef);
}

// 计算当前实际速度
int Creature::calculateSpeed() const
{
    double baseSpeed = static_cast<double>(m_baseStats.getStat(StatType::SPEED));
    double modifier = StatStages::calculateModifier(StatType::SPEED, m_statStages.getStage(StatType::SPEED));
    int finalSpeed = qRound(baseSpeed * modifier);

    // 麻痹状态会使速度减半
    if (m_statusCondition == StatusCondition::PARALYZE)
    {
        finalSpeed /= 2;
    }
    return qMax(1, finalSpeed);
}

// 计算指定技能属性对目标精灵的克制倍率
double Creature::getTypeEffectivenessAgainst(const Creature *target, ElementType skillType) const
{
    if (!target)
        return 1.0; // 如果没有目标，则无克制关系

    Type attackerSkillType(skillType);             // 攻击技能的属性
    Type defenderCreatureType = target->getType(); // 防守方精灵的属性

    return Type::calculateEffectiveness(attackerSkillType, defenderCreatureType); // 调用Type类的静态方法计算
}

// 判断技能是否与自身属性之一相同 (用于STAB加成)
bool Creature::hasTypeAdvantage(ElementType skillType) const
{
    return (m_type.getPrimaryType() == skillType || (m_type.hasDualType() && m_type.getSecondaryType() == skillType));
}

// 计算到下一级所需经验 (私有辅助)
int Creature::calculateExperienceToNextLevel() const
{
    if (m_level >= MAX_LEVEL)
        return 99999999; // 已满级，返回一个极大值
    // 简单示例公式：(等级^3 * 1.2) - (15 * 等级^2) + (100 * 等级) - 140  (类似宝可梦的中速组)
    // 或更简单的：下一级所需总经验 = (等级+1)^3 * 某个系数
    // 这里使用设计文档中的简单公式：基础经验 * 等级^2 / 100
    // 注意：这个公式可能是每级所需，也可能是总经验。假设是每级所需。
    double exp_needed = static_cast<double>(BASE_EXP_NEEDED * m_level * m_level) / 100.0;
    return qMax(1, static_cast<int>(exp_needed)); // 至少需要1点经验
}

// 等级提升时更新能力值 (私有辅助)
void Creature::updateStatsOnLevelUp()
{
    // 中文注释：精灵升级时，根据天赋成长值更新各项基础属性
    // 属性增长公式：新属性 = 旧属性 + (基础成长点 * 天赋倍率 / (某个系数，如10或50)) + 随机少量波动
    // 此处简化：直接按天赋值增加 (如果天赋值代表每级固定增长点)

    // 重新计算基础属性，这里假设天赋值是每级固定增加的点数
    // 注意：更复杂的成长系统会基于种族值、个体值、努力值和等级计算
    m_baseStats.setStat(StatType::HP, m_baseStats.getStat(StatType::HP) + m_talent.getGrowthRate(StatType::HP));
    m_baseStats.setStat(StatType::ATTACK, m_baseStats.getStat(StatType::ATTACK) + m_talent.getGrowthRate(StatType::ATTACK));
    m_baseStats.setStat(StatType::DEFENSE, m_baseStats.getStat(StatType::DEFENSE) + m_talent.getGrowthRate(StatType::DEFENSE));
    m_baseStats.setStat(StatType::SP_ATTACK, m_baseStats.getStat(StatType::SP_ATTACK) + m_talent.getGrowthRate(StatType::SP_ATTACK));
    m_baseStats.setStat(StatType::SP_DEFENSE, m_baseStats.getStat(StatType::SP_DEFENSE) + m_talent.getGrowthRate(StatType::SP_DEFENSE));
    m_baseStats.setStat(StatType::SPEED, m_baseStats.getStat(StatType::SPEED) + m_talent.getGrowthRate(StatType::SPEED));

    // 更新最大HP，并完全恢复HP和PP
    m_maxHP = m_baseStats.getStat(StatType::HP);
    m_currentHP = m_maxHP;
    m_currentPP = m_maxPP; // 升级时PP也回满
}

// 设置精灵等级 (主要用于调试或特殊情况)
void Creature::setLevel(int newLevel)
{
    m_level = qBound(1, newLevel, MAX_LEVEL); // 确保等级在有效范围内
    m_experience = 0;                         // 通常跳级会重置当前等级的经验
    // TODO: 需要根据新等级重新计算所有基础属性，这比较复杂
    // 简单处理：调用多次updateStatsOnLevelUp或一个基于等级的属性计算公式
    // 为简化，此处不完全重新计算属性，依赖于初始设置或更复杂的属性系统
    // 重新计算最大HP，并回满HP/PP
    m_maxHP = m_baseStats.getStat(StatType::HP); // 应该有一个基于等级的HP计算公式
    m_currentHP = m_maxHP;
    m_currentPP = m_maxPP;
}

// --- 具体精灵类实现 ---

// TungTungTung（木棍人）构造函数
TungTungTung::TungTungTung(int level)
    : Creature("Tung Tung Tung Tung Sahur", Type(ElementType::NORMAL), level) // 调用基类构造
{
    // 中文注释：木棍人 特化构造

    // 设置木棍人的基础属性 (种族值)
    // HP, 物攻, 特攻, 物防, 特防, 速度
    setBaseStats(BaseStats(100, 130, 60, 90, 70, 80)); // 假设这是1级时的基础值或种族值
    // 天赋 (影响升级成长) HP, ATK, SPA, DEF, SPD, SPE
    setTalent(Talent(10, 15, 5, 8, 7, 9)); // 假设这些是每级成长点数的天赋系数

    // 根据当前等级调整属性 (如果setBaseStats只是设定种族值)
    // updateStatsForLevel(); // 需要一个这样的方法

    // 学习初始技能
    learnSkill(new PhysicalSkill("猛力挥击", ElementType::NORMAL, 130, 3, 95));
    learnSkill(new MultiHitSkill("三重连打", ElementType::NORMAL, SkillCategory::PHYSICAL, 60, 4, 90, 3, 3)); // 3次60威力

    StatusSkill *hardenWoodBody = new StatusSkill("硬化木身", ElementType::NORMAL, 3, 100); // 命中100表示对自己使用通常必中
    hardenWoodBody->addEffect(new StatChangeEffect(StatType::DEFENSE, 3, true));            // 提升自身物防+3
    hardenWoodBody->addEffect(new StatChangeEffect(StatType::SP_DEFENSE, 3, true));         // 提升自身特防+3
    learnSkill(hardenWoodBody);

    CompositeSkill *armorPierceThrust = new CompositeSkill("破甲直刺", ElementType::NORMAL, SkillCategory::PHYSICAL, 70, 3, 100);
    armorPierceThrust->setEffectChance(80);                                                      // 80%的触发几率
    armorPierceThrust->addEffect(new StatChangeEffect(StatType::DEFENSE, -2, false));            // false表示对目标
    armorPierceThrust->addEffect(new ClearEffectsEffect(true, false, false, false, false, 100)); // 清除目标能力提升状态
    learnSkill(armorPierceThrust);

    // 第五技能: 不屈战魂
    setFifthSkill(new IndomitableSpiritSkill());
}

void TungTungTung::onTurnStart(BattleSystem* battle) { 
    Creature::onTurnStart(battle); 
}

void TungTungTung::onTurnEnd(BattleSystem* battle) { 
    Creature::onTurnEnd(battle); 
}

// BombardinoCrocodillo（鳄鱼轰炸机）构造函数
BombardinoCrocodillo::BombardinoCrocodillo(int level)
    : Creature("BombardinoCrocodillo", Type(ElementType::FLYING, ElementType::MACHINE), level)
{
    // 中文注释：鳄鱼轰炸机 特化构造
    setBaseStats(BaseStats(90, 115, 70, 100, 80, 95));
    setTalent(Talent(9, 12, 7, 11, 8, 10));

    // 钢翼切割
    CompositeSkill *steelWing = new CompositeSkill("钢翼切割", ElementType::MACHINE, SkillCategory::PHYSICAL, 75, 3, 95);
    steelWing->setEffectChance(50);                                         // 50%几率
    steelWing->addEffect(new StatChangeEffect(StatType::DEFENSE, 1, true)); // 提升自身物防+1
    learnSkill(steelWing);

    // 俯冲轰炸
    CompositeSkill *diveBomb = new CompositeSkill("俯冲轰炸", ElementType::FLYING, SkillCategory::PHYSICAL, 120, 4, 90);
    // 使用后下一回合自身无法行动 -> 通过添加一个持续1回合的TIRED状态实现
    diveBomb->addEffect(new StatusConditionEffect(StatusCondition::TIRED, 100)); // 100%对自己施加疲惫
    diveBomb->getEffects().first()->setTargetSelf(true);                         // 确保疲惫效果作用于自身
    learnSkill(diveBomb);

    CompositeSkill *alligatorFang = new CompositeSkill("鳄牙撕咬", ElementType::WATER, SkillCategory::PHYSICAL, 80, 3, 100);
    alligatorFang->setEffectChance(20);
    alligatorFang->addEffect(new StatusConditionEffect(StatusCondition::FEAR, 100)); // 100%几率在20%命中时触发害怕
    learnSkill(alligatorFang);

    learnSkill(new SpecialSkill("锁定导弹", ElementType::MACHINE, 80, 3, 101)); // 101命中视为必中

    // 第五技能: 空域压制
    setFifthSkill(new AirspaceSupremacySkill());
}
void BombardinoCrocodillo::onTurnStart(BattleSystem* battle) {
    Creature::onTurnStart(battle);
}

void BombardinoCrocodillo::onTurnEnd(BattleSystem* battle) {
    Creature::onTurnEnd(battle);
}

// TralaleroTralala（耐克鲨鱼）构造函数
TralaleroTralala::TralaleroTralala(int level)
    : Creature("耐克鲨鱼", Type(ElementType::WATER, ElementType::SHADOW), level)
{
    // 中文注释：耐克鲨鱼 特化构造
    setBaseStats(BaseStats(75, 100, 110, 60, 70, 125));
    setTalent(Talent(8, 10, 11, 6, 7, 14));

    learnSkill(new PhysicalSkill("暗影偷袭", ElementType::SHADOW, 40, 2, 100, 1)); // 先制+1

    // 激流勇进: 自身HP低于1/3时，威力提升50% (这个逻辑需要在Skill::getPower()重写或BattleSystem中处理)
    learnSkill(new SpecialSkill("激流勇进", ElementType::WATER, 80, 3, 100));

    learnSkill(new SpecialSkill("速度之星", ElementType::NORMAL, 60, 2, 101)); // 必中

    StatusSkill *opportunist = new StatusSkill("伺机待发", ElementType::SHADOW, 2, 100);
    opportunist->addEffect(new StatChangeEffect(StatType::SPEED, 2, true));
    learnSkill(opportunist);

    // 第五技能: 极速掠食
    setFifthSkill(new LifeSiphonFieldSkill());
}
void TralaleroTralala::onTurnStart(BattleSystem* battle) {
    Creature::onTurnStart(battle);
}

void TralaleroTralala::onTurnEnd(BattleSystem* battle) {
    Creature::onTurnEnd(battle);
}
// LiriliLarila（仙人掌大象）构造函数
LiriliLarila::LiriliLarila(int level)
    : Creature("LiriliLarila", Type(ElementType::GRASS, ElementType::GROUND), level)
{
    // 中文注释：仙人掌大象 特化构造
    setBaseStats(BaseStats(120, 90, 75, 110, 100, 55));
    setTalent(Talent(12, 9, 8, 12, 10, 6));

    // 寄生种子
    StatusSkill *leechSeed = new StatusSkill("寄生种子", ElementType::GRASS, 2, 90);
    auto leechSeedLambda = [](Creature *affected, Creature *source, BattleSystem *battle, TurnBasedEffect *effect)
    {
        if (!source || !affected || affected->isDead())
            return;

        int leechAmount = affected->getMaxHP() / 8;
        affected->takeDamage(leechAmount);
        
        // 使用battle系统的触发器记录伤害
        if (battle) {
            battle->triggerDamageCaused(affected, leechAmount);
        }
        
        // 只有当原始施法者(source)还存活时才尝试治疗
        if (!source->isDead()) {
            source->heal(leechAmount);
            if (battle) {
                battle->triggerHealingReceived(source, leechAmount);
            }
        }
    };
    TurnBasedEffect *leechEffect = new TurnBasedEffect(999, leechSeedLambda, false);
    leechEffect->setDescription("寄生种子效果");
    leechEffect->setTargetSelf(false);  // 效果应该作用于对手
    leechEffect->setOriginalSource(nullptr);  // 在apply时自动设置
    leechSeed->addEffect(leechEffect);
    learnSkill(leechSeed);

    // 沙尘尖刺 (Entry Hazard，需要在BattleSystem中特殊处理)
    learnSkill(new StatusSkill("沙尘尖刺", ElementType::GROUND, 3, 100));

    CompositeSkill *thornArmSlam = new CompositeSkill("针刺臂膀", ElementType::GRASS, SkillCategory::PHYSICAL, 70, 3, 100);
    thornArmSlam->setEffectChance(30);
    thornArmSlam->addEffect(new StatusConditionEffect(StatusCondition::POISON, 100));
    learnSkill(thornArmSlam);

    CompositeSkill *earthShaker = new CompositeSkill("大地摇晃", ElementType::GROUND, SkillCategory::SPECIAL, 90, 3, 100);
    earthShaker->setEffectChance(10);
    earthShaker->addEffect(new StatChangeEffect(StatType::SP_DEFENSE, -1, false));
    learnSkill(earthShaker);

    // 第五技能: 生命汲取领域
    setFifthSkill(new LifeSiphonFieldSkill());

}
void LiriliLarila::onTurnStart(BattleSystem* battle) { Creature::onTurnStart(battle); }
void LiriliLarila::onTurnEnd(BattleSystem* battle) { Creature::onTurnEnd(battle); }

// ChimpanziniBananini（香蕉绿猩猩）构造函数
class PrimalShiftSkill : public StatusSkill
{
public:
    PrimalShiftSkill() : StatusSkill("狂化变身", ElementType::NORMAL, 4, 100)
    {
    }
    bool use(Creature *user, Creature *target_unused, BattleSystem *battle_unused) override
    {
        // 中文注释：使用狂化变身技能
        // 调用基类的use进行PP消耗和基础判定
        if (!StatusSkill::use(user, target_unused, battle_unused))
            return false;

        if (ChimpanziniBananini *chimp = dynamic_cast<ChimpanziniBananini *>(user))
        {
            chimp->enterBerserkForm(3); // 3回合持续时间
            // if (battle_unused) battle_unused->addBattleLog(QString("%1 进入了狂暴形态!").arg(user->getName()));
        }
        else
        {
            // qWarning("PrimalShiftSkill used by a non-ChimpanziniBananini creature!");
            return false; // 非猩猩不能用
        }
        return true;
    }
    QString getDescription() const override
    {
        return "进入“狂暴形态”：物攻等级+2，速度等级+1，物防等级-1，特防等级-1。此效果持续3回合，结束后恢复原形态和能力等级。";
    }
};

ChimpanziniBananini::ChimpanziniBananini(int level)
    : Creature("香蕉绿猩猩", Type(ElementType::GRASS, ElementType::NORMAL), level),
      m_inBerserkForm(false), m_berserkFormDuration(0)
{
    setBaseStats(BaseStats(100, 125, 60, 95, 80, 90));
    setTalent(Talent(10, 13, 6, 10, 8, 9));

    learnSkill(new PhysicalSkill("香蕉猛击", ElementType::GRASS, 85, 3, 100));
    learnSkill(new PhysicalSkill("巨力冲拳", ElementType::NORMAL, 90, 3, 95));
    StatusSkill *jungleFortitude = new StatusSkill("丛林坚壁", ElementType::GRASS, 2, 100);
    jungleFortitude->addEffect(new StatChangeEffect(StatType::DEFENSE, 2, true));
    learnSkill(jungleFortitude);
    StatusSkill *primalRoar = new StatusSkill("野性咆哮", ElementType::NORMAL, 2, 100);
    primalRoar->addEffect(new StatChangeEffect(StatType::ATTACK, -1, false));
    primalRoar->addEffect(new StatChangeEffect(StatType::DEFENSE, -1, false));
    learnSkill(primalRoar);

    learnSkill(new PrimalShiftSkill()); // 使用自定义的变身技能

    setFifthSkill(new JungleKingStrikeSkill());
}
// 狂暴形态相关方法
void ChimpanziniBananini::enterBerserkForm(int duration)
{
    if (!m_inBerserkForm)
    {
        m_inBerserkForm = true;
        m_berserkFormDuration = duration;
        modifyStatStage(StatType::ATTACK, 2);
        modifyStatStage(StatType::SPEED, 1);
        modifyStatStage(StatType::DEFENSE, -1);
        modifyStatStage(StatType::SP_DEFENSE, -1);
    }
}
void ChimpanziniBananini::exitBerserkForm()
{
    if (m_inBerserkForm)
    {
        m_inBerserkForm = false;
        m_berserkFormDuration = 0;
        resetStatStages(); // 简化：重置所有阶段
    }
}
void ChimpanziniBananini::onTurnStart(BattleSystem* battle)
{
    Creature::onTurnStart(battle);
}
void ChimpanziniBananini::onTurnEnd(BattleSystem* battle) {
    Creature::onTurnEnd(battle);
    
    // 特殊逻辑：狂暴形态处理
    if (m_inBerserkForm) {
        m_berserkFormDuration--;
        if (m_berserkFormDuration <= 0) {
            exitBerserkForm();
            if (battle) {
                battle->addBattleLog(QString("%1 的狂暴形态结束了!").arg(getName()));
            }
        }
    }
}

// Luguanluguanlulushijiandaole（鹿管鹿管鹿鹿时间到了）构造函数
Luguanluguanlulushijiandaole::Luguanluguanlulushijiandaole(int level)
    : Creature("鹿管鹿管鹿鹿时间到了", Type(ElementType::LIGHT, ElementType::NORMAL), level),
      m_snapshotTurnsLeft(0)
{
    // 中文注释：鹿管鹿管鹿鹿时间到了 特化构造
    setBaseStats(BaseStats(80, 70, 110, 75, 90, 105));
    setTalent(Talent(8, 7, 12, 8, 10, 11));

    CompositeSkill *temporalRay = new CompositeSkill("时光射线", ElementType::LIGHT, SkillCategory::SPECIAL, 70, 3, 100);
    temporalRay->setEffectChance(20);
    temporalRay->addEffect(new StatChangeEffect(StatType::SPEED, -1, false));
    learnSkill(temporalRay);

    StatusSkill *rewindHeal = new StatusSkill("回溯疗愈", ElementType::NORMAL, 3, 100);
    rewindHeal->addEffect(new ClearEffectsEffect(false, false, true, false, true)); // 清除自身异常状态
    // 恢复至上回合结束时的HP，这个非常复杂，需要BattleSystem记录历史状态
    learnSkill(rewindHeal);

    StatusSkill *acceleratedVision = new StatusSkill("加速视界", ElementType::LIGHT, 2, 100);
    acceleratedVision->addEffect(new StatChangeEffect(StatType::SPEED, 1, true));
    // 本回合技能必定命中，需要一个临时状态或BattleSystem配合
    learnSkill(acceleratedVision);

    // 时光跳跃: 先制+3。使自身本回合免疫所有攻击和技能效果。
    // 免疫效果通过TurnBasedEffect标记，由BattleSystem检查
    StatusSkill *timeHop = new StatusSkill("时光跳跃", ElementType::LIGHT, 2, 100, 3); // 优先级3
    auto immunityLambda = [](Creature *aff, Creature *src, BattleSystem *b, TurnBasedEffect *self_eff) { /* 标记用 */ };
    TurnBasedEffect *hopImmunity = new TurnBasedEffect(1, immunityLambda, true); // 持续1回合，回合开始生效
    hopImmunity->setDescription("时光跳跃免疫");
    timeHop->addEffect(hopImmunity);
    timeHop->getEffects().first()->setTargetSelf(true);
    learnSkill(timeHop);

    // 第五技能: 时间悖论 (极其复杂，简化或标记为TODO)
    setFifthSkill(new TemporalParadoxSkill());
}

void Luguanluguanlulushijiandaole::recordBattleState() { /* TODO */ }
bool Luguanluguanlulushijiandaole::tryRevertBattleState(BattleSystem *battle_unused) { /* TODO */ return false; }
void Luguanluguanlulushijiandaole::onTurnStart(BattleSystem* battle) { Creature::onTurnStart(battle); }
void Luguanluguanlulushijiandaole::onTurnEnd(BattleSystem* battle)
{
    Creature::onTurnEnd(battle);
    if (m_snapshotTurnsLeft > 0)
        m_snapshotTurnsLeft--;
}
// CappuccinoAssassino（卡布奇诺忍者）构造函数
CappuccinoAssassino::CappuccinoAssassino(int level)
    : Creature("CappuccinoAssassino", Type(ElementType::SHADOW, ElementType::MACHINE), level),
      m_inShadowState(false)
{
    // 中文注释：卡布奇诺忍者 特化构造
    setBaseStats(BaseStats(70, 115, 80, 65, 70, 130));
    setTalent(Talent(7, 12, 8, 7, 7, 14));

    learnSkill(new MultiHitSkill("影手里剑", ElementType::SHADOW, SkillCategory::PHYSICAL, 25, 2, 100, 2, 3, 1));
    // 滚烫奇袭
    CompositeSkill *scaldingSurprise = new CompositeSkill("滚烫奇袭", ElementType::FIRE, SkillCategory::SPECIAL, 70, 3, 100);
    // 30%几率令目标烧伤。若目标速度低于自身，则烧伤几率提升至60%。 (条件几率需BattleSystem支持)
    scaldingSurprise->setEffectChance(30);                                              // 基础30%
    scaldingSurprise->addEffect(new StatusConditionEffect(StatusCondition::BURN, 100)); // 若触发setEffectChance，则100%烧伤
    learnSkill(scaldingSurprise);

    CompositeSkill *metalGrind = new CompositeSkill("金属研磨", ElementType::MACHINE, SkillCategory::PHYSICAL, 75, 3, 95);
    metalGrind->setEffectChance(30);
    metalGrind->addEffect(new StatChangeEffect(StatType::SPEED, -1, false));
    learnSkill(metalGrind);

    StatusSkill *swiftVanish = new StatusSkill("急速隐匿", ElementType::SHADOW, 2, 100);
    swiftVanish->addEffect(new StatChangeEffect(StatType::SPEED, 2, true));
    learnSkill(swiftVanish);

    // 第五技能: 绝影刺杀
    // 先制+1。若目标HP高于75%，则此技能威力提升50%；若目标HP低于25%，则此技能必定暴击。
    // (威力提升和必爆条件需BattleSystem或自定义Skill子类支持)
    setFifthSkill(new PhantomAssassinateSkill());
}

bool CappuccinoAssassino::isInShadowState() const { return m_inShadowState; }
void CappuccinoAssassino::enterShadowState() { m_inShadowState = true; }
void CappuccinoAssassino::exitShadowState() { m_inShadowState = false; }
void CappuccinoAssassino::onTurnStart(BattleSystem* battle)
{
    Creature::onTurnStart(battle);
    if (m_inShadowState)
    { /* 影子状态的回合开始效果 */
    }
}
void CappuccinoAssassino::onTurnEnd(BattleSystem* battle)
{
    Creature::onTurnEnd(battle);
    if (m_inShadowState)
    { /* 影子状态的回合结束效果, 例如解除 */
        // exitShadowState();
    }
}

TungTungTung::~TungTungTung() {}
BombardinoCrocodillo::~BombardinoCrocodillo() {}
TralaleroTralala::~TralaleroTralala() {}
LiriliLarila::~LiriliLarila() {}
ChimpanziniBananini::~ChimpanziniBananini() {}
Luguanluguanlulushijiandaole::~Luguanluguanlulushijiandaole() {}
CappuccinoAssassino::~CappuccinoAssassino() {}