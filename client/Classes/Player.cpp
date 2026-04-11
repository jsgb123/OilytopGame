#include "Player.h"
#include "GameConfig.h"

Player::Player(int id, const std::string& name)
	: _id(id)
	, _name(name)
	, _level(1)
	, _exp(0)
	, _maxExp(100)
	, _hp(100)
	, _maxHp(100)
	, _position(0, 0)
	, _targetPosition(0, 0)
	, _direction(0)
	, _isMoving(false)
	, _speed(GameConfig::PLAYER_SPEED)
{
}
void Player::setLevel(int level)
{
	_level = level;
	// 根据等级重新计算最大生命值和最大经验值
	_maxHp = 100 + (_level - 1) * 10;
	_maxExp = 100 * _level;

	// 升级时满血
	_hp = _maxHp;
}

void Player::setExp(long exp)
{
	_exp = exp;
}

void Player::setHp(int hp)
{
	_hp = std::max(0, std::min(hp, _maxHp));
}

void Player::takeDamage(int damage)
{
	_hp = std::max(0, _hp - damage);
}

void Player::heal(int amount)
{
	_hp = std::min(_maxHp, _hp + amount);
}

void Player::addExp(long exp)
{
	_exp += exp;

	// 检查升级
	while (_exp >= _maxExp && _level < 100)  // 最高100级
	{
		_exp -= _maxExp;
		_level++;
		_maxExp = 100 * _level;
		_maxHp = 100 + (_level - 1) * 10;
		_hp = _maxHp;  // 升级满血
	}
}
void Player::setDirection(float direction)
{
	_direction = direction;

	// 根据方向计算瞳孔偏移（最大偏移范围）
	float maxOffset = 4.0f;  // 最大偏移像素

	float rad = CC_DEGREES_TO_RADIANS(direction);
	_targetPupilOffset = cocos2d::Vec2(cosf(rad) * maxOffset, sinf(rad) * maxOffset);
}

void Player::updatePupilOffset(float delta)
{
	// 平滑移动瞳孔
	_pupilOffset = _pupilOffset.lerp(_targetPupilOffset, _pupilSmoothSpeed * delta);
}
void Player::setPosition(const cocos2d::Vec2& pos)
{
    _position = pos;
}

void Player::update(float delta)
{
    if (_isMoving)
    {
        cocos2d::Vec2 dir = (_targetPosition - _position);
        dir.normalize();
        cocos2d::Vec2 newPos = _position + dir * _speed * delta;
        
        if (newPos.distance(_targetPosition) < 1.0f)
        {
            _position = _targetPosition;
            _isMoving = false;
        }
        else
        {
            _position = newPos;
        }
    }
}

void Player::moveTo(const cocos2d::Vec2& target)
{
    _targetPosition = target;
    _isMoving = true;
    
    if (target != _position)
    {
        cocos2d::Vec2 dir = target - _position;
        dir.normalize();
        _direction = CC_RADIANS_TO_DEGREES(atan2f(dir.y, dir.x));
    }
}

void Player::setPositionImmediate(const cocos2d::Vec2& pos)
{
    _position = pos;
    _targetPosition = pos;
    _isMoving = false;

}

void Player::stopMoving()
{
    _isMoving = false;
    _targetPosition = _position;
}

cocos2d::Vec2 Player::getGridPosition(int tileSize) const
{
    return cocos2d::Vec2(
        (int)(_position.x / tileSize),
        (int)(_position.y / tileSize)
    );
}