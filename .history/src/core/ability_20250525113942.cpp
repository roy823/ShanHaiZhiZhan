#include "ability.h"
#include <cmath>
#include <algorithm>

// BaseStats 类实现
BaseStats::BaseStats(int hp, int attack, int spAttack, int defense, int spDefense, int speed)
    : m_hp(hp), m_attack(attack), m_spAttack(spAttack), 
      m_defense(defense), m_spDefense(spDefense), m_speed(speed) {
}

int BaseStats::getStat(StatType type) const {
    switch (type) {
        case StatType::HP:           return m_hp;
        case StatType::ATTACK:       return m_attack;
        case StatType::SP_ATTACK:    return m_spAttack;
        case StatType::DEFENSE:      return m_defense;
        case StatType::SP_DEFENSE:   return m_spDefense;
        case StatType::SPEED:        return m_speed;
        default:                     return 0;
    }
}

void BaseStats::setStat(StatType type, int value) {
    // 确保值不为负数
    value = std::max(0, value);
    
    switch (type) {
        case StatType::HP:           m_hp = value; break;
        case StatType::ATTACK:       m_attack = value; break;
        case StatType::SP_ATTACK:    m_spAttack = value; break;
        case StatType::DEFENSE:      m_defense = value; break;
        case StatType::SP_DEFENSE:   m_spDefense = value; break;
        case StatType::SPEED:        m_speed = value; break;
        default: break;
    }
}

void BaseStats::modifyStat(StatType type, int delta) {
    switch (type) {
        case StatType::HP:           m_hp = std::max(0, m_hp + delta); break;
        case StatType::ATTACK:       m_attack = std::max(0, m_attack + delta); break;
        case StatType::SP_ATTACK:    m_spAttack = std::max(0, m_spAttack + delta); break;
        case StatType::DEFENSE:      m_defense = std::max(0, m_defense + delta); break;
        case StatType::SP_DEFENSE:   m_spDefense = std::max(0, m_spDefense + delta); break;
        case StatType::SPEED:        m_speed = std::max(0, m_speed + delta); break;
        default: break;
    }
}

// StatStages 类实现
StatStages::StatStages()
    : m_attackStage(0), m_spAttackStage(0), m_defenseStage(0),
      m_spDefenseStage(0), m_speedStage(0), m_accuracyStage(0),
      m_evasionStage(0) {
}

int StatStages::getStage(StatType type) const {
    switch (type) {
        case StatType::ATTACK:       return m_attackStage;
        case StatType::SP_ATTACK:    return m_spAttackStage;
        case StatType::DEFENSE:      return m_defenseStage;
        case StatType::SP_DEFENSE:   return m_spDefenseStage;
        case StatType::SPEED:        return m_speedStage;
        case StatType::ACCURACY:     return m_accuracyStage;
        case StatType::EVASION:      return m_evasionStage;
        default:                     return 0;
    }
}

void StatStages::setStage(StatType type, int stage) {
    // 限制范围在-6到+6之间
    stage = clampStage(stage);
    
    switch (type) {
        case StatType::ATTACK:       m_attackStage = stage; break;
        case StatType::SP_ATTACK:    m_spAttackStage = stage; break;
        case StatType::DEFENSE:      m_defenseStage = stage; break;
        case StatType::SP_DEFENSE:   m_spDefenseStage = stage; break;
        case StatType::SPEED:        m_speedStage = stage; break;
        case StatType::ACCURACY:     m_accuracyStage = stage; break;
        case StatType::EVASION:      m_evasionStage = stage; break;
        default: break;
    }
}

void StatStages::modifyStage(StatType type, int delta) {
    switch (type) {
        case StatType::ATTACK:       m_attackStage = clampStage(m_attackStage + delta); break;
        case StatType::SP_ATTACK:    m_spAttackStage = clampStage(m_spAttackStage + delta); break;
        case StatType::DEFENSE:      m_defenseStage = clampStage(m_defenseStage + delta); break;
        case StatType::SP_DEFENSE:   m_spDefenseStage = clampStage(m_spDefenseStage + delta); break;
        case StatType::SPEED:        m_speedStage = clampStage(m_speedStage + delta); break;
        case StatType::ACCURACY:     m_accuracyStage = clampStage(m_accuracyStage + delta); break;
        case StatType::EVASION:      m_evasionStage = clampStage(m_evasionStage + delta); break;
        default: break;
    }
}

void StatStages::reset() {
    m_attackStage = 0;
    m_spAttackStage = 0;
    m_defenseStage = 0;
    m_spDefenseStage = 0;
    m_speedStage = 0;
    m_accuracyStage = 0;
    m_evasionStage = 0;
}

int StatStages::clampStage(int stage) const {
    return std::max(-6, std::min(stage, 6));
}

double StatStages::calculateModifier(StatType type, int stage) {
    // 根据不同属性类型计算修正值
    if (stage == 0) {
        return 1.0; // 无变化
    }
    
    if (stage > 0) {
        // 正向修正
        switch (type) {
            case StatType::ATTACK:
            case StatType::SP_ATTACK:
                return 1.0 + (stage * 0.5); // 每级+50%
                
            case StatType::DEFENSE:
            case StatType::SP_DEFENSE:
                return 1.0 + (stage * 0.5); // 每级+50%
                
            case StatType::SPEED:
                return 1.0 + (stage * 0.5); // 每级+50%
                
            case StatType::ACCURACY:
                return 1.0 + (stage * 0.5); // 每级+50%
                
            default:
                return 1.0;
        }
    } else {
        // 负向修正
        switch (type) {
            case StatType::ATTACK:
            case StatType::SP_ATTACK:
                return 1.0 / (1.0 - (stage * 0.25)); // 攻击-2时，降低50%
                
            case StatType::DEFENSE:
            case StatType::SP_DEFENSE:
                return 1.0 / (1.0 - (stage * 0.25)); // 防御-1时，降低33%
                
            case StatType::SPEED:
                return 1.0 / (1.0 - (stage * 0.25)); // 速度-3时，降低60%
                
            case StatType::ACCURACY:
                // 命中-1到-3每级减少15%，-4到-6每级减少10%
                if (stage >= -3) {
                    return 1.0 + (stage * 0.15);
                } else {
                    return 0.55 + ((stage + 3) * 0.10);
                }
                
            default:
                return 1.0;
        }
    }
}

// Talent 类实现
Talent::Talent(int hpGrowth, int attackGrowth, int spAttackGrowth,
               int defenseGrowth, int spDefenseGrowth, int speedGrowth)
    : m_hpGrowth(hpGrowth), m_attackGrowth(attackGrowth), m_spAttackGrowth(spAttackGrowth),
      m_defenseGrowth(defenseGrowth), m_spDefenseGrowth(spDefenseGrowth), m_speedGrowth(speedGrowth) {
}

int Talent::getGrowthRate(StatType type) const {
    switch (type) {
        case StatType::HP:           return m_hpGrowth;
        case StatType::ATTACK:       return m_attackGrowth;
        case StatType::SP_ATTACK:    return m_spAttackGrowth;
        case StatType::DEFENSE:      return m_defenseGrowth;
        case StatType::SP_DEFENSE:   return m_spDefenseGrowth;
        case StatType::SPEED:        return m_speedGrowth;
        default:                     return 1;
    }
}

void Talent::setGrowthRate(StatType type, int value) {
    // 确保成长率不小于1
    value = std::max(1, value);
    
    switch (type) {
        case StatType::HP:           m_hpGrowth = value; break;
        case StatType::ATTACK:       m_attackGrowth = value; break;
        case StatType::SP_ATTACK:    m_spAttackGrowth = value; break;
        case StatType::DEFENSE:      m_defenseGrowth = value; break;
        case StatType::SP_DEFENSE:   m_spDefenseGrowth = value; break;
        case StatType::SPEED:        m_speedGrowth = value; break;
        default: break;
    }
}
