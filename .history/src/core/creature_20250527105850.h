#ifndef CREATURE_H
#define CREATURE_H

#include <QString>
#include <QVector>
#include <QMap>
#include "type.h"
#include "ability.h"
#include "../battle/skill.h"
#include "../battle/effect.h"

// 前向声明
class BattleSystem;

// 第五技能类 - 继承自Skill以解决类型兼容问题
class FifthSkill : public Skill {
public:
    FifthSkill(const QString &name, ElementType type, int power, 
               int ppCost, int accuracy, int priority = 0);
    virtual ~FifthSkill();
    
    // 检查是否可以使用第五技能（特殊条件）
    virtual bool canUse(Creature *user, Creature *target, BattleSystem *battle) const;
};

// 精灵等级和经验值计算常量
const int MAX_LEVEL = 100;
const int BASE_EXP_NEEDED = 1000;

// 精灵基类
class Creature
{
public:
    Creature(const QString &name, const Type &type, int level = 1);
    virtual ~Creature();

    // 获取基本信息
    QString getName() const;
    void setName(const QString &name);
    virtual QString getResourceName() const;
    Type getType() const;
    void setType(const Type &type);
    int getLevel() const;
    void setLevel(int level);
    int getExperience() const;
    void setExperience(int exp);
    int getExperienceToNextLevel() const;

    // 属性和状态
    BaseStats getBaseStats() const;
    BaseStats getCurrentStats() const;
    StatStages &getStatStages();
    Talent getTalent() const;
    StatusCondition getStatusCondition() const;

    // 设置基本属性
    void setBaseStats(const BaseStats &stats);
    void setTalent(const Talent &talent);
    void setMaxPP(int maxPP);

    // 战斗相关属性
    int getCurrentHP() const;
    void setCurrentHP(int hp);
    int getMaxHP() const;
    int getCurrentPP() const;
    void setCurrentPP(int pp);
    int getMaxPP() const;

    // 精灵操作
    bool isDead() const;
    bool isWild() const;
    void setWild(bool isWild);
    bool canAct() const;
    void gainExperience(int exp);
    bool tryLevelUp();

    // 战斗状态操作
    int takeDamage(int damage);
    int heal(int amount);
    void consumePP(int amount);
    int restorePP(int amount);
    void setStatusCondition(StatusCondition condition);
    void clearStatusCondition();
    bool modifyStatStage(StatType stat, int delta);
    void resetStatStages();

    // 技能管理
    void learnSkill(Skill *skill);
    void forgetSkill(int index);
    bool hasSkill(const QString &skillName) const;
    Skill *getSkill(int index) const;
    QVector<Skill *> getSkills() const;
    int getSkillCount() const;
    void setFifthSkill(FifthSkill *skill);
    FifthSkill *getFifthSkill() const;

    // 回合效果
    void addTurnEffect(TurnBasedEffect *effect);
    void removeTurnEffect(TurnBasedEffect *effect);
    void clearTurnEffects();
    void clearAllTurnEffects(); // 兼容性方法，内部调用clearTurnEffects
    QVector<TurnBasedEffect *> getTurnEffects() const;

    // 战斗行为
    virtual bool useSkill(int skillIndex, Creature *target, BattleSystem *battle);
    virtual bool useFifthSkill(Creature *target, BattleSystem *battle);

    // 回合开始和结束事件
    virtual void onTurnStart();
    virtual void onTurnEnd();

    // 计算相关属性
    int calculateAttack() const;
    int calculateSpecialAttack() const;
    int calculateDefense() const;
    int calculateSpecialDefense() const;
    int calculateSpeed() const;

    // 克制系统计算
    double getTypeEffectivenessAgainst(const Creature *target, ElementType skillType) const;
    bool hasTypeAdvantage(ElementType skillType) const; // 判断技能是否具有STAB加成

protected:
    QString m_name;            // 精灵名称
    Type m_type;               // 精灵属性
    int m_level;               // 等级
    int m_experience;          // 当前经验值
    BaseStats m_baseStats;     // 基础属性
    StatStages m_statStages;   // 属性等级（战斗中临时变化）
    Talent m_talent;           // 天赋
    QVector<Skill *> m_skills; // 技能列表（普通技能）
    FifthSkill *m_fifthSkill;  // 第五技能

    int m_currentHP; // 当前生命值
    int m_maxHP;     // 最大生命值
    int m_currentPP; // 当前PP值
    int m_maxPP;     // 最大PP值

    bool m_isDead;   // 是否已死亡
    bool m_isWild;   // 是否为野生精灵

    StatusCondition m_statusCondition;        // 异常状态
    QVector<TurnBasedEffect *> m_turnEffects; // 回合效果

    // 计算升级所需经验值
    int calculateExperienceToNextLevel() const;

    // 升级时更新属性
    void updateStatsOnLevelUp();
    
    // 计算实际属性
    void calculateStats();
};

// 具体精灵类（木棍人）
class TungTungTung : public Creature
{
public:
    TungTungTung(int level = 1);
    virtual ~TungTungTung() = default;

    // 特殊行为或能力
    virtual void onTurnStart() override;
    virtual void onTurnEnd() override;
};

// 具体精灵类（鳄鱼轰炸机）
class BombardinoCrocodillo : public Creature
{
public:
    BombardinoCrocodillo(int level = 1);
    virtual ~BombardinoCrocodillo() = default;

    // 特殊行为或能力
    virtual void onTurnStart() override;
    virtual void onTurnEnd() override;
};

// 具体精灵类（耐克鲨鱼）
class TralaleroTralala : public Creature
{
public:
    TralaleroTralala(int level = 1);
    virtual ~TralaleroTralala() = default;

    // 特殊行为或能力
    virtual void onTurnStart() override;
    virtual void onTurnEnd() override;
};

// 具体精灵类（仙人掌大象）
class LiriliLarila : public Creature
{
public:
    LiriliLarila(int level = 1);
    virtual ~LiriliLarila() = default;

    // 特殊行为或能力
    virtual void onTurnStart() override;
    virtual void onTurnEnd() override;
};

// 具体精灵类（香蕉绿猩猩）
class ChimpanziniBananini : public Creature
{
public:
    ChimpanziniBananini(int level = 1);
    virtual ~ChimpanziniBananini() = default;

    // 变身状态
    bool isInBerserkForm() const;
    void enterBerserkForm(int duration);
    void exitBerserkForm();

    // 特殊行为或能力
    virtual void onTurnStart() override;
    virtual void onTurnEnd() override;

private:
    bool m_inBerserkForm;
    int m_berserkFormDuration;
};

// 具体精灵类（鹿管鹿管鹿鹿时间到了）
class Luguanluguanlulushijiandaole : public Creature
{
public:
    Luguanluguanlulushijiandaole(int level = 1);
    virtual ~Luguanluguanlulushijiandaole() = default;

    // 时间操控相关
    void recordBattleState();
    bool tryRevertBattleState(BattleSystem *battle);

    // 特殊行为或能力
    virtual void onTurnStart() override;
    virtual void onTurnEnd() override;

private:
    struct BattleSnapshot
    {
        int timestamp;
        QMap<QString, int> creatureHPs;
        QMap<QString, int> creaturePPs;
        QMap<QString, QMap<StatType, int>> creatureStatStages;
        QMap<QString, StatusCondition> creatureStatusConditions;
    };

    BattleSnapshot m_recordedState;
    int m_snapshotTurnsLeft;
};

// 具体精灵类（卡布奇诺忍者）
class CappuccinoAssassino : public Creature
{
public:
    CappuccinoAssassino(int level = 1);
    virtual ~CappuccinoAssassino() = default;

    // 影子潜伏状态
    bool isInShadowState() const;
    void enterShadowState();
    void exitShadowState();

    // 特殊行为或能力
    virtual void onTurnStart() override;
    virtual void onTurnEnd() override;

private:
    bool m_inShadowState;
};

#endif // CREATURE_H
