#include "gameengine.h"
#include "savesystem.h"
#include <QRandomGenerator>
#include <QDebug>

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
      m_battleSystem(nullptr),
      m_battlesWon(0),
      m_battlesLost(0) {
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

// 获取可用精灵列表
QVector<Creature*> GameEngine::getAvailableCreatures() const {
    return m_availableCreatures;
}

void GameEngine::addCreatureToPlayerTeam(Creature* creature) {
    if (creature) {
        m_playerTeam.append(creature);
        emit playerTeamChanged();
    }
}

// 添加可用精灵
void GameEngine::addAvailableCreature(Creature* creature) {
    if (creature) {
        m_availableCreatures.append(creature);
    }
}

void GameEngine::removeCreatureFromPlayerTeam(int index) {
    if (index >= 0 && index < m_playerTeam.size()) {
        delete m_playerTeam[index];
        m_playerTeam.removeAt(index);
        emit playerTeamChanged();
    }
}

// 移除可用精灵
void GameEngine::removeAvailableCreature(int index) {
    if (index >= 0 && index < m_availableCreatures.size()) {
        delete m_availableCreatures[index];
        m_availableCreatures.removeAt(index);
    }
}

// 清空可用精灵列表
void GameEngine::clearAvailableCreatures() {
    for (auto* creature : m_availableCreatures) {
        delete creature;
    }
    m_availableCreatures.clear();
}

// 清空玩家队伍
void GameEngine::clearPlayerTeam() {
    for (auto* creature : m_playerTeam) {
        delete creature;
    }
    m_playerTeam.clear();
}

// 获取战斗胜利次数
int GameEngine::getBattlesWon() const {
    return m_battlesWon;
}

// 获取战斗失败次数
int GameEngine::getBattlesLost() const {
    return m_battlesLost;
}

// 设置战斗胜利次数
void GameEngine::setBattlesWon(int value) {
    m_battlesWon = value;
}

// 设置战斗失败次数
void GameEngine::setBattlesLost(int value) {
    m_battlesLost = value;
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

// 创建精灵（指定类型）
Creature* GameEngine::createCreature(const QString& creatureName, const Type& type, int level) {
    // 创建新精灵实例
    Creature* creature = new Creature(creatureName, type, level);
    
    // 设置基础属性和技能
    // 注意：这里应该根据名称设置不同的基础属性和技能
    // 简单实现示例
    BaseStats baseStats;
    baseStats.setHp(50 + level * 5);
    baseStats.setAttack(40 + level * 3);
    baseStats.setDefense(40 + level * 3);
    baseStats.setSpecialAttack(40 + level * 3);
    baseStats.setSpecialDefense(40 + level * 3);
    baseStats.setSpeed(40 + level * 3);
    creature->setBaseStats(baseStats);
    
    // 返回创建的精灵
    return creature;
}

// 创建技能
Skill* GameEngine::createSkill(const QString& skillName, ElementType elementType, SkillCategory skillType,
                              int power, int accuracy, int maxPP) {
    // 根据技能类型创建不同的技能子类
    Skill* skill = nullptr;
    
    switch (skillType) {
        case SkillCategory::PHYSICAL:
            skill = new PhysicalSkill(skillName, elementType, power, accuracy, maxPP);
            break;
        case SkillCategory::SPECIAL:
            skill = new SpecialSkill(skillName, elementType, power, accuracy, maxPP);
            break;
        case SkillCategory::STATUS:
            skill = new StatusSkill(skillName, elementType, accuracy, maxPP);
            break;
    }
    
    return skill;
}

void GameEngine::startPvEBattle() {
    // 创建AI对手队伍
    int difficulty = 1; // 可以根据玩家进度或其他因素调整难度
    QVector<Creature*> aiTeam = createAITeam(difficulty);
    
    // 启动战斗
    startBattle(aiTeam, false);
}

void GameEngine::startPvPBattle() {
    // TODO: 实现PvP对战逻辑
    // 目前简单实现，使用AI作为对手
    QVector<Creature*> aiTeam = createAITeam(2, 3);
    
    // 启动PvP战斗
    startBattle(aiTeam, true);
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
