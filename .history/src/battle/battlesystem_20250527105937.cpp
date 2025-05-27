#include "battlesystem.h"
#include "../core/creature.h"
#include "../core/gameengine.h"
#include "skill.h"
#include "effect.h"
#include <QRandomGenerator>
#include <QDebug>
#include <algorithm>

// 构造函数
BattleSystem::BattleSystem(QObject *parent)
    : QObject(parent),
      m_playerTeam(nullptr),
      m_opponentTeam(nullptr),
      m_playerActiveCreatureIndex(0),
      m_opponentActiveCreatureIndex(0),
      m_currentTurn(0),
      m_isPlayerTurn(true),
      m_battleState(BattleState::NOT_STARTED)
{
}

// 析构函数
BattleSystem::~BattleSystem()
{
    // 清理战斗队列
    m_actionQueue.clear();
    
    // 清理战斗日志
    m_battleLog.clear();
}

// 初始化战斗
void BattleSystem::initBattle(QVector<Creature*> playerTeam, QVector<Creature*> opponentTeam)
{
    // 设置队伍
    m_playerTeam = playerTeam;
    m_opponentTeam = opponentTeam;
    
    // 初始化索引
    m_playerActiveCreatureIndex = 0;
    m_opponentActiveCreatureIndex = 0;
    
    // 初始化回合
    m_currentTurn = 0;
    m_isPlayerTurn = true;
    
    // 设置战斗状态
    m_battleState = BattleState::WAITING_FOR_INPUT;
    
    // 清理战斗队列
    m_actionQueue.clear();
    
    // 清理战斗日志
    m_battleLog.clear();
    
    // 添加战斗开始日志
    addBattleLog("战斗开始！");
    
    // 触发战斗开始事件
    emit battleStarted();
}

// 获取玩家队伍
QVector<Creature*> BattleSystem::getPlayerTeam() const
{
    return m_playerTeam;
}

// 获取对手队伍
QVector<Creature*> BattleSystem::getOpponentTeam() const
{
    return m_opponentTeam;
}

// 获取玩家当前精灵
Creature* BattleSystem::getPlayerActiveCreature() const
{
    if (m_playerActiveCreatureIndex >= 0 && m_playerActiveCreatureIndex < m_playerTeam.size()) {
        return m_playerTeam[m_playerActiveCreatureIndex];
    }
    return nullptr;
}

// 获取对手当前精灵
Creature* BattleSystem::getOpponentActiveCreature() const
{
    if (m_opponentActiveCreatureIndex >= 0 && m_opponentActiveCreatureIndex < m_opponentTeam.size()) {
        return m_opponentTeam[m_opponentActiveCreatureIndex];
    }
    return nullptr;
}

// 获取当前回合数
int BattleSystem::getCurrentTurn() const
{
    return m_currentTurn;
}

// 是否是玩家回合
bool BattleSystem::isPlayerTurn() const
{
    return m_isPlayerTurn;
}

// 获取战斗状态
BattleState BattleSystem::getBattleState() const
{
    return m_battleState;
}

// 获取战斗日志
QStringList BattleSystem::getBattleLog() const
{
    return m_battleLog;
}

// 添加战斗日志
void BattleSystem::addBattleLog(const QString &log)
{
    m_battleLog.append(log);
    emit battleLogUpdated(log);
}

// 队列玩家行动
void BattleSystem::queuePlayerAction(BattleAction action, int param1, int param2)
{
    if (m_battleState != BattleState::WAITING_FOR_INPUT) {
        return;
    }
    
    BattleActionItem item;
    item.action = action;
    item.param1 = param1;
    item.param2 = param2;
    item.actor = getPlayerActiveCreature();
    item.target = getOpponentActiveCreature();
    
    // 如果是技能，获取技能对象以确定优先级
    Skill *s = nullptr;
    if (action == BattleAction::USE_SKILL) {
        if (param1 == -1) s = item.actor->getFifthSkill(); // 第五技能
        else if (param1 >= 0 && param1 < item.actor->getSkillCount()) s = item.actor->getSkill(param1);
    }
    
    if (s) {
        item.priority = s->getPriority();
    } else {
        item.priority = 0;
    }
    
    m_actionQueue.append(item);
    
    // 如果是野外战斗，自动生成对手行动
    if (m_opponentTeam.size() == 1 && m_opponentTeam[0]->isWild()) {
        queueOpponentAction(BattleAction::USE_SKILL, QRandomGenerator::global()->bounded(4), 0);
    }
    
    // 如果队列中有两个行动（玩家和对手各一个），则执行行动队列
    if (m_actionQueue.size() >= 2) {
        m_battleState = BattleState::EXECUTING_ACTIONS;
        executeActionQueue();
    }
}

// 队列对手行动
void BattleSystem::queueOpponentAction(BattleAction action, int param1, int param2)
{
    BattleActionItem item;
    item.action = action;
    item.param1 = param1;
    item.param2 = param2;
    item.actor = getOpponentActiveCreature();
    item.target = getPlayerActiveCreature();
    
    // 如果是技能，获取技能对象以确定优先级
    Skill *s = nullptr;
    if (action == BattleAction::USE_SKILL) {
        if (param1 == -1) s = item.actor->getFifthSkill(); // 第五技能
        else if (param1 >= 0 && param1 < item.actor->getSkillCount()) s = item.actor->getSkill(param1);
    }
    
    if (s) {
        item.priority = s->getPriority();
    } else {
        item.priority = 0;
    }
    
    m_actionQueue.append(item);
}

// 执行行动队列
void BattleSystem::executeActionQueue()
{
    // 按优先级排序行动队列
    std::sort(m_actionQueue.begin(), m_actionQueue.end(), [](const BattleActionItem &a, const BattleActionItem &b) {
        if (a.priority != b.priority) {
            return a.priority > b.priority; // 优先级高的先行动
        }
        return a.actor->calculateSpeed() > b.actor->calculateSpeed(); // 速度快的先行动
    });
    
    // 执行行动
    while (!m_actionQueue.isEmpty()) {
        BattleActionItem item = m_actionQueue.takeFirst();
        Creature *actor = item.actor;
        Creature *target = item.target;
        
        // 检查行动者是否可以行动
        if (!actor->canAct()) {
            addBattleLog(QString("%1无法行动！").arg(actor->getName()));
            continue;
        }
        
        // 根据行动类型执行不同操作
        switch (item.action) {
            case BattleAction::USE_SKILL: {
                // 使用技能
                Skill *skillToUse = nullptr;
                if (item.param1 == -1) {
                    // 使用第五技能
                    skillToUse = actor->getFifthSkill();
                    if (skillToUse && !dynamic_cast<FifthSkill*>(skillToUse)->canUse(actor, target, this)) {
                        addBattleLog(QString("%1无法使用%2！").arg(actor->getName()).arg(skillToUse->getName()));
                        skillToUse = nullptr;
                    }
                }
                else if (item.param1 >= 0 && item.param1 < actor->getSkillCount()) skillToUse = actor->getSkill(item.param1);
                
                if (skillToUse) {
                    // 检查PP是否足够
                    if (actor->getCurrentPP() < skillToUse->getPPCost()) {
                        addBattleLog(QString("%1的PP不足，无法使用%2！").arg(actor->getName()).arg(skillToUse->getName()));
                        break;
                    }
                    
                    // 确定技能目标
                    Creature *skillTarget = target;
                    // 如果技能或效果是针对自身的，则目标改为自己
                    if (skillToUse->getTargetType() == TargetType::SELF || 
                        (skillToUse->getEffects().size() > 0 && 
                         skillToUse->getEffects().first()->getTargetType() == TargetType::SELF)){ 
                        skillTarget = actor;
                    }
                    
                    // 消耗PP
                    actor->consumePP(skillToUse->getPPCost());
                    
                    // 使用技能
                    addBattleLog(QString("%1使用了%2！").arg(actor->getName()).arg(skillToUse->getName()));
                    
                    bool hit = skillToUse->use(actor, skillTarget, this);
                    
                    // 如果是伤害技能，计算伤害
                    if (hit && skillToUse->getPower() > 0) {
                        // 计算伤害
                        int damage = skillToUse->calculateDamage(actor, skillTarget);
                        
                        // 计算属性克制
                        QString stabBonusText = actor->hasTypeAdvantage(skillToUse->getType()) ? "(属性一致加成) " : "";
                        double effectiveness = actor->getTypeEffectivenessAgainst(skillTarget, skillToUse->getType());
                        
                        // 应用伤害
                        int actualDamage = skillTarget->takeDamage(damage);
                        
                        // 添加伤害日志
                        QString effectivenessText;
                        if (effectiveness > 1.5) {
                            effectivenessText = "效果拔群！";
                        } else if (effectiveness < 0.5) {
                            effectivenessText = "效果不佳...";
                        }
                        
                        addBattleLog(QString("%1%2受到了%3点伤害！%4").
                                   arg(stabBonusText).
                                   arg(skillTarget->getName()).
                                   arg(actualDamage).
                                   arg(effectivenessText));
                        
                        // 检查目标是否死亡
                        if (skillTarget->isDead()) {
                            addBattleLog(QString("%1倒下了！").arg(skillTarget->getName()));
                            
                            // 检查战斗是否结束
                            checkBattleEnd();
                        }
                    }
                } else {
                    addBattleLog(QString("%1选择了无效的技能！").arg(actor->getName()));
                }
                break;
            }
            case BattleAction::RESTORE_PP: {
                // 恢复PP
                if (actor && actor->canAct() && actor->getCurrentPP() < actor->getMaxPP()) {
                    int amount = actor->getMaxPP() / 5; // 恢复20%的PP
                    int restored = actor->restorePP(amount);
                    addBattleLog(QString("%1恢复了%2点PP！").arg(actor->getName()).arg(restored));
                }
                break;
            }
            case BattleAction::SWITCH_CREATURE: {
                // 切换精灵
                int newIndex = item.param1;
                if (actor == getPlayerActiveCreature()) {
                    // 玩家切换精灵
                    if (newIndex >= 0 && newIndex < m_playerTeam.size() && newIndex != m_playerActiveCreatureIndex) {
                        // 当前精灵退场
                        addBattleLog(QString("%1退场了！").arg(actor->getName()));
                        
                        // 切换到新精灵
                        m_playerActiveCreatureIndex = newIndex;
                        Creature *newCreature = getPlayerActiveCreature();
                        
                        addBattleLog(QString("%1登场了！").arg(newCreature->getName()));
                        
                        // 触发精灵切换事件
                        emit playerCreatureSwitched(m_playerActiveCreatureIndex);
                    }
                } else {
                    // 对手切换精灵
                    if (newIndex >= 0 && newIndex < m_opponentTeam.size() && newIndex != m_opponentActiveCreatureIndex) {
                        // 当前精灵退场
                        addBattleLog(QString("%1退场了！").arg(actor->getName()));
                        
                        // 切换到新精灵
                        m_opponentActiveCreatureIndex = newIndex;
                        Creature *newCreature = getOpponentActiveCreature();
                        
                        addBattleLog(QString("%1登场了！").arg(newCreature->getName()));
                        
                        // 触发精灵切换事件
                        emit opponentCreatureSwitched(m_opponentActiveCreatureIndex);
                    }
                }
                break;
            }
            case BattleAction::ESCAPE: {
                // 逃跑
                if (m_opponentTeam.size() == 1 && m_opponentTeam[0]->isWild()) {
                    // 野外战斗可以直接逃跑
                    addBattleLog("成功逃跑！");
                    m_battleState = BattleState::PLAYER_ESCAPED;
                    emit battleEnded(BattleResult::PLAYER_ESCAPED);
                } else {
                    // 对战中不能逃跑
                    addBattleLog("对战中无法逃跑！");
                }
                break;
            }
            default:
                break;
        }
    }
    
    // 检查战斗是否结束
    if (m_battleState == BattleState::EXECUTING_ACTIONS) {
        // 如果战斗未结束，进入下一回合
        startNextTurn();
    }
}

// 开始下一回合
void BattleSystem::startNextTurn()
{
    // 增加回合计数
    m_currentTurn++;
    
    // 切换回合
    m_isPlayerTurn = !m_isPlayerTurn;
    
    // 获取当前行动的精灵
    Creature *activeCreature = m_isPlayerTurn ? getPlayerActiveCreature() : getOpponentActiveCreature();
    
    // 触发回合开始事件
    if (activeCreature) {
        activeCreature->onTurnStart();
    }
    
    // 添加回合开始日志
    addBattleLog(QString("第%1回合开始，%2的回合！").
               arg(m_currentTurn).
               arg(m_isPlayerTurn ? "玩家" : "对手"));
    
    // 设置战斗状态
    m_battleState = BattleState::WAITING_FOR_INPUT;
    
    // 触发回合开始事件
    emit turnStarted(m_currentTurn, m_isPlayerTurn);
    
    // 如果是对手回合，自动生成对手行动
    if (!m_isPlayerTurn) {
        // 简单AI：随机选择技能
        int skillIndex = QRandomGenerator::global()->bounded(getOpponentActiveCreature()->getSkillCount());
        queueOpponentAction(BattleAction::USE_SKILL, skillIndex, 0);
        
        // 立即执行玩家行动（因为对手是自动的）
        queuePlayerAction(BattleAction::WAIT, 0, 0);
    }
}

// 结束当前回合
void BattleSystem::endCurrentTurn()
{
    // 获取当前行动的精灵
    Creature *activeCreature = m_isPlayerTurn ? getPlayerActiveCreature() : getOpponentActiveCreature();
    
    // 触发回合结束事件
    if (activeCreature) {
        activeCreature->onTurnEnd();
    }
    
    // 添加回合结束日志
    addBattleLog(QString("%1的回合结束！").arg(m_isPlayerTurn ? "玩家" : "对手"));
    
    // 触发回合结束事件
    emit turnEnded(m_currentTurn, m_isPlayerTurn);
}

// 检查战斗是否结束
void BattleSystem::checkBattleEnd()
{
    // 检查玩家队伍是否全部阵亡
    bool playerLost = true;
    for (Creature *creature : m_playerTeam) {
        if (!creature->isDead()) {
            playerLost = false;
            break;
        }
    }
    
    // 检查对手队伍是否全部阵亡
    bool opponentLost = true;
    for (Creature *creature : m_opponentTeam) {
        if (!creature->isDead()) {
            opponentLost = false;
            break;
        }
    }
    
    // 根据结果设置战斗状态
    if (playerLost) {
        m_battleState = BattleState::PLAYER_LOST;
        addBattleLog("玩家失败了！");
        emit battleEnded(BattleResult::PLAYER_LOST);
    } else if (opponentLost) {
        m_battleState = BattleState::PLAYER_WON;
        addBattleLog("玩家获胜了！");
        
        // 如果是野外战斗，给予经验值
        if (m_opponentTeam.size() == 1 && m_opponentTeam[0]->isWild()) {
            int expGained = m_opponentTeam[0]->getLevel() * 50; // 简单公式
            getPlayerActiveCreature()->gainExperience(expGained);
            addBattleLog(QString("%1获得了%2点经验值！").arg(getPlayerActiveCreature()->getName()).arg(expGained));
        }
        
        emit battleEnded(BattleResult::PLAYER_WON);
    } else if (m_battleState == BattleState::PLAYER_ESCAPED) {
        // 已经在逃跑行动中设置了状态
        emit battleEnded(BattleResult::PLAYER_ESCAPED);
    } else {
        // 战斗未结束，检查当前精灵是否需要切换
        
        // 检查玩家当前精灵
        if (getPlayerActiveCreature() && getPlayerActiveCreature()->isDead()) {
            // 寻找下一个可用精灵
            for (int i = 0; i < m_playerTeam.size(); i++) {
                if (!m_playerTeam[i]->isDead()) {
                    m_playerActiveCreatureIndex = i;
                    addBattleLog(QString("%1登场了！").arg(getPlayerActiveCreature()->getName()));
                    emit playerCreatureSwitched(m_playerActiveCreatureIndex);
                    break;
                }
            }
        }
        
        // 检查对手当前精灵
        if (getOpponentActiveCreature() && getOpponentActiveCreature()->isDead()) {
            // 寻找下一个可用精灵
            for (int i = 0; i < m_opponentTeam.size(); i++) {
                if (!m_opponentTeam[i]->isDead()) {
                    m_opponentActiveCreatureIndex = i;
                    addBattleLog(QString("%1登场了！").arg(getOpponentActiveCreature()->getName()));
                    emit opponentCreatureSwitched(m_opponentActiveCreatureIndex);
                    break;
                }
            }
        }
    }
}
