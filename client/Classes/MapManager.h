#ifndef __MAP_MANAGER_H__
#define __MAP_MANAGER_H__

#include "cocos2d.h"
#include "Protocol.h"
#include <vector>
#include "GameConfig.h"

class MapManager : public cocos2d::Node
{
public:
	static MapManager* create();
	virtual bool init() override;

	void loadMapData(const MapDataMessage& data);
	void loadDefaultMap();

	bool isWalkable(int x, int y) const;
	bool isWalkable(const cocos2d::Vec2& gridPos) const;
	bool isWalkable(int tileType) const;

	int getTileType(int x, int y) const;
	cocos2d::Color4F getTileColor(int tileType) const;
	std::string getTileSymbol(int tileType) const;
	std::string getMapName() const { return _currentMapName; }
	// 获取所有 NPC
	const std::vector<NPCData>& getNPCs() const { return _npcs; }

	// 获取指定位置的 NPC
	NPCData* getNPCAt(int x, int y);
	const PortalInfo* getPortalAt(int x, int y) const;
	const std::vector<PortalInfo>& getPortals() const { return _portals; }
	// 路径显示
	void setPath(const std::vector<cocos2d::Vec2>& path);
	void clearPath();

	int getWidth() const { return _width; }
	int getHeight() const { return _height; }
	int getTileSize() const { return GameConfig::TILE_SIZE; }
	std::vector<cocos2d::Vec2>* findPath(const cocos2d::Vec2& start, const cocos2d::Vec2& target);

	std::function<void(const PortalInfo&)> _onPortalTouch;
	void setOnPortalTouch(std::function<void(const PortalInfo&)> callback) { _onPortalTouch = callback; }

private:
	void drawTile(int tileType, const cocos2d::Vec2& pos, const cocos2d::Vec2& center, int tileSize);
	void drawGrass(cocos2d::DrawNode* drawNode, const cocos2d::Vec2& center, int tileSize);
	void drawPath(cocos2d::DrawNode* drawNode, const cocos2d::Vec2& center, int tileSize);
	void drawWater(cocos2d::DrawNode* drawNode, const cocos2d::Vec2& center, int tileSize);
	void drawWall(cocos2d::DrawNode* drawNode, const cocos2d::Vec2& pos, int tileSize);
	void drawBuilding(cocos2d::DrawNode* drawNode, const cocos2d::Vec2& pos, const cocos2d::Vec2& center, int tileSize);
	void drawTree(cocos2d::DrawNode* drawNode, const cocos2d::Vec2& center, int tileSize);
	void drawFlower(cocos2d::DrawNode* drawNode, const cocos2d::Vec2& center, int tileSize);
	void drawWell(cocos2d::DrawNode* drawNode, const cocos2d::Vec2& center, int tileSize);
	void drawEntrance(cocos2d::DrawNode* drawNode, const cocos2d::Vec2& pos, int tileSize);
	void drawStone(cocos2d::DrawNode* drawNode, const cocos2d::Vec2& center, int tileSize);
	void drawBench(cocos2d::DrawNode* drawNode, const cocos2d::Vec2& pos, int tileSize);
	int _width;
	int _height;
	std::string _currentMapName;
	std::vector<std::vector<int>> _tileData;
	std::vector<NPCData> _npcs;
	std::vector<PortalInfo> _portals;

	std::vector<cocos2d::Vec2> _currentPath;
	cocos2d::Vec2 _targetPosition;

	void updateTileDisplay();
	void createNPCNodes();
	void createPortalNodes();
	void onPortalTouched(const PortalInfo& portal);
	// 启发函数
	int heuristic(const cocos2d::Vec2& a, const cocos2d::Vec2& b);
};

#endif