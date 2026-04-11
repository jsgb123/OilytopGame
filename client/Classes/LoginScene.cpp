#include "LoginScene.h"
#include "GameScene.h"
#include "GameConfig.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;
using namespace cocos2d::ui;

Scene* LoginScene::createScene()
{
    auto scene = Scene::create();
    auto layer = LoginScene::create();
    scene->addChild(layer);
    return scene;
}

bool LoginScene::init()
{
    if (!Scene::init())
        return false;
    
    // 初始化变量
    _network = WebSocketClient::getInstance();
    _isConnecting = false;
    _isSwitchingScene = false;
    _loginAttempts = 0;
    
    // 设置UI
    setupUI();
    
    // 设置网络回调
    _network->setOnConnected([this]() { onConnected(); });
    _network->setOnLoginResponse([this](LoginResponse* resp) { 
        if (resp->success)
            onLoginSuccess(resp);
        else
            onLoginFailed(resp->message);
    });
    _network->setOnDisconnected([this](const std::string& reason) { onDisconnected(reason); });
    _network->setOnError([this](const std::string& error) { onError(error); });
    
    // 设置默认值
    _serverAddressInput->setString("localhost:8080");
    _playerNameInput->setString(StringUtils::format(u8"玩家%d", rand() % 9000 + 1000));
    _passwordInput->setString("123456");
    
    updateUI();
    
    CCLOG(u8"LoginScene 初始化完成");
    return true;
}

void LoginScene::setupUI()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 深色背景色
    auto bgLayer = LayerColor::create(Color4B(20, 30, 40, 255));
    this->addChild(bgLayer);
    
    // 标题
    auto title = Label::createWithSystemFont("OilyTop", GameConfig::FONT_ARIAL, 48);
    title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 70));
    title->setColor(Color3B(200, 100, 50));
    this->addChild(title);
    
    // 副标题
    auto subtitle = Label::createWithSystemFont(u8"欢迎来到...", GameConfig::FONT_KAITI, 22);
    subtitle->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 120));
    subtitle->setColor(Color3B(150, 150, 150));
    this->addChild(subtitle);
    
    // 登录面板背景
    auto panelBg = LayerColor::create(Color4B(10, 10, 10, 180));
    panelBg->setContentSize(Size(400, 330));
    panelBg->setPosition(Vec2(visibleSize.width / 2 - 200, visibleSize.height / 2 - 175));
    this->addChild(panelBg);

	auto logo = Sprite::create("logo.png");
	if (logo)
	{
		// 设置图片位置（标题下方）
		logo->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 240));
		// 设置缩放比例
		logo->setScale(0.5f);
		this->addChild(logo);
	}
	else
	{
		CCLOG("Error: Failed to load logo.png");
	}

	// 状态标签
	_statusLabel = Label::createWithSystemFont(u8"等待连接", GameConfig::FONT_MSYAH, 14);
	_statusLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 40));
	_statusLabel->setColor(Color3B::GRAY);
	this->addChild(_statusLabel);

    // 服务器地址标签
    auto serverLabel = Label::createWithSystemFont(u8"服务器地址:", GameConfig::FONT_SIMHEI, 18);
    serverLabel->setPosition(Vec2(visibleSize.width / 2 - 135, visibleSize.height / 2 - 70));
    serverLabel->setColor(Color3B::WHITE);
    this->addChild(serverLabel);
    
    // 服务器地址输入框
    _serverAddressInput = TextField::create("localhost:8080", GameConfig::FONT_ARIAL, 18);
    _serverAddressInput->setPosition(Vec2(visibleSize.width / 2 + 20, visibleSize.height / 2 - 70));
    _serverAddressInput->setContentSize(Size(200, 30));
    _serverAddressInput->setMaxLength(50);
    _serverAddressInput->setPlaceHolderColor(Color3B::GRAY);
    this->addChild(_serverAddressInput);
    
    // 玩家名称标签
    auto nameLabel = Label::createWithSystemFont(u8"玩家名称:", GameConfig::FONT_MSYAH, 18);
    nameLabel->setPosition(Vec2(visibleSize.width / 2 - 135, visibleSize.height / 2 - 110));
    nameLabel->setColor(Color3B::WHITE);
    this->addChild(nameLabel);
    
    // 玩家名称输入框
    _playerNameInput = TextField::create(u8"玩家", GameConfig::FONT_SONG, 18);
    _playerNameInput->setPosition(Vec2(visibleSize.width / 2 + 20, visibleSize.height / 2 - 110));
    _playerNameInput->setContentSize(Size(200, 30));
    _playerNameInput->setMaxLength(20);
    _playerNameInput->setPlaceHolderColor(Color3B::GRAY);
    this->addChild(_playerNameInput);
    
    // 密码标签
    auto passwordLabel = Label::createWithSystemFont(u8"密码:", GameConfig::FONT_FANGSONG, 18);
    passwordLabel->setPosition(Vec2(visibleSize.width / 2 - 135, visibleSize.height / 2 - 150));
    passwordLabel->setColor(Color3B::WHITE);
    this->addChild(passwordLabel);
    
    // 密码输入框
    _passwordInput = TextField::create("123456", GameConfig::FONT_ARIAL, 18);
    _passwordInput->setPosition(Vec2(visibleSize.width / 2 + 20, visibleSize.height / 2 - 150));
    _passwordInput->setContentSize(Size(200, 30));
    _passwordInput->setMaxLength(20);
    _passwordInput->setPasswordEnabled(true);
    _passwordInput->setPlaceHolderColor(Color3B::GRAY);
    this->addChild(_passwordInput);
	_serverAddressInput->setEnabled(true);
	_playerNameInput->setEnabled(true);
	_passwordInput->setEnabled(true);
    // 连接按钮
    _connectButton = Button::create();
    _connectButton->setTitleText(u8"连接服务器");
    _connectButton->setTitleFontSize(20);
    _connectButton->setContentSize(Size(200, 50));
    _connectButton->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 200));
    _connectButton->addClickEventListener(CC_CALLBACK_1(LoginScene::onConnectButtonPressed, this));
    this->addChild(_connectButton); 
    
    // 版本信息
    auto versionLabel = Label::createWithSystemFont("Version 0.1.0", GameConfig::FONT_SIMHEI, 12);
    versionLabel->setPosition(Vec2(visibleSize.width - 60, 20));
    versionLabel->setColor(Color3B(80, 180, 180));
    this->addChild(versionLabel);
}

void LoginScene::updateUI()
{
    bool isConnected = _network && _network->isConnected();
    
    _serverAddressInput->setEnabled(!isConnected && !_isConnecting);
    _playerNameInput->setEnabled(!isConnected && !_isConnecting);
    _passwordInput->setEnabled(!isConnected && !_isConnecting);
    _connectButton->setEnabled(!isConnected && !_isConnecting);
    
    if (isConnected)
    {
        _connectButton->setTitleText(u8"已连接");
        _statusLabel->setString(u8"已连接到服务器");
        _statusLabel->setColor(Color3B::GREEN);
    }
    else if (_isConnecting)
    {
        _connectButton->setTitleText(u8"连接中...");
		_statusLabel->setString(StringUtils::format(u8"正在连接 #%d ...", _loginAttempts));
        _statusLabel->setColor(Color3B::YELLOW);
    }
    else
    {
        _connectButton->setTitleText(u8"连接服务器");
		_statusLabel->setString(u8"等待连接");
        _statusLabel->setColor(Color3B::GRAY);
    }
}

void LoginScene::onConnectButtonPressed(cocos2d::Ref* sender)
{
	CCLOG("==onConnectButtonPressed enter==");
    if (_isConnecting || (_network && _network->isConnected()))
        return;
    
    std::string serverAddress = _serverAddressInput->getString();
    std::string playerName = _playerNameInput->getString();
    std::string password = _passwordInput->getString();
    
    // 验证输入
    if (serverAddress.empty())
    {
        _statusLabel->setString(u8"请输入服务器地址");
        _statusLabel->setColor(Color3B::RED);
        return;
    }
    
    if (playerName.empty())
    {
        _statusLabel->setString(u8"请输入玩家名称");
        _statusLabel->setColor(Color3B::RED);
        return;
    }
    
    if (password.empty())
    {
        _statusLabel->setString(u8"请输入密码");
        _statusLabel->setColor(Color3B::RED);
        return;
    }
    
    _isConnecting = true;
    _loginAttempts++;
	updateUI();
	_connectButton->setEnabled(false);
	// ? 延迟执行连接，让 UI 有时间渲染
	this->scheduleOnce([this, serverAddress, playerName, password](float dt) {
		_network->connect("ws://" + serverAddress + "/ws");
	}, 0.05f, "connect_delay");

	CCLOG("==onConnectButtonPressed exit==");
}

void LoginScene::onConnected()
{
    CCLOG(u8"==[LoginScene::onConnected]连接已建立，发送登录信息==");
	updateUI();
    // 发送登录请求
    _network->sendLoginRequest(
        _playerNameInput->getString(),
        _passwordInput->getString()
    );
}

void LoginScene::onLoginSuccess(LoginResponse* resp)
{
    CCLOG(u8"==[LoginScene::onLoginSuccess]登录成功！玩家ID: %d, 名称: %s, 等级: %d", 
        resp->playerId, resp->playerName.c_str(), resp->level);
    
    _isConnecting = false;
    updateUI();
	_loginData = resp;
    // 延迟切换场景
    this->runAction(Sequence::create(
        DelayTime::create(0.5f),
        CallFunc::create(CC_CALLBACK_0(LoginScene::switchToGameScene, this)),
        nullptr
    ));
}

void LoginScene::onLoginFailed(const std::string& reason)
{
    CCLOG(u8"登录失败: %s", reason.c_str());
    
    _statusLabel->setString(StringUtils::format(u8"登录失败: %s", reason.c_str()));
    _statusLabel->setColor(Color3B::RED);
    
    _isConnecting = false;
    updateUI();
}

void LoginScene::onDisconnected(const std::string& reason)
{
	CCLOG("==LoginScene.onDisconnected callback==");
	// 如果正在切换场景，忽略回调
	if (_isSwitchingScene)
	{
		return;
	}
	if (_connectButton && _connectButton->getParent())
	{
		_connectButton->setEnabled(true);
	}
    _isConnecting = false;
    updateUI();
}

void LoginScene::onError(const std::string& error)
{
    CCLOG(u8"==LoginScene.onError callback==");
    _isConnecting = false;
    updateUI();
}

void LoginScene::switchToGameScene()
{
	if (_isSwitchingScene) return;
	_isSwitchingScene = true;

	CCLOG("Switching to GameScene...");

	// 先清除网络回调，避免切换过程中触发
	if (_network)
	{
		_network->setOnConnected(nullptr);
		_network->setOnDisconnected(nullptr);
		_network->setOnError(nullptr);
		_network->setOnLoginResponse(nullptr);
	}

	// 清除 UI 引用
	_connectButton = nullptr;
	_statusLabel = nullptr;

	// 然后切换场景
	auto gameScene = GameScene::createWithPlayerData(_loginData);
	Director::getInstance()->replaceScene(gameScene);
}

std::string LoginScene::getCurrentTime()
{
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", t);
    return std::string(buffer);
}