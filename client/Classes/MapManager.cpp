#include "MapManager.h"
#include "GameConfig.h"
#include "JsonHelper.h"
#include "ui/CocosGUI.h"
using namespace cocos2d;
// 节点结构
struct AStarNode
{
	Vec2 pos;
	int g;      // 起点到当前点的实际代价
	int h;      // 当前点到终点的估计代价
	int steps;  // 步数

	AStarNode(Vec2 p, int gCost, int hCost, int stepCount)
		: pos(p), g(gCost), h(hCost), steps(stepCount) {}

	int f() const { return g + h; }

	// 优先队列比较（小的优先）
	bool operator>(const AStarNode& other) const
	{
		if (f() != other.f())
			return f() > other.f();
		if (steps != other.steps)
			return steps > other.steps;
		return g > other.g;
	}
};

MapManager* MapManager::create()
{
	MapManager* ret = new MapManager();
	if (ret && ret->init())
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool MapManager::init()
{
	if (!Node::init())
		return false;

	loadDefaultMap();
	return true;
}
//网络地图
void MapManager::loadMapData(const MapDataMessage& data)
{
	_width = data.width;
	_height = data.height;
	_tileData.resize(_width, std::vector<int>(_height, 0));
	_currentMapName = data.mapName;
	// 转换一维数组到二维
	for (int y = 0; y < _height; y++)
	{
		for (int x = 0; x < _width; x++)
		{
			int idx = y * _width + x;
			if (idx < (int)data.tileData.size())
			{
				_tileData[x][y] = data.tileData[idx];
			}
		}
	}

	_npcs = data.npcs;
	_portals = data.portals;

	updateTileDisplay();
	createNPCNodes();
	createPortalNodes();
	CCLOG(u8"地图加载完成: %dx%d, NPC数量: %d, 传送门数量：%d", _width, _height, (int)_npcs.size(), _portals.size());
}
//本地地图
void MapManager::loadDefaultMap()
{
	_width = GameConfig::MAP_WIDTH;
	_height = GameConfig::MAP_HEIGHT;

	_tileData.resize(_width, std::vector<int>(_height, 0));

	// 初始化草地
	for (int x = 0; x < _width; x++)
	{
		for (int y = 0; y < _height; y++)
		{
			_tileData[x][y] = 0;
		}
	}

	// 边界
	for (int x = 0; x < _width; x++)
	{
		_tileData[x][0] = 3;
		_tileData[x][_height - 1] = 3;
	}
	for (int y = 0; y < _height; y++)
	{
		_tileData[0][y] = 3;
		_tileData[_width - 1][y] = 3;
	}

	// 道路（十字形）
	int centerX = _width / 2;
	int centerY = _height / 2;
	for (int x = 1; x < _width-1; x++) _tileData[x][centerY] = 1;
	for (int y = 1; y < _height-1; y++) _tileData[centerX][y] = 1;

	// 建筑
	for (int x = 10; x <= 15; x++)
	{
		for (int y = 8; y <= 12; y++)
		{
			_tileData[x][y] = 4;
		}
	}
	_tileData[12][10] = 8;

	// 杂货店（右上）
	for (int x = _width - 15; x <= _width - 10; x++)
	{
		for (int y = 8; y <= 12; y++)
		{
			_tileData[x][y] = 4;   // BUILDING
		}
	}
	_tileData[_width - 13][10] = 8;  // ENTRANCE

	// 旅馆（左下）
	for (int x = 10; x <= 15; x++)
	{
		for (int y = _height - 12; y <= _height - 8; y++)
		{
			_tileData[x][y] = 4;   // BUILDING
		}
	}
	_tileData[12][_height - 10] = 8;  // ENTRANCE

	// 树林区域
	for (int x = 5; x <= 12; x++)
	{
		for (int y = 20; y <= 28; y++)
		{
			if ((x + y) % 3 == 0)  // 间隔放置
				_tileData[x][y] = 5;   // TREE
		}
	}
	for (int x = 60; x <= 70; x++)
	{
		for (int y = 45; y <= 55; y++)
		{
			if ((x + y) % 3 == 0)
				_tileData[x][y] = 5;   // TREE
		}
	}
	// 单独大树
	_tileData[25][15] = 5;   // TREE
	_tileData[55][40] = 5;   // TREE
	_tileData[18][48] = 5;   // TREE
	_tileData[62][12] = 5;   // TREE


	// 河流（横向）
	for (int x = 20; x <= 60; x++)
	{
		if(_tileData[x][35]==0)
		_tileData[x][35] = 2;   // WATER
	}
	// 池塘（圆形区域）
	for (int x = 45; x <= 52; x++)
	{
		for (int y = 15; y <= 22; y++)
		{
			int dx = x - 48;
			int dy = y - 18;
			if (dx*dx + dy*dy <= 16)
				_tileData[x][y] = 2;   // WATER
		}
	}

	// ========== 花丛 ==========
	for (int x = 30; x <= 38; x++)
	{
		for (int y = 40; y <= 48; y++)
		{
			if ((x + y) % 4 == 0)
				_tileData[x][y] = 6;   // FLOWER
		}
	}
	// ========== 水井 ==========
	_tileData[centerX + 3][centerY + 2] = 7;   // WELL
	_tileData[centerX - 2][centerY - 1] = 7;   // WELL


	_tileData[35][28] = 9;   // STONE
	_tileData[44][32] = 9;   // STONE
	_tileData[52][38] = 9;   // STONE
	_tileData[15][35] = 9;   // STONE
	_tileData[65][20] = 9;   // STONE

							 // ========== 长椅 ==========
	_tileData[36][27] = 10;  // BENCH
	_tileData[42][27] = 10;  // BENCH
	_tileData[centerX-1][centerY + 4] = 10;  // BENCH
	updateTileDisplay();

	CCLOG(u8"默认地图加载完成: %dx%d", _width, _height);
}
NPCData* MapManager::getNPCAt(int x, int y)
{
	for (auto& npc : _npcs)
	{
		if (npc.x == x && npc.y == y)
		{
			return &npc;
		}
	}
	return nullptr;
}
const PortalInfo * MapManager::getPortalAt(int x, int y) const
{
	for (const auto& portal : _portals)
	{
		if (portal.x == x && portal.y == y)
		{
			return &portal;
		}
	}
	return nullptr;
}
//网格坐标
bool MapManager::isWalkable(int x, int y) const
{
	if (x < 0 || x >= _width || y < 0 || y >= _height)
		return false;

	int tileId = _tileData[x][y];
	return isWalkable(tileId);
}

bool MapManager::isWalkable(const cocos2d::Vec2 & gridPos) const
{
	// gridPos 已经是网格坐标 (格子坐标)
	return isWalkable((int)gridPos.x, (int)gridPos.y);
}

int MapManager::getTileType(int x, int y) const
{
	if (x < 0 || x >= _width || y < 0 || y >= _height)
		return 3;
	return _tileData[x][y];
}

Color4F MapManager::getTileColor(int tileType) const
{
	switch (tileType)
	{
	case 0:  // GRASS 草地
		return Color4F(0.4f, 0.6f, 0.3f, 1.0f);
	case 1:  // PATH 道路
		return Color4F(0.6f, 0.5f, 0.3f, 1.0f);
	case 2:  // WATER 水域
		return Color4F(0.2f, 0.4f, 0.8f, 1.0f);
	case 3:  // WALL 墙壁
		return Color4F(0.3f, 0.2f, 0.1f, 1.0f);
	case 4:  // BUILDING 建筑
		return Color4F(0.5f, 0.4f, 0.3f, 1.0f);
	case 5:  // TREE 树木
		return Color4F(0.2f, 0.5f, 0.2f, 1.0f);
	case 6:  // FLOWER 花丛
		return Color4F(0.8f, 0.5f, 0.8f, 1.0f);
	case 7:  // WELL 水井
		return Color4F(0.6f, 0.4f, 0.2f, 1.0f);
	case 8:  // ENTRANCE 入口
		return Color4F(0.7f, 0.5f, 0.2f, 1.0f);
	case 9:  // STONE 石头
		return Color4F(0.5f, 0.5f, 0.5f, 1.0f);
	case 10: // BENCH 长椅
		return Color4F(0.6f, 0.4f, 0.2f, 1.0f);
	default:
		return Color4F(1.0f, 1.0f, 1.0f, 1.0f);
	}
}
void MapManager::updateTileDisplay()
{
	this->removeAllChildren();

	int tileSize = GameConfig::TILE_SIZE;

	for (int y = 0; y < _height; y++)
	{
		for (int x = 0; x < _width; x++)
		{
			int tileType = _tileData[x][y];
			cocos2d::Vec2 pos(x * tileSize, y * tileSize);
			cocos2d::Vec2 center(pos.x + tileSize / 2, pos.y + tileSize / 2);

			// 绘制地形
			drawTile(tileType, pos, center, tileSize);
		}
	}
}

void MapManager::drawTile(int tileType, const cocos2d::Vec2& pos, const cocos2d::Vec2& center, int tileSize)
{
	auto drawNode = DrawNode::create();
	Color4F color = getTileColor(tileType);

	// 绘制背景方块
	cocos2d::Vec2 vertices[4];
	vertices[0] = pos;
	vertices[1] = cocos2d::Vec2(pos.x + tileSize, pos.y);
	vertices[2] = cocos2d::Vec2(pos.x + tileSize, pos.y + tileSize);
	vertices[3] = cocos2d::Vec2(pos.x, pos.y + tileSize);
	drawNode->drawPolygon(vertices, 4, color, 0.0f, Color4F::BLACK);

	// 根据类型绘制图案
	switch (tileType)
	{
	case 0: // 草地 - 加点状小草
		drawGrass(drawNode, center, tileSize);
		break;
	case 1: // 道路 - 加条纹
		drawPath(drawNode, center, tileSize);
		break;
	case 2: // 水域 - 加波浪
		drawWater(drawNode, center, tileSize);
		break;
	case 3: // 墙壁 - 加网格
		drawWall(drawNode, pos, tileSize);
		break;
	case 4: // 建筑 - 加屋顶
		drawBuilding(drawNode, pos, center, tileSize);
		break;
	case 5: // 树木
		drawTree(drawNode, center, tileSize);
		break;
	case 6: // 花丛
		drawFlower(drawNode, center, tileSize);
		break;
	case 7: // 水井
		drawWell(drawNode, center, tileSize);
		break;
	case 8: // 入口
		drawEntrance(drawNode, pos, tileSize);
		break;
	case 9: // 石头
		drawStone(drawNode, center, tileSize);
		break;
	case 10: // 长椅
		drawBench(drawNode, pos, tileSize);
		break;
	default:
		break;
	}

	this->addChild(drawNode);
}

void MapManager::createNPCNodes()
{
	int tileSize = GameConfig::TILE_SIZE;

	for (const auto& npc : _npcs)
	{
		auto label = cocos2d::Label::createWithSystemFont(npc.symbol + "\n" + npc.name, "Arial", 14);
		label->setPosition(cocos2d::Vec2(npc.x * tileSize + tileSize / 2, npc.y * tileSize + tileSize / 2));
		label->setColor(cocos2d::Color3B::BLACK);
		this->addChild(label);
	}
}

void MapManager::createPortalNodes()
{
	int tileSize = GameConfig::TILE_SIZE;

	for (const auto& portal : _portals)
	{
		// 创建传送门节点
		auto portalNode = Node::create();
		portalNode->setPosition(Vec2(portal.x * tileSize + tileSize / 2,
			portal.y * tileSize + tileSize / 2));

		// 绘制传送门图标
		auto drawNode = DrawNode::create();

		// 传送门底座（圆形）
		drawNode->drawSolidCircle(Vec2::ZERO, tileSize / 2, 0, 16,
			Color4F(0.5f, 0.3f, 0.8f, 0.8f));

		// 传送门内圈
		drawNode->drawCircle(Vec2::ZERO, tileSize / 4, 0, 16,
			2.0f,  // 线宽
			Color4F(0.8f, 0.6f, 1.0f, 1.0f));

		portalNode->addChild(drawNode);


		this->addChild(portalNode);

		CCLOG(u8"创建传送门: %s at (%d,%d) -> 地图%d",
			portal.name.c_str(), portal.x, portal.y, portal.targetMapId);
	}
}
void MapManager::onPortalTouched(const PortalInfo& portal)
{
	CCLOG(u8"触碰传送门: %s", portal.name.c_str());

	if (_onPortalTouch)
	{
		_onPortalTouch(portal);
	}
}

void MapManager::setPath(const std::vector<cocos2d::Vec2>& path)
{
	_currentPath = path;
}

void MapManager::clearPath()
{
	_currentPath.clear();
}

std::string MapManager::getTileSymbol(int tileType) const
{
	switch (tileType)
	{
	case 0: return "·";   // GRASS
	case 1: return "·";   // PATH
	case 2: return "≈";   // WATER
	case 3: return "#";   // WALL
	case 4: return "H";   // BUILDING (用 H 代替 ??)
	case 5: return "T";   // TREE (用 T 代替 ??)
	case 6: return "*";   // FLOWER (用 * 代替 ??)
	case 7: return "W";   // WELL (用 W 代替 ?)
	case 8: return ">";   // ENTRANCE (用 > 代替 ??)
	case 9: return "S";   // STONE (用 S 代替 ??)
	case 10: return "B";  // BENCH (用 B 代替 ??)
	default: return "?";
	}
}

bool MapManager::isWalkable(int tileType) const
{
	switch (tileType)
	{
	case 0:  // GRASS
	case 1:  // PATH
	case 6:  // FLOWER
	case 8:  // ENTRANCE
		return true;

	case 2:  // WATER
	case 3:  // WALL
	case 4:  // BUILDING
	case 5:  // TREE
	case 7:  // WELL
	case 9:  // STONE
	case 10: // BENCH
	default:
		return false;
	}
}

std::vector<Vec2>* MapManager::findPath(const Vec2& start, const Vec2& target)
{
	auto result = new std::vector<Vec2>();

	// 检查起点和终点是否有效
	if (!isWalkable(start) || !isWalkable(target))
	{
		CCLOG(u8"起点或终点不可行走");
		return result;
	}

	// 如果起点等于终点
	if (start.x == target.x && start.y == target.y)
	{
		result->push_back(start);
		return result;
	}

	// 优先队列（最小堆）
	std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openSet;
	std::map<Vec2, Vec2> cameFrom;
	std::map<Vec2, int> gScore;
	std::map<Vec2, int> stepCount;
	std::set<Vec2> closedSet;

	// 初始化
	gScore[start] = 0;
	stepCount[start] = 0;
	openSet.push(AStarNode(start, 0, heuristic(start, target), 0));

	// 8方向移动
	int dx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	int dy[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

	// 移动代价：直线10，斜线14
	int moveCosts[8] = { 10, 14, 10, 14, 10, 14, 10, 14 };

	while (!openSet.empty())
	{
		AStarNode current = openSet.top();
		openSet.pop();

		// 跳过已处理的节点
		if (closedSet.find(current.pos) != closedSet.end())
			continue;

		// 到达目标
		if (current.pos.x == target.x && current.pos.y == target.y)
		{
			// 重建路径
			Vec2 node = target;
			while (!(node.x == start.x && node.y == start.y))
			{
				result->push_back(node);
				node = cameFrom[node];
			}
			result->push_back(start);
			std::reverse(result->begin(), result->end());

			CCLOG(u8"路径找到: (%d,%d) -> (%d,%d), 共 %d 步, 代价 %d",
				(int)start.x, (int)start.y,
				(int)target.x, (int)target.y,
				(int)result->size(), gScore[target]);
			return result;
		}

		closedSet.insert(current.pos);

		// 探索邻居
		for (int i = 0; i < 8; i++)
		{
			Vec2 next(current.pos.x + dx[i], current.pos.y + dy[i]);

			// 边界检查
			if (next.x < 0 || next.x >= _width || next.y < 0 || next.y >= _height)
				continue;

			// 可行走检查
			if (!isWalkable(next))
				continue;

			// 斜角移动检查：防止穿墙
			if (i % 2 == 1)
			{
				Vec2 horizontal(current.pos.x + dx[i], current.pos.y);
				Vec2 vertical(current.pos.x, current.pos.y + dy[i]);
				if (!isWalkable(horizontal) || !isWalkable(vertical))
					continue;
			}

			int tentativeG = gScore[current.pos] + moveCosts[i];
			int tentativeSteps = stepCount[current.pos] + 1;

			// 如果找到更好的路径
			if (gScore.find(next) == gScore.end() || tentativeG < gScore[next])
			{
				cameFrom[next] = current.pos;
				gScore[next] = tentativeG;
				stepCount[next] = tentativeSteps;
				int h = heuristic(next, target);
				openSet.push(AStarNode(next, tentativeG, h, tentativeSteps));
			}
		}
	}

	CCLOG(u8"无法找到路径: (%d,%d) -> (%d,%d)",
		(int)start.x, (int)start.y, (int)target.x, (int)target.y);
	delete result;
	return nullptr;
}

int MapManager::heuristic(const Vec2& a, const Vec2& b)
{
	// 切比雪夫距离（支持八方向）
	int dx = abs(a.x - b.x);
	int dy = abs(a.y - b.y);
	return std::max(dx, dy) * 10;
}

// 草地 - 绿色背景 + 小草点
void MapManager::drawGrass(DrawNode* drawNode, const Vec2& center, int tileSize)
{
	for (int i = 0; i < 8; i++)
	{
		float offsetX = (rand() % (tileSize - 8)) - tileSize / 2 + 4;
		float offsetY = (rand() % (tileSize - 8)) - tileSize / 2 + 4;
		drawNode->drawDot(center + Vec2(offsetX, offsetY), 1.5f, Color4F(0.2f, 0.8f, 0.2f, 1.0f));
	}
}

// 道路 - 土色 + 条纹
void MapManager::drawPath(DrawNode* drawNode, const Vec2& center, int tileSize)
{
	drawNode->drawSegment(
		Vec2(center.x - tileSize / 3, center.y),
		Vec2(center.x + tileSize / 3, center.y),
		2.0f,
		Color4F(0.8f, 0.7f, 0.4f, 1.0f)
		);
}

// 水域 - 蓝色
void MapManager::drawWater(DrawNode* drawNode, const Vec2& center, int tileSize)
{
	for (int i = -3; i <= 3; i++)
	{
		float y = center.y + i * 2;
		drawNode->drawSegment(
			Vec2(center.x - tileSize / 3, y),
			Vec2(center.x + tileSize / 3, y),
			1.5f,
			Color4F(0.3f, 0.6f, 1.0f, 1.0f)
			);
	}
}

// 墙壁 - 灰色 + 网格
void MapManager::drawWall(DrawNode* drawNode, const Vec2& pos, int tileSize)
{
	// 横线
	for (int i = 1; i < 4; i++)
	{
		float y = pos.y + i * (tileSize / 4);
		drawNode->drawSegment(
			Vec2(pos.x + 2, y),
			Vec2(pos.x + tileSize - 2, y),
			1.5f,
			Color4F(0.2f, 0.15f, 0.1f, 1.0f)
			);
	}
	// 竖线
	for (int i = 1; i < 4; i++)
	{
		float x = pos.x + i * (tileSize / 4);
		drawNode->drawSegment(
			Vec2(x, pos.y + 2),
			Vec2(x, pos.y + tileSize - 2),
			1.5f,
			Color4F(0.2f, 0.15f, 0.1f, 1.0f)
			);
	}
}

// 建筑 - 棕色 + 屋顶
void MapManager::drawBuilding(DrawNode* drawNode, const Vec2& pos, const Vec2& center, int tileSize)
{
	// 屋顶（三角形）
	Vec2 roof[3] = {
		Vec2(center.x, pos.y + tileSize - 6),
		Vec2(center.x - tileSize / 3, pos.y + tileSize - 12),
		Vec2(center.x + tileSize / 3, pos.y + tileSize - 12)
	};
	drawNode->drawPolygon(roof, 3, Color4F(0.7f, 0.3f, 0.2f, 1.0f), 0.5f, Color4F::BLACK);

	// 门
	drawNode->drawSolidRect(
		Vec2(center.x + 4, pos.y + 2),
		Vec2(center.x - 4, pos.y + 10),
		Color4F(0.4f, 0.3f, 0.2f, 1.0f)
		);
}

// 树木 - 绿色圆形树冠 + 棕色树干
void MapManager::drawTree(DrawNode* drawNode, const Vec2& center, int tileSize)
{
	// 树干
	drawNode->drawSolidRect(
		Vec2(center.x - 3, center.y - 6),
		Vec2(center.x + 3, center.y + 2),
		Color4F(0.5f, 0.3f, 0.1f, 1.0f)
		);
	// 树冠
	drawNode->drawSolidCircle(center, tileSize / 3, 0, 16, Color4F(0.1f, 0.6f, 0.1f, 1.0f));
	drawNode->drawSolidCircle(Vec2(center.x - 3, center.y + 3), tileSize / 4, 0, 16, Color4F(0.1f, 0.6f, 0.1f, 1.0f));
	drawNode->drawSolidCircle(Vec2(center.x + 3, center.y + 3), tileSize / 4, 0, 16, Color4F(0.1f, 0.6f, 0.1f, 1.0f));
}

// 花丛 - 粉色小点
void MapManager::drawFlower(DrawNode* drawNode, const Vec2& center, int tileSize)
{
	for (int i = 0; i < 12; i++)
	{
		float angle = (i / 12.0f) * 360.0f;
		float rad = CC_DEGREES_TO_RADIANS(angle);
		float r = tileSize / 4;
		drawNode->drawDot(
			Vec2(center.x + cosf(rad) * r, center.y + sinf(rad) * r),
			2.0f,
			Color4F(0.9f, 0.5f, 0.9f, 1.0f)
			);
	}
	drawNode->drawDot(center, 3.0f, Color4F(1.0f, 0.8f, 0.2f, 1.0f));
}

// 水井 
void MapManager::drawWell(DrawNode* drawNode, const Vec2& center, int tileSize)
{
	// 蓝色圆点表示水井
	drawNode->drawSolidCircle(center, tileSize / 3, 0, 16, Color4F(0.2f, 0.6f, 1.0f, 0.9f));

	// 白色高光
	drawNode->drawDot(Vec2(center.x - 2, center.y + 2), 2.0f, Color4F::WHITE);
}

// 入口 - 拱形门
void MapManager::drawEntrance(DrawNode* drawNode, const Vec2& pos, int tileSize)
{
	drawNode->drawSolidRect(
		Vec2(pos.x + tileSize / 4, pos.y),
		Vec2(pos.x + tileSize * 3 / 4, pos.y + tileSize / 2),
		Color4F(0.7f, 0.5f, 0.2f, 1.0f)
		);
	// 拱形
	drawNode->drawSolidCircle(
		Vec2(pos.x + tileSize / 2, pos.y + tileSize / 2),
		tileSize / 4,
		0,
		16,
		Color4F(0.7f, 0.5f, 0.2f, 1.0f)
		);
}

// 石头 - 灰色不规则形状
void MapManager::drawStone(DrawNode* drawNode, const Vec2& center, int tileSize)
{
	Vec2 points[6];
	for (int i = 0; i < 6; i++)
	{
		float angle = (i / 6.0f) * 360.0f;
		float rad = CC_DEGREES_TO_RADIANS(angle);
		float r = tileSize / 3 + (rand() % 4);
		points[i] = Vec2(center.x + cosf(rad) * r, center.y + sinf(rad) * r);
	}
	drawNode->drawPolygon(points, 6, Color4F(0.4f, 0.4f, 0.4f, 1.0f), 1.0f, Color4F(0.2f, 0.2f, 0.2f, 1.0f));
}

// 长椅 - 长条形
void MapManager::drawBench(DrawNode* drawNode, const Vec2& pos, int tileSize)
{
	// 座位
	drawNode->drawSolidRect(
		Vec2(pos.x + 4, pos.y + tileSize / 2),
		Vec2(pos.x + tileSize - 4, pos.y + tileSize / 2 + 4),
		Color4F(0.6f, 0.4f, 0.2f, 1.0f)
		);
	// 靠背
	drawNode->drawSolidRect(
		Vec2(pos.x + 4, pos.y + tileSize / 2 + 4),
		Vec2(pos.x + tileSize - 4, pos.y + tileSize / 2 + 8),
		Color4F(0.5f, 0.3f, 0.1f, 1.0f)
		);
	// 腿
	drawNode->drawSolidRect(
		Vec2(pos.x + 6, pos.y + tileSize / 2 - 4),
		Vec2(pos.x + 10, pos.y + tileSize / 2),
		Color4F(0.5f, 0.3f, 0.1f, 1.0f)
		);
	drawNode->drawSolidRect(
		Vec2(pos.x + tileSize - 10, pos.y + tileSize / 2 - 4),
		Vec2(pos.x + tileSize - 6, pos.y + tileSize / 2),
		Color4F(0.5f, 0.3f, 0.1f, 1.0f)
		);
}
