#ifndef TYPE_H
#define TYPE_H

#include <QString>
#include <QMap>

// 元素属性枚举
enum class ElementType
{
    NORMAL,  // 普通
    FIRE,    // 火
    WATER,   // 水
    GRASS,   // 草
    ELECTRIC,// 电
    ICE,     // 冰
    FIGHTING,// 格斗
    POISON,  // 毒
    GROUND,  // 地面
    FLYING,  // 飞行
    PSYCHIC, // 超能力
    BUG,     // 虫
    ROCK,    // 岩石
    GHOST,   // 幽灵
    DRAGON,  // 龙
    DARK,    // 恶
    STEEL,   // 钢
    FAIRY,   // 妖精
    SHADOW,  // 暗影
    MECHANICAL, // 机械
    NONE     // 无属性
};

// 目标类型枚举
enum class TargetType {
    SELF,           // 作用于自身
    OPPONENT,       // 作用于对手
    ALLY,           // 作用于友方（多人战斗）
    ALL_OPPONENTS,  // 作用于所有对手
    ALL_ALLIES,     // 作用于所有友方
    ALL_CREATURES,  // 作用于场上所有精灵
    NONE            // 无目标
};

// 基础属性结构体
struct BaseStats
{
    int hp;             // 生命值
    int attack;         // 物理攻击
    int defense;        // 物理防御
    int specialAttack;  // 特殊攻击
    int specialDefense; // 特殊防御
    int speed;          // 速度

    BaseStats(int hp = 100, int atk = 100, int def = 100, int spAtk = 100, int spDef = 100, int spd = 100)
        : hp(hp), attack(atk), defense(def), specialAttack(spAtk), specialDefense(spDef), speed(spd) {}
};

// 属性类型
class Type
{
public:
    Type(ElementType primary = ElementType::NORMAL, ElementType secondary = ElementType::NONE);
    ~Type();

    ElementType getPrimaryType() const;
    ElementType getSecondaryType() const;
    bool hasType(ElementType type) const;

    // 获取属性名称
    static QString getElementTypeName(ElementType type);
    QString getPrimaryTypeName() const;
    QString getSecondaryTypeName() const;
    QString getFullTypeName() const;

    // 获取属性相克关系
    static double getTypeEffectiveness(ElementType attackType, ElementType defenseType);
    double getEffectivenessAgainst(ElementType attackType) const;

private:
    ElementType m_primaryType;   // 主属性
    ElementType m_secondaryType; // 副属性

    // 属性相克表 (静态成员，所有Type实例共享)
    static QMap<ElementType, QMap<ElementType, double>> typeEffectivenessChart;
    static void initTypeEffectivenessChart(); // 初始化属性相克表
    static bool chartInitialized; // 标记属性相克表是否已初始化
};

#endif // TYPE_H
