#ifndef __LOGIN_SCENE_H__
#define __LOGIN_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "WebSocketClient.h"

class LoginScene : public cocos2d::Scene
{
public:
	static cocos2d::Scene* createScene();
	virtual bool init() override;
	CREATE_FUNC(LoginScene);

private:
	// UI 组件
	cocos2d::ui::EditBox* _playerNameInput;
	cocos2d::ui::EditBox* _passwordInput;
	cocos2d::ui::Button* _connectButton;
	cocos2d::Label* _statusLabel;
	// UI 组件
	cocos2d::ui::CheckBox* _localServerCheckBox;
	cocos2d::ui::CheckBox* _remoteServerCheckBox;

	// 获取当前选择的服务器地址
	std::string getSelectedServerAddress();
	// 网络
	WebSocketClient* _network;

	// 状态
	bool _isConnecting;
	bool _isSwitchingScene = false;
	int _loginAttempts;
	LoginResponse* _loginData;  // 保存登录返回的数据

	// UI 初始化
	void setupUI();
	void updateUI();

	// 事件处理
	void onConnectButtonPressed(cocos2d::Ref* sender);
	void onConnected();
	void onLoginSuccess(LoginResponse* resp);
	void onLoginFailed(const std::string& reason);
	void onDisconnected();
	void onError(const std::string& error);

	// 场景切换
	void switchToGameScene();

	// 辅助方法
	std::string getCurrentTime();
};

#endif