#include "skill.h"
#include "../core/creature.h"
#include "battlesystem.h"
#include <QRandomGenerator>

// Skill基类实现
Skill::Skill(const QString& name, ElementType type, SkillCategory category, 
             int power, int ppCost, int accuracy, int priority)
    : m_name(name), m_type(type), m_category(category), 
      m_power(power), m_ppCost(ppCost), m_accuracy(accuracy), m_priority(priority) {
}

Skill::~Skill() {
    // 删除所有效果
    for (Effect* effect : m_effects) {
        delete effect;
    }
    m_effects.clear();
}

QString Skill::getName() const {
    return m_name;
}

ElementType Skill::getType() const {
    return m_type;
}

SkillCategory Skill::getCategory() const {
    return m_category;
}

int Skill::getPower() const {
    return m_power;
}

int Skill::getPPCost() const {
    return m_ppCost;
}

int Skill::getAccuracy() const {
    return m_accuracy;
}

int Skill::getPriority() const {
    return m_priority;
}

bool Skill::use(Creature* user, Creature* target, BattleSystem* battle) {
    // 检查用户是否有足够的PP
    if (user && user->getCurrentPP() >= m_ppCost) {
        // 消耗PP
        user->consumePP(m_ppCost);
        
        // 检查命中
        if (checkHit(user, target)) {
            // 如果是攻击技能，计算伤害
            if (m_category == SkillCategory::PHYSICAL || m_category == SkillCategory::SPECIAL) {
                int damage = calculateDamage(user, target);
                target->takeDamage(damage);
            }
            
            // 应用所有效果
            for (Effect* effect : m_effects) {
                effect->apply(user, target, battle);
            }
            
            return true;
        }
        
        // 未命中
        return false;
    }
    
    // PP不足，无法使用技能
    return false;
}

QString Skill::getDescription() const {
    QString description = m_name + "\n";
    description += "系别: " + Type::getTypeName(m_type) + "\n";
    
    switch (m_category) {
        case SkillCategory::PHYSICAL:
            description += "类别: 物理攻击\n";
            break;
        case SkillCategory::SPECIAL:
            description += "类别: 特殊攻击\n";
            break;
        case SkillCategory::STATUS:
            description += "类别: 属性\n";
            break;
    }
    
    description += "威力: " + QString::number(m_power) + "\n";
    description += "PP消耗: " + QString::number(m_ppCost) + "\n";
    
    if (isAlwaysHit()) {
        description += "命中: 必中\n";
    } else {
        description += "命中: " + QString::number(m_accuracy) + "%\n";
    }
    
    if (m_priority != 0) {
        description += "优先级: " + QString::number(m_priority) + "\n";
    }
    
    // 添加效果描述
    if (!m_effects.isEmpty()) {
        description += "效果:\n";
        for (Effect* effect : m_effects) {
            description += "- " + effect->getDescription() + "\n";
        }
    }
    
    return description;
}

bool Skill::isAlwaysHit() const {
    return m_accuracy >= 100;
}

void Skill::addEffect(Effect* effect) {
    if (effect) {
        m_effects.append(effect);
    }
}

const QVector<Effect*>& Skill::getEffects() const {
    return m_effects;
}

int Skill::calculateDamage(Creature* user, Creature* target) {
    if (!user || !target) {
        return 0;
    }
    
    // 实际伤害计算会由战斗系统处理
    // 这里返回基础威力
    return m_power;
}

bool Skill::checkHit(Creature* user, Creature* target) {
    if (!user || !target || isAlwaysHit()) {
        return true;
    }
    
    // 实际命中计算会由战斗系统处理
    int hitChance = QRandomGenerator::global()->bounded(100);
    return hitChance < m_accuracy;
}

// PhysicalSkill实现
PhysicalSkill::PhysicalSkill(const QString& name, ElementType type, int power, 
                             int ppCost, int accuracy, int priority)
    : Skill(name, type, SkillCategory::PHYSICAL, power, ppCost, accuracy, priority) {
}

int PhysicalSkill::calculateDamage(Creature* user, Creature* target) {
    if (!user || !target) {
        return 0;
    }
    
    // 基础伤害计算
    int userAttack = user->calculateAttack();
    int targetDefense = target->calculateDefense();
    
    // 计算伤害公式：(2 * 等级 / 5 + 2) * 威力 * 攻击 / 防御 / 50 + 2
    int damage = ((2 * user->getLevel() / 5 + 2) * m_power * userAttack / targetDefense) / 50 + 2;
    
    // 应用属性相克
    double typeModifier = user->getTypeEffectivenessAgainst(target, m_type);
    damage = static_cast<int>(damage * typeModifier);
    
    // 随机浮动（85%-100%）
    int randomFactor = QRandomGenerator::global()->bounded(85, 101);
    damage = damage * randomFactor / 100;
    
    return damage;
}

// SpecialSkill实现
SpecialSkill::SpecialSkill(const QString& name, ElementType type, int power, 
                           int ppCost, int accuracy, int priority)
    : Skill(name, type, SkillCategory::SPECIAL, power, ppCost, accuracy, priority) {
}

int SpecialSkill::calculateDamage(Creature* user, Creature* target) {
    if (!user || !target) {
        return 0;
    }
    
    // 基础伤害计算
    int userSpecialAttack = user->calculateSpecialAttack();
    int targetSpecialDefense = target->calculateSpecialDefense();
    
    // 计算伤害公式：(2 * 等级 / 5 + 2) * 威力 * 特攻 / 特防 / 50 + 2
    int damage = ((2 * user->getLevel() / 5 + 2) * m_power * userSpecialAttack / targetSpecialDefense) / 50 + 2;
    
    // 应用属性相克
    double typeModifier = user->getTypeEffectivenessAgainst(target, m_type);
    damage = static_cast<int>(damage * typeModifier);
    
    // 随机浮动（85%-100%）
    int randomFactor = QRandomGenerator::global()->bounded(85, 101);
    damage = damage * randomFactor / 100;
    
    return damage;
}

// StatusSkill实现
StatusSkill::StatusSkill(const QString& name, ElementType type, 
                         int ppCost, int accuracy, int priority)
    : Skill(name, type, SkillCategory::STATUS, 0, ppCost, accuracy, priority) {
}

bool StatusSkill::use(Creature* user, Creature* target, BattleSystem* battle) {
    // 检查用户是否有足够的PP
    if (user && user->getCurrentPP() >= m_ppCost) {
        // 消耗PP
        user->consumePP(m_ppCost);
        
        // 检查命中
        if (checkHit(user, target)) {
            // 应用所有效果
            for (Effect* effect : m_effects) {
                effect->apply(user, target, battle);
            }
            
            return true;
        }
        
        // 未命中
        return false;
    }
    
    // PP不足，无法使用技能
    return false;
}

// CompositeSkill实现
CompositeSkill::CompositeSkill(const QString& name, ElementType type, SkillCategory category, 
                               int power, int ppCost, int accuracy, int priority)
    : Skill(name, type, category, power, ppCost, accuracy, priority), m_effectChance(100) {
}

void CompositeSkill::setEffectChance(int chance) {
    m_effectChance = qBound(0, chance, 100);
}

int CompositeSkill::getEffectChance() const {
    return m_effectChance;
}

bool CompositeSkill::use(Creature* user, Creature* target, BattleSystem* battle) {
    // 基本使用逻辑与基类相同
    bool success = Skill::use(user, target, battle);
    
    if (success) {
        // 检查是否触发额外效果
        int random = QRandomGenerator::global()->bounded(100);
        if (random < m_effectChance) {
            // 应用所有效果
            for (Effect* effect : m_effects) {
                effect->apply(user, target, battle);
            }
        }
    }
    
    return success;
}

// MultiHitSkill实现
MultiHitSkill::MultiHitSkill(const QString& name, ElementType type, SkillCategory category, 
                             int power, int ppCost, int accuracy, 
                             int minHits, int maxHits, int priority)
    : Skill(name, type, category, power, ppCost, accuracy, priority),
      m_minHits(minHits), m_maxHits(maxHits) {
}

bool MultiHitSkill::use(Creature* user, Creature* target, BattleSystem* battle) {
    // 检查用户是否有足够的PP
    if (!user || user->getCurrentPP() < m_ppCost) {
        return false;
    }
    
    // 消耗PP
    user->consumePP(m_ppCost);
    
    // 计算攻击次数
    int hits = 0;
    if (m_minHits == m_maxHits) {
        hits = m_minHits;
    } else {
        hits = QRandomGenerator::global()->bounded(m_minHits, m_maxHits + 1);
    }
    
    // 执行多次攻击
    bool hitAtLeastOnce = false;
    for (int i = 0; i < hits; ++i) {
        if (checkHit(user, target)) {
            hitAtLeastOnce = true;
            
            // 如果是攻击技能，计算伤害
            if (m_category == SkillCategory::PHYSICAL || m_category == SkillCategory::SPECIAL) {
                int damage = calculateDamage(user, target);
                target->takeDamage(damage);
            }
        }
    }
    
    // 如果至少命中一次，应用效果
    if (hitAtLeastOnce) {
        for (Effect* effect : m_effects) {
            effect->apply(user, target, battle);
        }
        return true;
    }
    
    return false;
}

// PrioritySkill实现
PrioritySkill::PrioritySkill(const QString& name, ElementType type, SkillCategory category, 
                             int power, int ppCost, int accuracy, int priority)
    : Skill(name, type, category, power, ppCost, accuracy, priority) {
}

// FixedDamageSkill实现
FixedDamageSkill::FixedDamageSkill(const QString& name, ElementType type, 
                                  int damage, int ppCost, int accuracy, int priority)
    : Skill(name, type, SkillCategory::SPECIAL, 0, ppCost, accuracy, priority),
      m_fixedDamage(damage) {
}

int FixedDamageSkill::calculateDamage(Creature* user, Creature* target) {
    // 固定伤害不受属性和能力值影响
    return m_fixedDamage;
}

// HealingSkill实现
HealingSkill::HealingSkill(const QString& name, ElementType type, 
                          int ppCost, int accuracy, int healPercent, int priority)
    : StatusSkill(name, type, ppCost, accuracy, priority),
      m_healPercent(healPercent) {
}

bool HealingSkill::use(Creature* user, Creature* target, BattleSystem* battle) {
    // 检查用户是否有足够的PP
    if (!user || user->getCurrentPP() < m_ppCost) {
        return false;
    }
    
    // 消耗PP
    user->consumePP(m_ppCost);
    
    // 计算治疗量
    int maxHP = user->getMaxHP();
    int healAmount = maxHP * m_healPercent / 100;
    
    // 执行治疗
    user->heal(healAmount);
    
    // 应用额外效果
    for (Effect* effect : m_effects) {
        effect->apply(user, target, battle);
    }
    
    return true;
}

// StatChangeSkill实现
StatChangeSkill::StatChangeSkill(const QString& name, ElementType type, 
                                int ppCost, int accuracy, int priority)
    : StatusSkill(name, type, ppCost, accuracy, priority) {
}

void StatChangeSkill::addStatChange(StatType stat, int stages, bool targetSelf) {
    StatChange change;
    change.stat = stat;
    change.stages = stages;
    change.targetSelf = targetSelf;
    m_statChanges.append(change);
}

bool StatChangeSkill::use(Creature* user, Creature* target, BattleSystem* battle) {
    // 检查用户是否有足够的PP
    if (!user || user->getCurrentPP() < m_ppCost) {
        return false;
    }
    
    // 消耗PP
    user->consumePP(m_ppCost);
    
    // 检查命中
    if (!checkHit(user, target)) {
        return false;
    }
    
    // 应用能力变化
    for (const StatChange& change : m_statChanges) {
        if (change.targetSelf) {
            user->modifyStatStage(change.stat, change.stages);
        } else {
            target->modifyStatStage(change.stat, change.stages);
        }
    }
    
    // 应用额外效果
    for (Effect* effect : m_effects) {
        effect->apply(user, target, battle);
    }
    
    return true;
}

// FifthSkill实现
FifthSkill::FifthSkill(const QString& name, ElementType type, SkillCategory category, 
                      int power, int ppCost, int accuracy, int priority)
    : Skill(name, type, category, power, ppCost, accuracy, priority) {
}

bool FifthSkill::canUse(Creature* user, Creature* target) const {
    // 默认实现，可以在子类中重写以添加特殊条件
    return true;
}
