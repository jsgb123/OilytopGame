
package oily.top.game.npc;

import java.util.HashMap;
import java.util.Map;

/**
 *
 * @author QQ1466214431
 */
public class NPCTask {
    private int id;
    private String name;
    private String description;
    private TaskType type;
    private TaskStatus status;
    private Map<String, Object> requirements;
    private Map<String, Object> rewards;
    private int targetNPCId;
    private int targetMonsterId;
    private int requiredCount;
    private int currentProgress;
    
    public enum TaskType {
        TALK,           // 对话任务
        KILL,           // 击杀怪物
        COLLECT,        // 收集物品
        DELIVER,        // 送达物品
        ESCORT,         // 护送任务
        EXPLORE         // 探索任务
    }
    
    public enum TaskStatus {
        AVAILABLE,      // 可接取
        IN_PROGRESS,    // 进行中
        COMPLETED,      // 已完成
        REWARDED,       // 已领奖
        FAILED          // 已失败
    }
    
    public NPCTask(int id, String name, String description, TaskType type) {
        this.id = id;
        this.name = name;
        this.description = description;
        this.type = type;
        this.status = TaskStatus.AVAILABLE;
        this.requirements = new HashMap<>();
        this.rewards = new HashMap<>();
        this.currentProgress = 0;
    }
    
    // Getters and Setters
    public int getId() { return id; }
    public String getName() { return name; }
    public String getDescription() { return description; }
    public TaskType getType() { return type; }
    public TaskStatus getStatus() { return status; }
    public void setStatus(TaskStatus status) { this.status = status; }
    public Map<String, Object> getRequirements() { return requirements; }
    public Map<String, Object> getRewards() { return rewards; }
    public int getTargetNPCId() { return targetNPCId; }
    public void setTargetNPCId(int targetNPCId) { this.targetNPCId = targetNPCId; }
    public int getTargetMonsterId() { return targetMonsterId; }
    public void setTargetMonsterId(int targetMonsterId) { this.targetMonsterId = targetMonsterId; }
    public int getRequiredCount() { return requiredCount; }
    public void setRequiredCount(int requiredCount) { this.requiredCount = requiredCount; }
    public int getCurrentProgress() { return currentProgress; }
    public void setCurrentProgress(int currentProgress) { this.currentProgress = currentProgress; }
    
    // 添加需求
    public void addRequirement(String key, Object value) {
        requirements.put(key, value);
    }
    
    // 添加奖励
    public void addReward(String key, Object value) {
        rewards.put(key, value);
    }
    
    // 更新进度
    public void updateProgress(int amount) {
        currentProgress = Math.min(currentProgress + amount, requiredCount);
        if (currentProgress >= requiredCount && status == TaskStatus.IN_PROGRESS) {
            status = TaskStatus.COMPLETED;
        }
    }
    
    // 检查是否完成
    public boolean isComplete() {
        return status == TaskStatus.COMPLETED;
    }
    
    // 检查是否可以领取奖励
    public boolean canClaimReward() {
        return status == TaskStatus.COMPLETED;
    }
}