#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "cocos2d.h"

class Player : public cocos2d::Ref
{
public:
	Player(int id, const std::string& name);
	int getId() const { return _id; }
	const std::string& getName() const { return _name; }
	long getExp() const { return _exp; }
	long getMaxExp() const { return _maxExp; }
	int getHp() const { return _hp; }
	int getMaxHp() const { return _maxHp; }
	void setLevel(int level);
	void setExp(long exp);
	void setHp(int hp);
	int getLevel() const { return _level; }
	void setId(int id) { _id = id; }
	void setName(std::string name) { _name = name; }

	// 战斗相关方法
	void takeDamage(int damage);
	void heal(int amount);
	void addExp(long exp);
	bool isAlive() const { return _hp > 0; }
	float getHpPercent() const { return (float)_hp / _maxHp; }
	float getExpPercent() const { return (float)_exp / _maxExp; }

	// 移动相关
	cocos2d::Vec2 getPosition() const { return _position; }
	void setPosition(const cocos2d::Vec2& pos);
	void setPositionImmediate(const cocos2d::Vec2& pos);
	void moveTo(const cocos2d::Vec2& target);
	void update(float delta);
	bool isMoving() const { return _isMoving; }
	float getSpeed() const { return _speed; }
	void setDirection(float direction);
	float getDirection() const { return _direction; }
	cocos2d::Vec2 getGridPosition(int tileSize = 32) const;
	float getSpeed() { return _speed; }
	void updatePupilOffset(float delta);
	cocos2d::Vec2 getPupilOffset() const { return _pupilOffset; }
	void stopMoving();
	cocos2d::Vec2 getTargetPosition() const { return _targetPosition; }

private:
	int _id;     
	std::string _name;
	cocos2d::Vec2 _position;
	cocos2d::Vec2 _targetPosition;
	float _direction;
	int _level;
	long _exp;
	bool _isMoving;
	float _speed;
	long _maxExp;
	int _hp;
	int _maxHp;
	//cocos2d::Node* _sprite;    // 关联的精灵节点
	cocos2d::Action* _currentMoveAction;
	cocos2d::Vec2 _pupilOffset = cocos2d::Vec2(0, 0); // 瞳孔偏移量
	cocos2d::Vec2 _targetPupilOffset;
	float _pupilSmoothSpeed = 5.0f; // 瞳孔平滑速度
};

#endif