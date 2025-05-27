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
    LIGHT,   // 光
    MACHINE, // 机械（兼容旧代码）
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

// 基础属性结构体 - 前向声明，实际定义在ability.h中
class BaseStats;

// 属性类型
class Type
{
public:
    Type(ElementType primary = ElementType::NORMAL, ElementType secondary = ElementType::NONE);
    ~Type();

    ElementType getPrimaryType() const;
    ElementType getSecondaryType() const;
    bool hasType(ElementType type) const;
    bool hasDualType() const;
    QString getName() const;

    // 获取属性名称
    static QString getElementTypeName(ElementType type);
    static QString getTypeName(ElementType type);
    QString getPrimaryTypeName() const;
    QString getSecondaryTypeName() const;
    QString getFullTypeName() const;

    // 获取属性相克关系
    static double getTypeEffectiveness(ElementType attackType, ElementType defenseType);
    double getEffectivenessAgainst(ElementType attackType) const;
    
    // 计算属性克制系数
    static double calculateEffectiveness(const Type& attackType, const Type& defenseType);
    
    // 获取属性颜色
    static QString getElementTypeColor(ElementType type);

private:
    ElementType m_primaryType;   // 主属性
    ElementType m_secondaryType; // 副属性
    bool m_isDualType;           // 是否为双属性

    // 属性相克表 (静态成员，所有Type实例共享)
    static QMap<ElementType, QMap<ElementType, double>> typeEffectivenessChart;
    static void initTypeEffectivenessChart(); // 初始化属性相克表
    static bool chartInitialized; // 标记属性相克表是否已初始化
};

#endif // TYPE_H
