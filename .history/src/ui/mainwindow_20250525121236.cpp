#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "battlescene.h"
#include "preparescene.h"
#include "loadgamedialog.h"
#include "savegamedialog.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QSound>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_gameEngine(nullptr),
    m_stackedWidget(nullptr),
    m_mainMenuWidget(nullptr),
    m_battleScene(nullptr),
    m_prepareScene(nullptr) {
    
    ui->setupUi(this);
    
    // 设置窗口属性
    setWindowTitle("山海之战");
    setMinimumSize(800, 600);
    
    // 获取游戏引擎实例
    m_gameEngine = GameEngine::getInstance();
    m_gameEngine->init();
    
    // 连接信号
    connect(m_gameEngine, &GameEngine::gameStateChanged, this, &MainWindow::onGameStateChanged);
    connect(m_gameEngine, &GameEngine::battleStarting, this, &MainWindow::onBattleStarting);
    connect(m_gameEngine, &GameEngine::battleEnded, this, &MainWindow::onBattleEnded);
    
    // 设置UI
    setupMainMenu();
    setupScenes();
    
    // 显示主菜单
    switchToMainMenu();
    
    // 播放主菜单音乐
    playMenuMusic();
}

MainWindow::~MainWindow() {
    delete ui;
    
    // 清理资源
    if (m_gameEngine) {
        m_gameEngine->cleanup();
    }
}

void MainWindow::setupMainMenu() {
    // 创建主菜单界面
    m_mainMenuWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(m_mainMenuWidget);
    
    // 添加标题
    QLabel* titleLabel = new QLabel("山海之战", m_mainMenuWidget);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin: 20px;");
    layout->addWidget(titleLabel);
    
    // 添加按钮
    QPushButton* newGameBtn = new QPushButton("新游戏", m_mainMenuWidget);
    QPushButton* loadGameBtn = new QPushButton("载入游戏", m_mainMenuWidget);
    QPushButton* settingsBtn = new QPushButton("设置", m_mainMenuWidget);
    QPushButton* exitBtn = new QPushButton("退出游戏", m_mainMenuWidget);
    
    // 设置按钮样式
    QString btnStyle = "QPushButton { font-size: 18px; padding: 10px; min-width: 200px; margin: 5px; }";
    newGameBtn->setStyleSheet(btnStyle);
    loadGameBtn->setStyleSheet(btnStyle);
    settingsBtn->setStyleSheet(btnStyle);
    exitBtn->setStyleSheet(btnStyle);
    
    // 添加到布局
    layout->addWidget(newGameBtn);
    layout->addWidget(loadGameBtn);
    layout->addWidget(settingsBtn);
    layout->addWidget(exitBtn);
    layout->addStretch();
    
    // 连接按钮信号
    connect(newGameBtn, &QPushButton::clicked, this, &MainWindow::on_btnNewGame_clicked);
    connect(loadGameBtn, &QPushButton::clicked, this, &MainWindow::on_btnLoadGame_clicked);
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::on_btnSettings_clicked);
    connect(exitBtn, &QPushButton::clicked, this, &MainWindow::on_btnExit_clicked);
    
    // 设置布局
    m_mainMenuWidget->setLayout(layout);
}

void MainWindow::setupScenes() {
    // 创建场景堆栈
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);
    
    // 添加主菜单场景
    m_stackedWidget->addWidget(m_mainMenuWidget);
    
    // 创建战斗场景
    m_battleScene = new BattleScene(m_gameEngine, this);
    m_stackedWidget->addWidget(m_battleScene);
    
    // 创建准备场景
    m_prepareScene = new PrepareScene(m_gameEngine, this);
    m_stackedWidget->addWidget(m_prepareScene);
}

void MainWindow::switchToMainMenu() {
    m_stackedWidget->setCurrentWidget(m_mainMenuWidget);
}

void MainWindow::switchToBattleScene() {
    m_stackedWidget->setCurrentWidget(m_battleScene);
    m_battleScene->initScene();
}

void MainWindow::switchToPrepareScene() {
    m_stackedWidget->setCurrentWidget(m_prepareScene);
    m_prepareScene->refreshScene();
}

void MainWindow::playMenuMusic() {
    // 实现播放背景音乐的功能
    // 这里使用了QSound，实际项目可能会使用其他音频库
    // QSound::play(":/sounds/menu_music.wav");
}

void MainWindow::playBattleMusic() {
    // 实现播放战斗音乐的功能
    // QSound::play(":/sounds/battle_music.wav");
}

void MainWindow::playSoundEffect(const QString& effect) {
    // 实现播放音效的功能
    // QSound::play(":/sounds/" + effect + ".wav");
}

void MainWindow::on_btnNewGame_clicked() {
    // 创建新游戏
    m_gameEngine->createNewGame();
    
    // 显示游戏说明
    QMessageBox::information(this, "游戏说明", "欢迎来到山海之战！这是一个回合制对战游戏，玩家可以选择已有的几只精灵，与电脑或其他玩家进行对战。");
    
    // 切换到准备场景
    m_gameEngine->setGameState(GameState::PREPARATION);
}

void MainWindow::on_btnLoadGame_clicked() {
    // 显示加载对话框
    LoadGameDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString saveName = dialog.getSelectedSave();
        if (!saveName.isEmpty()) {
            // 加载存档
            if (m_gameEngine->loadGame(saveName)) {
                // 切换到准备场景
                m_gameEngine->setGameState(GameState::PREPARATION);
            } else {
                QMessageBox::critical(this, "错误", "无法加载存档");
            }
        }
    }
}

void MainWindow::on_btnSettings_clicked() {
    // 该功能尚未实现
    QMessageBox::information(this, "设置", "设置功能即将推出...");
}

void MainWindow::on_btnExit_clicked() {
    // 确认退出
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认退出", "确定要退出游戏吗?",
                                 QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // 退出程序
        QApplication::quit();
    }
}

void MainWindow::saveGame() {
    // 显示保存对话框
    SaveGameDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString saveName = dialog.getSaveName();
        if (!saveName.isEmpty()) {
            // 保存存档
            if (m_gameEngine->saveGame(saveName)) {
                QMessageBox::information(this, "成功", "游戏已成功保存");
            } else {
                QMessageBox::critical(this, "错误", "无法保存游戏");
            }
        }
    }
}

void MainWindow::onGameStateChanged(GameState state) {
    // 根据游戏状态切换场景
    switch (state) {
        case GameState::MAIN_MENU:
            switchToMainMenu();
            playMenuMusic();
            break;
            
        case GameState::PREPARATION:
            switchToPrepareScene();
            playMenuMusic();
            break;
            
        case GameState::BATTLE:
            switchToBattleScene();
            playBattleMusic();
            break;
            
        case GameState::GAME_OVER:
            // 显示游戏结束界面
            QMessageBox::information(this, "游戏结束", "感谢您的游玩！");
            switchToMainMenu();
            break;
    }
}

void MainWindow::onBattleStarting() {
    // 战斗即将开始
    playSoundEffect("battle_start");
}

void MainWindow::onBattleEnded(BattleResult result) {
    // 处理战斗结束
    QString resultMessage;
    
    switch (result) {
        case BattleResult::PLAYER_WIN:
            resultMessage = "恭喜！你赢得了战斗！";
            playSoundEffect("victory");
            break;
            
        case BattleResult::OPPONENT_WIN:
            resultMessage = "很遗憾，你输掉了战斗。";
            playSoundEffect("defeat");
            break;
            
        case BattleResult::DRAW:
            resultMessage = "战斗以平局结束。";
            playSoundEffect("draw");
            break;
            
        case BattleResult::ESCAPE:
            resultMessage = "你成功逃离了战斗。";
            playSoundEffect("escape");
            break;
            
        default:
            resultMessage = "战斗结束。";
            break;
    }
    
    QMessageBox::information(this, "战斗结果", resultMessage);
}
