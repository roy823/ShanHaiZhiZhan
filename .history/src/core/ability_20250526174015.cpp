// src/core/ability.cpp
#include "ability.h"
#include <cmath>     // 用于数学函数，例如 std::pow, std::sqrt (虽然这里没直接用，但能力计算可能需要)
#include <algorithm> // 用于 std::max, std::min

// --- BaseStats 类实现 ---
// 构造函数，初始化各项基础属性
BaseStats::BaseStats(int hp, int attack, int spAttack, int defense, int spDefense, int speed)
    : m_hp(hp), m_attack(attack), m_spAttack(spAttack),
      m_defense(defense), m_spDefense(spDefense), m_speed(speed) {
}

// 根据类型获取属性值
int BaseStats::getStat(StatType type) const {
    switch (type) {
        case StatType::HP:           return m_hp;
        case StatType::ATTACK:       return m_attack;
        case StatType::SP_ATTACK:    return m_spAttack;
        case StatType::DEFENSE:      return m_defense;
        case StatType::SP_DEFENSE:   return m_spDefense;
        case StatType::SPEED:        return m_speed;
        default:                     return 0; // 未知类型返回0
    }
}

// 根据类型设置属性值
void BaseStats::setStat(StatType type, int value) {
    value = std::max(0, value); // 确保属性值不为负

    switch (type) {
        case StatType::HP:           m_hp = value; break;
        case StatType::ATTACK:       m_attack = value; break;
        case StatType::SP_ATTACK:    m_spAttack = value; break;
        case StatType::DEFENSE:      m_defense = value; break;
        case StatType::SP_DEFENSE:   m_spDefense = value; break;
        case StatType::SPEED:        m_speed = value; break;
        default: break; // 未知类型不做任何事
    }
}

// 根据类型修改属性值 (增加或减少)
void BaseStats::modifyStat(StatType type, int delta) {
    switch (type) {
        // 使用现有setStat来确保值不为负的逻辑被应用
        case StatType::HP:           setStat(StatType::HP, m_hp + delta); break;
        case StatType::ATTACK:       setStat(StatType::ATTACK, m_attack + delta); break;
        case StatType::SP_ATTACK:    setStat(StatType::SP_ATTACK, m_spAttack + delta); break;
        case StatType::DEFENSE:      setStat(StatType::DEFENSE, m_defense + delta); break;
        case StatType::SP_DEFENSE:   setStat(StatType::SP_DEFENSE, m_spDefense + delta); break;
        case StatType::SPEED:        setStat(StatType::SPEED, m_speed + delta); break;
        default: break;
    }
}

// --- StatStages 类实现 ---
// 构造函数，初始化所有能力等级为0
StatStages::StatStages()
    : m_attackStage(0), m_spAttackStage(0), m_defenseStage(0),
      m_spDefenseStage(0), m_speedStage(0), m_accuracyStage(0),
      m_evasionStage(0) {
}

// 根据类型获取能力等级
int StatStages::getStage(StatType type) const {
    switch (type) {
        case StatType::ATTACK:       return m_attackStage;
        case StatType::SP_ATTACK:    return m_spAttackStage;
        case StatType::DEFENSE:      return m_defenseStage;
        case StatType::SP_DEFENSE:   return m_spDefenseStage;
        case StatType::SPEED:        return m_speedStage;
        case StatType::ACCURACY:     return m_accuracyStage;
        case StatType::EVASION:      return m_evasionStage;
        default:                     return 0; // HP没有等级变化，其他未知类型返回0
    }
}

// 根据类型设置能力等级，并确保在[-6, +6]范围内
void StatStages::setStage(StatType type, int stage) {
    stage = clampStage(stage); // 限制等级范围

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

// 根据类型修改能力等级 (增加或减少)，并确保在[-6, +6]范围内
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

// 重置所有能力等级为0
void StatStages::reset() {
    m_attackStage = 0;
    m_spAttackStage = 0;
    m_defenseStage = 0;
    m_spDefenseStage = 0;
    m_speedStage = 0;
    m_accuracyStage = 0;
    m_evasionStage = 0;
}

// 将能力等级限制在[-6, +6]的范围内
int StatStages::clampStage(int stage) const {
    return std::max(-6, std::min(stage, 6));
}

// 静态方法：根据能力类型和等级计算修正乘数
// 这个修正乘数会应用到基础属性上，得到战斗中的实际属性
double StatStages::calculateModifier(StatType type, int stage) {
    if (stage == 0) {
        return 1.0; // 等级为0，无修正
    }

    // 攻击、特攻、防御、特防、速度 的正向修正：每级+50% (2 -> 2x, 4 -> 3x, 6 -> 4x)
    // 对应公式： (2 + stage) / 2  或 1.0 + stage * 0.5
    if (type == StatType::ATTACK || type == StatType::SP_ATTACK ||
        type == StatType::DEFENSE || type == StatType::SP_DEFENSE ||
        type == StatType::SPEED) {
        if (stage > 0) {
            return (2.0 + stage) / 2.0;
        } else { // 负向修正，例如 -2 时 50% (即1/2)，-4 时 33% (即1/3)，-6 时 25% (即1/4)
                 // 对应公式： 2 / (2 - stage)
            return 2.0 / (2.0 - stage); // stage是负数，所以 2 - (-2) = 4, 2/4 = 0.5
        }
    }
    // 命中和闪避 的修正 (在宝可梦中通常有不同的分数系统，这里按设计文档)
    // 设计文档：命中+1 -> 1.5x; 命中-1 -> 0.85x, -2 -> 0.70x, -3 -> 0.55x, -4 -> 0.45x, -5 -> 0.35x, -6 -> 0.25x
    // 闪避通常与命中相反，但这里分开处理，假设闪避等级也用类似方式影响对方命中
    else if (type == StatType::ACCURACY || type == StatType::EVASION) { // 假设闪避也用此逻辑，但通常闪避是影响对方命中
        if (stage > 0) { // 正向修正，每级 +50%
            return 1.0 + (stage * 0.5); // 例如 stage = 1 -> 1.5; stage = 2 -> 2.0
        } else { // 负向修正 (按设计文档的命中下降规则)
            if (stage >= -3) { // 命中-1到-3每级减少15%
                return 1.0 + (stage * 0.15); // stage是负数, e.g., -1 -> 0.85
            } else { // 命中-4到-6每级减少10% (基于-3时的0.55基础)
                return 0.55 + ((stage + 3) * 0.10); // e.g., stage = -4 -> 0.55 + (-1*0.1) = 0.45
            }
        }
    }

    return 1.0; // 其他未知类型或HP不参与等级修正
}


// --- Talent 类实现 ---
// 构造函数，初始化各项天赋成长率
Talent::Talent(int hpGrowth, int attackGrowth, int spAttackGrowth,
               int defenseGrowth, int spDefenseGrowth, int speedGrowth)
    : m_hpGrowth(hpGrowth), m_attackGrowth(attackGrowth), m_spAttackGrowth(spAttackGrowth),
      m_defenseGrowth(defenseGrowth), m_spDefenseGrowth(spDefenseGrowth), m_speedGrowth(speedGrowth) {
    // 可以在这里添加对成长率的校验，例如确保它们是正数
    setHpGrowth(hpGrowth); // 使用setter进行校验
    setAttackGrowth(attackGrowth);
    setSpecialAttackGrowth(spAttackGrowth);
    setDefenseGrowth(defenseGrowth);
    setSpecialDefenseGrowth(spDefenseGrowth);
    setSpeedGrowth(speedGrowth);
}

// 根据类型获取天赋成长率
int Talent::getGrowthRate(StatType type) const {
    switch (type) {
        case StatType::HP:           return m_hpGrowth;
        case StatType::ATTACK:       return m_attackGrowth;
        case StatType::SP_ATTACK:    return m_spAttackGrowth;
        case StatType::DEFENSE:      return m_defenseGrowth;
        case StatType::SP_DEFENSE:   return m_spDefenseGrowth;
        case StatType::SPEED:        return m_speedGrowth;
        default:                     return 1; // 默认成长率为1 (无特殊加成)
    }
}

// 根据类型设置天赋成长率
void Talent::setGrowthRate(StatType type, int value) {
    value = std::max(0, value); // 天赋成长率通常不为负，至少为0或1，具体看设计

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