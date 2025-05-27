#include "type.h"
#include <QPair>

// 静态成员初始化
QMap<ElementType, QMap<ElementType, double>> Type::typeEffectivenessChart;
bool Type::chartInitialized = false;

Type::Type(ElementType primaryType, ElementType secondaryType) 
    : m_primaryType(primaryType), m_secondaryType(secondaryType), 
      m_isDualType(secondaryType != ElementType::NONE && secondaryType != primaryType) {
    // 确保类型关系表已初始化
    if (!chartInitialized) {
        initTypeEffectivenessChart();
    }
}

Type::~Type() {
    // 析构函数实现
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

bool Type::hasType(ElementType type) const {
    return m_primaryType == type || (m_isDualType && m_secondaryType == type);
}

QString Type::getTypeName(ElementType type) {
    switch (type) {
        case ElementType::NONE:         return "无";
        case ElementType::FIRE:         return "火";
        case ElementType::WATER:        return "水";
        case ElementType::GRASS:        return "草";
        case ElementType::GROUND:       return "地面";
        case ElementType::FLYING:       return "飞行";
        case ElementType::BUG:          return "虫";
        case ElementType::MACHINE:      return "机械";
        case ElementType::MECHANICAL:   return "机械";
        case ElementType::NORMAL:       return "普通";
        case ElementType::LIGHT:        return "光";
        case ElementType::SHADOW:       return "暗影";
        case ElementType::ELECTRIC:     return "电";
        case ElementType::ICE:          return "冰";
        case ElementType::FIGHTING:     return "格斗";
        case ElementType::POISON:       return "毒";
        case ElementType::PSYCHIC:      return "超能力";
        case ElementType::ROCK:         return "岩石";
        case ElementType::GHOST:        return "幽灵";
        case ElementType::DRAGON:       return "龙";
        case ElementType::DARK:         return "恶";
        case ElementType::STEEL:        return "钢";
        case ElementType::FAIRY:        return "妖精";
        default:                        return "未知";
    }
}

QString Type::getElementTypeName(ElementType type) {
    return getTypeName(type);
}

QString Type::getPrimaryTypeName() const {
    return getTypeName(m_primaryType);
}

QString Type::getSecondaryTypeName() const {
    return getTypeName(m_secondaryType);
}

QString Type::getFullTypeName() const {
    if (!m_isDualType) {
        return getPrimaryTypeName();
    } else {
        return getPrimaryTypeName() + "/" + getSecondaryTypeName();
    }
}

// 初始化类型克制关系表
void Type::initTypeEffectivenessChart() {
    if (chartInitialized) return;
    
    // 定义系数，按照需求中的配置
    const double SUPER_EFFECTIVE = 1.5;    // 克制
    const double NORMAL = 1.0;             // 普通
    const double NOT_VERY_EFFECTIVE = 0.75; // 微弱
    const double NO_EFFECT = 0.0;          // 无效
    
    // 草系攻击效果
    typeEffectivenessChart[ElementType::GRASS][ElementType::WATER] = SUPER_EFFECTIVE;
    typeEffectivenessChart[ElementType::GRASS][ElementType::GROUND] = SUPER_EFFECTIVE;
    typeEffectivenessChart[ElementType::GRASS][ElementType::LIGHT] = SUPER_EFFECTIVE;
    
    typeEffectivenessChart[ElementType::GRASS][ElementType::NORMAL] = NORMAL;
    typeEffectivenessChart[ElementType::GRASS][ElementType::SHADOW] = NORMAL;
    typeEffectivenessChart[ElementType::GRASS][ElementType::BUG] = NORMAL;
    
    typeEffectivenessChart[ElementType::GRASS][ElementType::GRASS] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::GRASS][ElementType::FIRE] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::GRASS][ElementType::FLYING] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::GRASS][ElementType::MACHINE] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::GRASS][ElementType::MECHANICAL] = NOT_VERY_EFFECTIVE;
    
    // 水系攻击效果
    typeEffectivenessChart[ElementType::WATER][ElementType::FIRE] = SUPER_EFFECTIVE;
    typeEffectivenessChart[ElementType::WATER][ElementType::GROUND] = SUPER_EFFECTIVE;
    
    typeEffectivenessChart[ElementType::WATER][ElementType::FLYING] = NORMAL;
    typeEffectivenessChart[ElementType::WATER][ElementType::MACHINE] = NORMAL;
    typeEffectivenessChart[ElementType::WATER][ElementType::MECHANICAL] = NORMAL;
    typeEffectivenessChart[ElementType::WATER][ElementType::NORMAL] = NORMAL;
    typeEffectivenessChart[ElementType::WATER][ElementType::LIGHT] = NORMAL;
    typeEffectivenessChart[ElementType::WATER][ElementType::SHADOW] = NORMAL;
    typeEffectivenessChart[ElementType::WATER][ElementType::BUG] = NORMAL;
    
    typeEffectivenessChart[ElementType::WATER][ElementType::GRASS] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::WATER][ElementType::WATER] = NOT_VERY_EFFECTIVE;
    
    // 火系攻击效果
    typeEffectivenessChart[ElementType::FIRE][ElementType::GRASS] = SUPER_EFFECTIVE;
    typeEffectivenessChart[ElementType::FIRE][ElementType::MACHINE] = SUPER_EFFECTIVE;
    typeEffectivenessChart[ElementType::FIRE][ElementType::MECHANICAL] = SUPER_EFFECTIVE;
    
    typeEffectivenessChart[ElementType::FIRE][ElementType::FLYING] = NORMAL;
    typeEffectivenessChart[ElementType::FIRE][ElementType::GROUND] = NORMAL;
    typeEffectivenessChart[ElementType::FIRE][ElementType::NORMAL] = NORMAL;
    typeEffectivenessChart[ElementType::FIRE][ElementType::LIGHT] = NORMAL;
    typeEffectivenessChart[ElementType::FIRE][ElementType::SHADOW] = NORMAL;
    typeEffectivenessChart[ElementType::FIRE][ElementType::BUG] = NORMAL;
    
    typeEffectivenessChart[ElementType::FIRE][ElementType::WATER] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::FIRE][ElementType::FIRE] = NOT_VERY_EFFECTIVE;
    
    // 飞行系攻击效果
    typeEffectivenessChart[ElementType::FLYING][ElementType::GRASS] = SUPER_EFFECTIVE;
    typeEffectivenessChart[ElementType::FLYING][ElementType::BUG] = SUPER_EFFECTIVE;
    
    typeEffectivenessChart[ElementType::FLYING][ElementType::WATER] = NORMAL;
    typeEffectivenessChart[ElementType::FLYING][ElementType::FIRE] = NORMAL;
    typeEffectivenessChart[ElementType::FLYING][ElementType::FLYING] = NORMAL;
    typeEffectivenessChart[ElementType::FLYING][ElementType::GROUND] = NORMAL;
    typeEffectivenessChart[ElementType::FLYING][ElementType::NORMAL] = NORMAL;
    typeEffectivenessChart[ElementType::FLYING][ElementType::LIGHT] = NORMAL;
    typeEffectivenessChart[ElementType::FLYING][ElementType::SHADOW] = NORMAL;
    
    typeEffectivenessChart[ElementType::FLYING][ElementType::MACHINE] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::FLYING][ElementType::MECHANICAL] = NOT_VERY_EFFECTIVE;
    
    // 地面对飞行系攻击无效
    typeEffectivenessChart[ElementType::GROUND][ElementType::FLYING] = NO_EFFECT;
    
    // 机械系攻击效果
    typeEffectivenessChart[ElementType::MACHINE][ElementType::GRASS] = NORMAL;
    typeEffectivenessChart[ElementType::MACHINE][ElementType::FLYING] = NORMAL;
    typeEffectivenessChart[ElementType::MACHINE][ElementType::GROUND] = NORMAL;
    typeEffectivenessChart[ElementType::MACHINE][ElementType::NORMAL] = NORMAL;
    typeEffectivenessChart[ElementType::MACHINE][ElementType::LIGHT] = NORMAL;
    typeEffectivenessChart[ElementType::MACHINE][ElementType::SHADOW] = NORMAL;
    typeEffectivenessChart[ElementType::MACHINE][ElementType::BUG] = NORMAL;
    
    typeEffectivenessChart[ElementType::MACHINE][ElementType::WATER] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::MACHINE][ElementType::FIRE] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::MACHINE][ElementType::MACHINE] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::MACHINE][ElementType::MECHANICAL] = NOT_VERY_EFFECTIVE;
    
    // 复制MACHINE的效果到MECHANICAL
    typeEffectivenessChart[ElementType::MECHANICAL] = typeEffectivenessChart[ElementType::MACHINE];
    
    // 地面系攻击效果
    typeEffectivenessChart[ElementType::GROUND][ElementType::FIRE] = SUPER_EFFECTIVE;
    typeEffectivenessChart[ElementType::GROUND][ElementType::MACHINE] = SUPER_EFFECTIVE;
    typeEffectivenessChart[ElementType::GROUND][ElementType::MECHANICAL] = SUPER_EFFECTIVE;
    
    typeEffectivenessChart[ElementType::GROUND][ElementType::WATER] = NORMAL;
    typeEffectivenessChart[ElementType::GROUND][ElementType::GROUND] = NORMAL;
    typeEffectivenessChart[ElementType::GROUND][ElementType::NORMAL] = NORMAL;
    typeEffectivenessChart[ElementType::GROUND][ElementType::LIGHT] = NORMAL;
    
    typeEffectivenessChart[ElementType::GROUND][ElementType::GRASS] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::GROUND][ElementType::SHADOW] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::GROUND][ElementType::BUG] = NOT_VERY_EFFECTIVE;
    
    // 普通系攻击效果
    typeEffectivenessChart[ElementType::NORMAL][ElementType::FIRE] = NORMAL;
    typeEffectivenessChart[ElementType::NORMAL][ElementType::WATER] = NORMAL;
    typeEffectivenessChart[ElementType::NORMAL][ElementType::GRASS] = NORMAL;
    typeEffectivenessChart[ElementType::NORMAL][ElementType::GROUND] = NORMAL;
    typeEffectivenessChart[ElementType::NORMAL][ElementType::FLYING] = NORMAL;
    typeEffectivenessChart[ElementType::NORMAL][ElementType::BUG] = NORMAL;
    typeEffectivenessChart[ElementType::NORMAL][ElementType::MACHINE] = NORMAL;
    typeEffectivenessChart[ElementType::NORMAL][ElementType::MECHANICAL] = NORMAL;
    typeEffectivenessChart[ElementType::NORMAL][ElementType::LIGHT] = NORMAL;
    typeEffectivenessChart[ElementType::NORMAL][ElementType::SHADOW] = NORMAL;
    
    // 光系攻击效果
    typeEffectivenessChart[ElementType::LIGHT][ElementType::SHADOW] = SUPER_EFFECTIVE;
    typeEffectivenessChart[ElementType::LIGHT][ElementType::BUG] = SUPER_EFFECTIVE;
    
    typeEffectivenessChart[ElementType::LIGHT][ElementType::WATER] = NORMAL;
    typeEffectivenessChart[ElementType::LIGHT][ElementType::FIRE] = NORMAL;
    typeEffectivenessChart[ElementType::LIGHT][ElementType::FLYING] = NORMAL;
    typeEffectivenessChart[ElementType::LIGHT][ElementType::GROUND] = NORMAL;
    typeEffectivenessChart[ElementType::LIGHT][ElementType::NORMAL] = NORMAL;
    
    typeEffectivenessChart[ElementType::LIGHT][ElementType::MACHINE] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::LIGHT][ElementType::MECHANICAL] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::LIGHT][ElementType::LIGHT] = NOT_VERY_EFFECTIVE;
    
    // 草系精灵不受光系攻击影响
    typeEffectivenessChart[ElementType::LIGHT][ElementType::GRASS] = NO_EFFECT;
    
    // 暗影系攻击效果
    typeEffectivenessChart[ElementType::SHADOW][ElementType::SHADOW] = SUPER_EFFECTIVE;
    
    typeEffectivenessChart[ElementType::SHADOW][ElementType::GRASS] = NORMAL;
    typeEffectivenessChart[ElementType::SHADOW][ElementType::WATER] = NORMAL;
    typeEffectivenessChart[ElementType::SHADOW][ElementType::FIRE] = NORMAL;
    typeEffectivenessChart[ElementType::SHADOW][ElementType::FLYING] = NORMAL;
    typeEffectivenessChart[ElementType::SHADOW][ElementType::GROUND] = NORMAL;
    typeEffectivenessChart[ElementType::SHADOW][ElementType::NORMAL] = NORMAL;
    typeEffectivenessChart[ElementType::SHADOW][ElementType::BUG] = NORMAL;
    
    typeEffectivenessChart[ElementType::SHADOW][ElementType::MACHINE] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::SHADOW][ElementType::MECHANICAL] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::SHADOW][ElementType::LIGHT] = NOT_VERY_EFFECTIVE;
    
    // 虫系攻击效果
    typeEffectivenessChart[ElementType::BUG][ElementType::GRASS] = SUPER_EFFECTIVE;
    typeEffectivenessChart[ElementType::BUG][ElementType::GROUND] = SUPER_EFFECTIVE;
    
    typeEffectivenessChart[ElementType::BUG][ElementType::FLYING] = NORMAL;
    typeEffectivenessChart[ElementType::BUG][ElementType::MACHINE] = NORMAL;
    typeEffectivenessChart[ElementType::BUG][ElementType::MECHANICAL] = NORMAL;
    typeEffectivenessChart[ElementType::BUG][ElementType::NORMAL] = NORMAL;
    typeEffectivenessChart[ElementType::BUG][ElementType::SHADOW] = NORMAL;
    
    typeEffectivenessChart[ElementType::BUG][ElementType::WATER] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::BUG][ElementType::FIRE] = NOT_VERY_EFFECTIVE;
    typeEffectivenessChart[ElementType::BUG][ElementType::LIGHT] = NOT_VERY_EFFECTIVE;
    
    chartInitialized = true;
}

double Type::getTypeEffectiveness(ElementType attackType, ElementType defenseType) {
    if (!chartInitialized) {
        initTypeEffectivenessChart();
    }
    
    if (typeEffectivenessChart.contains(attackType) && 
        typeEffectivenessChart[attackType].contains(defenseType)) {
        return typeEffectivenessChart[attackType][defenseType];
    }
    
    // 默认为普通效果
    return 1.0;
}

double Type::getEffectivenessAgainst(ElementType attackType) const {
    if (!m_isDualType) {
        return getTypeEffectiveness(attackType, m_primaryType);
    }
    
    double effectiveness1 = getTypeEffectiveness(attackType, m_primaryType);
    double effectiveness2 = getTypeEffectiveness(attackType, m_secondaryType);
    
    // 特殊计算规则
    if (effectiveness1 == 0.0 || effectiveness2 == 0.0) {
        return 0.0; // 如果有一个无效，则整体无效
    } else if (effectiveness1 == 1.5 && effectiveness2 == 1.5) {
        return 2.0; // 如果两个都是克制，则效果翻倍
    } else {
        return (effectiveness1 + effectiveness2) / 2.0; // 其他情况取平均值
    }
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
        case ElementType::NONE:         return "#000000"; // 黑色
        case ElementType::FIRE:         return "#FF0000"; // 红色
        case ElementType::WATER:        return "#0000FF"; // 蓝色
        case ElementType::GRASS:        return "#00FF00"; // 绿色
        case ElementType::GROUND:       return "#996633"; // 棕色
        case ElementType::FLYING:       return "#99CCFF"; // 浅蓝色
        case ElementType::BUG:          return "#99CC33"; // 黄绿色
        case ElementType::MACHINE:      return "#CCCCCC"; // 灰色
        case ElementType::MECHANICAL:   return "#CCCCCC"; // 灰色
        case ElementType::NORMAL:       return "#AAAAAA"; // 浅灰色
        case ElementType::LIGHT:        return "#FFFF00"; // 黄色
        case ElementType::SHADOW:       return "#660099"; // 紫色
        case ElementType::ELECTRIC:     return "#FFCC00"; // 黄色
        case ElementType::ICE:          return "#99FFFF"; // 浅蓝色
        case ElementType::FIGHTING:     return "#CC0000"; // 深红色
        case ElementType::POISON:       return "#9900CC"; // 紫色
        case ElementType::PSYCHIC:      return "#FF66CC"; // 粉色
        case ElementType::ROCK:         return "#CC9966"; // 棕色
        case ElementType::GHOST:        return "#9966CC"; // 紫色
        case ElementType::DRAGON:       return "#6666CC"; // 蓝紫色
        case ElementType::DARK:         return "#666666"; // 深灰色
        case ElementType::STEEL:        return "#CCCCCC"; // 银色
        case ElementType::FAIRY:        return "#FF99CC"; // 粉色
        default:                        return "#000000"; // 黑色（默认）
    }
}
