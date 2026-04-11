// Quest.h
#pragma once
#include "cocos2d.h"
#include <string>
#include <map>

enum class QuestType
{
	TALK,       // 对话任务
	KILL,       // 击杀怪物
	COLLECT,    // 收集物品
	DELIVER,    // 送达物品
	ESCORT,     // 护送任务
	EXPLORE     // 探索任务
};

enum class QuestStatus
{
	NOT_STARTED,    // 未接取
	IN_PROGRESS,    // 进行中
	COMPLETED,      // 已完成
	REWARDED        // 已领奖
};

class Quest
{
public:
	std::string id;
	std::string name;
	std::string description;
	QuestType type;
	QuestStatus status;
	int targetNPCId;
	int targetMonsterId;
	int requiredCount;
	int currentProgress;
	std::map<std::string, int> rewards;

	Quest();
	Quest(const std::string& _id, const std::string& _name, const std::string& _desc, QuestType _type);

	float getProgressPercent() const;
	bool isComplete() const;
	bool canClaimReward() const;
	void updateProgress(int amount);
	std::string getStatusString() const;
};