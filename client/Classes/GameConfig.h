#ifndef __GAME_CONFIG_H__
#define __GAME_CONFIG_H__

#include "cocos2d.h"

class GameConfig
{
public:
	// 地图配置
	static const int TILE_SIZE = 32;
	static const int MAP_WIDTH = 80;
	static const int MAP_HEIGHT = 60;

	// 网络配置
	static const std::string SERVER_URL;
	static const int RECONNECT_INTERVAL = 3000;

	// 玩家配置
	static const float PLAYER_SPEED;
	static const float SYNC_INTERVAL_MOVING;
	static const float SYNC_INTERVAL_IDLE;

	// 世界大小（像素）
	static int getWorldWidth() { return MAP_WIDTH * TILE_SIZE; }
	static int getWorldHeight() { return MAP_HEIGHT * TILE_SIZE; }


	// 字体文件路径
	static const std::string FONT_ARIAL;           // Arial, 中文不友好
	static const std::string FONT_SIMHEI;          // 黑体
	static const std::string FONT_SONG;          // 宋体
	static const std::string FONT_MSYAH;           // 微软雅黑
	static const std::string FONT_KAITI;           // 楷体
	static const std::string FONT_FANGSONG;        // 仿宋
};
// NPC 类型常量
class NPCType
{
public:
	static const int NORMAL = 0;    // 普通NPC
	static const int MERCHANT = 1;  // 商人
	static const int QUEST = 2;     // 任务NPC
	static const int PORTAL = 3;    // 传送NPC
	static const int TRAINER = 4;   // 训练师
};

// NPC 符号映射
class NPCSymbol
{
public:
	static const std::string ELDER;      //  村长
	static const std::string MERCHANT;   //  商人
	static const std::string BLACKSMITH; //  铁匠
	static const std::string INNKEEPER;  // 旅店老板
	static const std::string TRAINER;    //  导师
	static const std::string HUNTER;     //  猎户
	static const std::string GUARD;      //  守卫
	static const std::string DEFAULT;    //  默认
};
#endif