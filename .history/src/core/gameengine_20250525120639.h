#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>
#include "creature.h"
#include "../battle/battlesystem.h"

// 游戏模式
enum class GameMode {
    STORY_MODE,     // 故事模式
    PVP_MODE,       // PVP模式
    SANDBOX_MODE    // 沙盒模式
};

// 游戏状态
enum class GameState {
    MAIN_MENU,      // 主菜单
    PREPARATION,    // 备战界面
    BATTLE,         // 战斗中
    GAME_OVER       // 游戏结束
};

// 游戏引擎类
class GameEngine : public QObject {
    Q_OBJECT

public:
    static GameEngine* getInstance();
    
    // 初始化和清理
    void init();
    void cleanup();
    
    // 游戏状态控制
    GameState getGameState() const;
    void setGameState(GameState state);
    
    // 游戏模式控制
    GameMode getGameMode() const;
    void setGameMode(GameMode mode);
    
    // 获取战斗系统
    BattleSystem* getBattleSystem() const;
    
    // 获取玩家精灵队伍
    QVector<Creature*> getPlayerTeam() const;
    
    // 添加/移除精灵到玩家队伍
    void addCreatureToPlayerTeam(Creature* creature);
    void removeCreatureFromPlayerTeam(int index);
    
    // 创建新游戏
    void createNewGame();
    
    // 加载/保存游戏
    bool loadGame(const QString& filename);
    bool saveGame(const QString& filename);
    
    // 启动战斗
    void startBattle(const QVector<Creature*>& opponentTeam, bool isPvP = false);
    
    // 结束战斗
    void endBattle(BattleResult result);
    
    // 创建精灵实例
    Creature* createCreature(const QString& creatureName, int level = 1);
    
    // 创建精灵队伍（AI对手）
    QVector<Creature*> createAITeam(int difficulty, int teamSize = 3);

public slots:
    // 接收战斗结果
    void onBattleEnded(BattleResult result);

signals:
    // 游戏状态变化
    void gameStateChanged(GameState newState);
    
    // 玩家队伍变化
    void playerTeamChanged();
    
    // 游戏事件
    void newGameCreated();
    void gameLoaded();
    void gameSaved();
    
    // 战斗相关事件
    void battleStarting();
    void battleEnded(BattleResult result);
    
private:
    GameEngine(QObject* parent = nullptr);
    ~GameEngine();
    static GameEngine* s_instance;
    
    // 游戏状态
    GameState m_gameState;
    GameMode m_gameMode;
    
    // 战斗系统
    BattleSystem* m_battleSystem;
    
    // 玩家精灵队伍
    QVector<Creature*> m_playerTeam;
    
    // 所有可用的精灵模板
    QMap<QString, Creature*> m_creatureTemplates;
    
    // 初始化精灵模板
    void initCreatureTemplates();
    
    // 释放资源
    void releaseTeam(QVector<Creature*>& team);
};

#endif // GAMEENGINE_H
