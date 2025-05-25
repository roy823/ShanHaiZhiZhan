#include "gameengine.h"

// 静态实例初始化
GameEngine* GameEngine::s_instance = nullptr;

GameEngine* GameEngine::getInstance() {
    if (!s_instance) {
        s_instance = new GameEngine();
    }
    return s_instance;
}

GameEngine::GameEngine(QObject* parent)
    : QObject(parent),
      m_gameState(GameState::MAIN_MENU),
      m_gameMode(GameMode::STORY_MODE),
      m_battleSystem(nullptr) {
}

GameEngine::~GameEngine() {
    cleanup();
}

void GameEngine::init() {
    // 创建战斗系统
    m_battleSystem = new BattleSystem(this);
    
    // 连接信号
    connect(m_battleSystem, &BattleSystem::battleEnded, this, &GameEngine::onBattleEnded);
    
    // 初始化精灵模板
    initCreatureTemplates();
    
    // 设置初始游戏状态
    setGameState(GameState::MAIN_MENU);
}

void GameEngine::cleanup() {
    // 释放玩家队伍
    releaseTeam(m_playerTeam);
    m_playerTeam.clear();
    
    // 释放精灵模板
    for (auto it = m_creatureTemplates.begin(); it != m_creatureTemplates.end(); ++it) {
        delete it.value();
    }
    m_creatureTemplates.clear();
    
    // 删除战斗系统
    if (m_battleSystem) {
        delete m_battleSystem;
        m_battleSystem = nullptr;
    }
}

GameState GameEngine::getGameState() const {
    return m_gameState;
}

void GameEngine::setGameState(GameState state) {
    if (m_gameState != state) {
        m_gameState = state;
        emit gameStateChanged(state);
    }
}

GameMode GameEngine::getGameMode() const {
    return m_gameMode;
}

void GameEngine::setGameMode(GameMode mode) {
    m_gameMode = mode;
}

BattleSystem* GameEngine::getBattleSystem() const {
    return m_battleSystem;
}

QVector<Creature*> GameEngine::getPlayerTeam() const {
    return m_playerTeam;
}

void GameEngine::addCreatureToPlayerTeam(Creature* creature) {
    if (creature) {
        m_playerTeam.append(creature);
        emit playerTeamChanged();
    }
}

void GameEngine::removeCreatureFromPlayerTeam(int index) {
    if (index >= 0 && index < m_playerTeam.size()) {
        delete m_playerTeam[index];
        m_playerTeam.removeAt(index);
        emit playerTeamChanged();
    }
}

void GameEngine::createNewGame() {
    // 清理现有资源
    releaseTeam(m_playerTeam);
    m_playerTeam.clear();
    
    // 为玩家创建一支入门队伍（例如，从三个初始精灵中选一个）
    Creature* starterCreature = createCreature("TungTungTung", 5);
    if (starterCreature) {
        addCreatureToPlayerTeam(starterCreature);
    }
    
    // 设置游戏状态为准备阶段
    setGameState(GameState::PREPARATION);
    
    // 发出新游戏创建信号
    emit newGameCreated();
}

bool GameEngine::loadGame(const QString& filename) {
    // 这里应该实现游戏加载逻辑
    // 简单起见，这里只是一个占位实现
    
    // 清理现有资源
    releaseTeam(m_playerTeam);
    m_playerTeam.clear();
    
    // 假设加载成功，创建一个测试队伍
    addCreatureToPlayerTeam(createCreature("TungTungTung", 10));
    addCreatureToPlayerTeam(createCreature("BombardinoCrocodillo", 9));
    addCreatureToPlayerTeam(createCreature("TralaleroTralala", 8));
    
    setGameState(GameState::PREPARATION);
    emit gameLoaded();
    return true;
}

bool GameEngine::saveGame(const QString& filename) {
    // 这里应该实现游戏保存逻辑
    // 简单起见，这里只是一个占位实现
    emit gameSaved();
    return true;
}

void GameEngine::startBattle(const QVector<Creature*>& opponentTeam, bool isPvP) {
    if (m_playerTeam.isEmpty()) {
        return; // 不能没有精灵就开始战斗
    }
    
    // 设置战斗状态
    setGameState(GameState::BATTLE);
    
    // 初始化战斗系统
    m_battleSystem->initBattle(m_playerTeam, opponentTeam, isPvP);
    
    // 发出战斗开始信号
    emit battleStarting();
}

void GameEngine::endBattle(BattleResult result) {
    // 处理战斗结果
    if (result == BattleResult::PLAYER_WIN) {
        // 玩家胜利，可能获得经验值、新精灵等奖励
    } else if (result == BattleResult::OPPONENT_WIN) {
        // 玩家失败，可能需要治疗精灵
    }
    
    // 返回准备界面
    setGameState(GameState::PREPARATION);
    
    // 发出战斗结束信号
    emit battleEnded(result);
}

Creature* GameEngine::createCreature(const QString& creatureName, int level) {
    // 从模板创建精灵
    if (m_creatureTemplates.contains(creatureName)) {
        Creature* template_ = m_creatureTemplates[creatureName];
        Creature* newCreature = nullptr;
        
        // 根据名称创建相应的精灵类型
        if (creatureName == "TungTungTung") {
            newCreature = new TungTungTung(level);
        } else if (creatureName == "BombardinoCrocodillo") {
            newCreature = new BombardinoCrocodillo(level);
        } else if (creatureName == "TralaleroTralala") {
            newCreature = new TralaleroTralala(level);
        } else if (creatureName == "LiriliLarila") {
            newCreature = new LiriliLarila(level);
        } else if (creatureName == "ChimpanziniBananini") {
            newCreature = new ChimpanziniBananini(level);
        } else if (creatureName == "Luguanluguanlulushijiandaole") {
            newCreature = new Luguanluguanlulushijiandaole(level);
        } else if (creatureName == "CappuccinoAssassino") {
            newCreature = new CappuccinoAssassino(level);
        }
        
        return newCreature;
    }
    
    return nullptr;
}

QVector<Creature*> GameEngine::createAITeam(int difficulty, int teamSize) {
    QVector<Creature*> aiTeam;
    
    // 根据难度创建AI队伍
    QVector<QString> availableCreatures = {"TungTungTung", "BombardinoCrocodillo", "TralaleroTralala", 
                                         "LiriliLarila", "ChimpanziniBananini", 
                                         "Luguanluguanlulushijiandaole", "CappuccinoAssassino"};
    
    // 计算AI精灵等级
    int minLevel = 1 + difficulty * 5;
    int maxLevel = minLevel + 5;
    
    // 随机选择精灵组成队伍
    for (int i = 0; i < teamSize && i < availableCreatures.size(); ++i) {
        int randomIndex = QRandomGenerator::global()->bounded(availableCreatures.size());
        int randomLevel = QRandomGenerator::global()->bounded(minLevel, maxLevel + 1);
        
        Creature* aiCreature = createCreature(availableCreatures[randomIndex], randomLevel);
        if (aiCreature) {
            aiTeam.append(aiCreature);
        }
        
        availableCreatures.removeAt(randomIndex);
    }
    
    return aiTeam;
}

void GameEngine::onBattleEnded(BattleResult result) {
    endBattle(result);
}

void GameEngine::initCreatureTemplates() {
    // 创建各种精灵模板
    m_creatureTemplates["TungTungTung"] = new TungTungTung(1);
    m_creatureTemplates["BombardinoCrocodillo"] = new BombardinoCrocodillo(1);
    m_creatureTemplates["TralaleroTralala"] = new TralaleroTralala(1);
    m_creatureTemplates["LiriliLarila"] = new LiriliLarila(1);
    m_creatureTemplates["ChimpanziniBananini"] = new ChimpanziniBananini(1);
    m_creatureTemplates["Luguanluguanlulushijiandaole"] = new Luguanluguanlulushijiandaole(1);
    m_creatureTemplates["CappuccinoAssassino"] = new CappuccinoAssassino(1);
}

void GameEngine::releaseTeam(QVector<Creature*>& team) {
    for (Creature* creature : team) {
        delete creature;
    }
    team.clear();
}
