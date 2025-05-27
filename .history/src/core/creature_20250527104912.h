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
class FifthSkill;

// 精灵基类
class Creature
{
public:
    Creature(const QString &name, const Type &type, int level = 1);
    virtual ~Creature();

    // 获取基本信息
    QString getName() const;
    void setName(const QString &name);
    Type getType() const;
    void setType(const Type &type);
    int getLevel() const;
    void setLevel(int level);
    int getExperience() const;
    void setExperience(int exp);

    // 获取和设置属性
    BaseStats getBaseStats() const;
    void setBaseStats(const BaseStats &stats);
    Talent getTalent() const;
    void setTalent(const Talent &talent);
    StatStages &getStatStages();
    StatusCondition getStatusCondition() const;
    void setStatusCondition(StatusCondition condition);

    // 状态检查
    bool isDead() const;
    bool isWild() const;
    void setWild(bool isWild);

    // 技能管理
    void learnSkill(Skill *skill);
    QVector<Skill *> getSkills() const;
    void setFifthSkill(FifthSkill *skill);
    FifthSkill *getFifthSkill() const;

    // 战斗相关
    bool useSkill(int index, Creature *target, BattleSystem *battle);
    bool useFifthSkill(Creature *target, BattleSystem *battle);
    int takeDamage(int amount);
    int heal(int amount);
    int restorePP(int amount);
    bool modifyStatStage(StatType type, int delta);
    void resetStatStages();

    // 回合效果管理
    void addTurnEffect(TurnBasedEffect *effect);
    void removeTurnEffect(TurnBasedEffect *effect);
    void clearTurnEffects();
    QVector<TurnBasedEffect *> getTurnEffects() const;

    // 回合事件
    virtual void onTurnStart();
    virtual void onTurnEnd();

    // 获取当前属性
    int getCurrentHP() const;
    void setCurrentHP(int hp);
    int getMaxHP() const;
    int getCurrentPP() const;
    void setCurrentPP(int pp);
    int getMaxPP() const;
    int getAttack() const;
    int getSpecialAttack() const;
    int getDefense() const;
    int getSpecialDefense() const;
    int getSpeed() const;
    double getAccuracyModifier() const;
    double getEvasionModifier() const;

    // 信息显示
    QString getDetailedInfo() const;
    QString getBriefInfo() const;
    QString getBattleStatusInfo() const;
    QStringList getSkillSelectionInfo() const;

    // 静态工厂方法
    static Creature *createCreature(const QString &creatureType, int level);

protected:
    QString m_name;
    Type m_type;
    int m_level;
    int m_currentHP;
    int m_maxHP;
    int m_currentPP;
    int m_maxPP;
    int m_experience;
    bool m_isDead;
    bool m_isWild;

    // 属性相关
    BaseStats m_baseStats;
    int m_attack;
    int m_specialAttack;
    int m_defense;
    int m_specialDefense;
    int m_speed;
    Talent m_talent;
    StatStages m_statStages;
    StatusCondition m_statusCondition;

    // 技能和效果
    QVector<Skill *> m_skills;
    FifthSkill *m_fifthSkill;
    QVector<TurnBasedEffect *> m_turnEffects;

    // 计算属性
    void calculateStats();
};

// TungTungTung（木棍人）
class TungTungTung : public Creature
{
public:
    TungTungTung(int level = 1);
    virtual void onTurnStart() override;
    virtual void onTurnEnd() override;
};

// BombardinoCrocodillo（鳄鱼轰炸机）
class BombardinoCrocodillo : public Creature
{
public:
    BombardinoCrocodillo(int level = 1);
    virtual void onTurnStart() override;
    virtual void onTurnEnd() override;
};

// TralaleroTralala（耐克鲨鱼）
class TralaleroTralala : public Creature
{
public:
    TralaleroTralala(int level = 1);
    virtual void onTurnStart() override;
    virtual void onTurnEnd() override;
};

// LiriliLarila（仙人掌大象）
class LiriliLarila : public Creature
{
public:
    LiriliLarila(int level = 1);
    virtual void onTurnStart() override;
    virtual void onTurnEnd() override;
};

// ChimpanziniBananini（香蕉绿猩猩）
class ChimpanziniBananini : public Creature
{
public:
    ChimpanziniBananini(int level = 1);
    void enterBerserkForm(int duration);
    void exitBerserkForm();
    virtual void onTurnStart() override;
    virtual void onTurnEnd() override;

private:
    bool m_inBerserkForm;
    int m_berserkFormDuration;
};

// Luguanluguanlulushijiandaole（鹿管鹿管鹿鹿时间到了）
class Luguanluguanlulushijiandaole : public Creature
{
public:
    Luguanluguanlulushijiandaole(int level = 1);
    void recordBattleState();
    bool tryRevertBattleState(BattleSystem *battle);
    virtual void onTurnStart() override;
    virtual void onTurnEnd() override;

private:
    int m_snapshotTurnsLeft;
};

#endif // CREATURE_H
