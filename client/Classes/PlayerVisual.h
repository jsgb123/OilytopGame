#ifndef __PLAYER_VISUAL_H__
#define __PLAYER_VISUAL_H__

#include "cocos2d.h"

class PlayerVisual : public cocos2d::Node
{
public:
	static PlayerVisual* create();
	virtual bool init() override;

	void setPlayerColor(const cocos2d::Color4F& color);
	void setPlayerName(const std::string& name);
	void setIsLocalPlayer(bool isLocal);
	void setMoving(bool moving);
	void setDirection(float direction);
	void setPupilOffset(const cocos2d::Vec2& offset);
	void updateAnimation(float delta);

private:
	void createVisuals();

	// Œ∆¿Ì…˙≥…
	cocos2d::Texture2D* createGlowTexture();
	cocos2d::Texture2D* createCircleTexture(const cocos2d::Color4F& color);
	cocos2d::Texture2D* createCircleGradientTexture();
	cocos2d::Texture2D* createEyeTexture();
	cocos2d::Texture2D* createPupilTexture();
	cocos2d::Texture2D* createMouthTexture(bool isMoving);
	cocos2d::Texture2D* createStarTexture();
	cocos2d::Texture2D* createCrownTexture();

private:
	cocos2d::Node* _bodyContainer;
	cocos2d::Sprite* _glow;
	cocos2d::Sprite* _body;
	cocos2d::Sprite* _highlight;
	cocos2d::Sprite* _leftEye;
	cocos2d::Sprite* _rightEye;
	cocos2d::Sprite* _leftPupil;
	cocos2d::Sprite* _rightPupil;
	cocos2d::Sprite* _mouth;
	cocos2d::Sprite* _decoration;
	cocos2d::Label* _nameLabel;

	cocos2d::Color4F _playerColor;
	std::string _playerName;
	bool _isLocalPlayer;
	bool _isMoving;
	float _direction;
	cocos2d::Vec2 _pupilOffset;
	cocos2d::Vec2 _targetPupilOffset;
	float _time;

	const float EYE_X_OFFSET = 10.0f;
	const float EYE_Y_OFFSET = 10.0f;
	const float PUPIL_MAX_OFFSET = 4.0f;
};

#endif