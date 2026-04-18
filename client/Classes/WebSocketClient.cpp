#include "WebSocketClient.h"
#include "JsonHelper.h"
#include "GameConfig.h"

WebSocketClient* WebSocketClient::_instance = nullptr;

WebSocketClient* WebSocketClient::getInstance()
{
	if (!_instance)
	{
		_instance = new WebSocketClient();
	}
	return _instance;
}

WebSocketClient::WebSocketClient()
	: _webSocket(nullptr)
	, _connected(false)
	, _playerId(0)
{
}

WebSocketClient::~WebSocketClient()
{
	disconnect();
}

void WebSocketClient::connect(const std::string& url)
{
	CCLOG("WebSocketClient connect: %s", url.c_str());

	if (_webSocket != nullptr)
	{
		disconnect();
	}

	_webSocket = new cocos2d::network::WebSocket();
	_webSocket->init(*this, url);
}

void WebSocketClient::disconnect()
{
	if (_webSocket)
	{
		auto state = _webSocket->getReadyState();
		if (state != cocos2d::network::WebSocket::State::CLOSED)
		{
			_webSocket->close();
		}
		// 不在这里 delete，等 onClose 回调
	}

	_connected = false;
	_playerId = 0;
}
//void WebSocketClient::sendMessage(Message* msg)
//{
//    if (!_connected)
//    {
//        CCLOG("Not connected, message queued");
//        _sendQueue.push(msg->toJson());
//        return;
//    }
//    
//    std::string json = msg->toJson();
//    _webSocket->send(json);
//    CCLOG("Send: %s", json.c_str());
//    delete msg;
//}

void WebSocketClient::sendJson(const std::string& json)
{
	CCLOG("sendJson called, _webSocket=%p, _connected=%d", _webSocket, _connected);
	if (_webSocket && _connected)
	{
		_webSocket->send(json);
		return;
	}
}
void WebSocketClient::sendTalkToNPCRequest(int npcId)
{
	rapidjson::Document doc;
	doc.SetObject();
	auto& allocator = doc.GetAllocator();

	doc.AddMember("type", (int)MessageType::TALK_TO_NPC, allocator);

	rapidjson::Value data(rapidjson::kObjectType);
	data.AddMember("playerId", _playerId, allocator);  // 使用保存的 _playerId
	data.AddMember("npcId", npcId, allocator);
	doc.AddMember("data", data, allocator);

	sendJson(JsonHelper::toString(doc));
}

void WebSocketClient::sendAcceptQuestRequest(const std::string& questId, int npcId)
{
	rapidjson::Document doc;
	doc.SetObject();
	auto& allocator = doc.GetAllocator();

	doc.AddMember("type", (int)MessageType::ACCEPT_QUEST, allocator);

	rapidjson::Value data(rapidjson::kObjectType);
	data.AddMember("playerId", _playerId, allocator);
	data.AddMember("questId", rapidjson::Value(questId.c_str(), allocator), allocator);
	data.AddMember("npcId", npcId, allocator);
	doc.AddMember("data", data, allocator);

	sendJson(JsonHelper::toString(doc));
}

void WebSocketClient::sendCompleteQuestRequest(const std::string& questId)
{
	rapidjson::Document doc;
	doc.SetObject();
	auto& allocator = doc.GetAllocator();

	doc.AddMember("type", (int)MessageType::COMPLETE_QUEST, allocator);

	rapidjson::Value data(rapidjson::kObjectType);
	data.AddMember("playerId", _playerId, allocator);
	data.AddMember("questId", rapidjson::Value(questId.c_str(), allocator), allocator);
	doc.AddMember("data", data, allocator);

	sendJson(JsonHelper::toString(doc));
}
void WebSocketClient::sendLoginRequest(const std::string& name, const std::string& password)
{
	rapidjson::Document doc;
	doc.SetObject();
	auto& allocator = doc.GetAllocator();

	doc.AddMember("type", (int)MessageType::LOGIN_EVENT, allocator);

	rapidjson::Value data(rapidjson::kObjectType);
	data.AddMember("playerName", rapidjson::Value(name.c_str(), allocator), allocator);
	data.AddMember("password", rapidjson::Value(password.c_str(), allocator), allocator);
	doc.AddMember("data", data, allocator);

	sendJson(JsonHelper::toString(doc));
}

void WebSocketClient::sendPlayerMove(float x, float y, float direction)
{
	rapidjson::Document doc;
	doc.SetObject();
	auto& allocator = doc.GetAllocator();

	doc.AddMember("type", (int)MessageType::PLAYER_MOVE, allocator);

	rapidjson::Value data(rapidjson::kObjectType);
	data.AddMember("playerId", _playerId, allocator);
	data.AddMember("x", x, allocator);
	data.AddMember("y", y, allocator);
	data.AddMember("direction", direction, allocator);
	doc.AddMember("data", data, allocator);

	sendJson(JsonHelper::toString(doc));
}

void WebSocketClient::sendMapDataRequest(int mapId)
{
	rapidjson::Document doc;
	doc.SetObject();
	auto& allocator = doc.GetAllocator();

	doc.AddMember("type", (int)MessageType::MAP_DATA_REQUEST, allocator);

	rapidjson::Value data(rapidjson::kObjectType);
	data.AddMember("mapId", mapId, allocator);
	doc.AddMember("data", data, allocator);

	sendJson(JsonHelper::toString(doc));
}

void WebSocketClient::sendPathRequest(const cocos2d::Vec2& start, const cocos2d::Vec2& target)
{
	rapidjson::Document doc;
	doc.SetObject();
	auto& allocator = doc.GetAllocator();

	doc.AddMember("type", (int)MessageType::PATH_REQUEST, allocator);

	rapidjson::Value data(rapidjson::kObjectType);
	data.AddMember("startX", (int)start.x, allocator);
	data.AddMember("startY", (int)start.y, allocator);
	data.AddMember("targetX", (int)target.x, allocator);
	data.AddMember("targetY", (int)target.y, allocator);
	doc.AddMember("data", data, allocator);

	sendJson(JsonHelper::toString(doc));
}
//第一次握手连接成功
//发送字节 (224 bytes): 47 45 54 20 2F 77 73 20 48 54 54 50 2F 31 2E 31 0D 0A 50 72 61 67 6D 61 3A 20 6E 6F 2D 63 61 63 68 65 0D 0A 43 61 63 68 65 2D 43 6F 6E 74 72 6F 6C 3A 20 6E 6F 2D 63 61 63 68 65 0D 0A 48 6F 73 74 3A 20 6C 6F 63 61 6C 68 6F 73 74 0D 0A 4F 72 69 67 69 6E 3A 20 68 74 74 70 3A 2F 2F 6C 6F 63 61 6C 68 6F 73 74 3A 38 30 38 30 0D 0A 55 70 67 72 61 64 65 3A 20 77 65 62 73 6F 63 6B 65 74 0D 0A 43 6F 6E 6E 65 63 74 69 6F 6E 3A 20 55 70 67 72 61 64 65 0D 0A 53 65 63 2D 57 65 62 53 6F 63 6B 65 74 2D 4B 65 79 3A 20 4B 53 4F 2B 68 4F 46 73 31 71 35 53 6B 45 6E 78 38 62 76 70 36 77 3D 3D 0D 0A 53 65 63 2D 57 65 62 53 6F 63 6B 65 74 2D 56 65 72 73 69 6F 6E 3A 20 31 33 0D 0A 0D 0A
void WebSocketClient::onOpen(cocos2d::network::WebSocket* ws)
{
	CCLOG("WebSocketClient opened");
	_connected = true;

	//执行回调函数，_onConnected引用的是外部类传递过来的方法。
	//这里是LoginScene::onConnected()


	//sendQueuedMessages();
	if (_onConnected) _onConnected();
}

void WebSocketClient::onMessage(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::Data& data)
{
	std::string message((char*)data.bytes, data.len);
	CCLOG("WebSocketClient.Received: %s", message.c_str());
	processMessage(message);
}

void WebSocketClient::onClose(cocos2d::network::WebSocket* ws)
{
	CCLOG("WebSocketClient closed");
	if (_webSocket == ws)
	{
		delete _webSocket;
		_webSocket = nullptr;
	}

	_connected = false;

	if (_onDisconnected) _onDisconnected();
}

void WebSocketClient::onError(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::ErrorCode& error)
{
	_connected = false;
	if (_onError) _onError(u8"网络错误");
}

void WebSocketClient::processMessage(const std::string& json)
{
	rapidjson::Document doc;
	doc.Parse(json.c_str());

	if (doc.HasParseError())
	{
		CCLOG("[processMessage]JSON parse error");
		return;
	}

	int type = doc["type"].GetInt();
	const rapidjson::Value& data = doc["data"];

	switch ((MessageType)type)
	{
	case MessageType::LOGIN_EVENT:  // 11 - 登录响应
		handleLoginResponse(data);
		break;

	case MessageType::MAP_DATA_REQUEST:  // 8 - 地图数据响应
		handleMapDataResponse(data);
		break;

		//case MessageType::WORLD_STATE:  // 6
		//	//handleWorldState(data);
		//	break;
	case MessageType::ACCEPT_QUEST_RESPONSE:
		handleAcceptQuestResponse(data);
		break;
	case MessageType::AVAILABLE_QUESTS:  // 可接收任务
		handleAvailableQuests(data);
		break;

	case MessageType::QUEST_COMPLETE:  // 230 - 任务完成通知
		handleQuestComplete(data);
		break;

	case MessageType::NPC_DIALOG:  // 221 - NPC对话响应
		handleNPCDialog(data);
		break;

	default:
		CCLOG("Unknown message type: %d", type);
		break;
	}
}
// 添加任务响应处理
void WebSocketClient::handleAvailableQuests(const rapidjson::Value& data)
{
	const rapidjson::Value& quests = data["quests"];

	if (_onAvailableQuests)
	{
		_onAvailableQuests(quests);
	}
}

void WebSocketClient::handleQuestComplete(const rapidjson::Value& data)
{

	if (_onQuestComplete)
	{
		_onQuestComplete(data);
	}
}

void WebSocketClient::handleAcceptQuestResponse(const rapidjson::Value& data)
{
	bool success = data["success"].GetBool();

	if (success && _onAcceptQuestResponse)
	{
		Quest quest;
		quest.id = data["questId"].GetString();
		quest.name = data["questName"].GetString();
		quest.description = data["description"].GetString();
		quest.targetNPCId = data["targetNPCId"].GetInt();

		if (data.HasMember("requiredCount"))
			quest.requiredCount = data["requiredCount"].GetInt();
		if (data.HasMember("type"))
			quest.type = (QuestType)data["type"].GetInt();
		if (data.HasMember("rewards"))
		{
			const auto& rewards = data["rewards"];
			if (rewards.HasMember("exp"))
				quest.rewards["exp"] = rewards["exp"].GetInt();
			if (rewards.HasMember("gold"))
				quest.rewards["gold"] = rewards["gold"].GetInt();
		}

		_onAcceptQuestResponse(success, quest);
	}
	else if (_onAcceptQuestResponse)
	{
		Quest emptyQuest;
		_onAcceptQuestResponse(success, emptyQuest);
	}
}
void WebSocketClient::handleLoginResponse(const rapidjson::Value& data)
{
	LoginResponse* resp = new LoginResponse();
	resp->success = data["success"].GetBool();

	if (resp->success)
	{
		resp->playerId = data["playerId"].GetInt();
		resp->playerName = data["playerName"].GetString();
		resp->level = data["level"].GetInt();
		resp->exp = data["exp"].GetInt64();
		resp->mapId = data["mapId"].GetInt();
		resp->x = data["x"].GetFloat();
		resp->y = data["y"].GetFloat();
		resp->direction = data["direction"].GetFloat();
		_playerId = resp->playerId;
	}
	else
	{
		resp->message = data["message"].GetString();
	}

	if (_onLoginResponse) _onLoginResponse(resp);
	//delete resp;其他对象使用完后负责释放
}

void WebSocketClient::handleMapDataResponse(const rapidjson::Value& data)
{
	MapDataMessage* mapData = new MapDataMessage();
	mapData->success = data["success"].GetBool();

	if (mapData->success)
	{
		mapData->mapId = data["mapId"].GetInt();
		mapData->mapName = data["mapName"].GetString();
		mapData->width = data["width"].GetInt();
		mapData->height = data["height"].GetInt();

		const rapidjson::Value& tileData = data["tileData"];
		for (rapidjson::SizeType i = 0; i < tileData.Size(); i++)
		{
			mapData->tileData.push_back(tileData[i].GetInt());
		}

		const rapidjson::Value& npcs = data["npcs"];
		for (rapidjson::SizeType i = 0; i < npcs.Size(); i++)
		{
			NPCData npc;
			npc.id = npcs[i]["id"].GetInt();
			npc.name = npcs[i]["name"].GetString();
			npc.x = npcs[i]["x"].GetInt();
			npc.y = npcs[i]["y"].GetInt();
			npc.dialog = npcs[i]["dialog"].GetString();
			npc.symbol = npcs[i]["symbol"].GetString();
			mapData->npcs.push_back(npc);
		}
		//解析传送门数据
		if (data.HasMember("portals") && data["portals"].IsArray())
		{
			const rapidjson::Value& portals = data["portals"];
			for (rapidjson::SizeType i = 0; i < portals.Size(); i++)
			{
				PortalInfo portal;
				portal.id = portals[i]["id"].GetInt();
				portal.name = portals[i]["name"].GetString();
				portal.x = portals[i]["x"].GetInt();
				portal.y = portals[i]["y"].GetInt();
				portal.targetMapId = portals[i]["targetMapId"].GetInt();
				portal.targetX = portals[i]["targetX"].GetInt();
				portal.targetY = portals[i]["targetY"].GetInt();

				if (portals[i].HasMember("symbol") && portals[i]["symbol"].IsString())
					portal.symbol = portals[i]["symbol"].GetString();

				if (portals[i].HasMember("portalType") && portals[i]["portalType"].IsInt())
					portal.portalType = portals[i]["portalType"].GetInt();

				if (portals[i].HasMember("isActive") && portals[i]["isActive"].IsBool())
					portal.isActive = portals[i]["isActive"].GetBool();

				if (portals[i].HasMember("requiredLevel") && portals[i]["requiredLevel"].IsInt())
					portal.requiredLevel = portals[i]["requiredLevel"].GetInt();

				mapData->portals.push_back(portal);
			}
		}
		CCLOG("Map data loaded: %s (%dx%d), NPCs: %d, Portals: %d",
			mapData->mapName.c_str(), mapData->width, mapData->height,
			(int)mapData->npcs.size(), (int)mapData->portals.size());
	}
	else
	{
		mapData->error = data["error"].GetString();
		CCLOG("Map data error: %s", mapData->error.c_str());
	}

	if (_onMapDataResponse) _onMapDataResponse(mapData);
	delete mapData;
}
void WebSocketClient::handleNPCDialog(const rapidjson::Value & data)
{
	int npcId = 0;
	std::string npcName;
	std::string dialog;
	std::string symbol;

	if (data.HasMember("npcId") && data["npcId"].IsInt())
	{
		npcId = data["npcId"].GetInt();
	}

	if (data.HasMember("npcName") && data["npcName"].IsString())
	{
		npcName = data["npcName"].GetString();
	}

	if (data.HasMember("dialog") && data["dialog"].IsString())
	{
		dialog = data["dialog"].GetString();
	}

	if (data.HasMember("symbol") && data["symbol"].IsString())
	{
		symbol = data["symbol"].GetString();
	}

	CCLOG(u8"收到NPC对话: %s - %s", npcName.c_str(), dialog.c_str());

	if (_onNPCDialog)
	{
		_onNPCDialog(npcId, npcName, dialog);
	}
}
void WebSocketClient::sendQueuedMessages()
{

	std::lock_guard<std::mutex> lock(_queueMutex);
	while (!_sendQueue.empty())
	{
		std::string json = _sendQueue.front();
		_sendQueue.pop();
		_webSocket->send(json);
	}
}