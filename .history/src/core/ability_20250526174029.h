// src/core/ability.h
#ifndef ABILITY_H
#define ABILITY_H

#include <QString> // 通常不需要，除非有文本描述，但这里是纯数据类

// 能力相关的枚举和类定义

// 异常状态枚举 (StatusCondition)
enum class StatusCondition {
    NONE,       // 无状态 (正常)
    POISON,     // 中毒
    BURN,       // 烧伤
    FREEZE,     // 冻伤 (设计文档提及)
    PARALYZE,   // 麻痹 (设计文档提及“瘫痪”，这里用麻痹统一)
    SLEEP,      // 睡眠
    FEAR,       // 害怕 (设计文档提及)
    TIRED,      // 疲惫 (设计文档提及，通常是某些强力技能的副作用)
    BLEED,      // 流血 (设计文档提及)
    CONFUSION   // 混乱 (设计文档提及)
};

// 基础属性类型枚举 (StatType)
enum class StatType {
    HP,             // 生命值 (Health Points)
    ATTACK,         // 物理攻击 (Attack