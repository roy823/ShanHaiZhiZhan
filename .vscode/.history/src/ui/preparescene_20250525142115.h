// src/ui/preparescene.h
#ifndef PREPARESCENE_H
#define PREPARESCENE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
// #include <QGridLayout> // 如果用到更复杂的网格布局

#include "../core/gameengine.h" // 引入游戏引擎核心

// 前向声明
class Creature; // 精灵类

// 精灵详情显示部件 (用于显示选中精灵的详细信息)
class CreatureDetailWidget : public QWidget {
    Q_OBJECT

public:
    // 构造函数
    explicit CreatureDetailWidget(QWidget* parent = nullptr);

    // 更新显示的精灵信息
    void updateCreatureInfo(Creature* creature);

private:
    // UI元素指针
    QLabel* m_nameLabel;        // 精灵名称
    QLabel* m_typeLabel;        // 精灵属性 (类型)
    QLabel* m_levelLabel;       // 精灵等级
    QLabel* m_statsLabel;       // 精灵各项能力值 (HP, 攻击, 防御等)
    QListWidget* m_skillsList;  // 精灵技能列表

    // 初始化UI组件
    void setupUI();
};

// 备战场景类
class PrepareScene : public QWidget {
    Q_OBJECT

public:
    // 构造函数，传入游戏引擎实例和父控件
    explicit PrepareScene(GameEngine* gameEngine, QWidget *parent = nullptr);
    // 析构函数
    ~PrepareScene();

    // 公开的刷新场景接口 (例如从MainWindow切换过来时调用)
    void refreshScene();

private slots:
    // UI交互响应槽函数
    void onPlayerCreatureSelected(int index);       // 当玩家队伍列表中的精灵被选中时
    void onAvailableCreatureSelected(int index);    // 当可用精灵列表中的精灵被选中时
    void onAddCreatureClicked();                  // "添加到队伍"按钮点击
    void onRemoveCreatureClicked();               // "从队伍移除"按钮点击
    void onStartPvEBattleClicked();               // "开始PvE对战"按钮点击
    void onStartPvPBattleClicked();               // "开始PvP对战"按钮点击
    void onBackToMainMenuClicked();               // "返回主菜单"按钮点击

    // 游戏引擎信号响应槽函数
    void onPlayerTeamChanged(); // 当玩家的精灵队伍发生变化时 (例如添加或移除精灵)

private:
    // 游戏引擎实例指针
    GameEngine* m_gameEngine;

    // 当前在列表中选中的精灵的索引
    int m_selectedPlayerCreatureIndex;    // 玩家队伍列表中的选中索引
    int m_selectedAvailableCreatureIndex; // 可用精灵列表中的选中索引

    // UI组件指针
    QTabWidget* m_tabWidget; // 主选项卡控件 (队伍, 精灵库, 背包)

    // "队伍" 标签页相关组件
    QWidget* m_teamTab;                     // "队伍"标签页的Widget
    QListWidget* m_playerCreaturesList;     // 显示玩家当前队伍精灵的列表
    CreatureDetailWidget* m_playerCreatureDetail; // 显示选中队伍精灵详情的区域
    QPushButton* m_removeButton;            // 从队伍移除精灵的按钮
    QPushButton* m_startPvEButton;          // 开始PvE对战的按钮
    QPushButton* m_startPvPButton;          // 开始PvP对战的按钮

    // "精灵库" 标签页相关组件
    QWidget* m_creatureLibraryTab;                // "精灵库"标签页的Widget
    QListWidget* m_availableCreaturesList;        // 显示所有可用精灵的列表
    CreatureDetailWidget* m_availableCreatureDetail; // 显示选中可用精灵详情的区域
    QPushButton* m_addButton;                     // 添加精灵到队伍的按钮

    // "背包" 标签页相关组件
    QWidget* m_bagTab; // "背包"标签页的Widget (目前为占位)

    // 主要布局
    QVBoxLayout* m_mainLayout; // 备战场景的主垂直布局

    // 初始化UI界面元素
    void setupUI();

    // 更新UI列表显示
    void updatePlayerCreaturesList();    // 更新玩家队伍列表的显示内容
    void updateAvailableCreaturesList(); // 更新可用精灵列表的显示内容

    // 获取当前在列表中选中的精灵对象
    Creature* getSelectedPlayerCreature() const;       // 获取玩家队伍中选中的精灵
    Creature* getSelectedAvailableCreatureTemplate() const; // 获取可用精灵列表中选中的精灵模板

    // (已移除) 创建测试用对手队伍的方法，应由GameEngine处理
    // QVector<Creature*> createTestOpponentTeam();
};

#endif // PREPARESCENE_H