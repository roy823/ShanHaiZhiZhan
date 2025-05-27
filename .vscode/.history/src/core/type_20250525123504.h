#ifndef TYPE_H
#define TYPE_H

#include <QString>
#include <QMap>
#include <QPair>
#include <QVector>

// 精灵属性枚举
enum class ElementType {
    NONE,       // 无属性
    FIRE,       // 火
    WATER,      // 水
    GRASS,      // 草
    GROUND,     // 地面
    FLYING,     // 飞行
    BUG,        // 虫
    MACHINE,    // 机械/钢铁
    NORMAL,     // 普通
    LIGHT,      // 光
    SHADOW      // 暗影/黑暗
};

class Type {
public:
    // 构造函数：单属性
    explicit Type(ElementType primaryType);
    
    // 构造函数：双属性
    Type(ElementType primaryType, ElementType secondaryType);
    
    // 获取属性名称
    QString getName() const;
    
    // 获取主属性和副属性
    ElementType getPrimaryType() const;
    ElementType getSecondaryType() const;
    bool hasDualType() const;
    
    // 计算属性克制系数
    static double calculateEffectiveness(const Type& attackType, const Type& defenseType);
    
    // 属性名称
    static QString getTypeName(ElementType type);
    // 为了兼容UI代码的调用
    static QString getElementTypeName(ElementType type);
    // 获取属性对应的颜色
    static QString getElementTypeColor(ElementType type);
    
    // 获取属性克制关系
    static double getTypeEffectiveness(ElementType attackType, ElementType defenseType);
    
private:
    ElementType m_primaryType;      // 主属性
    ElementType m_secondaryType;    // 副属性
    bool m_isDualType;              // 是否为双属性
    
    // 类型克制关系的静态映射表
    static QMap<QPair<ElementType, ElementType>, double> s_typeEffectiveness;
    
    // 初始化类型克制关系表
    static void initTypeEffectivenessTable();
};

#endif // TYPE_H
