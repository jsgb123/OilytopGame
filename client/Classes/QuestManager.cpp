// QuestManager.cpp
#include "QuestManager.h"

QuestManager* QuestManager::_instance = nullptr;

QuestManager* QuestManager::getInstance()
{
	if (!_instance)
	{
		_instance = new QuestManager();
	}
	return _instance;
}

QuestManager::QuestManager()
{
	// 初始化任务模板
	Quest talkQuest("quest_001", "初次问候", "与村长对话", QuestType::TALK);
	talkQuest.targetNPCId = 1;
	talkQuest.rewards["exp"] = 100;
	talkQuest.rewards["gold"] = 50;
	_quests["quest_001"] = talkQuest;

	Quest killQuest("quest_002", "清除野猪", "击败5只野猪", QuestType::KILL);
	killQuest.targetMonsterId = 1001;
	killQuest.requiredCount = 5;
	killQuest.rewards["exp"] = 200;
	killQuest.rewards["gold"] = 100;
	_quests["quest_002"] = killQuest;
}

void QuestManager::addQuest(const Quest& quest)
{
	Quest newQuest = quest;
	newQuest.status = QuestStatus::IN_PROGRESS;
	_quests[quest.id] = newQuest;
	_activeQuests.push_back(newQuest);

	if (onQuestUpdated) onQuestUpdated();
}

void QuestManager::updateQuestProgress(const std::string& questId, int amount)
{
	auto it = _quests.find(questId);
	if (it != _quests.end() && it->second.status == QuestStatus::IN_PROGRESS)
	{
		it->second.updateProgress(amount);

		if (it->second.isComplete())
		{
			// 移动到已完成列表
			for (auto it2 = _activeQuests.begin(); it2 != _activeQuests.end(); ++it2)
			{
				if (it2->id == questId)
				{
					_completedQuests.push_back(*it2);
					_activeQuests.erase(it2);
					break;
				}
			}
		}

		if (onQuestUpdated) onQuestUpdated();
	}
}

std::vector<Quest>& QuestManager::getActiveQuests()
{
	return _activeQuests;
}

void QuestManager::onMonsterKill(int monsterId)
{
	for (auto& quest : _activeQuests)
	{
		if (quest.type == QuestType::KILL && quest.targetMonsterId == monsterId)
		{
			updateQuestProgress(quest.id, 1);
		}
	}
}

void QuestManager::onTalkToNPC(int npcId)
{
	for (auto& quest : _activeQuests)
	{
		if (quest.type == QuestType::TALK && quest.targetNPCId == npcId)
		{
			updateQuestProgress(quest.id, 1);
		}
	}
}