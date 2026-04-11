
package oily.top.game.quest;

import java.util.Map;
import oily.top.game.quest.Quest.QuestStatus;
import oily.top.game.quest.Quest.QuestType;

public class PlayerQuest {
    private String id;
    private String name;
    private String description;
    private QuestType type;
    private QuestStatus status;
    private int targetNPCId;
    private int requiredCount;
    private int currentProgress;
    private Map<String, Object> rewards;
    private String nextQuestId;
    
    public PlayerQuest(QuestTemplate template) {
        this.id = template.getId();
        this.name = template.getName();
        this.description = template.getDescription();
        this.type = template.getType();
        this.targetNPCId = template.getTargetNPCId();
        this.requiredCount = template.getRequiredCount();
        this.rewards = template.getRewards();
        this.nextQuestId = template.getNextQuestId();
        this.status = QuestStatus.NOT_STARTED;
        this.currentProgress = 0;
    }
    
    // Getters and Setters
    public String getId() { return id; }
    public String getName() { return name; }
    public String getDescription() { return description; }
    public QuestType getType() { return type; }
    public QuestStatus getStatus() { return status; }
    public void setStatus(QuestStatus status) { this.status = status; }
    public int getTargetNPCId() { return targetNPCId; }
    public int getRequiredCount() { return requiredCount; }
    public int getCurrentProgress() { return currentProgress; }
    public Map<String, Object> getRewards() { return rewards; }
    public String getNextQuestId() { return nextQuestId; }
    
    public void updateProgress(int amount) {
        currentProgress = Math.min(currentProgress + amount, requiredCount);
    }
    
    public boolean isComplete() {
        return currentProgress >= requiredCount;
    }
}