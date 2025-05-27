// src/ui/mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget> // 用于管理多个界面的堆叠显示
#include "../core/gameengine.h" // 引入游戏引擎

// 前向声明，避免不必要的头文件包含
class BattleScene;
class PrepareScene;

// 如果使用Qt Designer的.ui文件，则需要包含
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT // 声明为QObject，以便使用信号和槽机制

public:
    // 构造函数，parent为父控件指针
    explicit MainWindow(QWidget *parent = nullptr);
    // 析构函数
    ~MainWindow();

private slots:
    // 主菜单按钮点击响应槽函数
    void onNewGameClicked();    // "新游戏"按钮
    void onLoadGameClicked();   // "载入游戏"按钮
    void onSettingsClicked(); // "设置"按钮
    void onExitClicked();     // "退出游戏"按钮

    // 游戏状态改变响应槽函数
    void onGameStateChanged(GameState newState); // 当游戏状态改变时调用

    // 战斗相关信号响应槽函数
    void onBattleStarting();                 // 战斗即将开始时调用
    void onBattleEndedSignal(BattleResult result); // 战斗结束时调用 (改名以避免潜在冲突)

    // (可选) 保存游戏槽函数，如果界面上有保存按钮
    // void onSaveGameClicked();

private:
    Ui::MainWindow *ui; // 指向Qt Designer生成的UI类实例，如果未使用.ui则为nullptr

    // 游戏引擎实例
    GameEngine* m_gameEngine;

    // 场景管理
    QStackedWidget* m_stackedWidget;  // 用于切换不同游戏场景 (主菜单, 准备, 战斗等)
    QWidget* m_mainMenuWidget;      // 主菜单界面
    BattleScene* m_battleScene;     // 战斗场景界面
    PrepareScene* m_prepareScene;   // 备战场景界面

    // 初始化UI相关的方法
    void setupMainMenu(); // 创建和设置主菜单界面
    void setupScenes();   // 创建和设置其他游戏场景 (如战斗、准备场景)

    // 切换场景的方法
    void switchToMainMenu();    // 切换到主菜单界面
    void switchToBattleScene(); // 切换到战斗场景
    void switchToPrepareScene();// 切换到准备场景

    // 播放音频的方法 (占位符，具体实现需要音频库)
    void playMenuMusic();     // 播放主菜单背景音乐
    void playBattleMusic();   // 播放战斗背景音乐
    void playSoundEffect(const QString& effectName); // 播放音效

    // 保存游戏的方法 (如果由MainWindow触发)
    void saveGame();
};

#endif // MAINWINDOW_H