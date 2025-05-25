#ifndef SAVESYSTEM_H
#define SAVESYSTEM_H

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

#include "creature.h"
#include "gameengine.h"

class SaveSystem {
public:
    static SaveSystem* getInstance();
    
    // 保存游戏
    bool saveGame(const QString& saveName);
    
    // 载入游戏
    bool loadGame(const QString& saveName);
    
    // 获取所有可用存档
    QVector<QString> getAvailableSaves();
    
    // 删除存档
    bool deleteSave(const QString& saveName);
    
private:
    SaveSystem();
    ~SaveSystem();
    
    static SaveSystem* s_instance;
    
    // 获取存档路径
    QString getSavePath(const QString& saveName) const;
    
    // 将精灵转换为JSON
    QJsonObject creatureToJson(const Creature* creature) const;
    
    // 从JSON创建精灵
    Creature* createCreatureFromJson(const QJsonObject& json) const;
    
    // 获取游戏引擎实例
    GameEngine* getGameEngine() const;
};

#endif // SAVESYSTEM_H
