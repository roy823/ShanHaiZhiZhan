#include "creature.h"
#include "../battle/battlesystem.h"
#include <QRandomGenerator>

// Creature基类实现
Creature::Creature(const QString& name, const Type& type, int level)
    : m_name(name), m_type(type), m_level(level), m_experience(0),
      m_fifthSkill(nullptr), m_statusCondition(StatusCondition::NONE) {
    
    // 初始化最大生命值和PP值
    m_maxHP = 100 + (m_level * 10);
    m_currentHP = m_maxHP;
    
    m_maxPP = 8;
    m_currentPP = m_maxPP;
}

Creature::~Creature() {
    // 删除技能
    for (Skill* skill : m_skills) {
        delete skill;
    }
    m_skills.clear();
    
    // 删除第五技能
    if (m_fifthSkill) {
        delete m_fifthSkill;
        m_fifthSkill = nullptr;
    }
    
    // 删除回合效果
    for (TurnBasedEffect* effect : m_turnEffects) {
        delete effect;
    }
    m_turnEffects.clear();
}

QString Creature::getName() const {
    return m_name;
}

Type Creature::getType() const {
    return m_type;
}

int Creature::getLevel() const {
    return m_level;
}

int Creature::getExperience() const {
    return m_experience;
}

int Creature::getExperienceToNextLevel() const {
    return calculateExperienceToNextLevel();
}

BaseStats Creature::getBaseStats() const {
    return m_baseStats;
}

BaseStats Creature::getCurrentStats() const {
    // 计算当前实际属性，包括能力等级的修正
    BaseStats currentStats = m_baseStats;
    
    // 应用能力等级修正
    double attackMod = StatStages::calculateModifier(StatType::ATTACK, m_statStages.getStage(StatType::ATTACK));
    double spAttackMod = StatStages::calculateModifier(StatType::SP_ATTACK, m_statStages.getStage(StatType::SP_ATTACK));
    double defenseMod = StatStages::calculateModifier(StatType::DEFENSE, m_statStages.getStage(StatType::DEFENSE));
    double spDefenseMod = StatStages::calculateModifier(StatType::SP_DEFENSE, m_statStages.getStage(StatType::SP_DEFENSE));
    double speedMod = StatStages::calculateModifier(StatType::SPEED, m_statStages.getStage(StatType::SPEED));
    
    int attackValue = static_cast<int>(m_baseStats.getStat(StatType::ATTACK) * attackMod);
    int spAttackValue = static_cast<int>(m_baseStats.getStat(StatType::SP_ATTACK) * spAttackMod);
    int defenseValue = static_cast<int>(m_baseStats.getStat(StatType::DEFENSE) * defenseMod);
    int spDefenseValue = static_cast<int>(m_baseStats.getStat(StatType::SP_DEFENSE) * spDefenseMod);
    int speedValue = static_cast<int>(m_baseStats.getStat(StatType::SPEED) * speedMod);
    
    currentStats.setStat(StatType::ATTACK, attackValue);
    currentStats.setStat(StatType::SP_ATTACK, spAttackValue);
    currentStats.setStat(StatType::DEFENSE, defenseValue);
    currentStats.setStat(StatType::SP_DEFENSE, spDefenseValue);
    currentStats.setStat(StatType::SPEED, speedValue);
    
    return currentStats;
}

StatStages Creature::getStatStages() const {
    return m_statStages;
}

Talent Creature::getTalent() const {
    return m_talent;
}

StatusCondition Creature::getStatusCondition() const {
    return m_statusCondition;
}

void Creature::setBaseStats(const BaseStats& stats) {
    m_baseStats = stats;
    
    // 重新计算最大生命值
    m_maxHP = m_baseStats.getStat(StatType::HP);
    m_currentHP = m_maxHP;
}

void Creature::setTalent(const Talent& talent) {
    m_talent = talent;
}

void Creature::setMaxPP(int maxPP) {
    m_maxPP = maxPP;
    if (m_currentPP > m_maxPP) {
        m_currentPP = m_maxPP;
    }
}

int Creature::getCurrentHP() const {
    return m_currentHP;
}

int Creature::getMaxHP() const {
    return m_maxHP;
}

int Creature::getCurrentPP() const {
    return m_currentPP;
}

int Creature::getMaxPP() const {
    return m_maxPP;
}

bool Creature::isDead() const {
    return m_currentHP <= 0;
}

bool Creature::canAct() const {
    // 检查是否能行动（没有被控制类异常状态影响）
    if (isDead()) {
        return false;
    }
    
    // 检查异常状态
    switch (m_statusCondition) {
        case StatusCondition::PARALYZE:
        case StatusCondition::SLEEP:
        case StatusCondition::FEAR:
        case StatusCondition::TIRED:
            return false;
            
        case StatusCondition::CONFUSION:
            // 混乱状态有50%几率无法行动
            return QRandomGenerator::global()->bounded(100) >= 50;
            
        default:
            return true;
    }
}

void Creature::gainExperience(int exp) {
    if (m_level >= MAX_LEVEL) {
        return;
    }
    
    m_experience += exp;
    
    // 检查是否升级
    tryLevelUp();
}

bool Creature::tryLevelUp() {
    if (m_level >= MAX_LEVEL) {
        return false;
    }
    
    int expNeeded = calculateExperienceToNextLevel();
    
    if (m_experience >= expNeeded) {
        // 升级
        m_level++;
        m_experience -= expNeeded;
        
        // 更新属性
        updateStatsOnLevelUp();
        
        // 继续检查是否可以再次升级
        return tryLevelUp();
    }
    
    return false;
}

void Creature::takeDamage(int damage) {
    m_currentHP = qMax(0, m_currentHP - damage);
}

void Creature::heal(int amount) {
    m_currentHP = qMin(m_maxHP, m_currentHP + amount);
}

void Creature::consumePP(int amount) {
    m_currentPP = qMax(0, m_currentPP - amount);
}

void Creature::restorePP(int amount) {
    m_currentPP = qMin(m_maxPP, m_currentPP + amount);
}

void Creature::setStatusCondition(StatusCondition condition) {
    m_statusCondition = condition;
}

void Creature::clearStatusCondition() {
    m_statusCondition = StatusCondition::NONE;
}

void Creature::modifyStatStage(StatType stat, int delta) {
    m_statStages.modifyStage(stat, delta);
}

void Creature::resetStatStages() {
    m_statStages.reset();
}

void Creature::learnSkill(Skill* skill) {
    if (skill) {
        m_skills.append(skill);
    }
}

void Creature::forgetSkill(int index) {
    if (index >= 0 && index < m_skills.size()) {
        delete m_skills[index];
        m_skills.removeAt(index);
    }
}

bool Creature::hasSkill(const QString& skillName) const {
    for (const Skill* skill : m_skills) {
        if (skill->getName() == skillName) {
            return true;
        }
    }
    
    if (m_fifthSkill && m_fifthSkill->getName() == skillName) {
        return true;
    }
    
    return false;
}

Skill* Creature::getSkill(int index) const {
    if (index >= 0 && index < m_skills.size()) {
        return m_skills[index];
    }
    
    return nullptr;
}

QVector<Skill*> Creature::getSkills() const {
    return m_skills;
}

int Creature::getSkillCount() const {
    return m_skills.size();
}

void Creature::setFifthSkill(Skill* skill) {
    if (m_fifthSkill) {
        delete m_fifthSkill;
    }
    m_fifthSkill = skill;
}

Skill* Creature::getFifthSkill() const {
    return m_fifthSkill;
}

void Creature::addTurnEffect(TurnBasedEffect* effect) {
    if (effect) {
        m_turnEffects.append(effect);
    }
}

void Creature::removeTurnEffect(TurnBasedEffect* effect) {
    m_turnEffects.removeOne(effect);
    delete effect;
}

void Creature::clearAllTurnEffects() {
    for (TurnBasedEffect* effect : m_turnEffects) {
        delete effect;
    }
    m_turnEffects.clear();
}

QVector<TurnBasedEffect*> Creature::getTurnEffects() const {
    return m_turnEffects;
}

bool Creature::useSkill(int skillIndex, Creature* target, BattleSystem* battle) {
    Skill* skill = nullptr;
    
    if (skillIndex >= 0 && skillIndex < m_skills.size()) {
        skill = m_skills[skillIndex];
    } else if (skillIndex == -1 && m_fifthSkill) {
        // 使用第五技能
        skill = m_fifthSkill;
    }
    
    if (skill && target) {
        return skill->use(this, target, battle);
    }
    
    return false;
}

void Creature::onTurnStart() {
    // 回合开始时处理效果
    QVector<TurnBasedEffect*> activeEffects = m_turnEffects;
    for (TurnBasedEffect* effect : activeEffects) {
        effect->onTurnStart(this, nullptr, nullptr);
    }
    
    // 处理异常状态效果
    switch (m_statusCondition) {
        case StatusCondition::POISON:
        case StatusCondition::BURN:
        case StatusCondition::FREEZE:
            // 扣除最大生命值的1/8
            takeDamage(m_maxHP / 8);
            break;
            
        case StatusCondition::BLEED:
            // 扣除固定80点生命值
            takeDamage(80);
            break;
            
        case StatusCondition::CONFUSION:
            // 5%几率扣除50点生命值
            if (QRandomGenerator::global()->bounded(100) < 5) {
                takeDamage(50);
            }
            break;
            
        default:
            break;
    }
}

void Creature::onTurnEnd() {
    // 回合结束时处理效果
    QVector<TurnBasedEffect*> activeEffects = m_turnEffects;
    for (TurnBasedEffect* effect : activeEffects) {
        effect->onTurnEnd(this, nullptr, nullptr);
        
        // 检查效果是否结束
        if (effect->getDuration() <= 0) {
            removeTurnEffect(effect);
        }
    }
}

int Creature::calculateAttack() const {
    BaseStats currentStats = getCurrentStats();
    return currentStats.getStat(StatType::ATTACK);
}

int Creature::calculateSpecialAttack() const {
    BaseStats currentStats = getCurrentStats();
    return currentStats.getStat(StatType::SP_ATTACK);
}

int Creature::calculateDefense() const {
    BaseStats currentStats = getCurrentStats();
    return currentStats.getStat(StatType::DEFENSE);
}

int Creature::calculateSpecialDefense() const {
    BaseStats currentStats = getCurrentStats();
    return currentStats.getStat(StatType::SP_DEFENSE);
}

int Creature::calculateSpeed() const {
    BaseStats currentStats = getCurrentStats();
    int speed = currentStats.getStat(StatType::SPEED);
    
    // 特定异常状态会影响速度
    if (m_statusCondition == StatusCondition::PARALYZE) {
        speed = speed / 2;
    }
    
    return speed;
}

double Creature::getTypeEffectivenessAgainst(const Creature* target, ElementType skillType) const {
    if (!target) {
        return 1.0;
    }
    
    Type attackType(skillType);
    return Type::calculateEffectiveness(attackType, target->getType());
}

bool Creature::hasTypeAdvantage(ElementType skillType) const {
    // 检查技能属性是否与精灵属性相同（STAB加成）
    return skillType == m_type.getPrimaryType() || skillType == m_type.getSecondaryType();
}

int Creature::calculateExperienceToNextLevel() const {
    // 简单的经验计算公式：基础经验值 * 等级的平方
    return BASE_EXP_NEEDED * (m_level * m_level) / 100;
}

void Creature::updateStatsOnLevelUp() {
    // 根据天赋增加属性
    int hpGrowth = m_talent.getGrowthRate(StatType::HP);
    int attackGrowth = m_talent.getGrowthRate(StatType::ATTACK);
    int spAttackGrowth = m_talent.getGrowthRate(StatType::SP_ATTACK);
    int defenseGrowth = m_talent.getGrowthRate(StatType::DEFENSE);
    int spDefenseGrowth = m_talent.getGrowthRate(StatType::SP_DEFENSE);
    int speedGrowth = m_talent.getGrowthRate(StatType::SPEED);
    
    // 每个属性增加对应的成长值
    m_baseStats.modifyStat(StatType::HP, hpGrowth);
    m_baseStats.modifyStat(StatType::ATTACK, attackGrowth);
    m_baseStats.modifyStat(StatType::SP_ATTACK, spAttackGrowth);
    m_baseStats.modifyStat(StatType::DEFENSE, defenseGrowth);
    m_baseStats.modifyStat(StatType::SP_DEFENSE, spDefenseGrowth);
    m_baseStats.modifyStat(StatType::SPEED, speedGrowth);
    
    // 更新最大生命值
    m_maxHP = m_baseStats.getStat(StatType::HP);
    
    // 升级时恢复满生命值和PP
    m_currentHP = m_maxHP;
    m_currentPP = m_maxPP;
}

// TungTungTung（木棍人）精灵实现
TungTungTung::TungTungTung(int level) 
    : Creature("木棍人", Type(ElementType::NORMAL), level) {
    
    // 设置基础属性
    BaseStats stats(120, 140, 80, 100, 90, 90); // HP, 物攻, 特攻, 物防, 特防, 速度
    setBaseStats(stats);
    
    // 设置天赋
    Talent talent(2, 3, 1, 2, 1, 1); // 生命值成长中等，物攻成长高，其他成长正常
    setTalent(talent);
    
    // 学习技能
    learnSkill(new PhysicalSkill("猛力挥击", ElementType::NORMAL, 130, 3, 95));
    learnSkill(new MultiHitSkill("三重连打", ElementType::NORMAL, SkillCategory::PHYSICAL, 60, 4, 90, 3, 3));
    
    StatusSkill* hardenWoodBody = new StatusSkill("硬化木身", ElementType::NORMAL, 3, 100);
    hardenWoodBody->addEffect(new StatChangeEffect(StatType::DEFENSE, 3, true)); // 提升自身物防+3
    hardenWoodBody->addEffect(new StatChangeEffect(StatType::SP_DEFENSE, 3, true)); // 提升自身特防+3
    
    // 添加回合治疗效果
    TurnBasedEffect* healEffect = new TurnBasedEffect(3);
    healEffect->setEndTurnEffect([](Creature* source, Creature* target, BattleSystem* battle) {
        if (source) {
            source->heal(source->getMaxHP() * 30 / 100); // 恢复30%最大生命值
        }
    });
    hardenWoodBody->addEffect(healEffect);
    
    learnSkill(hardenWoodBody);
    
    // 破甲直刺
    CompositeSkill* armorPierceThrust = new CompositeSkill("破甲直刺", ElementType::NORMAL, SkillCategory::PHYSICAL, 70, 3, 100);
    armorPierceThrust->setEffectChance(80); // 80%的触发几率
    armorPierceThrust->addEffect(new StatChangeEffect(StatType::DEFENSE, -2, false)); // 降低目标物防-2
    armorPierceThrust->addEffect(new ClearEffect(false, true, false)); // 清除目标能力强化
    
    learnSkill(armorPierceThrust);
    
    // 设置第五技能 - 不屈战魂
    FifthSkill* indomitableSpirit = new FifthSkill("不屈战魂", ElementType::NORMAL, SkillCategory::STATUS, 0, 3, 100);
    indomitableSpirit->addEffect(new StatChangeEffect(StatType::ATTACK, 2, true)); // 提升自身物攻+2
    indomitableSpirit->addEffect(new StatChangeEffect(StatType::SPEED, 1, true)); // 提升自身速度+1
    indomitableSpirit->addEffect(new HealingEffect(33)); // 恢复1/3最大生命值
    
    setFifthSkill(indomitableSpirit);
}

void TungTungTung::onTurnStart() {
    Creature::onTurnStart();
    
    // 木棍人的特殊回合开始效果
}

void TungTungTung::onTurnEnd() {
    Creature::onTurnEnd();
    
    // 木棍人的特殊回合结束效果
}

// BombardinoCrocodillo（鳄鱼轰炸机）精灵实现
BombardinoCrocodillo::BombardinoCrocodillo(int level)
    : Creature("鳄鱼轰炸机", Type(ElementType::FLYING, ElementType::MACHINE), level) {
    
    // 设置基础属性
    BaseStats stats(110, 120, 110, 120, 90, 100); // HP, 物攻, 特攻, 物防, 特防, 速度
    setBaseStats(stats);
    
    // 设置天赋
    Talent talent(2, 2, 2, 3, 1, 2); // 生命值成长中等，物攻成长中等，物防高，其他成长正常
    setTalent(talent);
    
    // 学习技能
    learnSkill(new PhysicalSkill("钢翼切割", ElementType::MACHINE, 75, 3, 95));
    
    // 俯冲轰炸
    CompositeSkill* diveBomb = new CompositeSkill("俯冲轰炸", ElementType::FLYING, SkillCategory::PHYSICAL, 120, 4, 90);
    diveBomb->addEffect(new StatusConditionEffect(StatusCondition::TIRED, 100)); // 使用后下回合无法行动
    
    learnSkill(diveBomb);
    
    // 鳄牙撕咬
    CompositeSkill* alligatorFang = new CompositeSkill("鳄牙撕咬", ElementType::WATER, SkillCategory::PHYSICAL, 80, 3, 100);
    alligatorFang->setEffectChance(20); // 20%的触发几率
    alligatorFang->addEffect(new StatusConditionEffect(StatusCondition::FEAR, 100)); // 令目标害怕
    
    learnSkill(alligatorFang);
    
    learnSkill(new SpecialSkill("锁定导弹", ElementType::MACHINE, 80, 3, 100)); // 必中技能
    
    // 设置第五技能 - 空域压制
    FifthSkill* airspaceSupremacy = new FifthSkill("空域压制", ElementType::FLYING, SkillCategory::STATUS, 0, 3, 100);
    
    // 创建持续3回合的效果
    TurnBasedEffect* airspaceEffect = new TurnBasedEffect(3);
    airspaceEffect->setStartTurnEffect([](Creature* source, Creature* target, BattleSystem* battle) {
        if (target) {
            target->modifyStatStage(StatType::SPEED, -1); // 降低对方速度等级1级
        }
    });
    
    airspaceSupremacy->addEffect(airspaceEffect);
    
    setFifthSkill(airspaceSupremacy);
}

void BombardinoCrocodillo::onTurnStart() {
    Creature::onTurnStart();
    
    // 鳄鱼轰炸机的特殊回合开始效果
}

void BombardinoCrocodillo::onTurnEnd() {
    Creature::onTurnEnd();
    
    // 鳄鱼轰炸机的特殊回合结束效果
}

// TralaleroTralala（耐克鲨鱼）精灵实现
TralaleroTralala::TralaleroTralala(int level)
    : Creature("耐克鲨鱼", Type(ElementType::WATER, ElementType::SHADOW), level) {
    
    // 设置基础属性
    BaseStats stats(90, 110, 100, 80, 80, 140); // HP, 物攻, 特攻, 物防, 特防, 速度
    setBaseStats(stats);
    
    // 设置天赋
    Talent talent(1, 2, 2, 1, 1, 3); // 速度成长极高，物攻和特攻成长中等，其他正常
    setTalent(talent);
    
    // 学习技能
    PrioritySkill* shadowSneak = new PrioritySkill("暗影偷袭", ElementType::SHADOW, SkillCategory::PHYSICAL, 40, 2, 100, 1);
    learnSkill(shadowSneak);
    
    // 激流勇进
    SpecialSkill* ragingCurrent = new SpecialSkill("激流勇进", ElementType::WATER, 80, 3, 100);
    learnSkill(ragingCurrent);
    
    // 速度之星
    SpecialSkill* speedStar = new SpecialSkill("速度之星", ElementType::NORMAL, 60, 2, 100);
    learnSkill(speedStar);
    
    // 伺机待发
    StatusSkill* opportunist = new StatusSkill("伺机待发", ElementType::SHADOW, 2, 100);
    opportunist->addEffect(new StatChangeEffect(StatType::SPEED, 2, true)); // 提升自身速度+2
    learnSkill(opportunist);
    
    // 设置第五技能 - 极速掠食
    FifthSkill* blitzPredator = new FifthSkill("极速掠食", ElementType::WATER, SkillCategory::PHYSICAL, 100, 4, 95);
    blitzPredator->addEffect(new StatChangeEffect(StatType::SPEED, 1, true)); // 提升自身速度+1
    setFifthSkill(blitzPredator);
}

void TralaleroTralala::onTurnStart() {
    Creature::onTurnStart();
    
    // 耐克鲨鱼的特殊回合开始效果
}

void TralaleroTralala::onTurnEnd() {
    Creature::onTurnEnd();
    
    // 检查当前HP是否低于1/3，如果是则激活激流勇进的额外效果
}

// LiriliLarila（仙人掌大象）精灵实现
LiriliLarila::LiriliLarila(int level)
    : Creature("仙人掌大象", Type(ElementType::GRASS, ElementType::GROUND), level) {
    
    // 设置基础属性
    BaseStats stats(150, 80, 120, 130, 120, 70); // HP, 物攻, 特攻, 物防, 特防, 速度
    setBaseStats(stats);
    
    // 设置天赋
    Talent talent(3, 1, 2, 3, 2, 1); // 生命成长高，物防成长高，特攻和特防中等，其他正常
    setTalent(talent);
    
    // 学习技能
    StatusSkill* leechSeed = new StatusSkill("寄生种子", ElementType::GRASS, 2, 90);
    TurnBasedEffect* leechSeedEffect = new TurnBasedEffect(999); // 持续直到对手交换
    leechSeedEffect->setEndTurnEffect([](Creature* source, Creature* target, BattleSystem* battle) {
        if (source && target) {
            int leechAmount = target->getMaxHP() / 8;
            target->takeDamage(leechAmount);
            source->heal(leechAmount);
        }
    });
    leechSeed->addEffect(leechSeedEffect);
    learnSkill(leechSeed);
    
    // 沙尘尖刺
    StatusSkill* sandSpikes = new StatusSkill("沙尘尖刺", ElementType::GROUND, 3, 100);
    // 具体效果需要在战斗系统中实现
    learnSkill(sandSpikes);
    
    // 针刺臂膀
    CompositeSkill* thornArmSlam = new CompositeSkill("针刺臂膀", ElementType::GRASS, SkillCategory::PHYSICAL, 70, 3, 100);
    thornArmSlam->setEffectChance(30); // 30%的触发几率
    thornArmSlam->addEffect(new StatusConditionEffect(StatusCondition::POISON, 100)); // 令目标中毒
    learnSkill(thornArmSlam);
    
    // 大地摇晃
    CompositeSkill* earthShaker = new CompositeSkill("大地摇晃", ElementType::GROUND, SkillCategory::SPECIAL, 90, 3, 100);
    earthShaker->setEffectChance(10); // 10%的触发几率
    earthShaker->addEffect(new StatChangeEffect(StatType::SP_DEFENSE, -1, false)); // 降低目标特防-1
    learnSkill(earthShaker);
    
    // 设置第五技能 - 生命汲取领域
    FifthSkill* lifeSiphonField = new FifthSkill("生命汲取领域", ElementType::GRASS, SkillCategory::STATUS, 0, 4, 100);
    TurnBasedEffect* siphonFieldEffect = new TurnBasedEffect(3);
    siphonFieldEffect->setEndTurnEffect([](Creature* source, Creature* target, BattleSystem* battle) {
        if (source && target) {
            // 非草系精灵每回合损失最大体力的1/16
            if (target->getType().getPrimaryType() != ElementType::GRASS && 
                target->getType().getSecondaryType() != ElementType::GRASS) {
                target->takeDamage(target->getMaxHP() / 16);
            }
            
            // 自身每回合恢复最大体力的1/8
            source->heal(source->getMaxHP() / 8);
        }
    });
    lifeSiphonField->addEffect(siphonFieldEffect);
    setFifthSkill(lifeSiphonField);
}

void LiriliLarila::onTurnStart() {
    Creature::onTurnStart();
    
    // 仙人掌大象的特殊回合开始效果
}

void LiriliLarila::onTurnEnd() {
    Creature::onTurnEnd();
    
    // 仙人掌大象的特殊回合结束效果
}

// ChimpanziniBananini（香蕉绿猩猩）精灵实现
ChimpanziniBananini::ChimpanziniBananini(int level)
    : Creature("香蕉绿猩猩", Type(ElementType::GRASS, ElementType::NORMAL), level),
      m_inBerserkForm(false), m_berserkFormDuration(0) {
    
    // 设置基础属性
    BaseStats stats(130, 120, 70, 120, 100, 90); // HP, 物攻, 特攻, 物防, 特防, 速度
    setBaseStats(stats);
    
    // 设置天赋
    Talent talent(2, 2, 1, 3, 2, 1); // 物防成长高，生命和物攻中等，特防中等，其他正常
    setTalent(talent);
    
    // 学习技能
    PhysicalSkill* bananaSlam = new PhysicalSkill("香蕉猛击", ElementType::GRASS, 85, 3, 100);
    // 可以在战斗系统中实现10%几率恢复造成伤害25%的体力
    learnSkill(bananaSlam);
    
    learnSkill(new PhysicalSkill("巨力冲拳", ElementType::NORMAL, 90, 3, 95));
    
    StatusSkill* jungleFortitude = new StatusSkill("丛林坚壁", ElementType::GRASS, 2, 100);
    jungleFortitude->addEffect(new StatChangeEffect(StatType::DEFENSE, 2, true)); // 提升自身物防+2
    learnSkill(jungleFortitude);
    
    StatusSkill* primalRoar = new StatusSkill("野性咆哮", ElementType::NORMAL, 2, 100);
    primalRoar->addEffect(new StatChangeEffect(StatType::ATTACK, -1, false)); // 降低对手物攻-1
    primalRoar->addEffect(new StatChangeEffect(StatType::DEFENSE, -1, false)); // 降低对手物防-1
    learnSkill(primalRoar);
    
    // 狂化变身（核心变身技能）
    StatusSkill* primalShift = new StatusSkill("狂化变身", ElementType::NORMAL, 4, 100);
    primalShift->addEffect(new StatChangeEffect(StatType::ATTACK, 2, true)); // 提升自身物攻+2
    primalShift->addEffect(new StatChangeEffect(StatType::SPEED, 1, true)); // 提升自身速度+1
    primalShift->addEffect(new StatChangeEffect(StatType::DEFENSE, -1, true)); // 降低自身物防-1
    primalShift->addEffect(new StatChangeEffect(StatType::SP_DEFENSE, -1, true)); // 降低自身特防-1
    learnSkill(primalShift);
    
    // 设置第五技能 - 丛林之王强击
    FifthSkill* jungleKingStrike = new FifthSkill("丛林之王强击", ElementType::GRASS, SkillCategory::PHYSICAL, 130, 5, 90);
    setFifthSkill(jungleKingStrike);
}

bool ChimpanziniBananini::isInBerserkForm() const {
    return m_inBerserkForm;
}

void ChimpanziniBananini::enterBerserkForm(int duration) {
    m_inBerserkForm = true;
    m_berserkFormDuration = duration;
}

void ChimpanziniBananini::exitBerserkForm() {
    if (m_inBerserkForm) {
        m_inBerserkForm = false;
        m_berserkFormDuration = 0;
        
        // 重置能力等级变化
        resetStatStages();
    }
}

void ChimpanziniBananini::onTurnStart() {
    Creature::onTurnStart();
    
    // 香蕉绿猩猩的特殊回合开始效果
}

void ChimpanziniBananini::onTurnEnd() {
    Creature::onTurnEnd();
    
    // 处理狂暴形态持续时间
    if (m_inBerserkForm) {
        m_berserkFormDuration--;
        if (m_berserkFormDuration <= 0) {
            exitBerserkForm();
        }
    }
}

// Luguanluguanlulushijiandaole（鹿管鹿管鹿鹿时间到了）精灵实现
Luguanluguanlulushijiandaole::Luguanluguanlulushijiandaole(int level)
    : Creature("鹿管鹿管鹿鹿时间到了", Type(ElementType::LIGHT, ElementType::NORMAL), level),
      m_snapshotTurnsLeft(0) {
    
    // 设置基础属性
    BaseStats stats(100, 80, 130, 90, 120, 110); // HP, 物攻, 特攻, 物防, 特防, 速度
    setBaseStats(stats);
    
    // 设置天赋
    Talent talent(2, 1, 3, 1, 2, 2); // 特攻成长高，生命和特防中等，速度中等，其他正常
    setTalent(talent);
    
    // 学习技能
    learnSkill(new SpecialSkill("时光射线", ElementType::LIGHT, 70, 3, 100));
    
    // 回溯疗愈
    StatusSkill* rewindHeal = new StatusSkill("回溯疗愈", ElementType::NORMAL, 3, 100);
    rewindHeal->addEffect(new ClearEffect(true, false, false)); // 解除自身所有异常状态
    // 恢复至上回合结束时的HP需要在战斗系统中实现
    learnSkill(rewindHeal);
    
    // 加速视界
    StatusSkill* acceleratedVision = new StatusSkill("加速视界", ElementType::LIGHT, 2, 100);
    acceleratedVision->addEffect(new StatChangeEffect(StatType::SPEED, 1, true)); // 提升自身速度+1
    learnSkill(acceleratedVision);
    
    // 时光跳跃
    PrioritySkill* timeHop = new PrioritySkill("时光跳跃", ElementType::LIGHT, SkillCategory::STATUS, 0, 2, 100, 3);
    // 免疫所有攻击和技能效果需要在战斗系统中实现
    learnSkill(timeHop);
    
    // 设置第五技能 - 时间悖论
    FifthSkill* temporalParadox = new FifthSkill("时间悖论", ElementType::NORMAL, SkillCategory::STATUS, 0, 5, 100);
    setFifthSkill(temporalParadox);
}

void Luguanluguanlulushijiandaole::recordBattleState() {
    // 记录当前战场状态
    m_recordedState.timestamp = QDateTime::currentSecsSinceEpoch();
    m_snapshotTurnsLeft = 3;
    
    // 注意：实际的状态记录操作需要在战斗系统中完成
}

bool Luguanluguanlulushijiandaole::tryRevertBattleState(BattleSystem* battle) {
    // 尝试恢复到记录的状态
    if (m_snapshotTurnsLeft <= 0) {
        return false;
    }
    
    // 随机确定是否成功
    bool success = QRandomGenerator::global()->bounded(100) < 50;
    
    if (!success) {
        // 失败时自身陷入疲惫
        setStatusCondition(StatusCondition::TIRED);
    }
    
    // 重置倒计时
    m_snapshotTurnsLeft = 0;
    
    return success;
}

void Luguanluguanlulushijiandaole::onTurnStart() {
    Creature::onTurnStart();
    
    // 鹿管鹿管鹿鹿时间到了的特殊回合开始效果
}

void Luguanluguanlulushijiandaole::onTurnEnd() {
    Creature::onTurnEnd();
    
    // 更新倒计时
    if (m_snapshotTurnsLeft > 0) {
        m_snapshotTurnsLeft--;
    }
}

// CappuccinoAssassino（卡布奇诺忍者）精灵实现
CappuccinoAssassino::CappuccinoAssassino(int level)
    : Creature("卡布奇诺忍者", Type(ElementType::SHADOW, ElementType::MACHINE), level),
      m_inShadowState(false) {
    
    // 设置基础属性
    BaseStats stats(85, 130, 90, 70, 80, 135); // HP, 物攻, 特攻, 物防, 特防, 速度
    setBaseStats(stats);
    
    // 设置天赋
    Talent talent(1, 3, 1, 1, 1, 3); // 物攻和速度成长高，其他正常
    setTalent(talent);
    
    // 学习技能
    MultiHitSkill* shadowShuriken = new MultiHitSkill("影手里剑", ElementType::SHADOW, SkillCategory::PHYSICAL, 25, 2, 100, 2, 3, 1);
    learnSkill(shadowShuriken);
    
    // 滚烫奇袭
    CompositeSkill* scaldingSurprise = new CompositeSkill("滚烫奇袭", ElementType::FIRE, SkillCategory::SPECIAL, 70, 3, 100);
    scaldingSurprise->setEffectChance(30); // 基础30%的触发几率
    scaldingSurprise->addEffect(new StatusConditionEffect(StatusCondition::BURN, 100)); // 令目标烧伤
    // 如果目标速度低于自身，烧伤几率提升至60%的效果需要在战斗系统中实现
    learnSkill(scaldingSurprise);
    
    // 金属研磨
    CompositeSkill* metalGrind = new CompositeSkill("金属研磨", ElementType::MACHINE, SkillCategory::PHYSICAL, 75, 3, 95);
    metalGrind->setEffectChance(30); // 30%的触发几率
    metalGrind->addEffect(new StatChangeEffect(StatType::SPEED, -1, false)); // 降低目标速度-1
    learnSkill(metalGrind);
    
    // 急速隐匿
    StatusSkill* swiftVanish = new StatusSkill("急速隐匿", ElementType::SHADOW, 2, 100);
    swiftVanish->addEffect(new StatChangeEffect(StatType::SPEED, 2, true)); // 提升自身速度+2
    learnSkill(swiftVanish);
    
    // 设置第五技能 - 绝影刺杀
    FifthSkill* phantomAssassinate = new FifthSkill("绝影刺杀", ElementType::SHADOW, SkillCategory::PHYSICAL, 90, 4, 100, 1);
    setFifthSkill(phantomAssassinate);
}

bool CappuccinoAssassino::isInShadowState() const {
    return m_inShadowState;
}

void CappuccinoAssassino::enterShadowState() {
    m_inShadowState = true;
}

void CappuccinoAssassino::exitShadowState() {
    m_inShadowState = false;
}

void CappuccinoAssassino::onTurnStart() {
    Creature::onTurnStart();
    
    // 卡布奇诺忍者的特殊回合开始效果
}

void CappuccinoAssassino::onTurnEnd() {
    Creature::onTurnEnd();
    
    // 卡布奇诺忍者的特殊回合结束效果
    // 如处理影子状态
    if (m_inShadowState) {
        exitShadowState();
    }
}
