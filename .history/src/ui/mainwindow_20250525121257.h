#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "../core/gameengine.h"

// 前向声明
class BattleScene;
class PrepareScene;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 主菜单按钮响应
    void on_btnNewGame_clicked();
    void on_btnLoadGame_clicked();
    void on_btnSettings_clicked();
    void on_btnExit_clicked();
    
    // 游戏状态改变响应
    void onGameStateChanged(GameState state);
    
    // 其他响应
    void onBattleStarting();
    void onBattleEnded(BattleResult result);

private:
    Ui::MainWindow *ui;
    
    // 游戏引擎
    GameEngine* m_gameEngine;
    
    // 场景管理
    QStackedWidget* m_stackedWidget;
    QWidget* m_mainMenuWidget;
    BattleScene* m_battleScene;
    PrepareScene* m_prepareScene;
    
    // 保存游戏
    void saveGame();
    
    // 初始化UI
    void setupMainMenu();
    void setupScenes();
    
    // 切换场景
    void switchToMainMenu();
    void switchToBattleScene();
    void switchToPrepareScene();
    
    // 创建临时音效和背景音乐（简单实现，后续可扩展）
    void playMenuMusic();
    void playBattleMusic();
    void playSoundEffect(const QString& effect);
};

#endif // MAINWINDOW_H
