// src/battle/effect.cpp
#include "effect.h"
#include "../core/creature.h"     // 核心 - 精灵类
#include "battlesystem.h"         // 战斗 - 战斗系统类
#include <QRandomGenerator>       // Qt随机数生成器
#include <QStringList>            // 用于构建描述文本

// --- Effect基类实现 ---
Effect::Effect(EffectType type, int chance)
    : m_type(type),
      m_chance(qBound(0, chance, 100)), // 确保几率在0到100之间
      m_targetSelf(false)               // 默认效果目标是对方
{
}

Effect::~Effect()
{
    // 基类析构函数，目前无特殊操作
}

EffectType Effect::getType() const
{
    return m_type;
}

int Effect::getChance() const
{
    return m_chance;
}

void Effect::setChance(int chance)
{
    m_chance = qBound(0, chance, 100);
}

// 设置效果目标
void Effect::setTargetSelf(bool self)
{
    m_targetSelf = self;
}

// 效果是否作用于自身
bool Effect::isTargetSelf() const
{
    return m_targetSelf;
}

// 检查几率是否触发
bool Effect::checkChance() const
{
    if (m_chance >= 100) return true; // 100%几率必定触发
    if (m_chance <= 0) return false;  // 0%几率必定不触发
    return QRandomGenerator::global()->bounded(100) < m_chance; // [0, 99] < chance
}


// --- TurnBasedEffect 实现 ---
TurnBasedEffect::TurnBasedEffect(int duration,
                                 std::function<void(Creature*, Creature*, BattleSystem*, TurnBasedEffect* self)> effectFunc,
                                 bool onTurnStart,
                                 int chance)
    : Effect(EffectType::TURN_BASED, chance),
      m_initialDuration(duration),
      m_currentDuration(duration),
      m_effectLogic(effectFunc),
      m_onTurnStart(onTurnStart),
      m_description(QString("一个持续%1回合的效果。").arg(duration)) // 默认描述
{
    // 持续时间至少为1回合 (除非设计允许0回合瞬发，但通常回合效果至少1回合)
    if (m_currentDuration < 0) m_currentDuration = 0; // 0回合表示立即结束或无效
}

bool TurnBasedEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    if (!checkChance()) return false; // 未达到触发几率

    Creature* actualTarget = m_targetSelf ? source : target; // 判断效果的实际目标
    if (!actualTarget) return false;

    // 复制自身并添加到目标的持续效果列表中
    // 注意：这里需要深拷贝或传递所有权。简单的做法是让技能创建新的Effect实例
    // 或者，如果此Effect对象由技能拥有且每次使用都创建新的，则可以直接添加。
    // 为确保安全和灵活性，通常技能在use时会创建新的Effect实例。
    // 此处假设apply是将此effect实例（或其副本）添加到目标。
    // 更健壮的设计是Creature::addTurnEffect接收一个TurnBasedEffect的unique_ptr或shared_ptr，
    // 或者在addTurnEffect内部进行复制。
    // 为了简化，这里直接让目标添加此指针，但要注意生命周期管理。
    // 通常，被添加的TurnBasedEffect应该是一个新的实例。
    // 技能施放时: new TurnBasedEffect(...), 然后 effect->apply(...)
    // apply内部: target->addTurnEffect(this); // 这里的this是新创建的实例
    // 这里需要记录效果来源
    this->setOriginalSource(source); // 记录是谁施加的这个效果
    actualTarget->addTurnEffect(new TurnBasedEffect(*this)); // 添加一个副本给目标

    // battle->addBattleLog(QString("%1 对 %2 施加了 %3").arg(source ? source->getName() : "未知来源")
    //                      .arg(actualTarget->getName()).arg(getDescription()));
    return true;
}

QString TurnBasedEffect::getDescription() const
{
    return m_description.isEmpty() ? QString("持续 %1 回合的效果").arg(m_initialDuration) : m_description;
}

int TurnBasedEffect::getDuration() const
{
    return m_currentDuration;
}

void TurnBasedEffect::setDuration(int duration) {
    m_currentDuration = duration;
    if (m_currentDuration < 0) m_currentDuration = 0;
}


bool TurnBasedEffect::decrementDuration()
{
    if (m_currentDuration > 0)
    {
        m_currentDuration--;
    }
    return m_currentDuration <= 0; // 如果持续时间耗尽，返回true
}

void TurnBasedEffect::executeTurnLogic(Creature* affectedCreature, Creature* sourceCreature, BattleSystem* battle)
{
    if (m_effectLogic && affectedCreature) // 确保逻辑函数和受影响者存在
    {
        // sourceCreature 可能是nullptr，如果效果不是由特定精灵直接引发（例如场地天气）
        // 或者使用 this->getOriginalSource() 获取最初的施加者
        m_effectLogic(affectedCreature, this->getOriginalSource() ? this->getOriginalSource() : sourceCreature, battle, this);
    }
}


// --- StatusConditionEffect 实现 ---
StatusConditionEffect::StatusConditionEffect(StatusCondition condition, int chance)
    : Effect(EffectType::STATUS_CONDITION, chance), m_condition(condition)
{
}

StatusCondition StatusConditionEffect::getCondition() const
{
    return m_condition;
}

bool StatusConditionEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    if (!checkChance()) return false;

    Creature* actualTarget = m_targetSelf ? source : target;
    if (!actualTarget || !battle) return false;

    // 检查目标是否已经有相同的异常状态或免疫此异常状态
    if (actualTarget->getStatusCondition() == m_condition) {
        // battle->addBattleLog(QString("%1 已经处于 %2 状态了。").arg(actualTarget->getName()).arg(getDescription()));
        return false; // 已经有此状态，不再施加
    }
    // TODO: 添加免疫检查 (例如：火系免疫烧伤，毒系免疫中毒等)

    actualTarget->setStatusCondition(m_condition);
    // battle->addBattleLog(QString("%1 陷入了 %2 状态!").arg(actualTarget->getName()).arg(getDescription()));
    if (battle) battle->emitStatusChanged(actualTarget, actualTarget->getStatusCondition(), m_condition); // 发送信号
    return true;
}

QString StatusConditionEffect::getDescription() const
{
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
    case StatusCondition::NONE:      conditionText = "正常"; break;
    default:                         conditionText = "未知状态"; break;
    }
    if (m_chance < 100) return QString("有%1%几率使目标%2").arg(m_chance).arg(conditionText);
    return QString("使目标%1").arg(conditionText);
}


// --- StatChangeEffect 实现 ---
StatChangeEffect::StatChangeEffect(StatType stat, int stages, bool targetSelf, int chance)
    : Effect(EffectType::STAT_CHANGE, chance), m_stat(stat), m_stages(stages)
{
    setTargetSelf(targetSelf); // 使用基类的方法设置目标
}

StatType StatChangeEffect::getStat() const
{
    return m_stat;
}

int StatChangeEffect::getStages() const
{
    return m_stages;
}

bool StatChangeEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    if (!checkChance()) return false;

    Creature* actualTarget = m_targetSelf ? source : target;
    if (!actualTarget || !battle) return false;

    // TODO: 检查目标是否有防止能力下降的特性 (例如：白色烟雾) 或已达能力等级上限/下限
    int oldStage = actualTarget->getStatStages().getStage(m_stat);
    actualTarget->modifyStatStage(m_stat, m_stages);
    int newStage = actualTarget->getStatStages().getStage(m_stat);

    if (oldStage != newStage) { // 只有能力等级实际发生变化时才记录日志和发信号
        // QString statName = StatStages::getStatName(m_stat); // 假设有此方法
        // QString action = m_stages > 0 ? "提升" : "降低";
        // battle->addBattleLog(QString("%1 的 %2 %3了 %4 级!").arg(actualTarget->getName()).arg(statName).arg(action).arg(std::abs(m_stages)));
        if (battle) battle->emitStatStageChanged(actualTarget, m_stat, oldStage, newStage);
    } else {
        // battle->addBattleLog(QString("%1 的 %2 没有变化。").arg(actualTarget->getName()).arg(StatStages::getStatName(m_stat)));
        return false; // 能力未变化，效果视为未完全成功
    }
    return true;
}

QString StatChangeEffect::getDescription() const
{
    QString statName;
    // 将StatType转换为中文名 (这个通常应该在StatStages或一个工具类中)
    switch (m_stat) {
        case StatType::ATTACK: statName = "物攻"; break;
        case StatType::DEFENSE: statName = "物防"; break;
        case StatType::SP_ATTACK: statName = "特攻"; break;
        case StatType::SP_DEFENSE: statName = "特防"; break;
        case StatType::SPEED: statName = "速度"; break;
        case StatType::ACCURACY: statName = "命中"; break;
        case StatType::EVASION: statName = "闪避"; break;
        default: statName = "能力"; break;
    }
    QString targetDesc = m_targetSelf ? "自身" : "目标";
    QString actionDesc = m_stages > 0 ? QString("提升%1级").arg(m_stages) : QString("降低%1级").arg(-m_stages);

    if (m_chance < 100) return QString("有%1%几率使%2的%3%4").arg(m_chance).arg(targetDesc).arg(statName).arg(actionDesc);
    return QString("使%1的%2%3").arg(targetDesc).arg(statName).arg(actionDesc);
}


// --- ClearEffectsEffect 实现 ---
ClearEffectsEffect::ClearEffectsEffect(bool clearPositiveStatChanges, bool clearNegativeStatChanges,
                                       bool clearStatusConditions, bool clearTurnBasedEffects,
                                       bool targetSelf, int chance)
    : Effect(EffectType::CLEAR_EFFECTS, chance),
      m_clearPositiveStatChanges(clearPositiveStatChanges),
      m_clearNegativeStatChanges(clearNegativeStatChanges),
      m_clearStatusConditions(clearStatusConditions),
      m_clearTurnBasedEffects(clearTurnBasedEffects)
{
    setTargetSelf(targetSelf);
}

bool ClearEffectsEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    if (!checkChance()) return false;

    Creature* actualTarget = m_targetSelf ? source : target;
    if (!actualTarget || !battle) return false;

    bool somethingCleared = false;

    if (m_clearStatusConditions && actualTarget->getStatusCondition() != StatusCondition::NONE) {
        // battle->addBattleLog(QString("%1 的异常状态被清除了!").arg(actualTarget->getName()));
        actualTarget->clearStatusCondition();
        somethingCleared = true;
    }

    if (m_clearPositiveStatChanges || m_clearNegativeStatChanges) {
        StatStages currentStages = actualTarget->getStatStages();
        bool statsReset = false;
        for (StatType type : {StatType::ATTACK, StatType::DEFENSE, StatType::SP_ATTACK, StatType::SP_DEFENSE, StatType::SPEED, StatType::ACCURACY, StatType::EVASION}) {
            int stage = currentStages.getStage(type);
            if (m_clearPositiveStatChanges && stage > 0) {
                actualTarget->modifyStatStage(type, -stage); // 清除正向强化
                statsReset = true;
            }
            if (m_clearNegativeStatChanges && stage < 0) {
                actualTarget->modifyStatStage(type, -stage); // 清除负向弱化
                statsReset = true;
            }
        }
        if (statsReset) {
            // battle->addBattleLog(QString("%1 的能力变化被清除了!").arg(actualTarget->getName()));
            somethingCleared = true;
        }
    }

    if (m_clearTurnBasedEffects && !actualTarget->getTurnEffects().isEmpty()) {
        // battle->addBattleLog(QString("%1 的回合类效果被清除了!").arg(actualTarget->getName()));
        actualTarget->clearAllTurnEffects();
        somethingCleared = true;
    }

    return somethingCleared; // 如果确实清除了某些东西，则返回true
}

QString ClearEffectsEffect::getDescription() const
{
    QStringList clearedItems;
    if (m_clearPositiveStatChanges) clearedItems.append("能力提升");
    if (m_clearNegativeStatChanges) clearedItems.append("能力下降");
    if (m_clearStatusConditions) clearedItems.append("异常状态");
    if (m_clearTurnBasedEffects) clearedItems.append("回合效果");

    if (clearedItems.isEmpty()) return "无清除效果";

    QString targetDesc = m_targetSelf ? "自身" : "目标";
    QString itemsDesc = clearedItems.join("、");

    if (m_chance < 100) return QString("有%1%几率清除%2的%3").arg(m_chance).arg(targetDesc).arg(itemsDesc);
    return QString("清除%1的%2").arg(targetDesc).arg(itemsDesc);
}


// --- ImmunityEffect 实现 ---
ImmunityEffect::ImmunityEffect(int duration, bool immuneToStatus, ElementType immuneToSpecificTypeDamage, int chance)
    : Effect(EffectType::IMMUNITY, chance),
      m_duration(duration),
      m_immuneToStatus(immuneToStatus),
      m_immuneToTypeDamage(immuneToSpecificTypeDamage)
{
    // 免疫效果通常作用于自身
    setTargetSelf(true);
}

bool ImmunityEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    // 免疫效果通常是将一个特定的TurnBasedEffect施加给目标（通常是source自己）
    // 这个TurnBasedEffect的逻辑会在伤害计算或状态施加前检查
    if (!checkChance()) return false;

    Creature* actualTarget = m_targetSelf ? source : target; // 明确目标
    if (!actualTarget || !battle) return false;

    // 创建一个代表免疫状态的TurnBasedEffect
    // 这个 effectFunc 应该在 TurnBasedEffect 内部定义，或者在这里捕获必要的免疫参数
    // 简化版：假设Creature类有 m_isImmuneToStatus, m_isImmuneToType[ElementType] 等标志位
    // 并且TurnBasedEffect可以修改这些标志位

    // 示例：创建一个回合效果，该效果在执行时（例如，在BattleSystem的伤害计算前或状态施加前）
    // 会被检查，从而实现免疫。
    // 另一种方式是 TurnBasedEffect 包含一个回调，这个回调在特定事件发生时（如尝试施加状态）被调用。

    // 为了这个示例，我们创建一个描述性的TurnBasedEffect，实际的免疫逻辑需要BattleSystem在各处检查
    // 目标生物是否拥有这类“免疫”回合效果。

    QString immunityDesc = getDescription(); // 获取此免疫效果的描述
    auto immunityLogic = [this, immunityDesc](Creature* affected, Creature* src, BattleSystem* btl, TurnBasedEffect* self_effect) {
        // 这个回调函数在每回合的开始/结束时可以不做具体事情，
        // 关键是这个TurnBasedEffect本身的存在代表了免疫状态。
        // BattleSystem在尝试造成伤害或施加状态时，会检查affected精灵是否拥有这个类型的TurnBasedEffect。
        // btl->addBattleLog(QString("%1 仍处于“%2”效果下。").arg(affected->getName()).arg(immunityDesc));
    };

    TurnBasedEffect* tbe = new TurnBasedEffect(m_duration, immunityLogic, false, 100); // 假设回合结束时处理，100%触发
    tbe->setDescription(immunityDesc); // 设置描述
    tbe->setOriginalSource(source); // 记录效果来源

    actualTarget->addTurnEffect(tbe); // 给目标添加这个免疫标记效果

    // battle->addBattleLog(QString("%1 获得了“%2”效果!").arg(actualTarget->getName()).arg(immunityDesc));
    return true;
}

QString ImmunityEffect::getDescription() const
{
    QStringList immunities;
    if (m_immuneToStatus) immunities.append("异常状态");
    if (m_immuneToTypeDamage != ElementType::NONE) {
        immunities.append(QString("对%1属性伤害").arg(Type::getElementTypeName(m_immuneToTypeDamage)));
    }

    if (immunities.isEmpty()) return QString("持续%1回合的免疫效果").arg(m_duration);

    QString targetDesc = m_targetSelf ? "自身" : "目标";
    QString itemsDesc = immunities.join("和");
    QString baseDesc = QString("%1回合内免疫%2").arg(m_duration).arg(itemsDesc);

    if (m_chance < 100) return QString("有%1%几率使%2%3").arg(m_chance).arg(targetDesc).arg(baseDesc);
    return QString("使%1%2").arg(targetDesc).arg(baseDesc);
}


// --- HealingEffect 实现 ---
HealingEffect::HealingEffect(int amount, bool isPercentage, int chance)
    : Effect(EffectType::HEALING, chance), m_amount(amount), m_isPercentage(isPercentage)
{
    // 治疗效果通常作用于自身
    setTargetSelf(true);
}

bool HealingEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    if (!checkChance()) return false;

    Creature* actualTarget = m_targetSelf ? source : target;
    if (!actualTarget || !battle) return false;

    int healAmount = 0;
    if (m_isPercentage) {
        healAmount = actualTarget->getMaxHP() * m_amount / 100;
    } else {
        healAmount = m_amount;
    }

    if (healAmount <= 0) return false; // 没有实际治疗量

    actualTarget->heal(healAmount);
    // battle->addBattleLog(QString("%1 恢复了 %2点 HP!").arg(actualTarget->getName()).arg(healAmount));
    if (battle) battle->emitHealingReceived(actualTarget, healAmount);
    return true;
}

QString HealingEffect::getDescription() const
{
    QString amountDesc = m_isPercentage ? QString::number(m_amount) + "%最大HP" : QString::number(m_amount) + "点HP";
    QString targetDesc = m_targetSelf ? "自身" : "目标";

    if (m_chance < 100) return QString("有%1%几率回复%2的%3").arg(m_chance).arg(targetDesc).arg(amountDesc);
    return QString("回复%1的%2").arg(targetDesc).arg(amountDesc);
}


// --- FixedDamageEffect 实现 ---
FixedDamageEffect::FixedDamageEffect(int damageAmount, int chance)
    : Effect(EffectType::FIXED_DAMAGE, chance), m_damageAmount(damageAmount)
{
    // 固定伤害效果通常作用于对方
    setTargetSelf(false);
}

bool FixedDamageEffect::apply(Creature* source, Creature* target, BattleSystem* battle)
{
    if (!checkChance()) return false;

    Creature* actualTarget = m_targetSelf ? source : target; // 虽然通常对敌，但保持灵活性
    if (!actualTarget || !battle) return false;

    if (m_damageAmount <= 0) return false; // 无伤害

    actualTarget->takeDamage(m_damageAmount);
    // battle->addBattleLog(QString("%1 对 %2 造成了 %3点 固定伤害!").arg(source ? source->getName() : "效果来源").arg(actualTarget->getName()).arg(m_damageAmount));
    if (battle) battle->emitDamageCaused(actualTarget, m_damageAmount); // 发出伤害信号
    return true;
}

QString FixedDamageEffect::getDescription() const
{
    QString targetDesc = m_targetSelf ? "自身" : "目标";
    if (m_chance < 100) return QString("有%1%几率对%2造成%3点固定伤害").arg(m_chance).arg(targetDesc).arg(m_damageAmount);
    return QString("对%1造成%2点固定伤害").arg(targetDesc).arg(m_damageAmount);
}

// FailureCompensationEffect (效果未触发补偿) 的实现需要更具体的补偿效果类型
// 这里暂时留空或提供一个简单占位
// class FailureCompensationEffect : public Effect {
// public:
//     FailureCompensationEffect(Effect* compensation, int chance = 100)
//         : Effect(EffectType::FAILURE_COMPENSATION, chance), m_compensationEffect(compensation) {}
//     ~FailureCompensationEffect() { delete m_compensationEffect; }

//     bool apply(Creature* source, Creature* target, BattleSystem* battle) override {
//         if (checkChance() && m_compensationEffect) {
//             return m_compensationEffect->apply(source, target, battle);
//         }
//         return false;
//     }
//     QString getDescription() const override {
//         if (m_compensationEffect) return "若主要效果失败，则：" + m_compensationEffect->getDescription();
//         return "效果失败时有补偿。";
//     }
// private:
//     Effect* m_compensationEffect; // 补偿效果
// };