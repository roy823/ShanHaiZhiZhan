#ifndef PREPARESCENE_H
#define PREPARESCENE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include "../core/gameengine.h"

// 精灵详情显示部件
class CreatureDetailWidget : public QWidget {
    Q_OBJECT
    
public:
    CreatureDetailWidget(QWidget* parent = nullptr);
    
    // 更新显示的精灵信息
    void updateCreatureInfo(Creature* creature);
    
private:
    QLabel* m_nameLabel;
    QLabel* m_typeLabel;
    QLabel* m_levelLabel;
    QLabel* m_statsLabel;
    QListWidget* m_skillsList;
    
    // 设置UI
    void setupUI();
};

// 准备场景类
class PrepareScene : public QWidget {
    Q_OBJECT

public:
    explicit PrepareScene(GameEngine* gameEngine, QWidget *parent = nullptr);
    ~PrepareScene();
    
    // 刷新场景
    void refreshScene();
    
private slots:
    // UI交互响应
    void onPlayerCreatureSelected(int index);
    void onAvailableCreatureSelected(int index);
    void onAddCreatureClicked();
    void onRemoveCreatureClicked();
    void onStartPvEBattleClicked();
    void onStartPvPBattleClicked();
    void onBackToMainMenuClicked();
    
    // 游戏引擎响应
    void onPlayerTeamChanged();
    
private:
    // 游戏引擎
    GameEngine* m_gameEngine;
    
    // 当前选中的精灵索引
    int m_selectedPlayerCreatureIndex;
    int m_selectedAvailableCreatureIndex;
    
    // UI组件
    QTabWidget* m_tabWidget;
    
    // 队伍界面
    QWidget* m_teamTab;
    QListWidget* m_playerCreaturesList;
    CreatureDetailWidget* m_playerCreatureDetail;
    QPushButton* m_removeButton;
    QPushButton* m_startPvEButton;
    QPushButton* m_startPvPButton;
    
    // 精灵库界面
    QWidget* m_creatureLibraryTab;
    QListWidget* m_availableCreaturesList;
    CreatureDetailWidget* m_availableCreatureDetail;
    QPushButton* m_addButton;
    
    // 背包界面
    QWidget* m_bagTab;
    
    // 主要布局
    QVBoxLayout* m_mainLayout;
    
    // 设置UI
    void setupUI();
    
    // 更新UI
    void updatePlayerCreaturesList();
    void updateAvailableCreaturesList();
    
    // 获取当前选中的精灵
    Creature* getSelectedPlayerCreature() const;
    Creature* getSelectedAvailableCreature() const;
    
    // 创建测试用对手队伍
    QVector<Creature*> createTestOpponentTeam();
};

#endif // PREPARESCENE_H
