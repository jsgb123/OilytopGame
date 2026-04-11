#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "WebSocketClient.h"
#include "Player.h"
#include "MapManager.h"
#include "PlayerVisual.h"
#include "Protocol.h"
using namespace cocos2d;
class GameScene : public cocos2d::Scene
{
public:
	static cocos2d::Scene* createScene();
	static GameScene* createWithPlayerData(LoginResponse* playerData);
	virtual bool init() override;

	void interactWithNearbyNPC();

	void showQuestDialog(NPCData * npc, const rapidjson::Value & quests);

	void showNPCDialog(NPCData * npc);

	void showSimpleDialog(const std::string & npcName, const std::string & dialog);

	void showMessage(const std::string & text, const Color3B & color);

	void setupUIFixed();

	CREATE_FUNC(GameScene);

	void initWithPlayerData(LoginResponse* playerData);

private:
	bool _needResetCamera = false;
	bool _cameraInitialized = false;
	bool _isOnPortal;           // 是否站在传送门上
	int _currentPortalId;       // 当前传送门ID
	float _portalStayTime;      // 已停留时间
	float _portalRequiredTime;  // 需要停留的时间（秒）
	bool _portalTriggered;      // 是否已触发过（防止重复触发）

	NPCData* _currentInteractNPC;  // 当前交互的 NPC
	void acceptQuest(NPCData* npc);
	void checkPortalTrigger();
	void onPortalTrigger(const PortalInfo& portal);
	int _lastTriggeredPortalId;  // 避免重复触发
	float _portalTriggerCooldown;// 冷却时间

	// 玩家数据
	int _playerId;
	std::string _playerName;
	int _playerLevel;
	float _playerX;
	float _playerY;
	float _playerDirection;
	PlayerVisual* _playerVisual;  
	// 网络
	WebSocketClient* _network;

	// 游戏组件
	MapManager* _mapManager;
	Player* _localPlayer;

	// UI 组件
	cocos2d::ui::Layout* _currentDialogBg = nullptr;
	cocos2d::Label* _statusLabel;
	cocos2d::Label* _playerInfoLabel;
	cocos2d::Label* _levelLabel;
	cocos2d::Label* _fpsLabel;
	cocos2d::ui::Button* _disconnectBtn;
	cocos2d::Label* _coordLabel;  // 坐标显示标签
	cocos2d::Label* _mapNameLabel;  // 地图名称标签
	void updateCoordDisplay();     // 更新坐标显示
	cocos2d::ui::LoadingBar* _expBar;
	cocos2d::Label* _expLabel;
	cocos2d::ui::LoadingBar* _hpBar;
	cocos2d::Label* _hpLabel;
	void updatePlayerStats(int hp, int maxHp, int exp, int maxExp);
	// 任务面板相关
	cocos2d::ui::Layout* _questPanel;
	void updateQuestPanel();
	void showQuestPanel();
	void hideQuestPanel();
	// 键盘状态
	std::unordered_map<cocos2d::EventKeyboard::KeyCode, bool> _keyStates;
	bool isKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode) const;
	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
	void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
	// 同步相关
	float _moveTimer;
	float _syncTimer;
	cocos2d::Vec2 _lastSentPosition;

	// 寻路相关
	std::vector<cocos2d::Vec2> _currentPath;
	int _currentPathIndex;
	bool _isPathMoving;
	cocos2d::Vec2 _targetGridPosition;
	void setupCallbacks();
	void updateUI(float delta);
	cocos2d::Color4F getColorByLevel(int level);

	void onLoginResponse(LoginResponse* resp);
//	void onWorldState(WorldState* state);
	void onMapDataResponse(MapDataMessage* data);
	void onPathResponse(PathResponse* resp);

	void sendPositionUpdate();
	void handleKeyboardInput(float delta);

	void requestPathTo(const cocos2d::Vec2& targetGrid);
	void startPathMovement(const std::vector<cocos2d::Vec2>& path);
	void updatePathMovement(float delta);

	bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);

	void updatePlayerSpriteAppearance(Player* player);


	virtual void update(float delta) override;
};

#endif