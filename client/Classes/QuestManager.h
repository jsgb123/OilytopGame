// QuestManager.h
#pragma once
#include "cocos2d.h"
#include "Quest.h"
#include <vector>
#include <map>

class QuestManager
{
public:
	static QuestManager* getInstance();
	// 接取新任务（强制设为进行中）
	void acceptQuest(const Quest& quest);
	// 同步任务（保持原状态，用于从服务器加载）
	void syncQuest(const Quest& quest);
	// 任务管理
	void updateQuestProgress(const std::string& questId, int amount);
	void completeQuest(const std::string& questId);
	void claimReward(const std::string& questId);

	// 查询
	Quest* getQuest(const std::string& questId);
	std::vector<Quest>& getActiveQuests();
	std::vector<Quest>& getCompletedQuests();

	// 事件触发
	void onMonsterKill(int monsterId);
	void onTalkToNPC(int npcId);
	void onCollectItem(const std::string& itemId);

	// UI 回调
	std::function<void()> onQuestUpdated;

private:
	QuestManager();
	~QuestManager();

	static QuestManager* _instance;
	std::map<std::string, Quest> _quests;
	std::vector<Quest> _activeQuests;
	std::vector<Quest> _completedQuests;
};