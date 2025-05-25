#include "type.h"

// 静态成员初始化
QMap<QPair<ElementType, ElementType>, double> Type::s_typeEffectiveness;

Type::Type(ElementType primaryType) 
    : m_primaryType(primaryType), m_secondaryType(primaryType), m_isDualType(false) {
    // 确保类型关系表已初始化
    if (s_typeEffectiveness.isEmpty()) {
        initTypeEffectivenessTable();
    }
}

Type::Type(ElementType primaryType, ElementType secondaryType) 
    : m_primaryType(primaryType), m_secondaryType(secondaryType), m_isDualType(true) {
    // 确保类型关系表已初始化
    if (s_typeEffectiveness.isEmpty()) {
        initTypeEffectivenessTable();
    }
}

QString Type::getName() const {
    if (!m_isDualType) {
        return getTypeName(m_primaryType);
    } else {
        return getTypeName(m_primaryType) + "/" + getTypeName(m_secondaryType);
    }
}

ElementType Type::getPrimaryType() const {
    return m_primaryType;
}

ElementType Type::getSecondaryType() const {
    return m_secondaryType;
}

bool Type::hasDualType() const {
    return m_isDualType;
}

QString Type::getTypeName(ElementType type) {
    switch (type) {
        case ElementType::NONE:     return "无";
        case ElementType::FIRE:     return "火";
        case ElementType::WATER:    return "水";
        case ElementType::GRASS:    return "草";
        case ElementType::GROUND:   return "地面";
        case ElementType::FLYING:   return "飞行";
        case ElementType::BUG:      return "虫";
        case ElementType::MACHINE:  return "机械";
        case ElementType::NORMAL:   return "普通";
        case ElementType::LIGHT:    return "光";
        case ElementType::SHADOW:   return "暗影";
        default:                    return "未知";
    }
}

QString Type::getElementTypeName(ElementType type) {
    return getTypeName(type);
}

// 初始化类型克制关系表
void Type::initTypeEffectivenessTable() {
    // 定义系数，按照需求中的配置
    const double SUPER_EFFECTIVE = 1.5;    // 克制
    const double NORMAL = 1.0;             // 普通
    const double NOT_VERY_EFFECTIVE = 0.75; // 微弱
    const double NO_EFFECT = 0.0;          // 无效
    
    // 草系攻击效果
    s_typeEffectiveness[{ElementType::GRASS, ElementType::WATER}] = SUPER_EFFECTIVE;
    s_typeEffectiveness[{ElementType::GRASS, ElementType::GROUND}] = SUPER_EFFECTIVE;
    s_typeEffectiveness[{ElementType::GRASS, ElementType::LIGHT}] = SUPER_EFFECTIVE;
    
    s_typeEffectiveness[{ElementType::GRASS, ElementType::NORMAL}] = NORMAL;
    s_typeEffectiveness[{ElementType::GRASS, ElementType::SHADOW}] = NORMAL;
    s_typeEffectiveness[{ElementType::GRASS, ElementType::BUG}] = NORMAL;
    
    s_typeEffectiveness[{ElementType::GRASS, ElementType::GRASS}] = NOT_VERY_EFFECTIVE;
    s_typeEffectiveness[{ElementType::GRASS, ElementType::FIRE}] = NOT_VERY_EFFECTIVE;
    s_typeEffectiveness[{ElementType::GRASS, ElementType::FLYING}] = NOT_VERY_EFFECTIVE;
    s_typeEffectiveness[{ElementType::GRASS, ElementType::MACHINE}] = NOT_VERY_EFFECTIVE;
    
    // 水系攻击效果
    s_typeEffectiveness[{ElementType::WATER, ElementType::FIRE}] = SUPER_EFFECTIVE;
    s_typeEffectiveness[{ElementType::WATER, ElementType::GROUND}] = SUPER_EFFECTIVE;
    
    s_typeEffectiveness[{ElementType::WATER, ElementType::FLYING}] = NORMAL;
    s_typeEffectiveness[{ElementType::WATER, ElementType::MACHINE}] = NORMAL;
    s_typeEffectiveness[{ElementType::WATER, ElementType::NORMAL}] = NORMAL;
    s_typeEffectiveness[{ElementType::WATER, ElementType::LIGHT}] = NORMAL;
    s_typeEffectiveness[{ElementType::WATER, ElementType::SHADOW}] = NORMAL;
    s_typeEffectiveness[{ElementType::WATER, ElementType::BUG}] = NORMAL;
    
    s_typeEffectiveness[{ElementType::WATER, ElementType::GRASS}] = NOT_VERY_EFFECTIVE;
    s_typeEffectiveness[{ElementType::WATER, ElementType::WATER}] = NOT_VERY_EFFECTIVE;
    
    // 火系攻击效果
    s_typeEffectiveness[{ElementType::FIRE, ElementType::GRASS}] = SUPER_EFFECTIVE;
    s_typeEffectiveness[{ElementType::FIRE, ElementType::MACHINE}] = SUPER_EFFECTIVE;
    
    s_typeEffectiveness[{ElementType::FIRE, ElementType::FLYING}] = NORMAL;
    s_typeEffectiveness[{ElementType::FIRE, ElementType::GROUND}] = NORMAL;
    s_typeEffectiveness[{ElementType::FIRE, ElementType::NORMAL}] = NORMAL;
    s_typeEffectiveness[{ElementType::FIRE, ElementType::LIGHT}] = NORMAL;
    s_typeEffectiveness[{ElementType::FIRE, ElementType::SHADOW}] = NORMAL;
    s_typeEffectiveness[{ElementType::FIRE, ElementType::BUG}] = NORMAL;
    
    s_typeEffectiveness[{ElementType::FIRE, ElementType::WATER}] = NOT_VERY_EFFECTIVE;
    s_typeEffectiveness[{ElementType::FIRE, ElementType::FIRE}] = NOT_VERY_EFFECTIVE;
    
    // 飞行系攻击效果
    s_typeEffectiveness[{ElementType::FLYING, ElementType::GRASS}] = SUPER_EFFECTIVE;
    s_typeEffectiveness[{ElementType::FLYING, ElementType::BUG}] = SUPER_EFFECTIVE;
    
    s_typeEffectiveness[{ElementType::FLYING, ElementType::WATER}] = NORMAL;
    s_typeEffectiveness[{ElementType::FLYING, ElementType::FIRE}] = NORMAL;
    s_typeEffectiveness[{ElementType::FLYING, ElementType::FLYING}] = NORMAL;
    s_typeEffectiveness[{ElementType::FLYING, ElementType::GROUND}] = NORMAL;
    s_typeEffectiveness[{ElementType::FLYING, ElementType::NORMAL}] = NORMAL;
    s_typeEffectiveness[{ElementType::FLYING, ElementType::LIGHT}] = NORMAL;
    s_typeEffectiveness[{ElementType::FLYING, ElementType::SHADOW}] = NORMAL;
    
    s_typeEffectiveness[{ElementType::FLYING, ElementType::MACHINE}] = NOT_VERY_EFFECTIVE;
    
    // 地面对飞行系攻击无效
    s_typeEffectiveness[{ElementType::GROUND, ElementType::FLYING}] = NO_EFFECT;
    
    // 机械系攻击效果
    s_typeEffectiveness[{ElementType::MACHINE, ElementType::GRASS}] = NORMAL;
    s_typeEffectiveness[{ElementType::MACHINE, ElementType::FLYING}] = NORMAL;
    s_typeEffectiveness[{ElementType::MACHINE, ElementType::GROUND}] = NORMAL;
    s_typeEffectiveness[{ElementType::MACHINE, ElementType::NORMAL}] = NORMAL;
    s_typeEffectiveness[{ElementType::MACHINE, ElementType::LIGHT}] = NORMAL;
    s_typeEffectiveness[{ElementType::MACHINE, ElementType::SHADOW}] = NORMAL;
    s_typeEffectiveness[{ElementType::MACHINE, ElementType::BUG}] = NORMAL;
    
    s_typeEffectiveness[{ElementType::MACHINE, ElementType::WATER}] = NOT_VERY_EFFECTIVE;
    s_typeEffectiveness[{ElementType::MACHINE, ElementType::FIRE}] = NOT_VERY_EFFECTIVE;
    s_typeEffectiveness[{ElementType::MACHINE, ElementType::MACHINE}] = NOT_VERY_EFFECTIVE;
    
    // 地面系攻击效果
    s_typeEffectiveness[{ElementType::GROUND, ElementType::FIRE}] = SUPER_EFFECTIVE;
    s_typeEffectiveness[{ElementType::GROUND, ElementType::MACHINE}] = SUPER_EFFECTIVE;
    
    s_typeEffectiveness[{ElementType::GROUND, ElementType::WATER}] = NORMAL;
    s_typeEffectiveness[{ElementType::GROUND, ElementType::GROUND}] = NORMAL;
    s_typeEffectiveness[{ElementType::GROUND, ElementType::NORMAL}] = NORMAL;
    s_typeEffectiveness[{ElementType::GROUND, ElementType::LIGHT}] = NORMAL;
    
    s_typeEffectiveness[{ElementType::GROUND, ElementType::GRASS}] = NOT_VERY_EFFECTIVE;
    s_typeEffectiveness[{ElementType::GROUND, ElementType::SHADOW}] = NOT_VERY_EFFECTIVE;
    s_typeEffectiveness[{ElementType::GROUND, ElementType::BUG}] = NOT_VERY_EFFECTIVE;
    
    // 飞行系精灵不受地面系攻击影响
    s_typeEffectiveness[{ElementType::GROUND, ElementType::FLYING}] = NO_EFFECT;
    
    // 普通系攻击效果
    s_typeEffectiveness[{ElementType::NORMAL, ElementType::FIRE}] = NORMAL;
    s_typeEffectiveness[{ElementType::NORMAL, ElementType::WATER}] = NORMAL;
    s_typeEffectiveness[{ElementType::NORMAL, ElementType::GRASS}] = NORMAL;
    s_typeEffectiveness[{ElementType::NORMAL, ElementType::GROUND}] = NORMAL;
    s_typeEffectiveness[{ElementType::NORMAL, ElementType::FLYING}] = NORMAL;
    s_typeEffectiveness[{ElementType::NORMAL, ElementType::BUG}] = NORMAL;
    s_typeEffectiveness[{ElementType::NORMAL, ElementType::MACHINE}] = NORMAL;
    s_typeEffectiveness[{ElementType::NORMAL, ElementType::LIGHT}] = NORMAL;
    s_typeEffectiveness[{ElementType::NORMAL, ElementType::SHADOW}] = NORMAL;
    
    // 光系攻击效果
    s_typeEffectiveness[{ElementType::LIGHT, ElementType::SHADOW}] = SUPER_EFFECTIVE;
    s_typeEffectiveness[{ElementType::LIGHT, ElementType::BUG}] = SUPER_EFFECTIVE;
    
    s_typeEffectiveness[{ElementType::LIGHT, ElementType::WATER}] = NORMAL;
    s_typeEffectiveness[{ElementType::LIGHT, ElementType::FIRE}] = NORMAL;
    s_typeEffectiveness[{ElementType::LIGHT, ElementType::FLYING}] = NORMAL;
    s_typeEffectiveness[{ElementType::LIGHT, ElementType::GROUND}] = NORMAL;
    s_typeEffectiveness[{ElementType::LIGHT, ElementType::NORMAL}] = NORMAL;
    
    s_typeEffectiveness[{ElementType::LIGHT, ElementType::MACHINE}] = NOT_VERY_EFFECTIVE;
    s_typeEffectiveness[{ElementType::LIGHT, ElementType::LIGHT}] = NOT_VERY_EFFECTIVE;
    
    // 草系精灵不受光系攻击影响
    s_typeEffectiveness[{ElementType::LIGHT, ElementType::GRASS}] = NO_EFFECT;
    
    // 暗影系攻击效果
    s_typeEffectiveness[{ElementType::SHADOW, ElementType::SHADOW}] = SUPER_EFFECTIVE;
    
    s_typeEffectiveness[{ElementType::SHADOW, ElementType::GRASS}] = NORMAL;
    s_typeEffectiveness[{ElementType::SHADOW, ElementType::WATER}] = NORMAL;
    s_typeEffectiveness[{ElementType::SHADOW, ElementType::FIRE}] = NORMAL;
    s_typeEffectiveness[{ElementType::SHADOW, ElementType::FLYING}] = NORMAL;
    s_typeEffectiveness[{ElementType::SHADOW, ElementType::GROUND}] = NORMAL;
    s_typeEffectiveness[{ElementType::SHADOW, ElementType::NORMAL}] = NORMAL;
    s_typeEffectiveness[{ElementType::SHADOW, ElementType::BUG}] = NORMAL;
    
    s_typeEffectiveness[{ElementType::SHADOW, ElementType::MACHINE}] = NOT_VERY_EFFECTIVE;
    s_typeEffectiveness[{ElementType::SHADOW, ElementType::LIGHT}] = NOT_VERY_EFFECTIVE;
    
    // 虫系攻击效果
    s_typeEffectiveness[{ElementType::BUG, ElementType::GRASS}] = SUPER_EFFECTIVE;
    s_typeEffectiveness[{ElementType::BUG, ElementType::GROUND}] = SUPER_EFFECTIVE;
    
    s_typeEffectiveness[{ElementType::BUG, ElementType::FLYING}] = NORMAL;
    s_typeEffectiveness[{ElementType::BUG, ElementType::MACHINE}] = NORMAL;
    s_typeEffectiveness[{ElementType::BUG, ElementType::NORMAL}] = NORMAL;
    s_typeEffectiveness[{ElementType::BUG, ElementType::SHADOW}] = NORMAL;
    
    s_typeEffectiveness[{ElementType::BUG, ElementType::WATER}] = NOT_VERY_EFFECTIVE;
    s_typeEffectiveness[{ElementType::BUG, ElementType::FIRE}] = NOT_VERY_EFFECTIVE;
    s_typeEffectiveness[{ElementType::BUG, ElementType::LIGHT}] = NOT_VERY_EFFECTIVE;
}

double Type::getTypeEffectiveness(ElementType attackType, ElementType defenseType) {
    if (s_typeEffectiveness.isEmpty()) {
        initTypeEffectivenessTable();
    }
    
    QPair<ElementType, ElementType> key = {attackType, defenseType};
    if (s_typeEffectiveness.contains(key)) {
        return s_typeEffectiveness[key];
    }
    
    // 默认为普通效果
    return 1.0;
}

// 计算属性克制系数
double Type::calculateEffectiveness(const Type& attackType, const Type& defenseType) {
    // 单一属性攻击单一属性
    if (!attackType.hasDualType() && !defenseType.hasDualType()) {
        return getTypeEffectiveness(attackType.getPrimaryType(), defenseType.getPrimaryType());
    }
    
    // 单一属性攻击双属性
    if (!attackType.hasDualType() && defenseType.hasDualType()) {
        ElementType atkType = attackType.getPrimaryType();
        ElementType defType1 = defenseType.getPrimaryType();
        ElementType defType2 = defenseType.getSecondaryType();
        
        double coef1 = getTypeEffectiveness(atkType, defType1);
        double coef2 = getTypeEffectiveness(atkType, defType2);
        
        // 按照需求的特殊计算公式
        if (coef1 == 1.5 && coef2 == 1.5) {
            return 2.0; // 两个都是克制，效果翻倍
        } else if (coef1 == 0.0 || coef2 == 0.0) {
            return (coef1 + coef2) / 4.0; // 至少一个无效
        } else {
            return (coef1 + coef2) / 2.0; // 其他情况取平均值
        }
    }
    
    // 双属性攻击单一属性
    if (attackType.hasDualType() && !defenseType.hasDualType()) {
        ElementType atkType1 = attackType.getPrimaryType();
        ElementType atkType2 = attackType.getSecondaryType();
        ElementType defType = defenseType.getPrimaryType();
        
        double coef1 = getTypeEffectiveness(atkType1, defType);
        double coef2 = getTypeEffectiveness(atkType2, defType);
        
        // 按照需求的特殊计算公式
        if (coef1 == 1.5 && coef2 == 1.5) {
            return 2.0; // 两个都是克制，效果翻倍
        } else if (coef1 == 0.0 || coef2 == 0.0) {
            return (coef1 + coef2) / 4.0; // 至少一个无效
        } else {
            return (coef1 + coef2) / 2.0; // 其他情况取平均值
        }
    }
    
    // 双属性攻击双属性
    if (attackType.hasDualType() && defenseType.hasDualType()) {
        // 将防守方的双属性拆分为单属性
        ElementType defType1 = defenseType.getPrimaryType();
        ElementType defType2 = defenseType.getSecondaryType();
        
        // 计算攻击方双属性对防守方每个属性的效果
        double coeffVsDef1 = calculateEffectiveness(attackType, Type(defType1));
        double coeffVsDef2 = calculateEffectiveness(attackType, Type(defType2));
        
        // 最终系数为对两个防守方属性效果的平均值
        return (coeffVsDef1 + coeffVsDef2) / 2.0;
    }
    
    // 默认情况
    return 1.0;
}

QString Type::getElementTypeColor(ElementType type) {
    switch (type) {
        case ElementType::NONE:     return "#000000"; // 黑色
        case ElementType::FIRE:     return "#FF0000"; // 红色
        case ElementType::WATER:    return "#0000FF"; // 蓝色
        case ElementType::GRASS:    return "#00FF00"; // 绿色
        case ElementType::GROUND:   return "#996633"; // 棕色
        case ElementType::FLYING:   return "#99CCFF"; // 浅蓝色
        case ElementType::BUG:      return "#99CC33"; // 黄绿色
        case ElementType::MACHINE:  return "#CCCCCC"; // 灰色
        case ElementType::NORMAL:   return "#AAAAAA"; // 浅灰色
        case ElementType::LIGHT:    return "#FFFF00"; // 黄色
        case ElementType::SHADOW:   return "#660099"; // 紫色
        default:                    return "#000000"; // 黑色（默认）
    }
}
