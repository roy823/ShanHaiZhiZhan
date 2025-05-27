#include "creature.h"
#include "gameengine.h"
#include "../battle/battlesystem.h"
#include "../battle/skill.h"
#include "../battle/specialskills.h"
#include "../battle/effect.h"
#include <QRandomGenerator>
#include <QDebug>

// 基础构造函数
Creature::Creature(const QString &name, const Type &type, int level)
    : m_name(name),
      m_type(type),
      m_level(qBound(1, level, 100)),
      m_currentHP(0),
      m_currentPP(0),
      m_experience(0),
      m_statusCondition(StatusCondition::NONE),
      m_isDead(false),
      m_isWild(false),
      m_fifthSkill(nullptr)
{
    // 初始化基础属性
    m_baseStats = BaseStats(100, 100, 100, 100, 100, 100);
    m_talent = Talent(10, 10, 10, 10, 10, 10);
    
    // 计算实际属性
    calculateStats();
    
    // 初始化为满HP和满PP
    m_currentHP = m_maxHP;
    m_currentPP = m_maxPP;
}

// 析构函数
Creature::~Creature()
{
    // 清理技能
    qDeleteAll(m_skills);
    m_skills.clear();
    
    // 清理第五技能
    if (m_fifthSkill) {
        delete m_fifthSkill;
        m_fifthSkill = nullptr;
    }
    
    // 清理回合效果
    qDeleteAll(m_turnEffects);
    m_turnEffects.clear();
}

// 获取名称
QString Creature::getName() const
{
    return m_name;
}

// 设置名称
void Creature::setName(const QString &name)
{
    m_name = name;
}

// 获取类型
Type Creature::getType() const
{
    return m_type;
}

// 设置类型
void Creature::setType(const Type &type)
{
    m_type = type;
}

// 获取等级
int Creature::getLevel() const
{
    return m_level;
}

// 设置等级
void Creature::setLevel(int level)
{
    m_level = qBound(1, level, 100);
    calculateStats(); // 重新计算属性
}

// 获取当前HP
int Creature::getCurrentHP() const
{
    return m_currentHP;
}

// 设置当前HP
void Creature::setCurrentHP(int hp)
{
    m_currentHP = qBound(0, hp, m_maxHP);
    if (m_currentHP == 0) {
        m_isDead = true;
    } else {
        m_isDead = false;
    }
}

// 获取最大HP
int Creature::getMaxHP() const
{
    return m_maxHP;
}

// 获取当前PP
int Creature::getCurrentPP() const
{
    return m_currentPP;
}

// 设置当前PP
void Creature::setCurrentPP(int pp)
{
    m_currentPP = qBound(0, pp, m_maxPP);
}

// 获取最大PP
int Creature::getMaxPP() const
{
    return m_maxPP;
}

// 获取经验值
int Creature::getExperience() const
{
    return m_experience;
}

// 设置经验值
void Creature::setExperience(int exp)
{
    m_experience = qMax(0, exp);
}

// 获取基础属性
BaseStats Creature::getBaseStats() const
{
    return m_baseStats;
}

// 设置基础属性
void Creature::setBaseStats(const BaseStats &stats)
{
    m_baseStats = stats;
    calculateStats(); // 重新计算属性
}

// 获取天赋
Talent Creature::getTalent() const
{
    return m_talent;
}

// 设置天赋
void Creature::setTalent(const Talent &talent)
{
    m_talent = talent;
    calculateStats(); // 重新计算属性
}

// 获取能力等级
StatStages &Creature::getStatStages()
{
    return m_statStages;
}

// 获取状态异常
StatusCondition Creature::getStatusCondition() const
{
    return m_statusCondition;
}

// 设置状态异常
void Creature::setStatusCondition(StatusCondition condition)
{
    m_statusCondition = condition;
}

// 是否已死亡
bool Creature::isDead() const
{
    return m_isDead || m_currentHP <= 0;
}

// 是否为野生
bool Creature::isWild() const
{
    return m_isWild;
}

// 设置是否为野生
void Creature::setWild(bool isWild)
{
    m_isWild = isWild;
}

// 学习技能
void Creature::learnSkill(Skill *skill)
{
    if (skill && m_skills.size() < 4) {
        m_skills.append(skill);
    } else if (skill) {
        // 如果已有4个技能，则替换最后一个
        if (!m_skills.isEmpty()) {
            delete m_skills.last();
            m_skills.removeLast();
        }
        m_skills.append(skill);
    }
}

// 获取技能列表
QVector<Skill *> Creature::getSkills() const
{
    return m_skills;
}

// 设置第五技能
void Creature::setFifthSkill(FifthSkill *skill)
{
    if (m_fifthSkill) {
        delete m_fifthSkill;
    }
    m_fifthSkill = skill;
}

// 获取第五技能
FifthSkill *Creature::getFifthSkill() const
{
    return m_fifthSkill;
}

// 使用技能
bool Creature::useSkill(int index, Creature *target, BattleSystem *battle)
{
    if (index >= 0 && index < m_skills.size()) {
        Skill *skill = m_skills[index];
        if (skill) {
            // 检查PP是否足够
            if (m_currentPP < skill->getPPCost()) {
                return false;
            }
            
            // 扣除PP
            m_currentPP -= skill->getPPCost();
            
            // 使用技能
            return skill->use(this, target, battle);
        }
    }
    return false;
}

// 使用第五技能
bool Creature::useFifthSkill(Creature *target, BattleSystem *battle)
{
    if (m_fifthSkill) {
        // 检查是否可以使用
        if (!m_fifthSkill->canUse(this, target, battle)) {
            return false;
        }
        
        // 检查PP是否足够
        if (m_currentPP < m_fifthSkill->getPPCost()) {
            return false;
        }
        
        // 扣除PP
        m_currentPP -= m_fifthSkill->getPPCost();
        
        // 使用技能
        return m_fifthSkill->use(this, target, battle);
    }
    return false;
}

// 受到伤害
int Creature::takeDamage(int amount)
{
    int actualDamage = qMin(amount, m_currentHP);
    m_currentHP -= actualDamage;
    
    if (m_currentHP <= 0) {
        m_currentHP = 0;
        m_isDead = true;
    }
    
    return actualDamage;
}

// 治疗HP
int Creature::heal(int amount)
{
    int oldHP = m_currentHP;
    m_currentHP = qMin(m_currentHP + amount, m_maxHP);
    
    if (m_currentHP > 0) {
        m_isDead = false;
    }
    
    return m_currentHP - oldHP;
}

// 恢复PP
int Creature::restorePP(int amount)
{
    int oldPP = m_currentPP;
    m_currentPP = qMin(m_currentPP + amount, m_maxPP);
    return m_currentPP - oldPP;
}

// 修改能力等级
bool Creature::modifyStatStage(StatType type, int delta)
{
    m_statStages.modifyStage(type, delta);
    return true;
}

// 重置能力等级
void Creature::resetStatStages()
{
    m_statStages.reset();
}

// 添加回合效果
void Creature::addTurnEffect(TurnBasedEffect *effect)
{
    if (effect) {
        m_turnEffects.append(effect);
    }
}

// 移除回合效果
void Creature::removeTurnEffect(TurnBasedEffect *effect)
{
    if (effect) {
        m_turnEffects.removeOne(effect);
        delete effect;
    }
}

// 清除所有回合效果
void Creature::clearTurnEffects()
{
    qDeleteAll(m_turnEffects);
    m_turnEffects.clear();
}

// 获取回合效果列表
QVector<TurnBasedEffect *> Creature::getTurnEffects() const
{
    return m_turnEffects;
}

// 回合开始时调用
void Creature::onTurnStart()
{
    // 处理回合开始时的效果
    QVector<TurnBasedEffect *> effectsToRemove;
    
    for (TurnBasedEffect *effect : m_turnEffects) {
        if (effect && effect->isOnTurnStart()) {
            effect->executeTurnLogic(this, effect->getOriginalSource(), GameEngine::getInstance()->getBattleSystem());
            
            // 检查效果是否结束
            if (effect->decrementDuration()) {
                effectsToRemove.append(effect);
            }
        }
    }
    
    // 移除已结束的效果
    for (TurnBasedEffect *effect : effectsToRemove) {
        m_turnEffects.removeOne(effect);
        delete effect;
    }
}

// 回合结束时调用
void Creature::onTurnEnd()
{
    // 处理回合结束时的效果
    QVector<TurnBasedEffect *> effectsToRemove;
    
    for (TurnBasedEffect *effect : m_turnEffects) {
        if (effect && !effect->isOnTurnStart()) {
            effect->executeTurnLogic(this, effect->getOriginalSource(), GameEngine::getInstance()->getBattleSystem());
            
            // 检查效果是否结束
            if (effect->decrementDuration()) {
                effectsToRemove.append(effect);
            }
        }
    }
    
    // 移除已结束的效果
    for (TurnBasedEffect *effect : effectsToRemove) {
        m_turnEffects.removeOne(effect);
        delete effect;
    }
}

// 计算实际属性
void Creature::calculateStats()
{
    // 基础公式: (基础值 * 等级 / 50) * (1 + 天赋成长 / 10)
    m_maxHP = (m_baseStats.hp() * m_level / 50) * (1 + m_talent.hpGrowth() / 10);
    m_attack = (m_baseStats.attack() * m_level / 50) * (1 + m_talent.attackGrowth() / 10);
    m_specialAttack = (m_baseStats.specialAttack() * m_level / 50) * (1 + m_talent.specialAttackGrowth() / 10);
    m_defense = (m_baseStats.defense() * m_level / 50) * (1 + m_talent.defenseGrowth() / 10);
    m_specialDefense = (m_baseStats.specialDefense() * m_level / 50) * (1 + m_talent.specialDefenseGrowth() / 10);
    m_speed = (m_baseStats.speed() * m_level / 50) * (1 + m_talent.speedGrowth() / 10);
    
    // 确保最小值
    m_maxHP = qMax(m_maxHP, 10);
    m_attack = qMax(m_attack, 5);
    m_specialAttack = qMax(m_specialAttack, 5);
    m_defense = qMax(m_defense, 5);
    m_specialDefense = qMax(m_specialDefense, 5);
    m_speed = qMax(m_speed, 5);
    
    // 计算最大PP (简单公式: 等级 * 2 + 20)
    m_maxPP = m_level * 2 + 20;
}

// 获取实际攻击力 (考虑能力等级)
int Creature::getAttack() const
{
    double modifier = StatStages::calculateModifier(StatType::ATTACK, m_statStages.getStage(StatType::ATTACK));
    return qRound(m_attack * modifier);
}

// 获取实际特攻 (考虑能力等级)
int Creature::getSpecialAttack() const
{
    double modifier = StatStages::calculateModifier(StatType::SP_ATTACK, m_statStages.getStage(StatType::SP_ATTACK));
    return qRound(m_specialAttack * modifier);
}

// 获取实际防御 (考虑能力等级)
int Creature::getDefense() const
{
    double modifier = StatStages::calculateModifier(StatType::DEFENSE, m_statStages.getStage(StatType::DEFENSE));
    return qRound(m_defense * modifier);
}

// 获取实际特防 (考虑能力等级)
int Creature::getSpecialDefense() const
{
    double modifier = StatStages::calculateModifier(StatType::SP_DEFENSE, m_statStages.getStage(StatType::SP_DEFENSE));
    return qRound(m_specialDefense * modifier);
}

// 获取实际速度 (考虑能力等级)
int Creature::getSpeed() const
{
    double modifier = StatStages::calculateModifier(StatType::SPEED, m_statStages.getStage(StatType::SPEED));
    return qRound(m_speed * modifier);
}

// 获取命中率修正 (考虑能力等级)
double Creature::getAccuracyModifier() const
{
    return StatStages::calculateModifier(StatType::ACCURACY, m_statStages.getStage(StatType::ACCURACY));
}

// 获取闪避率修正 (考虑能力等级)
double Creature::getEvasionModifier() const
{
    return StatStages::calculateModifier(StatType::EVASION, m_statStages.getStage(StatType::EVASION));
}

// 获取详细信息
QString Creature::getDetailedInfo() const
{
    QString info = QString("名称: %1\n").arg(m_name);
    info += QString("类型: %1\n").arg(m_type.getFullTypeName());
    info += QString("等级: %1\n").arg(m_level);
    info += QString("HP: %1/%2\n").arg(m_currentHP).arg(m_maxHP);
    info += QString("PP: %1/%2\n").arg(m_currentPP).arg(m_maxPP);
    info += QString("攻击: %1 (基础: %2)\n").arg(getAttack()).arg(m_attack);
    info += QString("特攻: %1 (基础: %2)\n").arg(getSpecialAttack()).arg(m_specialAttack);
    info += QString("防御: %1 (基础: %2)\n").arg(getDefense()).arg(m_defense);
    info += QString("特防: %1 (基础: %2)\n").arg(getSpecialDefense()).arg(m_specialDefense);
    info += QString("速度: %1 (基础: %2)\n").arg(getSpeed()).arg(m_speed);
    
    // 添加状态信息
    if (m_statusCondition != StatusCondition::NONE) {
        QString statusName;
        switch (m_statusCondition) {
            case StatusCondition::POISON: statusName = "中毒"; break;
            case StatusCondition::BURN: statusName = "烧伤"; break;
            case StatusCondition::FREEZE: statusName = "冻伤"; break;
            case StatusCondition::PARALYZE: statusName = "麻痹"; break;
            case StatusCondition::SLEEP: statusName = "睡眠"; break;
            case StatusCondition::FEAR: statusName = "害怕"; break;
            case StatusCondition::TIRED: statusName = "疲惫"; break;
            case StatusCondition::BLEED: statusName = "流血"; break;
            case StatusCondition::CONFUSION: statusName = "混乱"; break;
            default: statusName = "无"; break;
        }
        info += QString("状态: %1\n").arg(statusName);
    }
    
    // 添加技能信息
    info += "\n技能:\n";
    for (Skill *skill : m_skills) {
        if (skill) {
            info += QString("- %1 (PP消耗: %2)\n").arg(skill->getName()).arg(skill->getPPCost());
        }
    }
    
    // 添加第五技能信息
    if (m_fifthSkill) {
        info += QString("\n第五技能: %1 (PP消耗: %2)\n").arg(m_fifthSkill->getName()).arg(m_fifthSkill->getPPCost());
    }
    
    return info;
}

// 获取简要信息
QString Creature::getBriefInfo() const
{
    return QString("%1 (Lv.%2) HP:%3/%4").arg(m_name).arg(m_level).arg(m_currentHP).arg(m_maxHP);
}

// 获取战斗状态信息
QString Creature::getBattleStatusInfo() const
{
    QString info = QString("%1 (Lv.%2)\n").arg(m_name).arg(m_level);
    info += QString("HP: %1/%2\n").arg(m_currentHP).arg(m_maxHP);
    
    // 添加状态信息
    if (m_statusCondition != StatusCondition::NONE) {
        QString statusName;
        switch (m_statusCondition) {
            case StatusCondition::POISON: statusName = "中毒"; break;
            case StatusCondition::BURN: statusName = "烧伤"; break;
            case StatusCondition::FREEZE: statusName = "冻伤"; break;
            case StatusCondition::PARALYZE: statusName = "麻痹"; break;
            case StatusCondition::SLEEP: statusName = "睡眠"; break;
            case StatusCondition::FEAR: statusName = "害怕"; break;
            case StatusCondition::TIRED: statusName = "疲惫"; break;
            case StatusCondition::BLEED: statusName = "流血"; break;
            case StatusCondition::CONFUSION: statusName = "混乱"; break;
            default: statusName = "无"; break;
        }
        info += QString("状态: %1\n").arg(statusName);
    }
    
    // 添加能力等级信息
    if (m_statStages.getStage(StatType::ATTACK) != 0) {
        info += QString("攻击: %+d\n").arg(m_statStages.getStage(StatType::ATTACK));
    }
    if (m_statStages.getStage(StatType::SP_ATTACK) != 0) {
        info += QString("特攻: %+d\n").arg(m_statStages.getStage(StatType::SP_ATTACK));
    }
    if (m_statStages.getStage(StatType::DEFENSE) != 0) {
        info += QString("防御: %+d\n").arg(m_statStages.getStage(StatType::DEFENSE));
    }
    if (m_statStages.getStage(StatType::SP_DEFENSE) != 0) {
        info += QString("特防: %+d\n").arg(m_statStages.getStage(StatType::SP_DEFENSE));
    }
    if (m_statStages.getStage(StatType::SPEED) != 0) {
        info += QString("速度: %+d\n").arg(m_statStages.getStage(StatType::SPEED));
    }
    
    return info;
}

// 获取技能选择信息
QStringList Creature::getSkillSelectionInfo() const
{
    QStringList skillInfo;
    for (Skill *skill : m_skills) {
        if (skill) {
            skillInfo << QString("%1 (PP消耗: %2)").arg(skill->getName()).arg(skill->getPPCost());
        }
    }
    
    if (m_fifthSkill) {
        skillInfo << QString("%1 (第五技能, PP消耗: %2)").arg(m_fifthSkill->getName()).arg(m_fifthSkill->getPPCost());
    }
    
    return skillInfo;
}

// 创建特定精灵实例
Creature *Creature::createCreature(const QString &creatureType, int level)
{
    if (creatureType == "TungTungTung") {
        return new TungTungTung(level);
    } else if (creatureType == "BombardinoCrocodillo") {
        return new BombardinoCrocodillo(level);
    } else if (creatureType == "TralaleroTralala") {
        return new TralaleroTralala(level);
    } else if (creatureType == "LiriliLarila") {
        return new LiriliLarila(level);
    } else if (creatureType == "ChimpanziniBananini") {
        return new ChimpanziniBananini(level);
    } else if (creatureType == "Luguanluguanlulushijiandaole") {
        return new Luguanluguanlulushijiandaole(level);
    } else {
        // 默认返回木棍人
        return new TungTungTung(level);
    }
}

// TungTungTung（木棍人）构造函数
TungTungTung::TungTungTung(int level)
    : Creature("TungTungTung", Type(ElementType::NORMAL), level)
{
    // 中文注释：木棍人 特化构造
    setBaseStats(BaseStats(80, 80, 80, 80, 80, 80));
    setTalent(Talent(8, 8, 8, 8, 8, 8));
    
    // 学习基础技能
    learnSkill(new PhysicalSkill("木棍敲击", ElementType::NORMAL, 40, 2, 100));
    learnSkill(new PhysicalSkill("木棍投掷", ElementType::NORMAL, 60, 3, 90));
    learnSkill(new SpecialSkill("木棍旋风", ElementType::NORMAL, 70, 3, 85));
    
    // 学习状态技能
    StatusSkill *woodenDefense = new StatusSkill("木质防御", ElementType::NORMAL, 2, 100);
    woodenDefense->addEffect(new StatChangeEffect(StatType::DEFENSE, 1, TargetType::SELF));
    learnSkill(woodenDefense);
    
    // 第五技能: 不屈战魂
    setFifthSkill(new IndomitableSpiritSkill());
    // HP低于1/2的条件检查在FifthSkill::canUse中或BattleSystem中处理
}

// 木棍人特殊回合开始逻辑 (如果需要)
void TungTungTung::onTurnStart() { Creature::onTurnStart(); }
// 木棍人特殊回合结束逻辑 (如果需要)
void TungTungTung::onTurnEnd() { Creature::onTurnEnd(); }

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
    steelWing->addEffect(new StatChangeEffect(StatType::DEFENSE, 1, TargetType::SELF)); // 提升自身物防+1
    learnSkill(steelWing);

    // 俯冲轰炸
    CompositeSkill *diveBomb = new CompositeSkill("俯冲轰炸", ElementType::FLYING, SkillCategory::PHYSICAL, 120, 4, 90);
    // 使用后下一回合自身无法行动 -> 通过添加一个持续1回合的TIRED状态实现
    diveBomb->addEffect(new StatusConditionEffect(StatusCondition::TIRED, 100)); // 100%对自己施加疲惫
    diveBomb->getEffects().first()->setTargetType(TargetType::SELF);                         // 确保疲惫效果作用于自身
    learnSkill(diveBomb);

    CompositeSkill *alligatorFang = new CompositeSkill("鳄牙撕咬", ElementType::WATER, SkillCategory::PHYSICAL, 80, 3, 100);
    alligatorFang->setEffectChance(20);
    alligatorFang->addEffect(new StatusConditionEffect(StatusCondition::FEAR, 100)); // 100%几率在20%命中时触发害怕
    learnSkill(alligatorFang);

    learnSkill(new SpecialSkill("锁定导弹", ElementType::MACHINE, 80, 3, 101)); // 101命中视为必中

    // 第五技能: 空域压制
    setFifthSkill(new AirspaceSupremacySkill());
}
void BombardinoCrocodillo::onTurnStart() { Creature::onTurnStart(); }
void BombardinoCrocodillo::onTurnEnd() { Creature::onTurnEnd(); }

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
    opportunist->addEffect(new StatChangeEffect(StatType::SPEED, 2, TargetType::SELF));
    learnSkill(opportunist);

    // 第五技能: 极速掠食
    setFifthSkill(new LifeSiphonFieldSkill());
}
void TralaleroTralala::onTurnStart() { Creature::onTurnStart(); }
void TralaleroTralala::onTurnEnd() { Creature::onTurnEnd(); }

// LiriliLarila（仙人掌大象）构造函数
LiriliLarila::LiriliLarila(int level)
    : Creature("LiriliLarila", Type(ElementType::GRASS, ElementType::GROUND), level)
{
    // 中文注释：仙人掌大象 特化构造
    setBaseStats(BaseStats(120, 90, 75, 110, 100, 55));
    setTalent(Talent(12, 9, 8, 12, 10, 6));

    // 寄生种子
    StatusSkill *leechSeed = new StatusSkill("寄生种子", ElementType::GRASS, 2, 90);
    auto leechSeedLambda = [](Creature *source, Creature *target, BattleSystem * /* battle_unused */, TurnBasedEffect * /* self_effect_unused */)
    {
        if (source && target && !target->isDead())
        { // 确保目标存活
            int leechAmount = target->getMaxHP() / 8;
            target->takeDamage(leechAmount);
            source->heal(leechAmount);
            // battle_unused->addBattleLog(QString("%1 从 %2 身上吸取了HP!").arg(source->getName()).arg(target->getName()));
        }
    };
    TurnBasedEffect *leechEffect = new TurnBasedEffect(999, leechSeedLambda, false); // 持续999回合(代表直到交换)，回合结束触发
    leechEffect->setDescription("寄生种子效果");
    leechSeed->addEffect(leechEffect); // 这个效果是施加给对方的
    leechSeed->getEffects().first()->setTargetType(TargetType::OPPONENT);
    learnSkill(leechSeed);

    // 沙尘尖刺 (Entry Hazard，需要在BattleSystem中特殊处理)
    learnSkill(new StatusSkill("沙尘尖刺", ElementType::GROUND, 3, 100));

    CompositeSkill *thornArmSlam = new CompositeSkill("针刺臂膀", ElementType::GRASS, SkillCategory::PHYSICAL, 70, 3, 100);
    thornArmSlam->setEffectChance(30);
    thornArmSlam->addEffect(new StatusConditionEffect(StatusCondition::POISON, 100));
    learnSkill(thornArmSlam);

    CompositeSkill *earthShaker = new CompositeSkill("大地摇晃", ElementType::GROUND, SkillCategory::SPECIAL, 90, 3, 100);
    earthShaker->setEffectChance(10);
    earthShaker->addEffect(new StatChangeEffect(StatType::SP_DEFENSE, -1, TargetType::OPPONENT));
    learnSkill(earthShaker);

    // 第五技能: 生命汲取领域
    setFifthSkill(new LifeSiphonFieldSkill());

}
void LiriliLarila::onTurnStart() { Creature::onTurnStart(); }
void LiriliLarila::onTurnEnd() { Creature::onTurnEnd(); }

// ChimpanziniBananini（香蕉绿猩猩）构造函数
class PrimalShiftSkill : public StatusSkill
{
public:
    PrimalShiftSkill() : StatusSkill("狂化变身", ElementType::NORMAL, 4, 100)
    {
    }
    bool use(Creature *user, Creature * /* target_unused */, BattleSystem * /* battle_unused */) override
    {
        // 中文注释：使用狂化变身技能
        // 调用基类的use进行PP消耗和基础判定 (虽然StatusSkill的use可能很简单)
        if (!StatusSkill::use(user, /* target_unused */ nullptr, /* battle_unused */ nullptr))
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
        return "进入\"狂暴形态\"：物攻等级+2，速度等级+1，物防等级-1，特防等级-1。此效果持续3回合，结束后恢复原形态和能力等级。";
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
    jungleFortitude->addEffect(new StatChangeEffect(StatType::DEFENSE, 2, TargetType::SELF));
    learnSkill(jungleFortitude);
    StatusSkill *primalRoar = new StatusSkill("野性咆哮", ElementType::NORMAL, 2, 100);
    primalRoar->addEffect(new StatChangeEffect(StatType::ATTACK, -1, TargetType::OPPONENT));
    primalRoar->addEffect(new StatChangeEffect(StatType::DEFENSE, -1, TargetType::OPPONENT));
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
void ChimpanziniBananini::onTurnStart()
{
    Creature::onTurnStart();
}
void ChimpanziniBananini::onTurnEnd()
{
    Creature::onTurnEnd();
    if (m_inBerserkForm)
    {
        m_berserkFormDuration--;
        if (m_berserkFormDuration <= 0)
        {
            exitBerserkForm();
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
    temporalRay->addEffect(new StatChangeEffect(StatType::SPEED, -1, TargetType::OPPONENT));
    learnSkill(temporalRay);

    StatusSkill *rewindHeal = new StatusSkill("回溯疗愈", ElementType::NORMAL, 3, 100);
    rewindHeal->addEffect(new ClearEffectsEffect(false, false, true, false, TargetType::SELF)); // 清除自身异常状态
    // 恢复至上回合结束时的HP，这个非常复杂，需要BattleSystem记录历史状态
    learnSkill(rewindHeal);

    StatusSkill *acceleratedVision = new StatusSkill("加速视界", ElementType::LIGHT, 2, 100);
    acceleratedVision->addEffect(new StatChangeEffect(StatType::SPEED, 1, TargetType::SELF));
    // 本回合技能必定命中，需要一个临时状态或BattleSystem配合
    learnSkill(acceleratedVision);

    // 时光跳跃: 先制+3。使自身本回合免疫所有攻击和技能效果。
    // 免疫效果通过TurnBasedEffect标记，由BattleSystem检查
    StatusSkill *timeHop = new StatusSkill("时光跳跃", ElementType::LIGHT, 2, 100, 3); // 优先级3
    auto immunityLambda = [](Creature * /* aff */, Creature * /* src */, BattleSystem * /* b */, TurnBasedEffect * /* self_eff */) { /* 标记用 */ };
    TurnBasedEffect *hopImmunity = new TurnBasedEffect(1, immunityLambda, true); // 持续1回合，回合开始生效
    hopImmunity->setDescription("时光跳跃免疫");
    timeHop->addEffect(hopImmunity);
    timeHop->getEffects().first()->setTargetType(TargetType::SELF);
    learnSkill(timeHop);

    // 第五技能: 时间悖论 (极其复杂，简化或标记为TODO)
    setFifthSkill(new TemporalParadoxSkill());
}

void Luguanluguanlulushijiandaole::recordBattleState() { /* TODO */ }
bool Luguanluguanlulushijiandaole::tryRevertBattleState(BattleSystem * /* battle_unused */) { /* TODO */ return false; }
void Luguanluguanlulushijiandaole::onTurnStart() { Creature::onTurnStart(); }
void Luguanluguanlulushijiandaole::onTurnEnd()
{
    Creature::onTurnEnd();
    if (m_snapshotTurnsLeft > 0)
    {
        m_snapshotTurnsLeft--;
        if (m_snapshotTurnsLeft == 0)
        {
            // 尝试触发时间悖论效果
            if (QRandomGenerator::global()->bounded(100) < 50)
            {
                // 50%几率成功
                tryRevertBattleState(GameEngine::getInstance()->getBattleSystem());
            }
            else
            {
                // 失败，自身陷入疲惫
                setStatusCondition(StatusCondition::TIRED);
            }
        }
    }
}
