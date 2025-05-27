#include "savesystem.h"

#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

// 静态实例
SaveSystem *SaveSystem::s_instance = nullptr;

SaveSystem *SaveSystem::getInstance()
{
    if (!s_instance)
    {
        s_instance = new SaveSystem();
    }
    return s_instance;
}

SaveSystem::SaveSystem()
{
    // 确保存档目录存在
    QDir saveDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/saves");
    if (!saveDir.exists())
    {
        saveDir.mkpath(".");
    }
}

SaveSystem::~SaveSystem()
{
    // 清理资源
}

QString SaveSystem::getSavePath(const QString &saveName) const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) +
           "/saves/" +
           saveName +
           ".json";
}

GameEngine *SaveSystem::getGameEngine() const
{
    return GameEngine::getInstance();
}

bool SaveSystem::saveGame(const QString &saveName)
{
    if (saveName.isEmpty()) {
        qWarning() << "保存失败：存档名称为空";
        return false;
    }

    GameEngine *gameEngine = getGameEngine();
    if (!gameEngine) {
        qWarning() << "保存失败：无法获取游戏引擎实例";
        return false;
    }

    // 打印存档路径（调试）
    QString savePath = getSavePath(saveName);
    qDebug() << "正在尝试保存游戏到:" << savePath;

    // 确保存档目录存在
    QDir saveDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/saves");
    if (!saveDir.exists()) {
        bool dirCreated = saveDir.mkpath(".");
        qDebug() << "创建存档目录:" << saveDir.path() << (dirCreated ? "成功" : "失败");
        if (!dirCreated) {
            qWarning() << "无法创建存档目录";
            return false;
        }
    }

    // 创建主JSON对象
    QJsonObject saveObject;

    // 保存基本信息
    saveObject["saveVersion"] = "1.0";
    saveObject["saveDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    saveObject["saveName"] = saveName;

    // 保存玩家队伍
    QJsonArray playerTeamArray;
    const QVector<Creature *> &playerTeam = gameEngine->getPlayerTeam();
    for (const Creature *creature : playerTeam) {
        if (creature) {
            playerTeamArray.append(creatureToJson(creature));
        }
    }
    saveObject["playerTeam"] = playerTeamArray;

    // 保存可用精灵
    QJsonArray availableCreaturesArray;
    const QVector<Creature *> &availableCreatures = gameEngine->getAvailableCreatures();
    for (const Creature *creature : availableCreatures) {
        if (creature) {
            availableCreaturesArray.append(creatureToJson(creature));
        }
    }
    saveObject["availableCreatures"] = availableCreaturesArray;

    // 保存游戏进度和统计数据
    QJsonObject progressObject;
    progressObject["battlesWon"] = gameEngine->getBattlesWon();
    progressObject["battlesLost"] = gameEngine->getBattlesLost();
    saveObject["progress"] = progressObject;

    // 将JSON写入文件
    QFile saveFile(savePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开文件进行写入:" << savePath << "错误:" << saveFile.errorString();
        return false;
    }

    // 写入JSON数据
    QByteArray jsonData = QJsonDocument(saveObject).toJson();
    qint64 bytesWritten = saveFile.write(jsonData);
    saveFile.close();

    if (bytesWritten <= 0) {
        qWarning() << "写入存档文件失败，未写入任何数据";
        return false;
    }

    qDebug() << "游戏成功保存到:" << savePath << "大小:" << bytesWritten << "字节";
    return true;
}

bool SaveSystem::loadGame(const QString &saveName)
{
    GameEngine *gameEngine = getGameEngine();

    // 打开并读取存档文件
    QFile saveFile(getSavePath(saveName));
    if (!saveFile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray saveData = saveFile.readAll();
    saveFile.close();

    QJsonDocument document = QJsonDocument::fromJson(saveData);
    if (document.isNull() || !document.isObject())
    {
        return false;
    }

    QJsonObject saveObject = document.object();

    // 清空现有队伍
    gameEngine->clearPlayerTeam();

    // 加载玩家队伍
    if (saveObject.contains("playerTeam") && saveObject["playerTeam"].isArray())
    {
        QJsonArray playerTeamArray = saveObject["playerTeam"].toArray();
        for (const QJsonValue &creatureValue : playerTeamArray)
        {
            if (creatureValue.isObject())
            {
                Creature *creature = createCreatureFromJson(creatureValue.toObject());
                if (creature)
                {
                    gameEngine->addCreatureToPlayerTeam(creature);
                }
            }
        }
    }

    // 加载可用精灵
    if (saveObject.contains("availableCreatures") && saveObject["availableCreatures"].isArray())
    {
        // 先清空现有的可用精灵
        gameEngine->clearAvailableCreatures();

        QJsonArray availableCreaturesArray = saveObject["availableCreatures"].toArray();
        for (const QJsonValue &creatureValue : availableCreaturesArray)
        {
            if (creatureValue.isObject())
            {
                Creature *creature = createCreatureFromJson(creatureValue.toObject());
                if (creature)
                {
                    gameEngine->addAvailableCreature(creature);
                }
            }
        }
    }

    // 加载游戏进度和统计数据
    if (saveObject.contains("progress") && saveObject["progress"].isObject())
    {
        QJsonObject progressObject = saveObject["progress"].toObject();
        gameEngine->setBattlesWon(progressObject["battlesWon"].toInt());
        gameEngine->setBattlesLost(progressObject["battlesLost"].toInt());
    }

    return true;
}

QVector<QString> SaveSystem::getAvailableSaves()
{
    QVector<QString> saves;

    QDir saveDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/saves");
    QFileInfoList fileInfoList = saveDir.entryInfoList(QStringList("*.json"), QDir::Files, QDir::Time);

    for (const QFileInfo &fileInfo : fileInfoList)
    {
        // 从文件名中提取存档名称
        QString saveName = fileInfo.baseName();
        saves.append(saveName);
    }

    return saves;
}

bool SaveSystem::deleteSave(const QString &saveName)
{
    QFile saveFile(getSavePath(saveName));
    return saveFile.remove();
}

QJsonObject SaveSystem::creatureToJson(const Creature *creature) const
{
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
    for (int i = 0; i < creature->getSkillCount(); ++i)
    {
        Skill *skill = creature->getSkill(i);
        if (skill)
        {
            QJsonObject skillObject;
            skillObject["name"] = skill->getName();
            skillObject["elementType"] = static_cast<int>(skill->getType());
            skillObject["skillCategory"] = static_cast<int>(skill->getCategory());
            skillObject["power"] = skill->getPower();
            skillObject["accuracy"] = skill->getAccuracy();
            skillsArray.append(skillObject);
        }
    }
    creatureObject["skills"] = skillsArray;

    // 保存第五技能（如果有）
    Skill *fifthSkill = creature->getFifthSkill();
    if (fifthSkill)
    {
        QJsonObject skillObject;
        skillObject["name"] = fifthSkill->getName();
        skillObject["elementType"] = static_cast<int>(fifthSkill->getType());
        skillObject["skillCategory"] = static_cast<int>(fifthSkill->getCategory());
        skillObject["power"] = fifthSkill->getPower();
        skillObject["accuracy"] = fifthSkill->getAccuracy();
        creatureObject["fifthSkill"] = skillObject;
    }

    // 保存状态条件
    creatureObject["statusCondition"] = static_cast<int>(creature->getStatusCondition());

    return creatureObject;
}

Creature *SaveSystem::createCreatureFromJson(const QJsonObject &json) const
{
    GameEngine *gameEngine = getGameEngine();

    // 获取基本信息
    QString name = json["name"].toString();
    int level = json["level"].toInt(1);

    // 获取类型信息
    QJsonObject typeObject = json["type"].toObject();
    ElementType primaryType = static_cast<ElementType>(typeObject["primary"].toInt());
    ElementType secondaryType = static_cast<ElementType>(typeObject["secondary"].toInt());

    // 创建精灵
    Creature *creature = gameEngine->createCreature(name, Type(primaryType, secondaryType), level);
    if (!creature)
    {
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
    if (currentHP < creature->getMaxHP())
    {
        creature->takeDamage(creature->getMaxHP() - currentHP);
    }

    int maxPP = json["maxPP"].toInt();
    creature->setMaxPP(maxPP);

    int currentPP = json["currentPP"].toInt();
    if (currentPP < maxPP)
    {
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
    for (const QJsonValue &skillValue : skillsArray)
    {
        QJsonObject skillObject = skillValue.toObject();

        QString skillName = skillObject["name"].toString();
        ElementType elementType = static_cast<ElementType>(skillObject["elementType"].toInt());
        SkillCategory skillType = static_cast<SkillCategory>(skillObject["skillCategory"].toInt());
        int power = skillObject["power"].toInt();
        int accuracy = skillObject["accuracy"].toInt();

        Skill *skill = gameEngine->createSkill(skillName, elementType, skillType, power, accuracy, maxPP);
        if (skill)
        {
            creature->learnSkill(skill);
        }
    }

    // 加载第五技能（如果有）
    if (json.contains("fifthSkill"))
    {
        QJsonObject skillObject = json["fifthSkill"].toObject();

        QString skillName = skillObject["name"].toString();
        ElementType elementType = static_cast<ElementType>(skillObject["elementType"].toInt());
        SkillCategory skillType = static_cast<SkillCategory>(skillObject["skillCategory"].toInt());
        int power = skillObject["power"].toInt();
        int accuracy = skillObject["accuracy"].toInt();

        Skill *skill = gameEngine->createSkill(skillName, elementType, skillType, power, accuracy, maxPP);
        if (skill)
        {
            creature->setFifthSkill(skill);
        }
    }

    // 设置状态条件
    StatusCondition statusCondition = static_cast<StatusCondition>(json["statusCondition"].toInt());
    if (statusCondition != StatusCondition::NONE)
    {
        creature->setStatusCondition(statusCondition);
    }

    return creature;
}

void SaveSystem::checkSaveDirectory()
{
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/saves";
    QDir dir(dirPath);
    
    qDebug() << "============= 存档目录信息 =============";
    qDebug() << "路径:" << dirPath;
    qDebug() << "存在:" << dir.exists();
    
    // 如果目录不存在，尝试创建
    if (!dir.exists()) {
        bool created = dir.mkpath(".");
        qDebug() << "创建结果:" << (created ? "成功" : "失败");
    }
    
    // 检查可访问性
    QFileInfo dirInfo(dirPath);
    qDebug() << "可读:" << dirInfo.isReadable();
    qDebug() << "可写:" << dirInfo.isWritable();
    
    // 尝试写入测试文件
    QString testFilePath = dirPath + "/test_write.tmp";
    QFile testFile(testFilePath);
    bool canOpen = testFile.open(QIODevice::WriteOnly);
    qDebug() << "测试文件可创建:" << canOpen;
    
    if (canOpen) {
        qint64 bytesWritten = testFile.write("测试内容");
        testFile.close();
        qDebug() << "测试文件写入:" << (bytesWritten > 0 ? "成功" : "失败") << "(" << bytesWritten << "字节)";
        
        // 尝试删除测试文件
        bool removed = QFile::remove(testFilePath);
        qDebug() << "测试文件删除:" << (removed ? "成功" : "失败");
    }
    
    qDebug() << "========================================";
}