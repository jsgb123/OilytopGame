package oily.top.game;

import java.util.HashMap;
import java.util.Map;

public class Player {
    private int id;
    private String name;
    private int mapId;      // 当前所在地图ID
    private int x;
    private int y;
    private float direction;
    private int level;
    private long experience;//经验值
    private Map<String, Object> attributes;
    
    public Player(int id, String name) {
        this.id = id;
        this.name = name;
        this.mapId = 1;  // 默认新手村地图
        this.x = 400;
        this.y = 300;
        this.direction = 0;
        this.level = 1;
        this.experience = 0;
        this.attributes = new HashMap<>();
    }
    
    public Player(int id, String name, int mapId, int x, int y) {
        this.id = id;
        this.name = name;
        this.mapId = mapId;
        this.x = x;
        this.y = y;
        this.direction = 0;
        this.level = 1;
        this.experience = 0;
        this.attributes = new HashMap<>();
    }
    
    // Getters and Setters
    public int getId() { return id; }
    public String getName() { return name; }
    public int getMapId() { return mapId; }
    public void setMapId(int mapId) { this.mapId = mapId; }
    public int getX() { return x; }
    public void setX(int x) { this.x = x; }
    public int getY() { return y; }
    public void setY(int y) { this.y = y; }
    public float getDirection() { return direction; }
    public void setDirection(float direction) { this.direction = direction; }
    public int getLevel() { return level; }
    public void setLevel(int level) { this.level = level; }
    public long getExperience() { return experience; }
    public void setExperience(long experience) { this.experience = experience; }
    
    public void setPosition(int x, int y) {
        this.x = x;
        this.y = y;
    }
    
    public void setPosition(int mapId, int x, int y) {
        this.mapId = mapId;
        this.x = x;
        this.y = y;
    }
    
    public Map<String, Object> getAttributes() { return attributes; }
    public void setAttribute(String key, Object value) { attributes.put(key, value); }
    public Object getAttribute(String key) { return attributes.get(key); }
    
    @Override
    public String toString() {
        return String.format("Player{id=%d, name='%s', mapId=%d, pos=(%d,%d), level=%d}", 
            id, name, mapId, x, y, level);
    }

    public void addExp(int exp) {
        throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
    }

    public void addGold(int gold) {
        throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
    }
}