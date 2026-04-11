#ifndef __WEBSOCKET_CLIENT_H__
#define __WEBSOCKET_CLIENT_H__

#include "cocos2d.h"
#include "network/WebSocket.h"
#include "Protocol.h"
#include <functional>
#include <queue>
#include <mutex>

class WebSocketClient : public cocos2d::network::WebSocket::Delegate
{
public:
	static WebSocketClient* getInstance();

	// 连接管理
	void connect(const std::string& url);
	void disconnect();
	bool isConnected() const { return _connected; }
	// 获取当前玩家ID
	int getPlayerId() const { return _playerId; }
	// 发送消息
	//void sendMessage(Message* msg);
	void sendJson(const std::string& json);
	void sendLoginRequest(const std::string& name, const std::string& password);
	void sendPlayerMove(float x, float y, float direction);
	void sendMapDataRequest(int mapId);
	void sendPathRequest(const cocos2d::Vec2& start, const cocos2d::Vec2& target);
	// 任务相关请求
	void sendTalkToNPCRequest(int npcId);
	void sendAcceptQuestRequest(const std::string& questId, int npcId);
	void sendCompleteQuestRequest(const std::string& questId);
	void sendClaimRewardRequest(const std::string& questId);
	void sendQuestListRequest();

	// 设置回调函数，提供给外部类设置要回调的函数。
	void setOnConnected(std::function<void()> callback) { _onConnected = callback; }
	void setOnDisconnected(std::function<void(const std::string&)> callback) { _onDisconnected = callback; }
	void setOnError(std::function<void(const std::string&)> callback) { _onError = callback; }

	void setOnLoginResponse(std::function<void(LoginResponse*)> callback) { _onLoginResponse = callback; }
	//void setOnWorldState(std::function<void(WorldState*)> callback) { _onWorldState = callback; }
	void setOnMapDataResponse(std::function<void(MapDataMessage*)> callback) { _onMapDataResponse = callback; }
	void setOnPathResponse(std::function<void(PathResponse*)> callback) { _onPathResponse = callback; }

	// 任务相关回调
	void setOnAvailableQuests(std::function<void(const rapidjson::Value&)> callback) { _onAvailableQuests = callback; }
	void setOnQuestComplete(std::function<void(const rapidjson::Value&)> callback) { _onQuestComplete = callback; }
	void setOnAcceptQuestResponse(std::function<void(bool success, const std::string& questId)> callback) { _onAcceptQuestResponse = callback; }
	void setOnNPCDialog(std::function<void(int npcId, const std::string& npcName, const std::string& dialog)> callback) { _onNPCDialog = callback; }
protected:
	// WebSocket 回调
	virtual void onOpen(cocos2d::network::WebSocket* ws) override;
	virtual void onMessage(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::Data& data) override;
	virtual void onClose(cocos2d::network::WebSocket* ws) override;
	virtual void onError(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::ErrorCode& error) override;

private:
	WebSocketClient();
	~WebSocketClient();

	static WebSocketClient* _instance;

	cocos2d::network::WebSocket* _webSocket;
	bool _connected;
	int _playerId;

	std::queue<std::string> _sendQueue;
	std::mutex _queueMutex;
	void processMessage(const std::string& json);
	void sendQueuedMessages();
	// 回调函数
	std::function<void()> _onConnected;
	std::function<void(const std::string&)> _onDisconnected;
	std::function<void(const std::string&)> _onError;
	std::function<void(LoginResponse*)> _onLoginResponse;
	//std::function<void(WorldState*)> _onWorldState;
	std::function<void(MapDataMessage*)> _onMapDataResponse;
	std::function<void(PathResponse*)> _onPathResponse;
	std::function<void(const rapidjson::Value&)> _onAvailableQuests;
	std::function<void(const rapidjson::Value&)> _onQuestComplete;
	std::function<void(bool success, const std::string& questId)> _onAcceptQuestResponse;
	std::function<void(int npcId, const std::string& npcName, const std::string& dialog)> _onNPCDialog;

	void handleNPCDialog(const rapidjson::Value& data);
	void handleAvailableQuests(const rapidjson::Value & data);
	void handleQuestComplete(const rapidjson::Value & data);
	void handleAcceptQuestResponse(const rapidjson::Value & data);
	void handleLoginResponse(const rapidjson::Value & data);
	void handleMapDataResponse(const rapidjson::Value & data);

};

#endif