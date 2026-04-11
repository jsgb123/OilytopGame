package map;

import java.util.List;
import oily.top.game.map.PathFinder;
import oily.top.game.map.StartingVillageMap;
import oily.top.game.npc.NPC;
import oily.top.game.npc.NPCTask;
import oily.top.game.portal.Portal;

/**
 *
 * @author QQ1466214431
 */
public class StartingVillageMapTest {

    public static void main(String[] args) {
        // 创建新手村地图
        StartingVillageMap village = new StartingVillageMap();

// 获取NPC
        NPC elder = village.getNPCAt(40, 27);
        if (elder != null) {
            System.out.println("找到NPC: " + elder.getName());
            System.out.println("对话: " + elder.getDialog());

            // 检查任务
            if (elder.hasTask()) {
                for (NPCTask task : elder.getTasks()) {
                    System.out.println("任务: " + task.getName());
                }
            }
        }

// 获取传送门
        Portal northPortal = village.getPortalAt(40, 0);
        if (northPortal != null) {
            System.out.println("找到传送门: " + northPortal.getName());
            System.out.println("目标地图: " + northPortal.getTargetMapId());
        }

// 寻路
        List<int[]> path = PathFinder.findPath(village, 40, 30, 12, 10);
        if (path != null) {
            System.out.println("找到路径，共 " + path.size() + " 步");
        }
    }
}
