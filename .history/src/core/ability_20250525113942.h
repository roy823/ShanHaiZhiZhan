#ifndef ABILITY_H
#define ABILITY_H

#include <QString>

// 能力状态枚举
enum class StatusCondition {
    NONE,       // 无状态
    POISON,     // 中毒
    BURN,       // 烧伤
    FREEZE,     // 冻伤
    PARALYZE,   // 麻痹
    SLEEP,      // 睡眠
    FEAR,       // 害怕
    TIRED,      // 疲惫
    BLEED,      // 流血
    CONFUSION   // 混乱
};

// 能力值类型
enum class StatType {
    HP,             // 血量
    ATTACK,         // 物攻
    SP_ATTACK,      // 特攻
    DEFENSE,        // 物防
    SP_DEFENSE,     // 特防
    SPEED,          // 速度
    ACCURACY,       // 命中
    EVASION         // 闪避
};

// 基础属性类
class BaseStats {
public:
    BaseStats(int hp = 100, int attack = 100, int spAttack = 100, 
             int defense = 100, int spDefense = 100, int speed = 100);
    
    // 获取和设置属性值
    int getStat(StatType type) const;
    void setStat(StatType type, int value);
    
    // 修改属性值
    void modifyStat(StatType type, int delta);
    
private:
    int m_hp;           // 生命值
    int m_attack;       // 物理攻击
    int m_spAttack;     // 特殊攻击
    int m_defense;      // 物理防御
    int m_spDefense;    // 特殊防御
    int m_speed;        // 速度
};

// 能力等级（用于战斗中的临时能力变化）
class StatStages {
public:
    StatStages();
    
    // 获取能力等级
    int getStage(StatType type) const;
    
    // 设置能力等级（-6到+6）
    void setStage(StatType type, int stage);
    
    // 修改能力等级
    void modifyStage(StatType type, int delta);
    
    // 重置所有能力等级
    void reset();
    
    // 计算能力等级对应的实际乘数
    static double calculateModifier(StatType type, int stage);
    
private:
    int m_attackStage;      // 物攻等级
    int m_spAttackStage;    // 特攻等级
    int m_defenseStage;     // 物防等级
    int m_spDefenseStage;   // 特防等级
    int m_speedStage;       // 速度等级
    int m_accuracyStage;    // 命中等级
    int m_evasionStage;     // 闪避等级
    
    // 确保能力等级在有效范围内（-6到+6）
    int clampStage(int stage) const;
};

// 天赋系统（影响能力成长）
class Talent {
public:
    Talent(int hpGrowth = 1, int attackGrowth = 1, int spAttackGrowth = 1,
           int defenseGrowth = 1, int spDefenseGrowth = 1, int speedGrowth = 1);
    
    // 获取成长值
    int getGrowthRate(StatType type) const;
    
    // 设置成长值
    void setGrowthRate(StatType type, int value);
    
private:
    int m_hpGrowth;          // 生命值成长
    int m_attackGrowth;      // 物攻成长
    int m_spAttackGrowth;    // 特攻成长
    int m_defenseGrowth;     // 物防成长
    int m_spDefenseGrowth;   // 特防成长
    int m_speedGrowth;       // 速度成长
};

#endif // ABILITY_H
