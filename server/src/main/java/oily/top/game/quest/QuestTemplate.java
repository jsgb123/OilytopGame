
package oily.top.game.quest;

import java.util.HashMap;
import java.util.Map;
import oily.top.game.quest.Quest.QuestType;

public class QuestTemplate {
    private String id;
    private String name;
    private String description;
    private QuestType type;
    private int targetNPCId;
    private int requiredCount;
    private Map<String, Object> rewards;
    private String prevQuestId;
    private String nextQuestId;
    
    public QuestTemplate(String id, String name, String description, QuestType type) {
        this.id = id;
        this.name = name;
        this.description = description;
        this.type = type;
        this.rewards = new HashMap<>();
        this.requiredCount = 1;
    }
    
    // Getters and Setters
    public String getId() { return id; }
    public String getName() { return name; }
    public String getDescription() { return description; }
    public QuestType getType() { return type; }
    public int getTargetNPCId() { return targetNPCId; }
    public void setTargetNPCId(int targetNPCId) { this.targetNPCId = targetNPCId; }
    public int getRequiredCount() { return requiredCount; }
    public void setRequiredCount(int requiredCount) { this.requiredCount = requiredCount; }
    public Map<String, Object> getRewards() { return rewards; }
    public void addReward(String key, Object value) { rewards.put(key, value); }
    public String getPrevQuestId() { return prevQuestId; }
    public void setPrevQuestId(String prevQuestId) { this.prevQuestId = prevQuestId; }
    public String getNextQuestId() { return nextQuestId; }
    public void setNextQuestId(String nextQuestId) { this.nextQuestId = nextQuestId; }
}
