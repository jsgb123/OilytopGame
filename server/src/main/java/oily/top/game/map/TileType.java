package oily.top.game.map;

public enum TileType {
    GRASS(0, "草地", true, "·"),
    PATH(1, "道路", true, "·"),
    WATER(2, "水域", false, "≈"),
    WALL(3, "墙壁", false, "#"),
    BUILDING(4, "建筑", false, "🏠"),
    TREE(5, "树木", false, "🌳"),
    FLOWER(6, "花丛", true, "🌸"),
    WELL(7, "水井", false, "⛲"),
    ENTRANCE(8, "入口", true, "🚪"),
    STONE(9, "石头", false, "🪨"),     
    BENCH(10, "长椅", false, "🪑");    
    
    private int code;
    private String name;
    private boolean walkable;
    private String symbol;
    
    TileType(int code, String name, boolean walkable, String symbol) {
        this.code = code;
        this.name = name;
        this.walkable = walkable;
        this.symbol = symbol;
    }
    
    public int getCode() { return code; }
    public String getName() { return name; }
    public boolean isWalkable() { return walkable; }
    public String getSymbol() { return symbol; }
    
    public static TileType fromCode(int code) {
        for (TileType type : values()) {
            if (type.code == code) {
                return type;
            }
        }
        return GRASS;
    }
}