package oily.top.game;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import oily.top.game.map.MapData;
import oily.top.game.map.StartingVillageMap;
import oily.top.game.map.TileType;
import oily.top.game.npc.NPC;
import oily.top.game.portal.Portal;

public class World {

    private static final Logger logger = LoggerFactory.getLogger(World.class);
    private static final World instance = new World();

    // 玩家管理
    private final Map<Integer, Player> players = new ConcurrentHashMap<>();
    private int nextPlayerId = 1;

    // 地图管理
    private final Map<Integer, MapData> maps = new HashMap<>();

    private World() {
        initMaps();
    }

    public static World getInstance() {
        return instance;
    }

    public NPC getNPCById(int mapId, int npcId) {
        MapData mapData = maps.get(mapId);
        if (mapData == null) {
            return null;
        }

        for (NPC npc : mapData.getNpcs()) {
            if (npc.getId() == npcId) {
                return npc;
            }
        }
        return null;
    }

    /**
     * 初始化所有地图
     */
    private void initMaps() {
        // 地图1：新手村
        maps.put(1, new StartingVillageMap());

        // 地图2：野外（待实现）
        // maps.put(2, new FieldMap());
        // 地图3：秘密区域（待实现）
        // maps.put(3, new SecretMap());
        logger.info("地图初始化完成，共 {} 个地图", maps.size());
    }

    /**
     * 创建新玩家
     */
    public Player createPlayer(String name) {
        int playerId = nextPlayerId++;

        // 获取新手村地图
        MapData startMap = maps.get(1);
        if (startMap == null) {
            logger.error("新手村地图不存在");
            return null;
        }

        // 使用地图的出生点
        int spawnX = startMap.getSpawnX();
        int spawnY = startMap.getSpawnY();

        Player player = new Player(playerId, name, 1, spawnX, spawnY);
        players.put(playerId, player);

        logger.info("创建玩家: {} (ID: {}), 出生点地图1({},{})", name, playerId, spawnX, spawnY);
        return player;
    }

    /**
     * 获取玩家
     */
    public Player getPlayer(int playerId) {
        return players.get(playerId);
    }

    /**
     * 移除玩家
     */
    public Player removePlayer(int playerId) {
        return players.remove(playerId);
    }

    /**
     * 获取所有玩家
     */
    public Collection<Player> getAllPlayers() {
        return players.values();
    }

    /**
     * 获取指定地图的玩家
     */
    public List<Player> getPlayersInMap(int mapId) {
        List<Player> result = new ArrayList<>();
        for (Player player : players.values()) {
            if (player.getMapId() == mapId) {
                result.add(player);
            }
        }
        return result;
    }

    /**
     * 获取地图数据
     */
    public MapData getMapData(int mapId) {
        return maps.get(mapId);
    }

    /**
     * 验证玩家移动
     */
    public boolean validateMove(Player player, int targetX, int targetY) {
        MapData map = maps.get(player.getMapId());
        if (map == null) {
            return false;
        }

        // 检查是否在边界内
        if (targetX < 0 || targetX >= map.getWidth()
                || targetY < 0 || targetY >= map.getHeight()) {
            return false;
        }

        // 检查是否可行走
        if (!map.isWalkable(targetX, targetY)) {
            return false;
        }

        // 检查移动距离（防止瞬移）
        int distance = Math.abs(targetX - player.getX()) + Math.abs(targetY - player.getY());
        if (distance > 2) {
            return false;  // 移动距离过大，可能是作弊
        }

        return true;
    }

    /**
     * 执行玩家移动
     */
    public boolean movePlayer(Player player, int targetX, int targetY) {
        if (!validateMove(player, targetX, targetY)) {
            return false;
        }

        player.setX(targetX);
        player.setY(targetY);
        return true;
    }

    /**
     * 检查传送门并传送
     */
    public boolean checkPortal(Player player, int x, int y) {
        MapData currentMap = maps.get(player.getMapId());
        if (currentMap == null) {
            return false;
        }

        Portal portal = currentMap.getPortalAt(x, y);
        if (portal == null || !portal.isActive()) {
            return false;
        }

        // 检查等级要求
        if (player.getLevel() < portal.getRequiredLevel()) {
            return false;
        }

        // 执行传送
        player.setMapId(portal.getTargetMapId());
        player.setPosition(portal.getTargetX(), portal.getTargetY());

        logger.info("玩家 {} 通过传送门 {} 传送到地图 {}",
                player.getName(), portal.getName(), portal.getTargetMapId());

        return true;
    }

    /**
     * 获取玩家所在位置的地形类型
     */
    public TileType getTileType(Player player) {
        MapData map = maps.get(player.getMapId());
        if (map == null) {
            return TileType.GRASS;
        }
        return map.getTile(player.getX(), player.getY());
    }

    /**
     * 获取玩家附近的NPC
     */
    public NPC getNearbyNPC(Player player, int radius) {
        MapData map = maps.get(player.getMapId());
        if (map == null) {
            return null;
        }

        int px = player.getX();
        int py = player.getY();

        for (NPC npc : map.getNpcs()) {
            int distance = Math.abs(npc.getX() - px) + Math.abs(npc.getY() - py);
            if (distance <= radius) {
                return npc;
            }
        }
        return null;
    }

    /**
     * 获取玩家数量
     */
    public int getPlayerCount() {
        return players.size();
    }

    // TODO 查询玩家待更新
    public Player getPlayerByName(String playerName) {
        for (Player player : players.values()) {
            if (player.getName().equals(playerName)) {
                return player;
            }
        }
        return null;
    }
}
