// src/ui/mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h" // Qt Designer 生成的UI类，如果未使用.ui文件，则不需要
#include "battlescene.h"    // 战斗场景
#include "preparescene.h"   // 准备场景
#include "loadgamedialog.h" // 加载游戏对话框
#include "savegamedialog.h" // 保存游戏对话框 (如果MainWindow提供保存入口)

#include <QMessageBox>    // 用于显示消息框
#include <QStackedWidget> // 用于管理不同场景的堆叠显示
#include <QVBoxLayout>    // 垂直布局
#include <QPushButton>    // 按钮
#include <QLabel>         // 标签
// #include <QSoundEffect> // 如果要使用音效

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(nullptr), // 如果未使用ui文件，则初始化为nullptr
    m_gameEngine(nullptr),
    m_stackedWidget(nullptr),
    m_mainMenuWidget(nullptr),
    m_battleScene(nullptr),
    m_prepareScene(nullptr) {

    // ui->setupUi(this); // 如果使用ui文件，则取消注释

    // 设置窗口属性
    setWindowTitle("山海之战"); // 窗口标题
    setMinimumSize(800, 600);  // 最小窗口尺寸

    // 获取游戏引擎单例
    m_gameEngine = GameEngine::getInstance();
    m_gameEngine->init(); // 初始化游戏引擎

    // 连接游戏引擎的信号到MainWindow的槽
    connect(m_gameEngine, &GameEngine::gameStateChanged, this, &MainWindow::onGameStateChanged); // 游戏状态改变
    connect(m_gameEngine, &GameEngine::battleStarting, this, &MainWindow::onBattleStarting);     // 战斗即将开始
    connect(m_gameEngine, &GameEngine::battleEnded, this, &MainWindow::onBattleEndedSignal);       // 战斗结束 (改名以避免与槽函数重名)
    connect(m_gameEngine, &GameEngine::returnToMainMenu, this, &MainWindow::switchToMainMenu); // 返回主菜单信号

    // 设置UI界面 (主菜单和各场景)
    setupMainMenu();  // 创建主菜单界面
    setupScenes();    // 创建并添加其他场景 (战斗、准备)

    // 初始显示主菜单
    switchToMainMenu();

    // 播放主菜单音乐 (如果实现)
    playMenuMusic();
}

MainWindow::~MainWindow() {
    // delete ui; // 如果使用ui文件

    // 游戏引擎的清理应由其自身管理或在main.cpp中 QApplication 退出前进行
    // if (m_gameEngine) {
    //     m_gameEngine->cleanup(); // GameEngine是单例，通常不在这里delete
    // }
    // QStackedWidget及其中的子Widget会自动被父窗口析构时删除
}

void MainWindow::setupMainMenu() {
    // 创建主菜单界面Widget
    m_mainMenuWidget = new QWidget(this); // 父对象为MainWindow
    QVBoxLayout* layout = new QVBoxLayout(m_mainMenuWidget); // 主菜单使用垂直布局

    // 添加标题
    QLabel* titleLabel = new QLabel("山海之战", m_mainMenuWidget);
    titleLabel->setAlignment(Qt::AlignCenter); // 居中对齐
    titleLabel->setStyleSheet("font-size: 48px; font-weight: bold; margin-bottom: 30px; color: #333;"); // 标题样式
    layout->addWidget(titleLabel);
    layout->addStretch(1); // 添加弹性空间，将标题推向上方

    // 创建按钮
    QPushButton* newGameBtn = new QPushButton("新游戏", m_mainMenuWidget);
    QPushButton* loadGameBtn = new QPushButton("载入游戏", m_mainMenuWidget);
    QPushButton* settingsBtn = new QPushButton("设置", m_mainMenuWidget); // 设置按钮
    QPushButton* exitBtn = new QPushButton("退出游戏", m_mainMenuWidget);

    // 设置按钮统一样式
    QString btnStyle = "QPushButton { font-size: 20px; padding: 12px 25px; min-width: 220px; margin: 10px; border-radius: 8px; background-color: #4CAF50; color: white; } QPushButton:hover { background-color: #45a049; } QPushButton:pressed { background-color: #3e8e41; }";
    newGameBtn->setStyleSheet(btnStyle);
    loadGameBtn->setStyleSheet(btnStyle);
    settingsBtn->setStyleSheet(btnStyle);
    // 为退出按钮设置不同颜色
    exitBtn->setStyleSheet("QPushButton { font-size: 20px; padding: 12px 25px; min-width: 220px; margin: 10px; border-radius: 8px; background-color: #f44336; color: white; } QPushButton:hover { background-color: #e53935; } QPushButton:pressed { background-color: #d32f2f; }");


    // 将按钮添加到布局，并使用弹性空间使其居中
    layout->addWidget(newGameBtn, 0, Qt::AlignCenter);
    layout->addWidget(loadGameBtn, 0, Qt::AlignCenter);
    layout->addWidget(settingsBtn, 0, Qt::AlignCenter);
    layout->addWidget(exitBtn, 0, Qt::AlignCenter);
    layout->addStretch(2); // 添加更多弹性空间，将按钮组推向中心

    // 连接按钮的点击信号到相应的槽函数
    connect(newGameBtn, &QPushButton::clicked, this, &MainWindow::onNewGameClicked);
    connect(loadGameBtn, &QPushButton::clicked, this, &MainWindow::onLoadGameClicked);
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(exitBtn, &QPushButton::clicked, this, &MainWindow::onExitClicked);

    // 为主菜单Widget设置布局
    m_mainMenuWidget->setLayout(layout);
}

void MainWindow::setupScenes() {
    // 创建QStackedWidget用于管理不同的游戏场景
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget); // 将stackedWidget设置为主窗口的中央控件

    // 将主菜单Widget添加到stackedWidget中
    if (m_mainMenuWidget) {
        m_stackedWidget->addWidget(m_mainMenuWidget);
    }

    // 创建战斗场景并添加到stackedWidget
    m_battleScene = new BattleScene(m_gameEngine, this); // 父对象为MainWindow
    m_stackedWidget->addWidget(m_battleScene);

    // 创建准备场景并添加到stackedWidget
    m_prepareScene = new PrepareScene(m_gameEngine, this); // 父对象为MainWindow
    m_stackedWidget->addWidget(m_prepareScene);
}

void MainWindow::switchToMainMenu() {
    if (m_stackedWidget && m_mainMenuWidget) {
        m_stackedWidget->setCurrentWidget(m_mainMenuWidget); // 切换到主菜单界面
        playMenuMusic(); // 播放主菜单音乐
    }
}

void MainWindow::switchToBattleScene() {
    if (m_stackedWidget && m_battleScene) {
        m_stackedWidget->setCurrentWidget(m_battleScene); // 切换到战斗场景
        m_battleScene->initScene(); // 初始化战斗场景的显示内容
        playBattleMusic(); // 播放战斗音乐
    }
}

void MainWindow::switchToPrepareScene() {
    if (m_stackedWidget && m_prepareScene) {
        m_stackedWidget->setCurrentWidget(m_prepareScene); // 切换到准备场景
        m_prepareScene->refreshScene(); // 刷新准备场景的内容
        playMenuMusic(); // 通常准备界面使用主菜单或舒缓音乐
    }
}

void MainWindow::playMenuMusic() {
    // TODO: 实现播放主菜单背景音乐的功能
    // 例如: QSoundEffect* menuMusic = new QSoundEffect(this);
    // menuMusic->setSource(QUrl::fromLocalFile(":/sounds/menu_music.wav"));
    // menuMusic->setLoopCount(QSoundEffect::Infinite); menuMusic->play();
    qDebug() << "播放主菜单音乐 (占位符)";
}

void MainWindow::playBattleMusic() {
    // TODO: 实现播放战斗背景音乐的功能
    qDebug() << "播放战斗音乐 (占位符)";
}

void MainWindow::playSoundEffect(const QString& effectName) {
    // TODO: 实现播放特定音效的功能
    // 例如: QSoundEffect* sfx = new QSoundEffect(this);
    // sfx->setSource(QUrl::fromLocalFile(":/sounds/" + effectName + ".wav"));
    // sfx->play();
    qDebug() << "播放音效: " << effectName << " (占位符)";
}

// Slots implementation
void MainWindow::onNewGameClicked() {
    // 提示玩家新游戏会覆盖未保存的进度 (如果适用)
    // QMessageBox::StandardButton reply = QMessageBox::question(this, "新游戏",
    //                                         "开始新游戏会丢失未保存的进度，确定吗?",
    //                                         QMessageBox::Yes | QMessageBox::No);
    // if (reply == QMessageBox::No) {
    //     return;
    // }

    m_gameEngine->createNewGame(); // 通知游戏引擎创建新游戏

    // 显示游戏说明 (根据设计文档)
    QMessageBox::information(this, "游戏说明", "欢迎来到山海之战！这是一个回合制对战游戏，玩家可以选择已有的几只精灵，与电脑或其他玩家进行对战。");

    // 切换到准备场景 (通过改变游戏状态触发)
    m_gameEngine->setGameState(GameState::PREPARATION);
}

void MainWindow::onLoadGameClicked() {
    LoadGameDialog dialog(this); // 创建并显示加载游戏对话框
    if (dialog.exec() == QDialog::Accepted) { // 如果用户点击了"载入"
        QString saveName = dialog.getSelectedSave(); // 获取选择的存档名称
        if (!saveName.isEmpty()) {
            if (m_gameEngine->loadGame(saveName)) { // 尝试加载游戏
                // 加载成功后，通常进入准备场景或上次保存的状态
                m_gameEngine->setGameState(GameState::PREPARATION); // 假设加载后进入准备场景
                QMessageBox::information(this, "载入成功", QString("存档 '%1' 已成功载入。").arg(saveName));
            } else {
                QMessageBox::critical(this, "载入失败", QString("无法载入存档 '%1'。文件可能已损坏或不存在。").arg(saveName));
            }
        }
    }
}

void MainWindow::onSettingsClicked() {
    // TODO: 实现设置功能，例如音量调整、显示设置等
    QMessageBox::information(this, "设置", "设置功能正在开发中，敬请期待！");
}

void MainWindow::onExitClicked() {
    // 弹出确认退出对话框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认退出", "确定要退出山海之战吗?",
                                 QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QApplication::quit(); // 退出应用程序
    }
}

// 如果有保存游戏的按钮或菜单项，可以实现此槽函数
void MainWindow::saveGame() {
    // 检查当前是否适合保存 (例如不在战斗中)
    if (m_gameEngine->getGameState() == GameState::BATTLE) {
        QMessageBox::warning(this, "无法保存", "战斗中无法保存游戏。");
        return;
    }

    SaveGameDialog dialog(this); // 创建保存游戏对话框
    if (dialog.exec() == QDialog::Accepted) {
        QString saveName = dialog.getSaveName(); // 获取用户输入的存档名
        if (!saveName.isEmpty()) {
            if (m_gameEngine->saveGame(saveName)) {
                QMessageBox::information(this, "保存成功", QString("游戏已成功保存为 '%1'。").arg(saveName));
            } else {
                QMessageBox::critical(this, "保存失败", "保存游戏时发生错误。");
            }
        } else {
            QMessageBox::warning(this, "无效名称", "存档名称不能为空。");
        }
    }
}

void MainWindow::onGameStateChanged(GameState newState) {
    // 根据新的游戏状态切换到相应的场景
    switch (newState) {
        case GameState::MAIN_MENU:
            switchToMainMenu();
            break;
        case GameState::PREPARATION:
            switchToPrepareScene();
            break;
        case GameState::BATTLE:
            // 战斗开始通常由 onBattleStarting 触发场景切换
            // 但如果直接设置状态到BATTLE，也应切换
            switchToBattleScene();
            break;
        case GameState::GAME_OVER:
            // 显示游戏结束信息，并返回主菜单
            QMessageBox::information(this, "游戏结束", "本次游戏已结束。感谢您的游玩！");
            switchToMainMenu(); // 或者显示一个专门的游戏结束画面
            break;
        default:
            qWarning() << "未知的游戏状态：" << static_cast<int>(newState);
            break;
    }
}

void MainWindow::onBattleStarting() {
    // 游戏引擎发来战斗开始信号，切换到战斗场景
    // GameState 应该已经由 GameEngine 设置为 BATTLE
    if (m_gameEngine->getGameState() == GameState::BATTLE) {
         switchToBattleScene();
    } else {
        // 如果状态不是BATTLE，这可能是一个逻辑问题，或者状态设置延迟
        qWarning("MainWindow::onBattleStarting called but GameState is not BATTLE.");
        m_gameEngine->setGameState(GameState::BATTLE); // 确保状态正确
        // switchToBattleScene(); // setGameState 会触发 onGameStateChanged，进而调用 switchToBattleScene
    }
}

void MainWindow::onBattleEndedSignal(BattleResult result) { // Renamed to avoid conflict
    // 显示战斗结果信息框
    QString resultMessage;
    switch (result) {
        case BattleResult::PLAYER_WIN:
            resultMessage = "恭喜你，获得了战斗的胜利！";
            playSoundEffect("victory_fanfare"); // 播放胜利音效
            break;
        case BattleResult::OPPONENT_WIN:
            resultMessage = "很遗憾，战斗失败了。再接再厉！";
            playSoundEffect("defeat_tune"); // 播放失败音效
            break;
        case BattleResult::DRAW:
            resultMessage = "战斗以平局结束。";
            break;
        case BattleResult::ESCAPE:
            resultMessage = "你成功地从战斗中逃脱了！";
            break;
        default:
            resultMessage = "战斗结束了。";
    }
    QMessageBox::information(this, "战斗结果", resultMessage);

    // 战斗结束后，游戏状态通常会返回到准备阶段或地图界面
    // GameEngine 应该负责设置正确的后续状态
    // m_gameEngine->setGameState(GameState::PREPARATION); // 这会由GameEngine在内部逻辑处理后发出gameStateChanged信号
}