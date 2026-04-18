#include "GameScene.h"
#include "GameConfig.h"
#include "LoginScene.h"
#include "SimpleAudioEngine.h"
#include <unordered_map>
#include <PlayerVisual.h>
#include "QuestManager.h"

USING_NS_CC;
using namespace cocos2d::ui;

Scene* GameScene::createScene()
{
	auto scene = Scene::create();
	auto layer = GameScene::create();
	scene->addChild(layer);
	return scene;
}

GameScene* GameScene::createWithPlayerData(LoginResponse* playerData)
{
	GameScene* scene = new GameScene();
	if (scene && scene->init())
	{
		scene->initWithPlayerData(playerData);
		scene->autorelease();
		return scene;
	}
	CC_SAFE_DELETE(scene);
	return nullptr;
}
void GameScene::initWithPlayerData(LoginResponse* playerData)
{
	if (!playerData) return;

	_playerId = playerData->playerId;
	_playerName = playerData->playerName;
	_playerLevel = playerData->level;
	_playerX = playerData->x* GameConfig::TILE_SIZE;
	_playerY = playerData->y* GameConfig::TILE_SIZE;
	_playerDirection = playerData->direction;
	delete playerData;
	// 更新已存在的玩家对象
	if (_localPlayer)
	{
		_localPlayer->setId(_playerId);
		_localPlayer->setName(_playerName);
		_localPlayer->setLevel(_playerLevel);
		_localPlayer->setPositionImmediate(Vec2(_playerX, _playerY));
		_localPlayer->setDirection(_playerDirection);
		// 更新视觉组件
		if (_playerVisual)
		{
			_playerVisual->setPlayerName(_playerName);
			_playerVisual->setPlayerColor(getColorByLevel(_playerLevel));
			_playerVisual->setPosition(_localPlayer->getPosition());
		}

		if (_playerInfoLabel)
		{
			_playerInfoLabel->setString(StringUtils::format("%s",
				_playerName.c_str()));
		}

		if (_levelLabel)
		{
			_levelLabel->setString(StringUtils::format("Level: %d", _playerLevel));
		}

		_lastSentPosition = _localPlayer->getPosition();
		CCLOG("==initWithPlayerData to game scene==");

	}
}
Color4F GameScene::getColorByLevel(int level)
{
	float r = 0.3f + (level % 10) * 0.05f;
	float g = 0.6f - (level % 10) * 0.03f;
	float b = 0.9f;

	r = std::min(0.9f, std::max(0.3f, r));
	g = std::min(0.8f, std::max(0.3f, g));

	return Color4F(r, g, b, 1.0f);
}
bool GameScene::init()
{
	CCLOG("GameScene::init");

	if (!Scene::init())
		return false;

	_isOnPortal = false;
	_currentPortalId = -1;
	_portalStayTime = 0;
	_portalRequiredTime = 1.0f;  // 需要停留1秒
	_portalTriggered = false;     // 未触发

	// 初始化默认值
	_playerId = 0;
	_playerName = "";
	_playerLevel = 1;
	_playerX = 400;
	_playerY = 300;
	_playerDirection = 0;

	_network = WebSocketClient::getInstance();
	_moveTimer = 0;
	_syncTimer = 0;
	_isPathMoving = false;
	_currentPathIndex = 0;

	// 创建地图
	_mapManager = MapManager::create();

	// 设置回调
	setupCallbacks();
	this->addChild(_mapManager, 0);

	// 创建临时玩家
	_localPlayer = new Player(0, "Loading...");
	_localPlayer->setPositionImmediate(Vec2(_playerX, _playerY));
	_localPlayer->setLevel(1);
	_lastSentPosition = _localPlayer->getPosition();

	// 创建玩家视觉
	_playerVisual = PlayerVisual::create();
	if (_playerVisual)
	{
		_playerVisual->setPlayerName("Loading...");
		_playerVisual->setIsLocalPlayer(true);
		_playerVisual->setPosition(_localPlayer->getPosition());
		this->addChild(_playerVisual, 10);
	}

	setupUIFixed();
	// 请求地图数据
	if (_network && _network->isConnected())
	{
		_network->sendMapDataRequest(1);
		//_mapManager->loadDefaultMap();
	}

	//TODO: 键盘监听
	auto keyboardListener = EventListenerKeyboard::create();
	keyboardListener->onKeyPressed = CC_CALLBACK_2(GameScene::onKeyPressed, this);
	keyboardListener->onKeyReleased = CC_CALLBACK_2(GameScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

	//TODO: 鼠标点击监听
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
	//TODO: 恢复游戏窗口
	auto resumeListener = EventListenerCustom::create(EVENT_COME_TO_FOREGROUND, [this](EventCustom* event) {
		_needResetCamera = true;
	});
	_eventDispatcher->addEventListenerWithSceneGraphPriority(resumeListener, this);

	this->scheduleUpdate();
	CCLOG(u8"GameScene init 完成");
	return true;
}
void GameScene::interactWithNearbyNPC()
{
	if (!_localPlayer || !_mapManager) return;

	Vec2 playerGrid = _localPlayer->getGridPosition();
	// 先检查玩家当前位置是否有NPC（重叠）
	NPCData* npc = _mapManager->getNPCAt((int)playerGrid.x, (int)playerGrid.y);
	if (npc)
	{
		_currentInteractNPC = npc;
		_network->sendTalkToNPCRequest(npc->id);
		return;
	}

	int tileSize = GameConfig::TILE_SIZE;

	// 搜索周围8个方向的NPC
	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dy = -1; dy <= 1; dy++)
		{
			if (dx == 0 && dy == 0) continue;

			int checkX = (int)playerGrid.x + dx;
			int checkY = (int)playerGrid.y + dy;

			// 直接从 MapManager 获取 NPC
			NPCData* npc = _mapManager->getNPCAt(checkX, checkY);
			if (npc)
			{
				CCLOG("Found NPC: %s at grid (%d, %d)", npc->name.c_str(), npc->x, npc->y);
				_currentInteractNPC = npc;  // 保存当前 NPC
				_network->sendTalkToNPCRequest(npc->id);
				return;
			}
		}
	}
	showMessage(u8"附近没有NPC", Color3B::YELLOW);
}
void GameScene::showMessage(const std::string& text, const Color3B& color)
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	unsigned short uiMask = (unsigned short)CameraFlag::USER2;

	// 背景
	auto bg = ui::Layout::create();
	bg->setContentSize(Size(text.length() * 12 + 40, 40));
	bg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	bg->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 150));
	bg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	bg->setBackGroundColor(Color3B(0, 0, 0));
	bg->setBackGroundColorOpacity(150);
	bg->setCameraMask(uiMask);
	this->addChild(bg, 300);

	// 文字
	auto label = Label::createWithSystemFont(text, GameConfig::FONT_MSYAH, 18);
	label->setPosition(Vec2(bg->getContentSize().width / 2, bg->getContentSize().height / 2));
	label->setColor(color);
	label->setCameraMask(uiMask);
	bg->addChild(label);

	// 动画效果
	bg->setScale(0.5f);
	bg->runAction(Sequence::create(
		ScaleTo::create(0.1f, 1.0f),
		DelayTime::create(1.5f),
		FadeOut::create(0.3f),
		CallFunc::create([bg]() {
		bg->removeFromParent();
	}),
		nullptr
		));
}
void GameScene::showQuestDialog(NPCData* npc, const rapidjson::Value& quests)
{
	if (_currentDialogBg)
	{
		_currentDialogBg->removeFromParent();
		_currentDialogBg = nullptr;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	unsigned short uiMask = (unsigned short)CameraFlag::USER2;

	// 对话框尺寸
	int _w = 400;
	int _h = 300;

	// 根据任务数量动态调整高度
	int questCount = (int)quests.Size();
	if (questCount > 0)
	{
		_h = 180 + questCount * 70;
	}

	auto dialogBg = ui::Layout::create();
	dialogBg->setContentSize(Size(_w, _h));
	dialogBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	dialogBg->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	dialogBg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	dialogBg->setBackGroundColor(Color3B(0, 0, 0));
	dialogBg->setBackGroundColorOpacity(200);
	dialogBg->setCameraMask(uiMask);
	this->addChild(dialogBg, 300);

	// NPC 名称（顶部居中）
	auto nameLabel = Label::createWithSystemFont(npc->name, GameConfig::FONT_MSYAH, 24);
	nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	nameLabel->setPosition(Vec2(_w / 2, _h - 15));
	nameLabel->setColor(Color3B::YELLOW);
	nameLabel->setCameraMask(uiMask);
	dialogBg->addChild(nameLabel);

	// 分隔线
	auto line = ui::Layout::create();
	line->setContentSize(Size(_w - 40, 2));
	line->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	line->setPosition(Vec2(_w / 2, _h - 42));
	line->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	line->setBackGroundColor(Color3B(100, 100, 100));
	line->setCameraMask(uiMask);
	dialogBg->addChild(line);

	float startY = _h - 65;
	int btnCount = 0;

	for (rapidjson::SizeType i = 0; i < quests.Size(); i++)
	{
		std::string questId = quests[i]["questId"].GetString();
		std::string questName = quests[i]["questName"].GetString();
		std::string description = quests[i]["description"].GetString();

		// 任务按钮
		auto questBtn = ui::Button::create();
		questBtn->setTitleText(questName);
		questBtn->setTitleFontSize(16);
		questBtn->setContentSize(Size(_w - 60, 35));
		questBtn->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
		questBtn->setPosition(Vec2(_w / 2, startY - btnCount * 65));
		questBtn->setCameraMask(uiMask);
		questBtn->addClickEventListener([this, questId, npc](Ref* sender) {
			//TODO: 接收任务正在开发
			showMessage(u8"在这开发中...");
			//_network->sendAcceptQuestRequest(questId, npc->id);
			if (_currentDialogBg)
			{
				_currentDialogBg->removeFromParent();
				_currentDialogBg = nullptr;
			}
		});
		dialogBg->addChild(questBtn);

		// 任务描述
		auto descLabel = Label::createWithSystemFont(description, GameConfig::FONT_KAITI, 12);
		descLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
		descLabel->setPosition(Vec2(_w / 2, startY - btnCount * 65 - 25));
		descLabel->setColor(Color3B::GRAY);
		descLabel->setCameraMask(uiMask);
		dialogBg->addChild(descLabel);

		btnCount++;
	}

	// 如果没有任务，显示提示
	if (btnCount == 0)
	{
		auto noQuestLabel = Label::createWithSystemFont(u8"暂时没有可接取的任务", GameConfig::FONT_KAITI, 16);
		noQuestLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
		noQuestLabel->setPosition(Vec2(_w / 2, _h / 2));
		noQuestLabel->setColor(Color3B::GRAY);
		noQuestLabel->setCameraMask(uiMask);
		dialogBg->addChild(noQuestLabel);
	}

	// 关闭按钮（底部居中）
	auto closeBtn = ui::Button::create();
	closeBtn->setTitleText(u8"==== 关闭 ====");
	closeBtn->setTitleFontSize(16);
	closeBtn->setContentSize(Size(70, 30));
	closeBtn->setAnchorPoint(Vec2::ANCHOR_MIDDLE_BOTTOM);
	closeBtn->setPosition(Vec2(_w / 2, 15));
	closeBtn->addClickEventListener([this, dialogBg](Ref* sender) {
		if (_currentDialogBg == dialogBg)
		{
			_currentDialogBg = nullptr;
		}
		dialogBg->removeFromParent();
	});
	closeBtn->setCameraMask(uiMask);
	dialogBg->addChild(closeBtn);

	_currentDialogBg = dialogBg;
}
void GameScene::showNPCDialog(NPCData* npc)
{
	if (!npc) return;
	if (_currentDialogBg)
	{
		_currentDialogBg->removeFromParent();
		_currentDialogBg = nullptr;
	}
	auto visibleSize = Director::getInstance()->getVisibleSize();
	unsigned short uiMask = (unsigned short)CameraFlag::USER2;
	// 创建对话框
	auto dialogBg = ui::Layout::create();
	dialogBg->setContentSize(Size(500, 200));
	dialogBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	dialogBg->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	dialogBg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	dialogBg->setBackGroundColor(Color3B(0, 0, 0));
	dialogBg->setBackGroundColorOpacity(200);
	dialogBg->setCameraMask(uiMask);
	this->addChild(dialogBg, 300);

	// NPC 名称
	auto nameLabel = Label::createWithSystemFont(npc->name, GameConfig::FONT_MSYAH, 24);
	nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	nameLabel->setPosition(Vec2(250, 180));
	nameLabel->setColor(Color3B::YELLOW);
	nameLabel->setCameraMask(uiMask);
	dialogBg->addChild(nameLabel);

	// 对话内容
	auto dialogLabel = Label::createWithSystemFont(npc->dialog, GameConfig::FONT_KAITI, 18);
	dialogLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	dialogLabel->setPosition(Vec2(250, 100));
	dialogLabel->setColor(Color3B::WHITE);
	dialogLabel->setCameraMask(uiMask);
	dialogBg->addChild(dialogLabel);

	// 关闭按钮
	auto closeBtn = ui::Button::create();
	closeBtn->setTitleText(u8"关闭");
	closeBtn->setTitleFontSize(18);
	closeBtn->setContentSize(Size(80, 35));
	closeBtn->setAnchorPoint(Vec2::ANCHOR_MIDDLE_BOTTOM);
	closeBtn->setPosition(Vec2(250, 20));
	closeBtn->addClickEventListener([this, dialogBg](Ref* sender) {
		if (_currentDialogBg == dialogBg)
		{
			_currentDialogBg = nullptr;
		}
		dialogBg->removeFromParent();
	});
	closeBtn->setCameraMask(uiMask);
	dialogBg->addChild(closeBtn);
	_currentDialogBg = dialogBg;
	// 如果 NPC 有任务，显示任务按钮
	if (npc->hasQuest)
	{
		auto questBtn = ui::Button::create();
		questBtn->setTitleText(u8"接受任务");
		questBtn->setTitleFontSize(18);
		questBtn->setContentSize(Size(100, 35));
		questBtn->setAnchorPoint(Vec2::ANCHOR_MIDDLE_BOTTOM);
		questBtn->setPosition(Vec2(140, 20));
		questBtn->addClickEventListener([this, npc](Ref* sender) {
			acceptQuest(npc);
			if (_currentDialogBg)
			{
				_currentDialogBg->removeFromParent();
				_currentDialogBg = nullptr;
			}
		});
		questBtn->setCameraMask(uiMask);
		dialogBg->addChild(questBtn);

		// 调整关闭按钮位置
		closeBtn->setPosition(Vec2(360, 20));
	}
}
void GameScene::showSimpleDialog(const std::string& npcName, const std::string& dialog)
{
	if (_currentDialogBg)
	{
		_currentDialogBg->removeFromParent();
		_currentDialogBg = nullptr;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	unsigned short uiMask = (unsigned short)CameraFlag::USER2;

	int _w = 400;
	int _h = 200;

	auto dialogBg = ui::Layout::create();
	dialogBg->setContentSize(Size(_w, _h));
	dialogBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	dialogBg->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	dialogBg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	dialogBg->setBackGroundColor(Color3B(0, 0, 0));
	dialogBg->setBackGroundColorOpacity(200);
	dialogBg->setCameraMask(uiMask);
	this->addChild(dialogBg, 300);

	// NPC 名称
	auto nameLabel = Label::createWithSystemFont(npcName, GameConfig::FONT_MSYAH, 24);
	nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	nameLabel->setPosition(Vec2(_w / 2, _h - 15));
	nameLabel->setColor(Color3B::YELLOW);
	nameLabel->setCameraMask(uiMask);
	dialogBg->addChild(nameLabel);

	// 分隔线
	auto line = ui::Layout::create();
	line->setContentSize(Size(_w - 40, 2));
	line->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	line->setPosition(Vec2(_w / 2, _h - 35));
	line->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	line->setBackGroundColor(Color3B(100, 100, 100));
	line->setCameraMask(uiMask);
	dialogBg->addChild(line);

	// 对话内容
	auto dialogLabel = Label::createWithSystemFont(dialog, GameConfig::FONT_KAITI, 16);
	dialogLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	dialogLabel->setPosition(Vec2(_w / 2, _h / 2));
	dialogLabel->setColor(Color3B::WHITE);
	dialogLabel->setCameraMask(uiMask);
	dialogBg->addChild(dialogLabel);

	// 关闭按钮
	auto closeBtn = ui::Button::create();
	closeBtn->setTitleText(u8"关闭");
	closeBtn->setTitleFontSize(16);
	closeBtn->setContentSize(Size(70, 30));
	closeBtn->setAnchorPoint(Vec2::ANCHOR_MIDDLE_BOTTOM);
	closeBtn->setPosition(Vec2(_w / 2, 15));
	closeBtn->addClickEventListener([this, dialogBg](Ref* sender) {
		if (_currentDialogBg == dialogBg)
		{
			_currentDialogBg = nullptr;
		}
		dialogBg->removeFromParent();
	});
	closeBtn->setCameraMask(uiMask);
	dialogBg->addChild(closeBtn);

	_currentDialogBg = dialogBg;
}

void GameScene::acceptQuest(NPCData* npc)
{
	if (!npc || !_network) return;

	// 发送接受任务请求
	_network->sendAcceptQuestRequest(npc->questId, npc->id);

	CCLOG(u8"接受任务: %s (ID: %s)", npc->name.c_str(), npc->questId.c_str());
}
//每一秒执行一次该函数
void GameScene::checkPortalTrigger()
{
	if (!_localPlayer || !_mapManager) return;

	int tileSize = GameConfig::TILE_SIZE;
	int gridX = (int)(_localPlayer->getPosition().x / tileSize);
	int gridY = (int)(_localPlayer->getPosition().y / tileSize);

	const PortalInfo* portal = _mapManager->getPortalAt(gridX, gridY);

	if (portal && portal->isActive)
	{
		if (!_isOnPortal)
		{
			// 刚进入传送门，重置状态
			_isOnPortal = true;
			_currentPortalId = portal->id;
			_portalStayTime = 0;
			_portalTriggered = false;  // 重置触发标志
			CCLOG(u8"进入传送门: %s", portal->name.c_str());
		}
		else if (_currentPortalId == portal->id && !_portalTriggered)
		{
			// 累加时间
			_portalStayTime += 0.1f;

			if (_portalStayTime >= _portalRequiredTime)
			{
				// 触发传送
				onPortalTrigger(*portal);
				_portalTriggered = true;  // 标记已触发，防止再次触发
			}
		}
	}
	else {
		// 离开传送门，重置计时
		if (_isOnPortal)
		{
			_isOnPortal = false;
			_currentPortalId = -1;
			_portalStayTime = 0;
			_portalTriggered = false;
			CCLOG(u8"离开传送门，计时重置");
		}
	}
}
void GameScene::onPortalTrigger(const PortalInfo & portal)
{
	CCLOG(u8"触发传送门: %s -> 地图%d", portal.name.c_str(), portal.targetMapId);

	// 显示提示
	showMessage(StringUtils::format(u8"传送到: %s", portal.name.c_str()), Color3B::YELLOW);

	// 发送传送请求到服务器
	// _network->sendTeleportRequest(portal.id);
}

void GameScene::updateQuestPanel()
{
	// 获取当前进行中的任务
	auto& activeQuests = QuestManager::getInstance()->getActiveQuests();

	if (!_questPanel)
	{
		// 如果面板不存在，创建它
		showQuestPanel();
		return;
	}

	// 清除旧内容
	_questPanel->removeAllChildren();

	auto visibleSize = Director::getInstance()->getVisibleSize();
	unsigned short uiMask = (unsigned short)CameraFlag::USER2;

	float y = visibleSize.height - 80;
	int taskCount = 0;

	for (auto& quest : activeQuests)
	{
		// 任务项容器
		auto questItem = ui::Layout::create();
		questItem->setContentSize(Size(280, 70));
		questItem->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
		questItem->setPosition(Vec2(10, y));
		questItem->setCameraMask(uiMask);
		_questPanel->addChild(questItem);

		// 任务名称
		auto nameLabel = Label::createWithSystemFont(quest.name, GameConfig::FONT_MSYAH, 16);
		nameLabel->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
		nameLabel->setPosition(Vec2(10, 60));
		nameLabel->setColor(Color3B::YELLOW);
		nameLabel->setCameraMask(uiMask);
		questItem->addChild(nameLabel);

		// 任务描述
		auto descLabel = Label::createWithSystemFont(quest.description, GameConfig::FONT_KAITI, 12);
		descLabel->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
		descLabel->setPosition(Vec2(10, 40));
		descLabel->setColor(Color3B::WHITE);
		descLabel->setCameraMask(uiMask);
		questItem->addChild(descLabel);

		// 进度条背景
		auto progressBg = ui::Layout::create();
		progressBg->setContentSize(Size(200, 12));
		progressBg->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
		progressBg->setPosition(Vec2(10, 20));
		progressBg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
		progressBg->setBackGroundColor(Color3B(50, 50, 50));
		progressBg->setCameraMask(uiMask);
		questItem->addChild(progressBg);

		// 进度条
		auto progressBar = ui::LoadingBar::create();
		progressBar->setContentSize(Size(200, 12));
		progressBar->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
		progressBar->setPosition(Vec2(0, 0));
		progressBar->setPercent(quest.getProgressPercent() * 100);
		progressBar->setColor(Color3B::GREEN);
		progressBar->setCameraMask(uiMask);
		progressBg->addChild(progressBar);

		// 进度文字
		auto progressLabel = Label::createWithSystemFont(
			StringUtils::format("%d/%d", quest.currentProgress, quest.requiredCount),
			GameConfig::FONT_MSYAH, 10);
		progressLabel->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
		progressLabel->setPosition(Vec2(220, 2));
		progressLabel->setColor(Color3B::WHITE);
		progressLabel->setCameraMask(uiMask);
		progressBg->addChild(progressLabel);

		y -= 80;
		taskCount++;
	}

	// 如果没有任务，显示提示
	if (taskCount == 0)
	{
		auto emptyLabel = Label::createWithSystemFont(u8"暂无进行中的任务", GameConfig::FONT_KAITI, 16);
		emptyLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
		emptyLabel->setPosition(Vec2(150, 200));
		emptyLabel->setColor(Color3B::GRAY);
		emptyLabel->setCameraMask(uiMask);
		_questPanel->addChild(emptyLabel);
	}
}
void GameScene::showQuestPanel()
{
	if (_questPanel)
	{
		_questPanel->setVisible(true);
		updateQuestPanel();
		return;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	unsigned short uiMask = (unsigned short)CameraFlag::USER2;

	// 创建任务面板
	_questPanel = ui::Layout::create();
	_questPanel->setContentSize(Size(320, visibleSize.height - 100));
	_questPanel->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
	_questPanel->setPosition(Vec2(visibleSize.width - 10, visibleSize.height - 10));
	_questPanel->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	_questPanel->setBackGroundColor(Color3B(0, 0, 0));
	_questPanel->setBackGroundColorOpacity(180);
	_questPanel->setCameraMask(uiMask);
	this->addChild(_questPanel, 250);

	// 标题
	auto titleLabel = Label::createWithSystemFont(u8"任务追踪", GameConfig::FONT_MSYAH, 20);
	titleLabel->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	titleLabel->setPosition(Vec2(15, visibleSize.height - 35));
	titleLabel->setColor(Color3B::YELLOW);
	titleLabel->setCameraMask(uiMask);
	_questPanel->addChild(titleLabel);

	// 分隔线
	auto line = ui::Layout::create();
	line->setContentSize(Size(290, 2));
	line->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	line->setPosition(Vec2(15, visibleSize.height - 45));
	line->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	line->setBackGroundColor(Color3B(100, 100, 100));
	line->setCameraMask(uiMask);
	_questPanel->addChild(line);

	updateQuestPanel();
}
void GameScene::hideQuestPanel()
{
	if (_questPanel)
	{
		_questPanel->setVisible(false);
	}
}
void GameScene::setupUIFixed()
{
	auto visibleSize = Director::getInstance()->getVisibleSize();

	//创建正交摄像机
	auto uiCamera = Camera::createOrthographic(visibleSize.width, visibleSize.height, 1, 1000);
	uiCamera->setCameraFlag(CameraFlag::USER2);
	uiCamera->setDepth(100);
	// 摄像机位置固定在左下角
	uiCamera->setPosition3D(Vec3(0, 0, 500));
	this->addChild(uiCamera);

	unsigned short uiMask = (unsigned short)CameraFlag::USER2;
	// 添加坐标显示标签（屏幕左下角）
	_coordLabel = Label::createWithSystemFont("Grid: (0, 0)", "Arial", 16);
	_coordLabel->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	_coordLabel->setPosition(Vec2(20, 20));
	_coordLabel->setColor(Color3B::YELLOW);
	_coordLabel->setCameraMask(uiMask);
	this->addChild(_coordLabel, 200);

	_mapNameLabel = Label::createWithSystemFont("Loading...", GameConfig::FONT_MSYAH, 18);
	_mapNameLabel->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	_mapNameLabel->setPosition(Vec2(20, 40));
	_mapNameLabel->setColor(Color3B::YELLOW);
	_mapNameLabel->setCameraMask(uiMask);
	this->addChild(_mapNameLabel, 200);
	//玩家名
	_playerInfoLabel = Label::createWithSystemFont("Player:Loading", GameConfig::FONT_MSYAH, 18);
	_playerInfoLabel->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	_playerInfoLabel->setPosition(Vec2(20, visibleSize.height - 16));
	_playerInfoLabel->setCameraMask(uiMask);
	this->addChild(_playerInfoLabel);
	_levelLabel = Label::createWithSystemFont("Level:1", GameConfig::FONT_MSYAH, 18);
	_levelLabel->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	_levelLabel->setPosition(Vec2(100, visibleSize.height - 16));
	_levelLabel->setCameraMask(uiMask);
	this->addChild(_levelLabel);

	// 生命值背景（左上角，玩家信息下方）
	auto hpBg = ui::Layout::create();
	hpBg->setContentSize(Size(100, 22));
	hpBg->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	hpBg->setPosition(Vec2(20, visibleSize.height - 36));
	hpBg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	hpBg->setBackGroundColor(Color3B(100, 0, 0));
	hpBg->setBackGroundColorOpacity(200);
	hpBg->setCameraMask(uiMask);
	this->addChild(hpBg, 200);
	// 生命条
	_hpBar = ui::LoadingBar::create();
	_hpBar->setContentSize(Size(100, 22));
	_hpBar->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	_hpBar->setPosition(Vec2(0, 0));
	_hpBar->setColor(Color3B(255, 50, 50));
	_hpBar->setPercent(100);
	hpBg->addChild(_hpBar);
	// 生命值文字
	_hpLabel = Label::createWithSystemFont("HP: 100/100", GameConfig::FONT_MSYAH, 12);
	_hpLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	_hpLabel->setPosition(Vec2(50, 10));
	_hpLabel->setColor(Color3B::WHITE);
	_hpLabel->setCameraMask(uiMask);
	hpBg->addChild(_hpLabel);

	// 经验值背景
	auto expBg = ui::Layout::create();
	expBg->setContentSize(Size(100, 20));
	expBg->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	expBg->setPosition(Vec2(20, visibleSize.height - 60));
	expBg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	expBg->setBackGroundColor(Color3B(50, 50, 50));
	expBg->setBackGroundColorOpacity(200);
	expBg->setCameraMask(uiMask);
	this->addChild(expBg, 200);

	// 经验条
	_expBar = ui::LoadingBar::create();
	_expBar->setContentSize(Size(100, 20));
	_expBar->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	_expBar->setPosition(Vec2(0, 0));
	_expBar->setColor(Color3B(50, 150, 255));
	_expBar->setPercent(0);
	expBg->addChild(_expBar);

	// 经验值文字
	_expLabel = Label::createWithSystemFont("EXP: 0/100", GameConfig::FONT_MSYAH, 12);
	_expLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	_expLabel->setPosition(Vec2(50, 10));
	_expLabel->setColor(Color3B::WHITE);
	_expLabel->setCameraMask(uiMask);
	expBg->addChild(_expLabel);

	_statusLabel = Label::createWithSystemFont(u8"在线 |", GameConfig::FONT_MSYAH, 18);
	_statusLabel->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
	_statusLabel->setPosition(Vec2(visibleSize.width - 55, visibleSize.height - 16));
	_statusLabel->setCameraMask(uiMask);
	this->addChild(_statusLabel);

	_disconnectBtn = ui::Button::create();
	_disconnectBtn->setTitleText(u8"退出");
	_disconnectBtn->setTitleFontSize(18);
	_disconnectBtn->setContentSize(Size(100, 40));
	_disconnectBtn->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
	_disconnectBtn->setPosition(Vec2(visibleSize.width - 16, visibleSize.height - 16));
	_disconnectBtn->setCameraMask(uiMask);
	_disconnectBtn->addClickEventListener([this](Ref* sender) {
		_network->disconnect();
		Director::getInstance()->replaceScene(LoginScene::createScene());
	});
	this->addChild(_disconnectBtn);
	// 帮助按钮（屏幕右下角）
	_helpButton = ui::Button::create();
	_helpButton->setTitleText(u8"帮助");
	_helpButton->setTitleFontSize(20);
	_helpButton->setContentSize(Size(80, 40));
	_helpButton->setAnchorPoint(Vec2::ANCHOR_BOTTOM_RIGHT);
	_helpButton->setPosition(Vec2(visibleSize.width - 10, 10));
	_helpButton->addClickEventListener([this](Ref* sender) {
		showHelpPanel();
	});
	_helpButton->setCameraMask(uiMask);
	this->addChild(_helpButton, 200);

	// 帮助面板（初始隐藏）
	createHelpPanel();
}
void GameScene::createHelpPanel()
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	unsigned short uiMask = (unsigned short)CameraFlag::USER2;

	// 帮助面板背景
	_helpPanel = ui::Layout::create();
	_helpPanel->setContentSize(Size(500, 350));
	_helpPanel->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	_helpPanel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	_helpPanel->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	_helpPanel->setBackGroundColor(Color3B(0, 0, 0));
	_helpPanel->setBackGroundColorOpacity(220);
	_helpPanel->setCameraMask(uiMask);
	_helpPanel->setVisible(false);
	this->addChild(_helpPanel, 300);

	// 标题
	auto titleLabel = Label::createWithSystemFont(u8"游戏帮助", GameConfig::FONT_MSYAH, 25);
	titleLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	titleLabel->setPosition(Vec2(250, 330));
	titleLabel->setColor(Color3B::YELLOW);
	titleLabel->setCameraMask(uiMask);
	_helpPanel->addChild(titleLabel);

	// 分隔线
	auto line = ui::Layout::create();
	line->setContentSize(Size(460, 2));
	line->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	line->setPosition(Vec2(250, 300));
	line->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	line->setBackGroundColor(Color3B(100, 100, 100));
	line->setCameraMask(uiMask);
	_helpPanel->addChild(line);

	// 帮助内容
	std::vector<std::string> helpTexts = {
		u8"● 按 E 键：与附近NPC对话",
		u8"● 按 Q 键：打开/关闭任务面板",
		u8"● 方向键 / WASD：移动角色",
		u8"● 鼠标左键：点击地面自动寻路",
		u8"● ESC：关闭对话框 ",
		u8"● 传送门：传送到其他地图",
		u8"● 与NPC对话可接取任务"
	};

	float startY = 270;
	for (int i = 0; i < (int)helpTexts.size(); i++)
	{
		auto helpLabel = Label::createWithSystemFont(helpTexts[i], GameConfig::FONT_KAITI, 16);
		helpLabel->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
		helpLabel->setPosition(Vec2(40, startY - i * 35));
		helpLabel->setColor(Color3B::WHITE);
		helpLabel->setCameraMask(uiMask);
		_helpPanel->addChild(helpLabel);
	}

	// 关闭按钮
	auto closeBtn = ui::Button::create();
	closeBtn->setTitleText(u8"关闭");
	closeBtn->setTitleFontSize(18);
	closeBtn->setContentSize(Size(80, 35));
	closeBtn->setAnchorPoint(Vec2::ANCHOR_MIDDLE_BOTTOM);
	closeBtn->setPosition(Vec2(250, 20));
	closeBtn->addClickEventListener([this](Ref* sender) {
		hideHelpPanel();
	});
	closeBtn->setCameraMask(uiMask);
	_helpPanel->addChild(closeBtn);
}

void GameScene::showHelpPanel()
{
	if (_helpPanel)
	{
		_helpPanel->setVisible(true);

		// 添加弹出动画
		_helpPanel->setScale(0.8f);
		_helpPanel->runAction(ScaleTo::create(0.1f, 1.0f));
	}
}

void GameScene::hideHelpPanel()
{
	if (_helpPanel)
	{
		_helpPanel->setVisible(false);
	}
}
void GameScene::updateCoordDisplay()
{
	if (!_localPlayer || !_coordLabel) return;

	// 获取玩家格子坐标
	int tileSize = GameConfig::TILE_SIZE;
	int gridX = (_localPlayer->getGridPosition().x);
	int gridY = (_localPlayer->getGridPosition().y);

	// 更新标签文字
	_coordLabel->setString(StringUtils::format("(%d, %d)", gridX, gridY));
}
void GameScene::updatePlayerStats(int hp, int maxHp, int exp, int maxExp)
{
	// 更新生命条
	if (_hpBar)
	{
		int percent = (hp * 100) / maxHp;
		percent = std::max(0, std::min(100, percent));
		_hpBar->setPercent(percent);
	}

	if (_hpLabel)
	{
		_hpLabel->setString(StringUtils::format("HP: %d/%d", hp, maxHp));
		// 根据血量百分比改变颜色
		int percent = (hp * 100) / maxHp;
		if (percent > 60)
			_hpLabel->setColor(Color3B::GREEN);
		else if (percent > 30)
			_hpLabel->setColor(Color3B::YELLOW);
		else
			_hpLabel->setColor(Color3B::RED);
	}

	// 更新经验条
	if (_expBar)
	{
		int percent = (exp * 100) / maxExp;
		percent = std::max(0, std::min(100, percent));
		_expBar->setPercent(percent);
	}

	if (_expLabel)
	{
		_expLabel->setString(StringUtils::format("EXP: %d/%d", exp, maxExp));
	}
}
void GameScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	_keyStates[keyCode] = true;

	switch (keyCode)
	{
	case EventKeyboard::KeyCode::KEY_ESCAPE:
		if (_helpPanel && _helpPanel->isVisible())
		{
			hideHelpPanel();
			return;
		}
		if (_currentDialogBg)
		{
			_currentDialogBg->removeFromParent();
			_currentDialogBg = nullptr;
		}
		break;

	case EventKeyboard::KeyCode::KEY_E:
		interactWithNearbyNPC();
		break;

	case EventKeyboard::KeyCode::KEY_Q:
		if (_questPanel && _questPanel->isVisible())
		{
			hideQuestPanel();
		}
		else
		{
			showQuestPanel();
		}
		break;

	default:
		break;
	}
}
void GameScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	_keyStates[keyCode] = false;
}
bool GameScene::isKeyPressed(EventKeyboard::KeyCode keyCode) const
{
	auto it = _keyStates.find(keyCode);
	return it != _keyStates.end() && it->second;
}

void GameScene::updatePlayerSpriteAppearance(Player* player)
{
	if (!player) return;

	int level = player->getLevel();

	// 根据等级计算颜色
	float r = 0.3f + (level % 10) * 0.05f;
	float g = 0.6f - (level % 10) * 0.03f;
	float b = 0.9f;

	r = std::min(0.9f, std::max(0.3f, r));
	g = std::min(0.8f, std::max(0.3f, g));

	// 只更新 PlayerVisual 的颜色
	if (_playerVisual)
	{
		_playerVisual->setPlayerColor(Color4F(r, g, b, 1.0f));
	}
}

void GameScene::setupCallbacks()
{
	_network->setOnLoginResponse([this](LoginResponse* resp) {
		onLoginResponse(resp);
	});

	//_network->setOnWorldState([this](WorldState* state) {
	//	onWorldState(state);
	//});

	_network->setOnMapDataResponse([this](MapDataMessage* data) {
		onMapDataResponse(data);
	});

	_network->setOnPathResponse([this](PathResponse* resp) {
		onPathResponse(resp);
	});

	_network->setOnAcceptQuestResponse([this](bool success, const Quest& quest) {
		if (success)
		{
			QuestManager::getInstance()->acceptQuest(quest);
			updateQuestPanel();
		}
		else
		{
			showMessage(u8"接受任务失败", Color3B::RED);
		}
	});

	_network->setOnQuestComplete([this](const rapidjson::Value& data) {
		std::string questId = data["questId"].GetString();
		std::string questName = data["questName"].GetString();
		const rapidjson::Value& rewards = data["rewards"];
		showMessage(StringUtils::format(u8"任务完成: %s", questName.c_str()), Color3B::GREEN);
		updateQuestPanel();
	});
	// NPC 对话回调
	_network->setOnNPCDialog([this](int npcId, const std::string& npcName, const std::string& dialog) {
		CCLOG("NPC对话: %s - %s", npcName.c_str(), dialog.c_str());

		// 显示普通对话（不需要任务按钮）
		showSimpleDialog(npcName, dialog);
	});
	// 可接取任务回调: 与NCP对话，获得可接的任务。
	_network->setOnAvailableQuests([this](const rapidjson::Value& quests) {
		CCLOG(u8"收到可接取任务，数量: %d", quests.Size());

		// 获取当前对话的 NPC
		if (_currentInteractNPC && quests.Size() > 0)
		{
			showQuestDialog(_currentInteractNPC, quests);
		}
		else if (quests.Size() == 0)
		{
			showMessage(u8"暂无可用任务", Color3B::GRAY);
		}
	});

	_mapManager->setOnPortalTouch([this](const PortalInfo& portal) {
		CCLOG(u8"传送门触发: %s -> 地图%d", portal.name.c_str(), portal.targetMapId);

		// 发送传送请求到服务器
		// _network->sendTeleportRequest(portal.id);

		// 显示提示
		showMessage(StringUtils::format(u8"传送到: %s", portal.name.c_str()), Color3B::YELLOW);
	});

}

void GameScene::onLoginResponse(LoginResponse* resp)
{
	CCLOG("GameScene::onLoginResponse - success: %d", resp->success);

	if (resp->success)
	{
		_playerId = resp->playerId;
		_playerName = resp->playerName;
		_playerLevel = resp->level;

		_localPlayer->setId(_playerId);
		_localPlayer->setName(_playerName);
		_localPlayer->setLevel(_playerLevel);
		_localPlayer->setPositionImmediate(Vec2(resp->x, resp->y));
		_localPlayer->setDirection(resp->direction);

		updatePlayerSpriteAppearance(_localPlayer);

		_lastSentPosition = _localPlayer->getPosition();

		CCLOG("Login success: ID=%d, Name=%s, Level=%d, Pos=(%.0f,%.0f)",
			resp->playerId, resp->playerName.c_str(), resp->level, resp->x, resp->y);
	}
	else
	{
		_statusLabel->setString("Login failed: " + resp->message);
		_statusLabel->setColor(Color3B::RED);
		CCLOG("Login failed: %s", resp->message.c_str());
	}
}

void GameScene::onMapDataResponse(MapDataMessage* data)
{
	if (data->success)
	{
		_mapManager->loadMapData(*data);
		if (_mapNameLabel)
		{
			_mapNameLabel->setString(_mapManager->getMapName());
		}
		CCLOG("==[onMapDataResponse]Map loaded: %s", data->mapName.c_str());
	}
	else
	{
		CCLOG("Map load failed: %s", data->error.c_str());
	}
}

void GameScene::onPathResponse(PathResponse* resp)
{
	if (resp->success)
	{
		CCLOG("Path found, steps: %d", (int)resp->path.size());
		startPathMovement(resp->path);
	}
	else
	{
		CCLOG("Path failed: %s", resp->error.c_str());
	}
}

void GameScene::update(float delta)
{
	if (!_localPlayer) return;

	_localPlayer->update(delta);

	// 更新视觉组件动画
	if (_playerVisual)
	{
		_playerVisual->updateAnimation(delta);
		_playerVisual->setMoving(_localPlayer->isMoving() || _isPathMoving);
		_playerVisual->setDirection(_localPlayer->getDirection());
		_playerVisual->setPosition(_localPlayer->getPosition());
	}

	if (_isPathMoving)
	{
		updatePathMovement(delta);
	}
	else
	{
		handleKeyboardInput(delta);
	}
	// 更新摄像机，跟随玩家
	auto camera = Camera::getDefaultCamera();
	if (camera && _localPlayer)
	{
		float zoom = camera->getScaleX();
		Size visibleSize = Director::getInstance()->getVisibleSize();

		float halfWidth = (visibleSize.width / 2) / zoom;
		float halfHeight = (visibleSize.height / 2) / zoom;

		Vec2 targetPos = _localPlayer->getPosition();

		// 限制摄像机边界
		float minX = halfWidth;
		float maxX = GameConfig::getWorldWidth() - halfWidth;
		float minY = halfHeight;
		float maxY = GameConfig::getWorldHeight() - halfHeight;

		// 如果地图比屏幕小，则居中
		if (minX > maxX)
		{
			targetPos.x = GameConfig::getWorldWidth() / 2;
		}
		else
		{
			targetPos.x = std::max(minX, std::min(targetPos.x, maxX));
		}

		if (minY > maxY)
		{
			targetPos.y = GameConfig::getWorldHeight() / 2;
		}
		else
		{
			targetPos.y = std::max(minY, std::min(targetPos.y, maxY));
		}
		if (_needResetCamera)
		{
			camera->setPosition(targetPos);
			_needResetCamera = false;
		}
		else {
			Vec2 currentPos = camera->getPosition();
			Vec2 newPos = currentPos.lerp(targetPos, 0.2f);
			newPos.x = floorf(newPos.x);
			newPos.y = floorf(newPos.y);
			camera->setPosition(newPos);
		}
	}
	_syncTimer += delta;
	float syncInterval = (_localPlayer->isMoving() || _isPathMoving) ?
		GameConfig::SYNC_INTERVAL_MOVING : GameConfig::SYNC_INTERVAL_IDLE;

	if (_syncTimer >= syncInterval)
	{
		//sendPositionUpdate();
		_syncTimer = 0;
	}
	updateUI(delta);

	// 每帧检测传送门
	static float checkTimer = 0;
	checkTimer += delta;
	if (checkTimer >= 0.1f)  // 每0.1秒检测一次
	{
		checkTimer = 0;
		checkPortalTrigger();
	}
}
//键盘按键移动
void GameScene::handleKeyboardInput(float delta)
{
	Vec2 input = Vec2::ZERO;

	if (isKeyPressed(EventKeyboard::KeyCode::KEY_W)) input.y += 1;
	if (isKeyPressed(EventKeyboard::KeyCode::KEY_S)) input.y -= 1;
	if (isKeyPressed(EventKeyboard::KeyCode::KEY_A)) input.x -= 1;
	if (isKeyPressed(EventKeyboard::KeyCode::KEY_D)) input.x += 1;
	if (isKeyPressed(EventKeyboard::KeyCode::KEY_UP_ARROW)) input.y += 1;
	if (isKeyPressed(EventKeyboard::KeyCode::KEY_DOWN_ARROW)) input.y -= 1;
	if (isKeyPressed(EventKeyboard::KeyCode::KEY_LEFT_ARROW)) input.x -= 1;
	if (isKeyPressed(EventKeyboard::KeyCode::KEY_RIGHT_ARROW)) input.x += 1;

	if (input.length() > 0)
	{
		input.normalize();
		Vec2 newPos = _localPlayer->getPosition() + input * _localPlayer->getSpeed() * delta;
		newPos.x = std::max(0.0f, std::min(newPos.x, (float)GameConfig::getWorldWidth()));
		newPos.y = std::max(0.0f, std::min(newPos.y, (float)GameConfig::getWorldHeight()));


		int tileSize = GameConfig::TILE_SIZE;
		Vec2 targetGrid = Vec2((int)(newPos.x / tileSize), (int)(newPos.y / tileSize));

		if (_mapManager->isWalkable(targetGrid))
		{
			_localPlayer->moveTo(newPos);
		}
		// 更新方向（用于眼珠）
		float angle = CC_RADIANS_TO_DEGREES(atan2f(input.y, input.x));
		_localPlayer->setDirection(angle);
	}
}

void GameScene::sendPositionUpdate()
{
	if (!_localPlayer) return;

	Vec2 pos = _localPlayer->getPosition();
	float dir = _localPlayer->getDirection();

	if (pos.distance(_lastSentPosition) > 5.0f)
	{
		_network->sendPlayerMove(pos.x, pos.y, dir);
		_lastSentPosition = pos;
	}
}

void GameScene::updateUI(float delta)
{

	if (_levelLabel)
	{
		_levelLabel->setString(StringUtils::format("Level: %d", _localPlayer->getLevel()));
	}

	// 更新生命值显示
	if (_hpBar && _localPlayer)
	{
		int percent = (int)(_localPlayer->getHpPercent() * 100);
		_hpBar->setPercent(percent);
	}

	if (_hpLabel && _localPlayer)
	{
		_hpLabel->setString(StringUtils::format("HP: %d/%d",
			_localPlayer->getHp(), _localPlayer->getMaxHp()));

		// 根据血量百分比改变颜色
		float percent = _localPlayer->getHpPercent();
		if (percent > 0.6f)
			_hpLabel->setColor(Color3B::GREEN);
		else if (percent > 0.3f)
			_hpLabel->setColor(Color3B::YELLOW);
		else
			_hpLabel->setColor(Color3B::RED);
	}

	// 更新经验值显示
	if (_expBar && _localPlayer)
	{
		int percent = (int)(_localPlayer->getExpPercent() * 100);
		_expBar->setPercent(percent);
	}

	if (_expLabel && _localPlayer)
	{
		_expLabel->setString(StringUtils::format("EXP: %ld/%ld",
			_localPlayer->getExp(), _localPlayer->getMaxExp()));
	}

	// 网络状态
	if (_network && _network->isConnected())
	{
		_statusLabel->setString(u8"在线|");
	}
	else
	{
		_statusLabel->setString(u8"离线|");
		_statusLabel->setColor(Color3B::RED);
	}
	updateCoordDisplay();
}
//鼠标点击
bool GameScene::onTouchBegan(Touch* touch, Event* event)
{
	// 屏幕坐标
	Vec2 screenPos = touch->getLocation();

	auto camera = Camera::getDefaultCamera();
	if (!camera) return true;

	// 窗口大小
	Size visibleSize = Director::getInstance()->getVisibleSize();  // 1024x600
	float zoom = camera->getScaleX();  // 0.4
	Vec2 cameraPos = camera->getPosition();

	// 屏幕坐标转世界坐标
	float worldX = cameraPos.x + (screenPos.x - visibleSize.width / 2) / zoom;
	float worldY = cameraPos.y + (screenPos.y - visibleSize.height / 2) / zoom;

	int tileSize = GameConfig::TILE_SIZE;  // 32
	int gridX = (int)(worldX / tileSize);
	int gridY = (int)(worldY / tileSize);

	CCLOG(u8"屏幕:(%.0f,%.0f) -> 世界:(%.0f,%.0f) -> 网格:(%d,%d)",
		screenPos.x, screenPos.y, worldX, worldY, gridX, gridY);

	if (gridX >= 0 && gridX < GameConfig::MAP_WIDTH &&
		gridY >= 0 && gridY < GameConfig::MAP_HEIGHT)
	{
		requestPathTo(Vec2(gridX, gridY));
	}
	return true;
}
void GameScene::startPathMovement(const std::vector<Vec2>& pixelPath)
{
	if (pixelPath.size() < 2) return;

	_currentPath = pixelPath;
	_currentPathIndex = 1;
	_isPathMoving = true;
	_targetGridPosition = _currentPath.back();

}

void GameScene::updatePathMovement(float delta)
{
	if (!_isPathMoving || _currentPath.empty() || _currentPathIndex >= (int)_currentPath.size())
	{
		_isPathMoving = false;
		if (_mapManager) _mapManager->clearPath();
		return;
	}

	Vec2 targetPos = _currentPath[_currentPathIndex];
	Vec2 currentPos = _localPlayer->getPosition();
	Vec2 deltaPos = targetPos - currentPos;
	float distance = deltaPos.length();

	// 提前切换阈值（8-10像素）
	float arrivalThreshold = 10.0f;

	if (distance <= arrivalThreshold)
	{
		// 提前切换到下一个路径点
		_currentPathIndex++;

		// 如果还有下一个点，继续移动
		if (_currentPathIndex < (int)_currentPath.size())
		{
			// 重新计算方向和距离
			targetPos = _currentPath[_currentPathIndex];
			deltaPos = targetPos - currentPos;
			distance = deltaPos.length();
		}
		else
		{
			// 到达终点，停止移动
			_isPathMoving = false;
			if (_mapManager) _mapManager->clearPath();

			return;
		}
	}

	float speed = _localPlayer->getSpeed();
	float moveDistance = speed * delta;

	if (moveDistance >= distance)
	{
		_localPlayer->setPositionImmediate(targetPos);
		_currentPathIndex++;
		return;
	}

	Vec2 direction = deltaPos / distance;
	Vec2 newPos = currentPos + direction * moveDistance;
	_localPlayer->setPosition(newPos);
	// 更新方向（用于眼珠）
	float angle = CC_RADIANS_TO_DEGREES(atan2f(direction.y, direction.x));
	_localPlayer->setDirection(angle);
}

void GameScene::requestPathTo(const Vec2& targetGrid)
{
	if (!_mapManager || !_mapManager->isWalkable(targetGrid))
	{
		CCLOG(u8"目标位置不可行走: (%.0f,%.0f)", targetGrid.x, targetGrid.y);
		return;
	}

	Vec2 currentGrid = _localPlayer->getGridPosition();

	if (currentGrid.x == targetGrid.x && currentGrid.y == targetGrid.y)
	{
		CCLOG(u8"已经在目标位置");
		return;
	}

	CCLOG(u8"寻路: (%d,%d) -> (%d,%d)",
		(int)currentGrid.x, (int)currentGrid.y,
		(int)targetGrid.x, (int)targetGrid.y);

	// 使用网格坐标 A* 寻路
	auto gridPath = _mapManager->findPath(currentGrid, targetGrid);

	if (gridPath && gridPath->size() > 0)
	{
		std::vector<Vec2> pixelPath;
		int tileSize = GameConfig::TILE_SIZE;

		for (const auto& grid : *gridPath)
		{
			pixelPath.push_back(Vec2(
				grid.x * tileSize + tileSize / 2.0f,
				grid.y * tileSize + tileSize / 2.0f
				));
		}

		startPathMovement(pixelPath);
		delete gridPath;
	}
	else
	{
		CCLOG(u8"无法找到路径");
	}
}
