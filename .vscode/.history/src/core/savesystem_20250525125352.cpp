#include "savesystem.h"

#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

// 静态实例
SaveSystem* SaveSystem::s_instance = nullptr;

SaveSystem* SaveSystem::getInstance() {
    if (!s_instance) {
        s_instance = new SaveSystem();
    }
    return s_instance;
}

SaveSystem::SaveSystem() {
    // 确保存档目录存在
    QDir saveDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/saves");
    if (!saveDir.exists()) {
        saveDir.mkpath(".");
    }
}

SaveSystem::~SaveSystem() {
    // 清理资源
}

QString SaveSystem::getSavePath(const QString& saveName) const {
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + 
           "/saves/" + 
           saveName + 
           ".json";
}

GameEngine* SaveSystem::getGameEngine() const {
    return GameEngine::getInstance();
}

bool SaveSystem::saveGame(const QString& saveName) {
    GameEngine* gameEngine = getGameEngine();
    
    // 创建主JSON对象
    QJsonObject saveObject;
    
    // 保存基本信息
    saveObject["saveVersion"] = "1.0";
    saveObject["saveDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    saveObject["saveName"] = saveName;
    
    // 保存玩家队伍
    QJsonArray playerTeamArray;
    const QVector<Creature*>& playerTeam = gameEngine->getPlayerTeam();
    for (const Creature* creature : playerTeam) {
        playerTeamArray.append(creatureToJson(creature));
    }
    saveObject["playerTeam"] = playerTeamArray;
    
    // 保存可用精灵
    QJsonArray availableCreaturesArray;
    const QVector<Creature*>& availableCreatures = gameEngine->getAvailableCreatures();
    for (const Creature* creature : availableCreatures) {
        availableCreaturesArray.append(creatureToJson(creature));
    }
    saveObject["availableCreatures"] = availableCreaturesArray;
    
    // 保存游戏进度和统计数据
    QJsonObject progressObject;
    progressObject["battlesWon"] = gameEngine->getBattlesWon();
    progressObject["battlesLost"] = gameEngine->getBattlesLost();
    saveObject["progress"] = progressObject;
    
    // 将JSON写入文件
    QFile saveFile(getSavePath(saveName));
    if (!saveFile.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    saveFile.write(QJsonDocument(saveObject).toJson());
    saveFile.close();
    
    return true;
}

bool SaveSystem::loadGame(const QString& saveName) {
    GameEngine* gameEngine = getGameEngine();
    
    // 打开并读取存档文件
    QFile saveFile(getSavePath(saveName));
    if (!saveFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray saveData = saveFile.readAll();
    saveFile.close();
    
    QJsonDocument document = QJsonDocument::fromJson(saveData);
    if (document.isNull() || !document.isObject()) {
        return false;
    }
    
    QJsonObject saveObject = document.object();
    
    // 清空现有队伍
    gameEngine->clearPlayerTeam();
    
    // 加载玩家队伍
    if (saveObject.contains("playerTeam") && saveObject["playerTeam"].isArray()) {
        QJsonArray playerTeamArray = saveObject["playerTeam"].toArray();
        for (const QJsonValue& creatureValue : playerTeamArray) {
            if (creatureValue.isObject()) {
                Creature* creature = createCreatureFromJson(creatureValue.toObject());
                if (creature) {
                    gameEngine->addCreatureToPlayerTeam(creature);
                }
            }
        }
    }
    
    // 加载可用精灵
    if (saveObject.contains("availableCreatures") && saveObject["availableCreatures"].isArray()) {
        // 先清空现有的可用精灵
        gameEngine->clearAvailableCreatures();
        
        QJsonArray availableCreaturesArray = saveObject["availableCreatures"].toArray();
        for (const QJsonValue& creatureValue : availableCreaturesArray) {
            if (creatureValue.isObject()) {
                Creature* creature = createCreatureFromJson(creatureValue.toObject());
                if (creature) {
                    gameEngine->addAvailableCreature(creature);
                }
            }
        }
    }
    
    // 加载游戏进度和统计数据
    if (saveObject.contains("progress") && saveObject["progress"].isObject()) {
        QJsonObject progressObject = saveObject["progress"].toObject();
        gameEngine->setBattlesWon(progressObject["battlesWon"].toInt());
        gameEngine->setBattlesLost(progressObject["battlesLost"].toInt());
    }
    
    return true;
}

QVector<QString> SaveSystem::getAvailableSaves() {
    QVector<QString> saves;
    
    QDir saveDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/saves");
    QFileInfoList fileInfoList = saveDir.entryInfoList(QStringList("*.json"), QDir::Files, QDir::Time);
    
    for (const QFileInfo& fileInfo : fileInfoList) {
        // 从文件名中提取存档名称
        QString saveName = fileInfo.baseName();
        saves.append(saveName);
    }
    
    return saves;
}

bool SaveSystem::deleteSave(const QString& saveName) {
    QFile saveFile(getSavePath(saveName));
    return saveFile.remove();
}

QJsonObject SaveSystem::creatureToJson(const Creature* creature) const {
    QJsonObject creatureObject;
    
    // 保存基本信息
    creatureObject["name"] = creature->getName();
    creatureObject["level"] = creature->getLevel();
    creatureObject["experience"] = creature->getExperience();
    
    // 保存类型信息
    QJsonObject typeObject;
    Type type = creature->getType();
    typeObject["primary"] = static_cast<int>(type.getPrimaryType());
    typeObject["secondary"] = static_cast<int>(type.getSecondaryType());
    creatureObject["type"] = typeObject;
      // 保存能力值
    QJsonObject statsObject;
    BaseStats baseStats = creature->getBaseStats();
    statsObject["hp"] = baseStats.hp();
    statsObject["attack"] = baseStats.attack();
    statsObject["defense"] = baseStats.defense();
    statsObject["specialAttack"] = baseStats.specialAttack();
    statsObject["specialDefense"] = baseStats.specialDefense();
    statsObject["speed"] = baseStats.speed();
    creatureObject["baseStats"] = statsObject;
    
    // 保存战斗属性
    creatureObject["currentHP"] = creature->getCurrentHP();
    creatureObject["maxHP"] = creature->getMaxHP();
    creatureObject["currentPP"] = creature->getCurrentPP();
    creatureObject["maxPP"] = creature->getMaxPP();
      // 保存天赋
    QJsonObject talentObject;
    Talent talent = creature->getTalent();
    talentObject["hpGrowth"] = talent.hpGrowth();
    talentObject["attackGrowth"] = talent.attackGrowth();
    talentObject["defenseGrowth"] = talent.defenseGrowth();
    talentObject["specialAttackGrowth"] = talent.specialAttackGrowth();
    talentObject["specialDefenseGrowth"] = talent.specialDefenseGrowth();
    talentObject["speedGrowth"] = talent.speedGrowth();
    creatureObject["talent"] = talentObject;
    
    // 保存技能
    QJsonArray skillsArray;
    for (int i = 0; i < creature->getSkillCount(); ++i) {
        Skill* skill = creature->getSkill(i);
        if (skill) {
            QJsonObject skillObject;
            skillObject["name"] = skill->getName();
            skillObject["elementType"] = static_cast<int>(skill->getType());
            skillObject["skillType"] = static_cast<int>(skill->getSkillType());
            skillObject["power"] = skill->getPower();
            skillObject["accuracy"] = skill->getAccuracy();
            skillObject["currentPP"] = skill->getCurrentPP();
            skillObject["maxPP"] = skill->getMaxPP();
            skillsArray.append(skillObject);
        }
    }
    creatureObject["skills"] = skillsArray;
    
    // 保存第五技能（如果有）
    Skill* fifthSkill = creature->getFifthSkill();
    if (fifthSkill) {
        QJsonObject skillObject;
        skillObject["name"] = fifthSkill->getName();
        skillObject["elementType"] = static_cast<int>(fifthSkill->getType());
        skillObject["skillCategory"] = static_cast<int>(fifthSkill->getCategory());
        skillObject["power"] = fifthSkill->getPower();
        skillObject["accuracy"] = fifthSkill->getAccuracy();
        skillObject["currentPP"] = fifthSkill->getCurrentPP();
        skillObject["maxPP"] = fifthSkill->getMaxPP();
        creatureObject["fifthSkill"] = skillObject;
    }
    
    // 保存状态条件
    creatureObject["statusCondition"] = static_cast<int>(creature->getStatusCondition());
    
    return creatureObject;
}

Creature* SaveSystem::createCreatureFromJson(const QJsonObject& json) const {
    GameEngine* gameEngine = getGameEngine();
    
    // 获取基本信息
    QString name = json["name"].toString();
    int level = json["level"].toInt(1);
    
    // 获取类型信息
    QJsonObject typeObject = json["type"].toObject();
    ElementType primaryType = static_cast<ElementType>(typeObject["primary"].toInt());
    ElementType secondaryType = static_cast<ElementType>(typeObject["secondary"].toInt());
    
    // 创建精灵
    Creature* creature = gameEngine->createCreature(name, Type(primaryType, secondaryType), level);
    if (!creature) {
        return nullptr;
    }
    
    // 设置经验值
    int experience = json["experience"].toInt();
    creature->gainExperience(experience);
      // 设置能力值
    QJsonObject statsObject = json["baseStats"].toObject();
    BaseStats baseStats;
    baseStats.setHp(statsObject["hp"].toInt());
    baseStats.setAttack(statsObject["attack"].toInt());
    baseStats.setDefense(statsObject["defense"].toInt());
    baseStats.setSpecialAttack(statsObject["specialAttack"].toInt());
    baseStats.setSpecialDefense(statsObject["specialDefense"].toInt());
    baseStats.setSpeed(statsObject["speed"].toInt());
    creature->setBaseStats(baseStats);
    
    // 设置战斗属性
    int currentHP = json["currentHP"].toInt();
    if (currentHP < creature->getMaxHP()) {
        creature->takeDamage(creature->getMaxHP() - currentHP);
    }
    
    int maxPP = json["maxPP"].toInt();
    creature->setMaxPP(maxPP);
    
    int currentPP = json["currentPP"].toInt();
    if (currentPP < maxPP) {
        creature->consumePP(maxPP - currentPP);
    }
      // 设置天赋
    QJsonObject talentObject = json["talent"].toObject();
    Talent talent;
    talent.setHpGrowth(talentObject["hpGrowth"].toInt());
    talent.setAttackGrowth(talentObject["attackGrowth"].toInt());
    talent.setDefenseGrowth(talentObject["defenseGrowth"].toInt());
    talent.setSpecialAttackGrowth(talentObject["specialAttackGrowth"].toInt());
    talent.setSpecialDefenseGrowth(talentObject["specialDefenseGrowth"].toInt());
    talent.setSpeedGrowth(talentObject["speedGrowth"].toInt());
    creature->setTalent(talent);
    
    // 加载技能
    QJsonArray skillsArray = json["skills"].toArray();
    for (const QJsonValue& skillValue : skillsArray) {
        QJsonObject skillObject = skillValue.toObject();
        
        QString skillName = skillObject["name"].toString();
        ElementType elementType = static_cast<ElementType>(skillObject["elementType"].toInt());
        SkillType skillType = static_cast<SkillType>(skillObject["skillType"].toInt());
        int power = skillObject["power"].toInt();
        int accuracy = skillObject["accuracy"].toInt();
        int currentPP = skillObject["currentPP"].toInt();
        int maxPP = skillObject["maxPP"].toInt();
        
        Skill* skill = gameEngine->createSkill(skillName, elementType, skillType, power, accuracy, maxPP);
        if (skill) {
            if (currentPP < maxPP) {
                skill->usePP(maxPP - currentPP);
            }
            creature->learnSkill(skill);
        }
    }
    
    // 加载第五技能（如果有）
    if (json.contains("fifthSkill")) {
        QJsonObject skillObject = json["fifthSkill"].toObject();
        
        QString skillName = skillObject["name"].toString();
        ElementType elementType = static_cast<ElementType>(skillObject["elementType"].toInt());
        SkillType skillType = static_cast<SkillType>(skillObject["skillType"].toInt());
        int power = skillObject["power"].toInt();
        int accuracy = skillObject["accuracy"].toInt();
        int currentPP = skillObject["currentPP"].toInt();
        int maxPP = skillObject["maxPP"].toInt();
        
        Skill* skill = gameEngine->createSkill(skillName, elementType, skillType, power, accuracy, maxPP);
        if (skill) {
            if (currentPP < maxPP) {
                skill->usePP(maxPP - currentPP);
            }
            creature->setFifthSkill(skill);
        }
    }
    
    // 设置状态条件
    StatusCondition statusCondition = static_cast<StatusCondition>(json["statusCondition"].toInt());
    if (statusCondition != StatusCondition::NONE) {
        creature->setStatusCondition(statusCondition);
    }
    
    return creature;
}
