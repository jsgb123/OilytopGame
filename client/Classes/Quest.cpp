// Quest.cpp
#include "Quest.h"

Quest::Quest()
	: id("")
	, name("")
	, description("")
	, type(QuestType::TALK)
	, status(QuestStatus::NOT_STARTED)
	, targetNPCId(0)
	, targetMonsterId(0)
	, requiredCount(0)
	, currentProgress(0)
{
}

Quest::Quest(const std::string& _id, const std::string& _name, const std::string& _desc, QuestType _type)
	: id(_id)
	, name(_name)
	, description(_desc)
	, type(_type)
	, status(QuestStatus::NOT_STARTED)
	, targetNPCId(0)
	, targetMonsterId(0)
	, requiredCount(0)
	, currentProgress(0)
{
}

float Quest::getProgressPercent() const
{
	if (requiredCount <= 0) return 1.0f;
	return (float)currentProgress / requiredCount;
}

bool Quest::isComplete() const
{
	return status == QuestStatus::COMPLETED;
}

bool Quest::canClaimReward() const
{
	return status == QuestStatus::COMPLETED;
}

void Quest::updateProgress(int amount)
{
	if (status != QuestStatus::IN_PROGRESS) return;
	currentProgress = std::min(currentProgress + amount, requiredCount);
	if (currentProgress >= requiredCount)
	{
		status = QuestStatus::COMPLETED;
	}
}

std::string Quest::getStatusString() const
{
	switch (status)
	{
	case QuestStatus::NOT_STARTED: return "未接取";
	case QuestStatus::IN_PROGRESS: return "进行中";
	case QuestStatus::COMPLETED:   return "可领取";
	case QuestStatus::REWARDED:    return "已完成";
	default: return "未知";
	}
}