package oily.top.game.map;

import java.util.ArrayList;
import java.util.List;
import oily.top.game.npc.NPC;
import oily.top.game.portal.Portal;

public class MapData {
    private int mapId;
    private String mapName;
    private int width=80;//网格大小
    private int height=60;
    private TileType[][] tiles;
    private List<NPC> npcs;
    private List<Portal> portals;
    private int spawnX;
    private int spawnY;
    
    public MapData(int mapId, String mapName) {
        this.mapId = mapId;
        this.mapName = mapName;
        this.tiles = new TileType[width][height];
        this.npcs = new ArrayList<>();
        this.portals = new ArrayList<>();
    }
    
    // Getters
    public int getMapId() { return mapId; }
    public String getMapName() { return mapName; }
    public int getWidth() { return width; }
    public int getHeight() { return height; }
    public TileType[][] getTiles() { return tiles; }
    public List<NPC> getNpcs() { return npcs; }
    public List<Portal> getPortals() { return portals; }
    public int getSpawnX() { return spawnX; }
    public int getSpawnY() { return spawnY; }
    
    // Setters
    public void setSpawnX(int spawnX) { this.spawnX = spawnX; }
    public void setSpawnY(int spawnY) { this.spawnY = spawnY; }
    
    public void setTile(int x, int y, TileType type) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            tiles[x][y] = type;
        }
    }
    
    public TileType getTile(int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return TileType.WALL;
        }
        return tiles[x][y];
    }
    
    public boolean isWalkable(int x, int y) {
        return getTile(x, y).isWalkable();
    }
    
    public void addNPC(NPC npc) {
        npcs.add(npc);
    }
    
    public void addPortal(Portal portal) {
        portals.add(portal);
    }
    
    public NPC getNPCAt(int x, int y) {
        for (NPC npc : npcs) {
            if (npc.getX() == x && npc.getY() == y) {
                return npc;
            }
        }
        return null;
    }
    
    public Portal getPortalAt(int x, int y) {
        for (Portal portal : portals) {
            if (portal.getSourceX() == x && portal.getSourceY() == y) {
                return portal;
            }
        }
        return null;
    }
    
    // 序列化为一维数组供客户端使用
    public int[] toTileArray() {
        int[] array = new int[width * height];
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                array[y * width + x] = tiles[x][y].getCode();
            }
        }
        return array;
    }
}