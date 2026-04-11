package oily.top.game.portal;

import oily.top.game.map.TileType;

public class Portal {
    private int id;
    private String name;
    private int sourceX;
    private int sourceY;
    private int targetMapId;
    private int targetX;
    private int targetY;
    private TileType type;
    private boolean isActive;
    private int requiredLevel;
    private String requiredItem;
    private String symbol;
    
    /**
     * 
     * @param id
     * @param name
     * @param sourceX
     * @param sourceY
     * @param targetMapId
     * @param targetX
     * @param targetY 
     */
    public Portal(int id, String name, int sourceX, int sourceY, 
                  int targetMapId, int targetX, int targetY) {
        this.id = id;
        this.name = name;
        this.sourceX = sourceX;
        this.sourceY = sourceY;
        this.targetMapId = targetMapId;
        this.targetX = targetX;
        this.targetY = targetY;
        this.type = TileType.ENTRANCE;
        this.isActive = true;
        this.symbol = "🚪";
    }
    
    // Getters
    public int getId() { return id; }
    public String getName() { return name; }
    public int getSourceX() { return sourceX; }
    public int getSourceY() { return sourceY; }
    public int getTargetMapId() { return targetMapId; }
    public int getTargetX() { return targetX; }
    public int getTargetY() { return targetY; }
    public TileType getType() { return type; }
    public boolean isActive() { return isActive; }
    public int getRequiredLevel() { return requiredLevel; }
    public String getRequiredItem() { return requiredItem; }
    public String getSymbol() { return symbol; }
    
    // Setters
    public void setType(TileType type) { this.type = type; }
    public void setActive(boolean active) { isActive = active; }
    public void setRequiredLevel(int level) { this.requiredLevel = level; }
    public void setRequiredItem(String item) { this.requiredItem = item; }
    public void setSymbol(String symbol) { this.symbol = symbol; }
}