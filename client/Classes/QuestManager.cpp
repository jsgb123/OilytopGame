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

void QuestManager::acceptQuest(const Quest& quest)
{
	Quest newQuest = quest;
	newQuest.status = QuestStatus::IN_PROGRESS;
	newQuest.currentProgress = 0;
	_quests[quest.id] = newQuest;
	_activeQuests.push_back(newQuest);

	if (onQuestUpdated) onQuestUpdated();
	CCLOG("接取任务: %s", quest.name.c_str());
}

void QuestManager::syncQuest(const Quest& quest)
{
	_quests[quest.id] = quest;

	if (quest.status == QuestStatus::IN_PROGRESS)
	{
		_activeQuests.push_back(quest);
	}
	else if (quest.status == QuestStatus::COMPLETED || quest.status == QuestStatus::REWARDED)
	{
		_completedQuests.push_back(quest);
	}

	if (onQuestUpdated) onQuestUpdated();
	CCLOG("同步任务: %s, 状态: %d", quest.name.c_str(), (int)quest.status);
}

Quest* QuestManager::getQuest(const std::string& id)
{
	auto it = _quests.find(id);
	if (it != _quests.end())
	{
		return &it->second;
	}
	return nullptr;
}

// ? 修复：只保留一个实现
std::vector<Quest>& QuestManager::getActiveQuests()
{
	// 重新整理进行中的任务
	_activeQuests.clear();
	for (auto& pair : _quests)
	{
		if (pair.second.status == QuestStatus::IN_PROGRESS)
		{
			_activeQuests.push_back(pair.second);
		}
	}
	return _activeQuests;
}

void QuestManager::updateQuestProgress(const std::string& questId, int amount)
{
	auto it = _quests.find(questId);
	if (it != _quests.end() && it->second.status == QuestStatus::IN_PROGRESS)
	{
		it->second.updateProgress(amount);

		if (it->second.isComplete())
		{
			it->second.status = QuestStatus::COMPLETED;

			// ? 修复：使用迭代器安全删除
			for (auto it2 = _activeQuests.begin(); it2 != _activeQuests.end(); )
			{
				if (it2->id == questId)
				{
					_completedQuests.push_back(*it2);
					it2 = _activeQuests.erase(it2);
				}
				else
				{
					++it2;
				}
			}
		}

		if (onQuestUpdated) onQuestUpdated();
	}
}

void QuestManager::onMonsterKill(int monsterId)
{
	for (auto& quest : getActiveQuests())
	{
		if (quest.type == QuestType::KILL && quest.targetMonsterId == monsterId)
		{
			updateQuestProgress(quest.id, 1);
		}
	}
}

void QuestManager::onTalkToNPC(int npcId)
{
	for (auto& quest : getActiveQuests())
	{
		if (quest.type == QuestType::TALK && quest.targetNPCId == npcId)
		{
			updateQuestProgress(quest.id, 1);
		}
	}
}