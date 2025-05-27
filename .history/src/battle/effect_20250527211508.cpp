// src/battle/effect.cpp
#include "effect.h"
#include "../core/creature.h"     // 核心 - 精灵类
#include "battlesystem.h"         // 战斗 - 战斗系统类
#include <QRandomGenerator>       // Qt随机数生成器
#include <QStringList>            // 用于构建描述文本
#include "../core/type.h"         // <<--- 确保包含type.h (effect.h已包含，这里是防御性)

// --- Effect基类实现 ---
Effect::Effect(EffectType type, int chance)
    : m_type(type),
      m_chance(qBound(0, chance, 100)), // 确保几率在0到100之间
      m_targetSelf(false)               // 默认效果目标是对方
{
    // 中文注释：效果基类构造函数
    // type: 效果的类型
    // chance: 效果触发的基础几率 (0-100)
    // m_targetSelf: 效果是否作用于使用者自身 (默认为false，即作用于对方)
}

Effect::~Effect()
{
    // 中文注释：效果基类析构函数，目前无特殊资源管理
}

EffectType Effect::getType() const
{
    // 中文注释：获取效果的类型
    return m_type;
}

int Effect::getChance() const
{
    // 中文注释：获取效果触发的基础几率
    return m_chance;
}

void Effect::setChance(int chance)
{
    // 中文注释：设置效果触发的基础几率，并确保在0-100范围内
    m_chance = qBound(0, chance, 100);
}

// 设置效果目标
void Effect::setTargetSelf(bool self)
{
    // 中文注释：设置效果是作用于自身还是对方
    m_targetSelf = self;
}

// 效果是否作用于自身
bool Effect::isTargetSelf() const
{
    // 中文注释：判断效果是否作用于自身
    return m_targetSelf;
}

// 检查几率是否触发
bool Effect::checkChance() const
{
    // 中文注释：根据m_chance判断效果是否触发
    if (m_chance >= 100) return true; // 100%几率必定触发
    if (m_chance <= 0) return false;  // 0%几率必定不触发
    return QRandomGenerator::global()->bounded(100) < m_chance; // 生成[0, 99]的随机数，与几率比较
}


// --- TurnBasedEffect 实现 ---
TurnBasedEffect::TurnBasedEffect(int duration,
                                 std::function<void(Creature*, Creature*, BattleSystem*, TurnBasedEffect* self)> effectFunc,
                                 bool onTurnStart,
                                 int chance)
    : Effect(EffectType::TURN_BASED, chance),      // 调用基类构造
      m_initialDuration(duration),                  // 记录初始持续回合
      m_currentDuration(duration),                  // 当前剩余回合
      m_effectLogic(effectFunc),                    // 存储效果逻辑函数
      m_onTurnStart(onTurnStart),                   // 标记是在回合开始还是结束时执行
      m_description(QString("一个持续%1回合的效果。").arg(duration)), // 默认描述
      m_originalSource(nullptr)                     // 初始化效果来源为空
{
    // 中文注释：回合类效果构造函数
    // duration: 效果持续回合数
    // effectFunc: 每回合执行的具体逻辑 (lambda或函数指针)
    // onTurnStart: true表示回合开始时执行逻辑，false表示回合结束时
    // chance: 效果施加的成功率

    // 确保持续时间不为负
    if (m_currentDuration < 0) m_currentDuration = 0;
}

bool TurnBasedEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    // 中文注释：应用回合类效果
    // 1. 检查触发几率
    // 2. 确定实际目标 (自身或对方)
    // 3. 创建效果的副本并添加到目标的持续效果列表中
    // 4. 记录效果来源

    if (!checkChance()) return false; // 未达到触发几率

    Creature* actualTarget = m_targetSelf ? source : target; // 判断效果的实际目标
    if (!actualTarget) {
        // qWarning() << "TurnBasedEffect::apply: Actual target is null.";
        return false;
    }

    // 创建此效果的一个新实例（副本）并添加到目标的列表中
    // 这样做是为了确保每个施加的效果都是独立的，有自己的持续时间等状态
    TurnBasedEffect* effectInstance = new TurnBasedEffect(*this); // 使用拷贝构造函数创建副本
    effectInstance->setOriginalSource(source); // 记录是谁施加了这个效果的实例

    actualTarget->addTurnEffect(effectInstance); // 将新创建的实例添加到目标

    // if (battle && source) { // 日志记录应由BattleSystem更上层处理
    //     battle->addBattleLog(QString("%1 对 %2 施加了 \"%3\"").arg(source->getName()).arg(actualTarget->getName()).arg(getDescription()));
    // }
    return true;
}

QString TurnBasedEffect::getDescription() const
{
    // 中文注释：获取回合类效果的描述文本
    return m_description.isEmpty() ? QString("持续 %1 回合的效果").arg(m_initialDuration) : m_description;
}

int TurnBasedEffect::getDuration() const
{
    // 中文注释：获取当前剩余的持续回合数
    return m_currentDuration;
}

void TurnBasedEffect::setDuration(int duration) {
    // 中文注释：设置当前剩余的持续回合数
    m_currentDuration = duration;
    if (m_currentDuration < 0) m_currentDuration = 0;
}


bool TurnBasedEffect::decrementDuration()
{
    // 中文注释：将效果的持续时间减少1回合
    // 如果持续时间减至0或以下，返回true表示效果已结束
    if (m_currentDuration > 0)
    {
        m_currentDuration--;
    }
    return m_currentDuration <= 0;
}

void TurnBasedEffect::executeTurnLogic(Creature* affectedCreature, Creature* sourceCreature_unused, BattleSystem* battle)
{
    // 中文注释：执行存储在该回合效果中的具体逻辑
    // affectedCreature: 当前受此效果影响的精灵
    // sourceCreature_unused: 效果的原始来源（可能已离场或非直接相关），通常用this->getOriginalSource()
    // battle: 战斗系统实例
    if (m_effectLogic && affectedCreature)
    {
        // 调用存储的lambda或函数，传递必要的上下文
        // 注意：sourceCreature参数在此处可能不准确，应使用m_originalSource
        m_effectLogic(affectedCreature, m_originalSource, battle, this);
    }
}


// --- StatusConditionEffect 实现 ---
StatusConditionEffect::StatusConditionEffect(StatusCondition condition, int chance)
    : Effect(EffectType::STATUS_CONDITION, chance), // 调用基类构造
      m_condition(condition)                        // 初始化要施加的异常状态
{
    // 中文注释：异常状态效果构造函数
    // condition: 要施加的具体异常状态
    // chance: 施加的成功率
}

StatusCondition StatusConditionEffect::getCondition() const
{
    // 中文注释：获取此效果要施加的异常状态类型
    return m_condition;
}

bool StatusConditionEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    // 中文注释：应用异常状态效果
    // 1. 检查触发几率
    // 2. 确定实际目标
    // 3. 检查目标是否已处于该状态或免疫
    // 4. 施加状态并通知战斗系统

    if (!checkChance()) return false; // 未达到触发几率

    Creature* actualTarget = m_targetSelf ? source : target; // 确定效果的实际目标
    if (!actualTarget || !battle) {
        // qWarning() << "StatusConditionEffect::apply: Actual target or battle system is null.";
        return false;
    }

    // 检查目标是否已经有相同的异常状态
    if (actualTarget->getStatusCondition() == m_condition && m_condition != StatusCondition::NONE) {
        // battle->addBattleLog(QString("%1 已经处于 %2 状态了。").arg(actualTarget->getName()).arg(getDescription()));
        return false; // 目标已处于该状态，效果应用失败
    }

    // if (m_condition == StatusCondition::BURN && actualTarget->hasType(ElementType::FIRE)) return false;

    StatusCondition oldStatus = actualTarget->getStatusCondition(); // 记录旧状态
    actualTarget->setStatusCondition(m_condition);                  // 施加新状态

    // battle->addBattleLog(QString("%1 陷入了 %2 状态!").arg(actualTarget->getName()).arg(getDescription()));
    // battle->emitStatusChanged(actualTarget, oldStatus, m_condition); // 信号由BattleSystem发出
    

    return true;
}

QString StatusConditionEffect::getDescription() const
{
    // 中文注释：获取异常状态效果的描述文本
    QString conditionText;
    switch (m_condition)
    {
    case StatusCondition::POISON:    conditionText = "中毒"; break;
    case StatusCondition::BURN:      conditionText = "烧伤"; break;
    case StatusCondition::FREEZE:    conditionText = "冻伤"; break;
    case StatusCondition::PARALYZE:  conditionText = "麻痹"; break;
    case StatusCondition::SLEEP:     conditionText = "睡眠"; break;
    case StatusCondition::FEAR:      conditionText = "害怕"; break;
    case StatusCondition::TIRED:     conditionText = "疲惫"; break;
    case StatusCondition::BLEED:     conditionText = "流血"; break;
    case StatusCondition::CONFUSION: conditionText = "混乱"; break;
    case StatusCondition::NONE:      conditionText = "正常"; break; // 一般不作为施加效果
    default:                         conditionText = "未知状态"; break;
    }
    if (m_chance < 100) return QString("有%1%几率使%2%3").arg(m_chance).arg(m_targetSelf ? "自身" : "目标").arg(conditionText);
    return QString("使%1%2").arg(m_targetSelf ? "自身" : "目标").arg(conditionText);
}


// --- StatChangeEffect 实现 ---
StatChangeEffect::StatChangeEffect(StatType stat, int stages, bool targetSelf, int chance)
    : Effect(EffectType::STAT_CHANGE, chance), // 调用基类构造
      m_stat(stat),                             // 初始化目标能力类型
      m_stages(stages)                          // 初始化变化等级
{
    // 中文注释：能力变化效果构造函数
    // stat: 要改变的能力类型 (如物攻、速度)
    // stages: 变化量 (正数为提升，负数为降低)
    // targetSelf: true表示作用于使用者，false表示作用于对方
    // chance: 效果触发几率
    setTargetSelf(targetSelf); // 设置效果是作用于自身还是对方
}

StatType StatChangeEffect::getStat() const
{
    // 中文注释：获取效果要改变的能力类型
    return m_stat;
}

int StatChangeEffect::getStages() const
{
    // 中文注释：获取能力变化的等级数
    return m_stages;
}

bool StatChangeEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    // 中文注释：应用能力变化效果
    // 1. 检查触发几率
    // 2. 确定实际目标
    // 3. 修改目标的能力等级，并处理边界情况（如已达上限/下限）
    // 4. 通知战斗系统

    if (!checkChance()) return false; // 未达到触发几率

    Creature* actualTarget = m_targetSelf ? source : target; // 确定效果的实际目标
    if (!actualTarget || !battle) {
        // qWarning() << "StatChangeEffect::apply: Actual target or battle system is null.";
        return false;
    }

    int oldStage = actualTarget->getStatStages().getStage(m_stat); // 获取旧的能力等级
    actualTarget->modifyStatStage(m_stat, m_stages);               // 修改能力等级
    int newStage = actualTarget->getStatStages().getStage(m_stat); // 获取新的能力等级

    if (oldStage != newStage) { // 只有当能力等级实际发生变化时才记录和发信号
        // battle->addBattleLog(QString("%1 的 %2 %3了 %4 级!").arg(actualTarget->getName()).arg("某能力名称").arg(m_stages > 0 ? "提升" : "降低").arg(std::abs(m_stages)));
        // battle->emitStatStageChanged(actualTarget, m_stat, oldStage, newStage); // 信号由BattleSystem发出
    } else {
        // battle->addBattleLog(QString("%1 的 %2 没有变化。").arg(actualTarget->getName()).arg("某能力名称"));
        return false; // 能力等级未发生实际变化，效果可能因已达上限/下限而“失败”
    }
    return true;
}

QString StatChangeEffect::getDescription() const
{
    // 中文注释：获取能力变化效果的描述文本
    QString statName;
    switch (m_stat) { // 将StatType枚举转换为可读的中文名称
        case StatType::ATTACK: statName = "物攻"; break;
        case StatType::DEFENSE: statName = "物防"; break;
        case StatType::SP_ATTACK: statName = "特攻"; break;
        case StatType::SP_DEFENSE: statName = "特防"; break;
        case StatType::SPEED: statName = "速度"; break;
        case StatType::ACCURACY: statName = "命中"; break;
        case StatType::EVASION: statName = "闪避"; break;
        default: statName = "某项能力"; break;
    }
    QString targetDesc = m_targetSelf ? "自身" : "目标"; // 描述是针对自身还是目标
    QString actionDesc = m_stages > 0 ? QString("提升%1级").arg(m_stages) : QString("降低%1级").arg(-m_stages); // 描述是提升还是降低，及等级数

    if (m_chance < 100) return QString("有%1%几率使%2的%3%4").arg(m_chance).arg(targetDesc).arg(statName).arg(actionDesc);
    return QString("使%1的%2%3").arg(targetDesc).arg(statName).arg(actionDesc);
}


// --- ClearEffectsEffect 实现 ---
ClearEffectsEffect::ClearEffectsEffect(bool clearPositiveStatChanges, bool clearNegativeStatChanges,
                                       bool clearStatusConditions, bool clearTurnBasedEffects,
                                       bool targetSelf, int chance)
    : Effect(EffectType::CLEAR_EFFECTS, chance),                     // 调用基类构造
      m_clearPositiveStatChanges(clearPositiveStatChanges),         // 是否清除能力提升
      m_clearNegativeStatChanges(clearNegativeStatChanges),         // 是否清除能力下降
      m_clearStatusConditions(clearStatusConditions),               // 是否清除异常状态
      m_clearTurnBasedEffects(clearTurnBasedEffects)                // 是否清除所有回合类效果
{
    // 中文注释：清除类效果构造函数
    setTargetSelf(targetSelf); // 设置效果是作用于自身还是对方
}

bool ClearEffectsEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    // 中文注释：应用清除效果
    if (!checkChance()) return false; // 检查触发几率

    Creature* actualTarget = m_targetSelf ? source : target; // 确定实际目标
    if (!actualTarget || !battle) return false;

    bool somethingWasCleared = false; // 标记是否真的清除了某些东西

    // 清除异常状态
    if (m_clearStatusConditions && actualTarget->getStatusCondition() != StatusCondition::NONE) {
        StatusCondition oldStatus = actualTarget->getStatusCondition();
        actualTarget->clearStatusCondition();
        // battle->addBattleLog(QString("%1 的异常状态被清除了!").arg(actualTarget->getName()));
        // battle->emitStatusChanged(actualTarget, oldStatus, StatusCondition::NONE); // 信号由BattleSystem发出
        somethingWasCleared = true;
    }

    // 清除能力等级变化
    bool statsChanged = false;
    StatStages currentStages = actualTarget->getStatStages(); // 获取当前能力等级
    for (StatType type : {StatType::ATTACK, StatType::DEFENSE, StatType::SP_ATTACK, StatType::SP_DEFENSE, StatType::SPEED, StatType::ACCURACY, StatType::EVASION}) {
        int stage = currentStages.getStage(type);
        if (m_clearPositiveStatChanges && stage > 0) { // 如果要清除能力提升且当前等级为正
            actualTarget->modifyStatStage(type, -stage); // 将该能力等级重置为0 (通过减去当前等级实现)
            statsChanged = true;
        }
        if (m_clearNegativeStatChanges && stage < 0) { // 如果要清除能力下降且当前等级为负
            actualTarget->modifyStatStage(type, -stage); // 将该能力等级重置为0
            statsChanged = true;
        }
    }
    if (statsChanged) {
        // battle->addBattleLog(QString("%1 的能力等级变化被清除了!").arg(actualTarget->getName()));
        // BattleSystem应在外部获取新旧stage并发送信号
        somethingWasCleared = true;
    }

    // 清除所有回合类效果
    if (m_clearTurnBasedEffects && !actualTarget->getTurnEffects().isEmpty()) {
        actualTarget->clearAllTurnEffects();
        // battle->addBattleLog(QString("%1 的所有回合类效果被清除了!").arg(actualTarget->getName()));
        somethingWasCleared = true;
    }

    return somethingWasCleared; // 如果成功清除了任何效果，则返回true
}

QString ClearEffectsEffect::getDescription() const
{
    // 中文注释：获取清除类效果的描述文本
    QStringList clearedItems; // 用于存放被清除项目的描述
    if (m_clearPositiveStatChanges) clearedItems.append("能力提升");
    if (m_clearNegativeStatChanges) clearedItems.append("能力下降");
    if (m_clearStatusConditions) clearedItems.append("异常状态");
    if (m_clearTurnBasedEffects) clearedItems.append("回合效果");

    if (clearedItems.isEmpty()) return "无特定清除目标"; // 如果没有指定清除任何东西

    QString targetDesc = m_targetSelf ? "自身" : "目标"; // 描述目标
    QString itemsDesc = clearedItems.join("、");          // 将所有清除项用顿号连接

    if (m_chance < 100) return QString("有%1%几率清除%2的%3").arg(m_chance).arg(targetDesc).arg(itemsDesc);
    return QString("清除%1的%2").arg(targetDesc).arg(itemsDesc);
}


// --- ImmunityEffect 实现 ---
ImmunityEffect::ImmunityEffect(int duration, bool immuneToStatus, ElementType immuneToSpecificTypeDamage, int chance)
    : Effect(EffectType::IMMUNITY, chance),                // 调用基类构造
      m_duration(duration),                                 // 免疫效果持续回合数
      m_immuneToStatus(immuneToStatus),                     // 是否免疫所有异常状态
      m_immuneToTypeDamage(immuneToSpecificTypeDamage)      // 免疫哪个属性的伤害 (NONE表示不免疫特定属性)
{
    // 中文注释：免疫效果构造函数
    setTargetSelf(true); // 免疫效果通常是施加给使用者自身
}

bool ImmunityEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    // 中文注释：应用免疫效果
    // 免疫效果是通过给目标添加一个特殊的TurnBasedEffect来实现的。
    // BattleSystem在进行伤害计算或状态施加前，会检查目标是否拥有此类“免疫标记”效果。

    if (!checkChance()) return false; // 检查触发几率

    Creature* actualTarget = m_targetSelf ? source : target; // 确定实际目标
    if (!actualTarget || !battle) return false;

    // 创建一个代表此免疫状态的TurnBasedEffect
    // 这个TurnBasedEffect主要起标记作用，其具体免疫逻辑由BattleSystem在相关检查点实现
    QString desc = getDescription(); // 获取此免疫效果的完整描述
    auto immunityLogic = [desc](Creature* affected, Creature* src_unused, BattleSystem* btl_unused, TurnBasedEffect* self_effect_unused) {
        // 这个lambda在每回合可以为空，或者仅用于日志记录
        // 主要的免疫判断在BattleSystem中进行，通过检查精灵是否携带此类型的TurnBasedEffect
        // (例如，通过TurnBasedEffect的特定描述或一个专门的标志位来识别免疫效果)
        // btl->addBattleLog(QString("%1 仍处于 '%2' 状态。").arg(affected->getName()).arg(desc));
    };

    TurnBasedEffect* immunityMarkerEffect = new TurnBasedEffect(m_duration, immunityLogic, false, 100); // 持续m_duration回合
    immunityMarkerEffect->setDescription(desc);                       // 设置描述，方便识别
    immunityMarkerEffect->setOriginalSource(source);                  // 记录效果来源

    actualTarget->addTurnEffect(immunityMarkerEffect); // 将免疫标记效果添加到目标

    // battle->addBattleLog(QString("%1 获得了 \"%2\"!").arg(actualTarget->getName()).arg(desc));
    return true;
}

QString ImmunityEffect::getDescription() const
{
    // 中文注释：获取免疫效果的描述文本
    QStringList immunities; // 存储免疫项目的列表
    if (m_immuneToStatus) immunities.append("异常状态");
    if (m_immuneToTypeDamage != ElementType::NONE) { // 如果免疫特定属性伤害
        immunities.append(QString("对%1属性伤害").arg(Type::getElementTypeName(m_immuneToTypeDamage)));
    }

    if (immunities.isEmpty()) return QString("持续%1回合的通用免疫").arg(m_duration); // 如果没有具体免疫项

    QString targetDesc = m_targetSelf ? "自身" : "目标";        // 描述目标
    QString itemsDesc = immunities.join("和");                  // 用“和”连接免疫项
    QString baseDesc = QString("在%1回合内免疫%2").arg(m_duration).arg(itemsDesc); // 构建基础描述

    if (m_chance < 100) return QString("有%1%几率使%2%3").arg(m_chance).arg(targetDesc).arg(baseDesc);
    return QString("使%1%2").arg(targetDesc).arg(baseDesc);
}


// --- HealingEffect 实现 ---
HealingEffect::HealingEffect(int amount, bool isPercentage, int chance)
    : Effect(EffectType::HEALING, chance), // 调用基类构造
      m_amount(amount),                     // 治疗量 (具体数值或百分比)
      m_isPercentage(isPercentage)          // 标记是数值治疗还是百分比治疗
{
    // 中文注释：治疗效果构造函数
    setTargetSelf(true); // 治疗效果通常作用于使用者自身
}

bool HealingEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    // 中文注释：应用治疗效果
    if (!checkChance()) return false; // 检查触发几率

    Creature* actualTarget = m_targetSelf ? source : target; // 确定实际治疗目标
    if (!actualTarget || !battle) return false;

    int healAmountCalculated = 0; // 计算出的实际治疗量
    if (m_isPercentage) { // 如果是百分比治疗
        healAmountCalculated = actualTarget->getMaxHP() * m_amount / 100;
    } else { // 如果是固定数值治疗
        healAmountCalculated = m_amount;
    }

    if (healAmountCalculated <= 0) return false; // 如果计算出的治疗量为0或负，则无效

    actualTarget->heal(healAmountCalculated); // 执行治疗
    // battle->addBattleLog(QString("%1 恢复了 %2点 HP!").arg(actualTarget->getName()).arg(healAmountCalculated));
    // battle->emitHealingReceived(actualTarget, healAmountCalculated); // 信号由BattleSystem发出
    return true;
}

QString HealingEffect::getDescription() const
{
    // 中文注释：获取治疗效果的描述文本
    QString amountDesc = m_isPercentage ? QString::number(m_amount) + "%最大HP" : QString::number(m_amount) + "点HP"; // 根据类型格式化治疗量描述
    QString targetDesc = m_targetSelf ? "自身" : "目标"; // 描述目标

    if (m_chance < 100) return QString("有%1%几率回复%2的%3").arg(m_chance).arg(targetDesc).arg(amountDesc);
    return QString("回复%1的%2").arg(targetDesc).arg(amountDesc);
}


// --- FixedDamageEffect 实现 ---
FixedDamageEffect::FixedDamageEffect(int damageAmount, int chance)
    : Effect(EffectType::FIXED_DAMAGE, chance), // 调用基类构造
      m_damageAmount(damageAmount)              // 初始化固定伤害值
{
    // 中文注释：固定伤害效果构造函数
    setTargetSelf(false); // 固定伤害效果通常作用于对方
}

bool FixedDamageEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    // 中文注释：应用固定伤害效果
    if (!checkChance()) return false; // 检查触发几率

    Creature* actualTarget = m_targetSelf ? source : target; // 确定实际目标
    if (!actualTarget || !battle) return false;

    if (m_damageAmount <= 0) return false; // 如果伤害值为0或负，则无效

    actualTarget->takeDamage(m_damageAmount); // 对目标造成固定伤害
    // battle->addBattleLog(QString("%1 对 %2 造成了 %3点固定伤害!").arg(source ? source->getName() : "效果来源").arg(actualTarget->getName()).arg(m_damageAmount));
    // battle->emitDamageCaused(actualTarget, m_damageAmount); // 信号由BattleSystem发出
    return true;
}

QString FixedDamageEffect::getDescription() const
{
    // 中文注释：获取固定伤害效果的描述文本
    QString targetDesc = m_targetSelf ? "自身" : "目标"; // 描述目标
    if (m_chance < 100) return QString("有%1%几率对%2造成%3点固定伤害").arg(m_chance).arg(targetDesc).arg(m_damageAmount);
    return QString("对%1造成%2点固定伤害").arg(targetDesc).arg(m_damageAmount);
}