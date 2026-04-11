#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "cocos2d.h"
#include "json/rapidjson.h"
#include "json/document.h"
#include <string>
#include <vector>

// 消息类型枚举
enum class MessageType : int
{
	HEARTBEAT = 99,
	CONNECT_REQUEST = 1,
	CONNECT_RESPONSE = 2,
	PLAYER_MOVE = 3,
	PLAYER_JOIN = 4,
	PLAYER_LEAVE = 5,
	WORLD_STATE = 6,
	CHAT_MESSAGE = 7,
	MAP_DATA_REQUEST = 8,
	PATH_REQUEST = 9,
	PATH_RESPONSE = 10,
	LOGIN_EVENT = 11,
	//任务类
	ACCEPT_QUEST = 200,
	QUEST_PROGRESS = 201,
	COMPLETE_QUEST = 202,
	CLAIM_REWARD = 203,//给奖励
	QUEST_LIST = 204,
	ACCEPT_QUEST_RESPONSE = 210,  // 接受任务响应
	QUEST_PROGRESS_RESPONSE = 211,// 任务进度响应
	COMPLETE_QUEST_RESPONSE = 212,// 完成任务响应
	CLAIM_REWARD_RESPONSE = 213,  // 领取奖励响应
	QUEST_LIST_RESPONSE = 214,    // 任务列表响应

	TALK_TO_NPC = 220,      // 请求
	NPC_DIALOG = 221,       // 响应（普通对话）
	AVAILABLE_QUESTS = 222, // 响应（任务列表）

	QUEST_COMPLETE = 230,  // 任务完成通知
};


/**
* NPC 数据类
*/
struct NPCData
{
public:
	int id;                     // NPC ID
	std::string name;           // NPC 名称
	int x;                      // 网格 X 坐标
	int y;                      // 网格 Y 坐标
	std::string dialog;         // 对话内容
	std::string symbol;         // 显示符号
	int npcType;                // NPC 类型: 0=普通, 1=商人, 2=任务, 3=传送
	bool hasQuest;              // 是否有任务
	std::string questId;  // 任务ID
	std::string questName;      // 任务名称
	std::string questDesc;      // 任务描述

	NPCData()
		: id(0)
		, x(0)
		, y(0)
		, npcType(0)
		, hasQuest(false)
	{
	}

	NPCData(int _id, const std::string& _name, int _x, int _y, const std::string& _dialog, const std::string& _symbol = "??")
		: id(_id)
		, name(_name)
		, x(_x)
		, y(_y)
		, dialog(_dialog)
		, symbol(_symbol)
		, npcType(0)
		, hasQuest(false)
	{
	}

	std::string toString() const
	{
		return cocos2d::StringUtils::format("NPC[%d] %s at (%d,%d): %s",
			id, name.c_str(), x, y, dialog.c_str());
	}
};

// 传送门数据
struct PortalInfo
{
	int id;
	std::string name;
	int x;
	int y;
	int targetMapId;
	int targetX;
	int targetY;
	std::string symbol;
	int portalType;  // 0=入口,1=出口,2=副本,3=传送,4=秘密
	bool isActive;
	int requiredLevel;
	PortalInfo() : id(0), x(0), y(0), targetMapId(0),
		targetX(0), targetY(0), portalType(0),
		isActive(true), requiredLevel(0), symbol("??") {}
	std::string toString() const
	{
		return cocos2d::StringUtils::format("Portal[%d] %s at (%d,%d) -> map%d",
			id, name.c_str(), x, y, targetMapId);
	}
};


struct MapDataMessage
{
public:
	bool success;
	int mapId;
	std::string mapName;
	int width;
	int height;
	std::vector<int> tileData;
	std::vector<NPCData> npcs;
	std::vector<PortalInfo> portals;
	std::string error;

	MapDataMessage()
		: success(false)
		, mapId(0)
		, width(0)
		, height(0)
	{
	}

};

// ==================== 登录响应 ====================

struct LoginResponse
{
public:
	bool success;
	std::string message;
	int playerId;
	std::string playerName;
	int level;
	long exp;
	int mapId;
	float x;
	float y;
	float direction;

	LoginResponse()
		: success(false)
		, playerId(0)
		, level(1)
		, exp(0)
		, mapId(1)
		, x(400)
		, y(300)
		, direction(0)
	{
	}

};

// ==================== 路径响应 ====================

struct PathResponse
{
public:
	bool success;
	std::vector<cocos2d::Vec2> path;
	std::string error;

	PathResponse()
		: success(false)
	{
	}

};

#endif // __PROTOCOL_H__