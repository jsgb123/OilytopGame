package oily.top.game.map;

import oily.top.game.npc.NPC;
import oily.top.game.npc.NPCTask;
import oily.top.game.portal.Portal;

/**
 * 新手村地图实现
 *
 * @author QQ1466214431
 */
public class StartingVillageMap extends MapData {

    public StartingVillageMap() {
        super(1, "新手村");//网格数
        initMap();
        initNPCs();
        initPortals();

    }

    private void initMap() {
        setSpawnX(40);//出生地坐标
        setSpawnY(30);
        // 初始化所有格子为草地
        for (int x = 0; x < getWidth(); x++) {
            for (int y = 0; y < getHeight(); y++) {
                getTiles()[x][y] = TileType.GRASS;
            }
        }

        // 创建地图边界（墙壁）
        for (int x = 0; x < getWidth(); x++) {
            getTiles()[x][0] = TileType.WALL;
            getTiles()[x][getHeight() - 1] = TileType.WALL;
        }
        for (int y = 0; y < getHeight(); y++) {
            getTiles()[0][y] = TileType.WALL;
            getTiles()[getWidth() - 1][y] = TileType.WALL;
        }

        // 创建道路（十字形）
        for (int x = 1; x < getWidth() - 1; x++) {
            getTiles()[x][getHeight() / 2] = TileType.PATH;
        }
        for (int y = 1; y < getHeight() - 1; y++) {
            getTiles()[getWidth() / 2][y] = TileType.PATH;
        }

        // 创建建筑 - 旅店（左上区域）
        for (int x = 9; x < 12; x++) {
            for (int y = 48; y < 52; y++) {
                getTiles()[x][y] = TileType.BUILDING;
            }
        }
        addFlowersAroundBuilding(9, 48, 11, 51,1);

        // 创建建筑 - 铁匠附近（左下区域）
        getTiles()[10][11] = TileType.BUILDING;
        getTiles()[11][12] = TileType.BUILDING;
        getTiles()[10][12] = TileType.BUILDING;

        // 创建建筑 - 商人附近（右下区域）
        getTiles()[69][11] = TileType.BUILDING;
        getTiles()[70][12] = TileType.BUILDING;
        getTiles()[69][12] = TileType.BUILDING;
        getTiles()[69][9] = TileType.BUILDING;

        // 创建建筑 - 猎户附近（右上区域）
        getTiles()[64][45] = TileType.BUILDING;
        getTiles()[64][50] = TileType.BUILDING;
        getTiles()[63][46] = TileType.BUILDING;

//        村长附近
        getTiles()[32][25] = TileType.BUILDING;

        // 树木 - 成片树林
        createForest(5, 15, 14, 24);
        createForest(67, 49, 73, 55);
        // 树木 - 单独大树（地标）
        getTiles()[20][12] = TileType.TREE;
        getTiles()[41][32] = TileType.TREE;
        getTiles()[15][35] = TileType.TREE;
        getTiles()[48][10] = TileType.TREE;
        // 花丛 - 成片花海
        createFlowerGarden(58, 15, 65, 20);   // x:22-30, y:25-32
        createFlowerGarden(21, 41, 31, 49);   // x:35-42, y:12-18

        // 道路两边装饰
        decorateRoadSides();

        // 水井（固定位置）
        getTiles()[getWidth() / 2 + 2][getHeight() / 2 + 1] = TileType.WELL;

        // 添加石头装饰
        getTiles()[28][22] = TileType.STONE;
        getTiles()[30][39] = TileType.STONE;
        getTiles()[57][17] = TileType.STONE;
        // 添加长椅
        getTiles()[29][24] = TileType.BENCH;
        getTiles()[34][24] = TileType.BENCH;
    }

    /**
     * 在建筑周围放置花丛（可配置空隙）
     *
     * @param startX 建筑起始X
     * @param startY 建筑起始Y
     * @param endX 建筑结束X
     * @param endY 建筑结束Y
     * @param gap 空隙格数（0=紧贴建筑，1=隔1格，2=隔2格...）
     */
    private void addFlowersAroundBuilding(int startX, int startY, int endX, int endY, int gap) {
        int flowerDistance = gap + 1;  // 花丛距离建筑的距离

        for (int x = startX - flowerDistance; x <= endX + flowerDistance; x++) {
            for (int y = startY - flowerDistance; y <= endY + flowerDistance; y++) {
                // 只放最外圈
                boolean isOuterRing = (x == startX - flowerDistance || x == endX + flowerDistance
                        || y == startY - flowerDistance || y == endY + flowerDistance);
                // 排除建筑本身和空隙区域
                boolean notInBuildingArea = (x < startX - gap || x > endX + gap
                        || y < startY - gap || y > endY + gap);

                if (isOuterRing && notInBuildingArea && getTiles()[x][y] == TileType.GRASS) {
                    getTiles()[x][y] = TileType.FLOWER;
                }
            }
        }
    }

    /**
     * 道路两边装饰花、树、石头
     */
    private void decorateRoadSides() {
        int width = getWidth();
        int height = getHeight();
        int centerX = width / 2;
        int centerY = height / 2;

        // ========== 1. 横向道路两旁装饰 ==========
        for (int x = 1; x < width - 1; x++) {
            // 道路上方 (y = centerY - 1)
            int upY = centerY - 1;
            if (getTiles()[x][upY] == TileType.GRASS) {
                if (x % 4 == 0) {
                    getTiles()[x][upY] = TileType.TREE;      // 树木
                } else if (x % 4 == 1) {
                    getTiles()[x][upY] = TileType.FLOWER;    // 花丛
                } else if (x % 4 == 2) {
                    getTiles()[x][upY] = TileType.STONE;     // 石头
                }
            }

            // 道路下方 (y = centerY + 1)
            int downY = centerY + 1;
            if (getTiles()[x][downY] == TileType.GRASS) {
                if (x % 4 == 0) {
                    getTiles()[x][downY] = TileType.STONE;   // 石头
                } else if (x % 4 == 1) {
                    getTiles()[x][downY] = TileType.TREE;    // 树木
                } else if (x % 4 == 2) {
                    getTiles()[x][downY] = TileType.FLOWER;  // 花丛
                }
            }
        }

        // ========== 2. 纵向道路两旁装饰 ==========
        for (int y = 1; y < height - 1; y++) {
            // 道路左侧 (x = centerX - 1)
            int leftX = centerX - 1;
            if (getTiles()[leftX][y] == TileType.GRASS) {
                if (y % 4 == 0) {
                    getTiles()[leftX][y] = TileType.FLOWER;  // 花丛
                } else if (y % 4 == 1) {
                    getTiles()[leftX][y] = TileType.TREE;    // 树木
                } else if (y % 4 == 2) {
                    getTiles()[leftX][y] = TileType.STONE;   // 石头
                }
            }

            // 道路右侧 (x = centerX + 1)
            int rightX = centerX + 1;
            if (getTiles()[rightX][y] == TileType.GRASS) {
                if (y % 4 == 0) {
                    getTiles()[rightX][y] = TileType.STONE;  // 石头
                } else if (y % 4 == 1) {
                    getTiles()[rightX][y] = TileType.FLOWER; // 花丛
                } else if (y % 4 == 2) {
                    getTiles()[rightX][y] = TileType.TREE;   // 树木
                }
            }
        }

        // ========== 3. 十字路口四角装饰 ==========
        // 左上角
        if (getTiles()[centerX - 1][centerY - 1] == TileType.GRASS) {
            getTiles()[centerX - 1][centerY - 1] = TileType.TREE;
        }
        // 右上角
        if (getTiles()[centerX + 1][centerY - 1] == TileType.GRASS) {
            getTiles()[centerX + 1][centerY - 1] = TileType.STONE;
        }
        // 左下角
        if (getTiles()[centerX - 1][centerY + 1] == TileType.GRASS) {
            getTiles()[centerX - 1][centerY + 1] = TileType.FLOWER;
        }
        // 右下角
        if (getTiles()[centerX + 1][centerY + 1] == TileType.GRASS) {
            getTiles()[centerX + 1][centerY + 1] = TileType.TREE;
        }

        // ========== 4. 道路入口装饰 ==========
        // 北门入口
        for (int x = centerX - 3; x <= centerX + 3; x++) {
            if (getTiles()[x][3] == TileType.GRASS) {
                if (x % 2 == 0) {
                    getTiles()[x][3] = TileType.FLOWER;
                } else {
                    getTiles()[x][3] = TileType.STONE;
                }
            }
        }

        // 南门入口
        for (int x = centerX - 3; x <= centerX + 3; x++) {
            if (getTiles()[x][height - 4] == TileType.GRASS) {
                if (x % 2 == 0) {
                    getTiles()[x][height - 4] = TileType.TREE;
                } else {
                    getTiles()[x][height - 4] = TileType.FLOWER;
                }
            }
        }

        // 东门入口
        for (int y = centerY - 3; y <= centerY + 3; y++) {
            if (getTiles()[width - 4][y] == TileType.GRASS) {
                if (y % 2 == 0) {
                    getTiles()[width - 4][y] = TileType.STONE;
                } else {
                    getTiles()[width - 4][y] = TileType.TREE;
                }
            }
        }

        // 西门入口
        for (int y = centerY - 3; y <= centerY + 3; y++) {
            if (getTiles()[3][y] == TileType.GRASS) {
                if (y % 2 == 0) {
                    getTiles()[3][y] = TileType.FLOWER;
                } else {
                    getTiles()[3][y] = TileType.STONE;
                }
            }
        }
    }

    /**
     * 创建一片森林
     */
    private void createForest(int startX, int startY, int endX, int endY) {
        for (int x = startX; x <= endX; x++) {
            for (int y = startY; y <= endY; y++) {
                // 只在草地和道路上放置，不覆盖建筑
                if ((getTiles()[x][y] == TileType.GRASS)
                        && Math.random() > 0.6) {  // 60% 密度
                    getTiles()[x][y] = TileType.TREE;
                }
            }
        }
    }

    /**
     * 创建一片花海
     */
    private void createFlowerGarden(int startX, int startY, int endX, int endY) {
        for (int x = startX; x <= endX; x++) {
            for (int y = startY; y <= endY; y++) {
                // 只在草地和道路上放置
                if ((getTiles()[x][y] == TileType.GRASS)
                        && Math.random() > 0.5) {  // 50% 密度
                    getTiles()[x][y] = TileType.FLOWER;
                }
            }
        }
    }

    /**
     * 道路两旁的花丛
     */
    private void createFlowerAlongPath() {
        int centerX = getWidth() / 2;
        int centerY = getHeight() / 2;

        // 横向道路两旁
        for (int x = 1; x < getWidth() - 1; x++) {
            // 道路上方
            if (getTiles()[x][centerY - 1] == TileType.GRASS && x % 3 == 0) {
                getTiles()[x][centerY - 1] = TileType.FLOWER;
            }
            // 道路下方
            if (getTiles()[x][centerY + 1] == TileType.GRASS && x % 3 == 1) {
                getTiles()[x][centerY + 1] = TileType.FLOWER;
            }
        }

        // 纵向道路两旁
        for (int y = 1; y < getHeight() - 1; y++) {
            // 道路左侧
            if (getTiles()[centerX - 1][y] == TileType.GRASS && y % 3 == 2) {
                getTiles()[centerX - 1][y] = TileType.FLOWER;
            }
            // 道路右侧
            if (getTiles()[centerX + 1][y] == TileType.GRASS && y % 3 == 0) {
                getTiles()[centerX + 1][y] = TileType.FLOWER;
            }
        }
    }

    private void initNPCs() {
        // 村长
        NPC elder = new NPC(1, "村长", getWidth() / 2 - 3, getHeight() / 2 - 3,
                "欢迎来到新手村！我是这里的村长。");
        elder.setDialogueType(NPC.NPCDialogueType.GREETING);
        addNPC(elder);

        // 铁匠
        NPC blacksmith = new NPC(2, "铁匠", 12, 10,
                "想要武器吗？我可以帮你打造！");
        blacksmith.setDialogueType(NPC.NPCDialogueType.TRADE);
        addNPC(blacksmith);

        // 商人
        NPC merchant = new NPC(3, "商人", getWidth() - 13, 10,
                "各种道具应有尽有！");
        merchant.setDialogueType(NPC.NPCDialogueType.TRADE);
        addNPC(merchant);

        // 旅店老板
        NPC innkeeper = new NPC(4, "旅店老板", 12, getHeight() - 10,
                "需要休息恢复体力吗？");
        innkeeper.setDialogueType(NPC.NPCDialogueType.TRAINING);
        addNPC(innkeeper);

        // 新手导师（带任务）
        NPC mentor = new NPC(5, "新手导师", getWidth() / 2 + 5, getHeight() / 2 - 2,
                "让我教你游戏的玩法吧！");
        mentor.setDialogueType(NPC.NPCDialogueType.QUEST);

        // 为导师添加任务
        NPCTask firstTask = new NPCTask(101, "初次问候", "与村长对话", NPCTask.TaskType.TALK);
        firstTask.setTargetNPCId(1); // 村长ID
        firstTask.addReward("exp", 100);
        firstTask.addReward("gold", 50);
        mentor.addTask(firstTask);

        addNPC(mentor);

        // 猎户
        NPC hunter = new NPC(6, "猎户", getWidth() / 5 * 4, getHeight() / 5 * 4,
                "森林里有野猪出没，需要帮助吗？");
        hunter.setDialogueType(NPC.NPCDialogueType.QUEST);

        NPCTask killTask = new NPCTask(102, "清除野猪", "击败5只野猪", NPCTask.TaskType.KILL);
        killTask.setTargetMonsterId(1001);
        killTask.setRequiredCount(5);
        killTask.addReward("exp", 200);
        killTask.addReward("gold", 100);
        killTask.addReward("item", "野猪皮");
        hunter.addTask(killTask);

        addNPC(hunter);
    }

    private void initPortals() {
        // 通往野外的传送门（北门）
        Portal northPortal = new Portal(1, "北门", getWidth() / 2 , getHeight()-2,
                2, getWidth() / 2, getHeight() - 10);
        northPortal.setType(TileType.ENTRANCE);
        addPortal(northPortal);

        // 通往野外的传送门（南门）
        Portal southPortal = new Portal(2, "南门", getWidth() / 2 ,1,
                3, getWidth() / 2, 5);
        southPortal.setType(TileType.ENTRANCE);
        addPortal(southPortal);

        // 通往野外的传送门（东门）
        Portal eastPortal = new Portal(3, "东门", getWidth() - 2, getHeight() / 2,
                4, 0, getHeight() / 2);
        eastPortal.setType(TileType.ENTRANCE);
        addPortal(eastPortal);

        // 通往野外的传送门（西门）
        Portal westPortal = new Portal(4, "西门", 1, getHeight() / 2,
                5, getWidth() - 2, getHeight() / 2);
        westPortal.setType(TileType.ENTRANCE);
        addPortal(westPortal);

        // 秘密传送门（需要等级5）
        Portal secretPortal = new Portal(5, "秘密通道", 25, 25,
                6, 10, 10);
        secretPortal.setType(TileType.ENTRANCE);
        secretPortal.setRequiredLevel(5);
        secretPortal.setActive(false); // 初始不可用
        addPortal(secretPortal);
    }
}
