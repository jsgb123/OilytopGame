
package oily.top.game.map;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.PriorityQueue;
import java.util.Set;

/**
 *
 * @author QQ1466214431
 */
public class PathFinder {
    private static final int[][] DIRECTIONS = {
        {0, 1}, {1, 0}, {0, -1}, {-1, 0},  // 四方向
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}  // 斜方向
    };
    
    private static class Node implements Comparable<Node> {
        int x, y;
        int g;  // 起点到当前点的实际代价
        int h;  // 当前点到终点的估计代价
        Node parent;
        
        Node(int x, int y) {
            this.x = x;
            this.y = y;
        }
        
        int f() { return g + h; }
        
        @Override
        public int compareTo(Node o) {
            return Integer.compare(this.f(), o.f());
        }
        
        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (!(obj instanceof Node)) return false;
            Node other = (Node) obj;
            return x == other.x && y == other.y;
        }
        
        @Override
        public int hashCode() {
            return x * 10000 + y;
        }
    }
    
    public static List<int[]> findPath(MapData map, int startX, int startY, 
                                       int targetX, int targetY) {
        if (!map.isWalkable(targetX, targetY)) {
            return null;
        }
        
        PriorityQueue<Node> openSet = new PriorityQueue<>();
        Map<Node, Node> allNodes = new HashMap<>();
        Set<Node> closedSet = new HashSet<>();
        
        Node start = new Node(startX, startY);
        Node target = new Node(targetX, targetY);
        
        start.g = 0;
        start.h = heuristic(start, target);
        openSet.add(start);
        allNodes.put(start, start);
        
        while (!openSet.isEmpty()) {
            Node current = openSet.poll();
            
            if (current.x == targetX && current.y == targetY) {
                return reconstructPath(current);
            }
            
            closedSet.add(current);
            
            for (int[] dir : DIRECTIONS) {
                int nx = current.x + dir[0];
                int ny = current.y + dir[1];
                
                if (!map.isWalkable(nx, ny)) continue;
                
                Node neighbor = new Node(nx, ny);
                if (closedSet.contains(neighbor)) continue;
                
                // 计算移动代价
                int moveCost = (dir[0] != 0 && dir[1] != 0) ? 14 : 10;
                int tentativeG = current.g + moveCost;
                
                Node existing = allNodes.get(neighbor);
                if (existing == null) {
                    neighbor.g = tentativeG;
                    neighbor.h = heuristic(neighbor, target);
                    neighbor.parent = current;
                    openSet.add(neighbor);
                    allNodes.put(neighbor, neighbor);
                } else if (tentativeG < existing.g) {
                    existing.g = tentativeG;
                    existing.parent = current;
                    // 更新优先队列中的位置
                    openSet.remove(existing);
                    openSet.add(existing);
                }
            }
        }
        
        return null; // 无路径
    }
    
    private static int heuristic(Node a, Node b) {
        // 欧几里得距离
        int dx = Math.abs(a.x - b.x);
        int dy = Math.abs(a.y - b.y);
        return (int)(Math.sqrt(dx*dx + dy*dy) * 10);
    }
    
    private static List<int[]> reconstructPath(Node node) {
        List<int[]> path = new ArrayList<>();
        while (node != null) {
            path.add(0, new int[]{node.x, node.y});
            node = node.parent;
        }
        return path;
    }
}
