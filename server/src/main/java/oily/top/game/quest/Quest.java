package oily.top.game.quest;

import java.util.HashMap;
import java.util.Map;

public class Quest {
    private String id;
    private String name;
    private String description;
    private QuestType type;
    private QuestStatus status;
    private Map<String, Object> requirements;
    private Map<String, Object> rewards;
    private int targetNPCId;
    private int targetMonsterId;
    private int requiredCount;
    private int currentProgress;
    
    public enum QuestType {
        TALK,           // 对话任务
        KILL,           // 击杀怪物
        COLLECT,        // 收集物品
        DELIVER,        // 送达物品
        ESCORT,         // 护送任务
        EXPLORE         // 探索任务
    }
    
    public enum QuestStatus {
        NOT_STARTED,    // 未接取
        IN_PROGRESS,    // 进行中
        COMPLETED,      // 已完成
        REWARDED        // 已领奖
    }
    
    public Quest(String id, String name, String description, QuestType type) {
        this.id = id;
        this.name = name;
        this.description = description;
        this.type = type;
        this.status = QuestStatus.NOT_STARTED;
        this.requirements = new HashMap<>();
        this.rewards = new HashMap<>();
        this.currentProgress = 0;
    }
    
    // Getters and Setters
    public String getId() { return id; }
    public String getName() { return name; }
    public String getDescription() { return description; }
    public QuestType getType() { return type; }
    public QuestStatus getStatus() { return status; }
    public void setStatus(QuestStatus status) { this.status = status; }
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
    
    public void addRequirement(String key, Object value) {
        requirements.put(key, value);
    }
    
    public void addReward(String key, Object value) {
        rewards.put(key, value);
    }
    
    public void updateProgress(int amount) {
        currentProgress = Math.min(currentProgress + amount, requiredCount);
        if (currentProgress >= requiredCount && status == QuestStatus.IN_PROGRESS) {
            status = QuestStatus.COMPLETED;
        }
    }
    
    public boolean isComplete() {
        return status == QuestStatus.COMPLETED;
    }
    
    public boolean canClaimReward() {
        return status == QuestStatus.COMPLETED;
    }
}