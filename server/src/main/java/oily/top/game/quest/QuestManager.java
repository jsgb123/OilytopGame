package oily.top.game.quest;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import oily.top.game.Player;
import oily.top.game.quest.Quest.QuestStatus;
import oily.top.game.quest.Quest.QuestType;

public class QuestManager {
    private static QuestManager instance = new QuestManager();
    private Map<String, QuestTemplate> questTemplates;
    private Map<Integer, Map<String, PlayerQuest>> playerQuests;
    
    private QuestManager() {
        questTemplates = new HashMap<>();
        playerQuests = new ConcurrentHashMap<>();
        loadQuestTemplates();
    }
    
    public static QuestManager getInstance() {
        return instance;
    }
    
    private void loadQuestTemplates() {
        // 新手引导任务1：与新手导师对话接取任务
        QuestTemplate guideQuest1 = new QuestTemplate(
            "quest_guide_001", 
            "新手入门", 
            "与新手导师对话", 
            QuestType.TALK
        );
        guideQuest1.setTargetNPCId(5); // 新手导师 ID
        guideQuest1.setRequiredCount(1);
        guideQuest1.addReward("exp", 0);
        guideQuest1.setNextQuestId("quest_guide_002");
        questTemplates.put("quest_guide_001", guideQuest1);
        
        // 新手引导任务2：移动到村长处
        QuestTemplate guideQuest2 = new QuestTemplate(
            "quest_guide_002", 
            "拜访村长", 
            "与村长对话", 
            QuestType.TALK
        );
        guideQuest2.setTargetNPCId(1); // 村长 ID
        guideQuest2.setRequiredCount(1);
        guideQuest2.addReward("exp", 100);
        guideQuest2.addReward("gold", 50);
        questTemplates.put("quest_guide_002", guideQuest2);
    }

    // 获取任务模板
    public QuestTemplate getQuestTemplate(String questId) {
        return questTemplates.get(questId);
    }
    
    // 获取所有任务模板（可选）
    public Map<String, QuestTemplate> getAllQuestTemplates() {
        return questTemplates;
    }

    // 获取玩家可接取的任务
    public List<QuestTemplate> getAvailableQuests(int playerId, int npcId) {
        List<QuestTemplate> available = new ArrayList<>();
        
        for (QuestTemplate template : questTemplates.values()) {
            if (template.getTargetNPCId() == npcId) {
                // 检查前置任务
                if (template.getPrevQuestId() == null || 
                    isQuestCompleted(playerId, template.getPrevQuestId())) {
                    available.add(template);
                }
            }
        }
        return available;
    }
    // 获取玩家已完成的任务
    public List<PlayerQuest> getCompletedQuests(int playerId) {
        Map<String, PlayerQuest> quests = playerQuests.get(playerId);
        if (quests == null) return new ArrayList<>();
        
        List<PlayerQuest> completedQuests = new ArrayList<>();
        for (PlayerQuest quest : quests.values()) {
            if (quest.getStatus() == QuestStatus.COMPLETED || 
                quest.getStatus() == QuestStatus.REWARDED) {
                completedQuests.add(quest);
            }
        }
        return completedQuests;
    }
    // 接受任务
    public boolean acceptQuest(int playerId, String questId, int npcId) {
        QuestTemplate template = questTemplates.get(questId);
        if (template == null || template.getTargetNPCId() != npcId) {
            return false;
        }
        
        // 检查是否已完成
        if (isQuestCompleted(playerId, questId)) {
            return false;
        }
        
        // 检查是否已接取
        if (hasActiveQuest(playerId, questId)) {
            return false;
        }
        
        PlayerQuest playerQuest = new PlayerQuest(template);
        playerQuest.setStatus(QuestStatus.IN_PROGRESS);
        
        playerQuests.computeIfAbsent(playerId, k -> new ConcurrentHashMap<>())
                    .put(questId, playerQuest);
        
        return true;
    }
    // 检查任务是否已完成
    public boolean isQuestCompleted(int playerId, String questId) {
        Map<String, PlayerQuest> quests = playerQuests.get(playerId);
        if (quests == null) return false;
        PlayerQuest quest = quests.get(questId);
        return quest != null && quest.getStatus() == QuestStatus.REWARDED;
    }
    
    // 检查任务是否在进行中
    public boolean isQuestInProgress(int playerId, String questId) {
        Map<String, PlayerQuest> quests = playerQuests.get(playerId);
        if (quests == null) return false;
        PlayerQuest quest = quests.get(questId);
        return quest != null && quest.getStatus() == QuestStatus.IN_PROGRESS;
    }
    // 更新任务进度
    public void updateQuestProgress(int playerId, String questId, int amount) {
        Map<String, PlayerQuest> quests = playerQuests.get(playerId);
        if (quests == null) return;
        
        PlayerQuest quest = quests.get(questId);
        if (quest != null && quest.getStatus() == QuestStatus.IN_PROGRESS) {
            quest.updateProgress(amount);
            
            if (quest.isComplete()) {
                quest.setStatus(QuestStatus.COMPLETED);
            }
        }
    }
    
    // 完成任务并发放奖励
    public boolean completeQuest(int playerId, String questId, Player player) {
        Map<String, PlayerQuest> quests = playerQuests.get(playerId);
        if (quests == null) return false;
        
        PlayerQuest quest = quests.get(questId);
        if (quest == null || !quest.isComplete()) {
            return false;
        }
        
        // 发放奖励
        for (Map.Entry<String, Object> entry : quest.getRewards().entrySet()) {
            switch (entry.getKey()) {
                case "exp":
                    int exp = (int) entry.getValue();
                    player.addExp(exp);
                    break;
                case "gold":
                    int gold = (int) entry.getValue();
                    player.addGold(gold);
                    break;
            }
        }
        
        quest.setStatus(QuestStatus.REWARDED);
        
        // 自动接取下一个任务
        if (quest.getNextQuestId() != null) {
            //acceptQuest(playerId, quest.getNextQuestId(), player.getX(), player.getY());
        }
        
        return true;
    }
    

    
    private boolean hasActiveQuest(int playerId, String questId) {
        Map<String, PlayerQuest> quests = playerQuests.get(playerId);
        if (quests == null) return false;
        PlayerQuest quest = quests.get(questId);
        return quest != null && quest.getStatus() == QuestStatus.IN_PROGRESS;
    }

// 获取玩家所有任务
    public Map<String, PlayerQuest> getPlayerQuests(int playerId) {
        return playerQuests.getOrDefault(playerId, new HashMap<>());
    }
    
}