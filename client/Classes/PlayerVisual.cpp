#include "PlayerVisual.h"
#include "GameConfig.h"

USING_NS_CC;

PlayerVisual* PlayerVisual::create()
{
	PlayerVisual* ret = new PlayerVisual();
	if (ret && ret->init())
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool PlayerVisual::init()
{
	if (!Node::init())
		return false;

	_playerColor = Color4F(0.3f, 0.6f, 0.9f, 1.0f);
	_playerName = "";
	_isLocalPlayer = false;
	_isMoving = false;
	_direction = 0;
	_pupilOffset = Vec2::ZERO;
	_targetPupilOffset = Vec2::ZERO;
	_time = 0;

	createVisuals();

	return true;
}

void PlayerVisual::createVisuals()
{
	_bodyContainer = Node::create();
	this->addChild(_bodyContainer);

	// 1. 外发光光晕
	_glow = Sprite::createWithTexture(createGlowTexture());
	_glow->setColor(Color3B(255, 255, 255));
	_glow->setOpacity(76);
	_glow->setScale(1.2f);
	_bodyContainer->addChild(_glow);

	// 2. 主体圆形
	_body = Sprite::createWithTexture(createCircleTexture(_playerColor));
	_bodyContainer->addChild(_body);

	// 3. 内圈高光
	_highlight = Sprite::createWithTexture(createCircleGradientTexture());
	_highlight->setColor(Color3B::WHITE);
	_highlight->setOpacity(153);
	_highlight->setScale(0.7f);
	_bodyContainer->addChild(_highlight);

	// 4. 眼睛
	_leftEye = Sprite::createWithTexture(createEyeTexture());
	_leftEye->setPosition(Vec2(-EYE_X_OFFSET, EYE_Y_OFFSET));
	_leftEye->setColor(Color3B::WHITE);
	_bodyContainer->addChild(_leftEye);

	_rightEye = Sprite::createWithTexture(createEyeTexture());
	_rightEye->setPosition(Vec2(EYE_X_OFFSET, EYE_Y_OFFSET));
	_rightEye->setColor(Color3B::WHITE);
	_bodyContainer->addChild(_rightEye);

	// 5. 瞳孔
	_leftPupil = Sprite::createWithTexture(createPupilTexture());
	_leftPupil->setPosition(Vec2(-EYE_X_OFFSET, EYE_Y_OFFSET));
	_leftPupil->setColor(Color3B::BLACK);
	_bodyContainer->addChild(_leftPupil);

	_rightPupil = Sprite::createWithTexture(createPupilTexture());
	_rightPupil->setPosition(Vec2(EYE_X_OFFSET, EYE_Y_OFFSET));
	_rightPupil->setColor(Color3B::BLACK);
	_bodyContainer->addChild(_rightPupil);

	// 6. 嘴巴
	_mouth = Sprite::createWithTexture(createMouthTexture(false));
	_mouth->setPosition(Vec2(0, -4));
	_bodyContainer->addChild(_mouth);

	// 7. 头顶装饰
	if (_isLocalPlayer)
	{
		_decoration = Sprite::createWithTexture(createCrownTexture());
		_decoration->setPosition(Vec2(0, 20));
		_decoration->setScale(0.5f);
	}
	else
	{
		_decoration = Sprite::createWithTexture(createStarTexture());
		_decoration->setPosition(Vec2(0, 20));
		_decoration->setScale(0.4f);
		_decoration->setColor(Color3B(255, 215, 0));
	}
	_bodyContainer->addChild(_decoration);

	// 8. 名字标签
	_nameLabel = Label::createWithSystemFont(_playerName, GameConfig::FONT_SONG, 14);
	_nameLabel->setHorizontalAlignment(TextHAlignment::CENTER);
	_nameLabel->setPosition(Vec2(0, 30));
	_nameLabel->setColor(Color3B::WHITE);
	this->addChild(_nameLabel);
}

void PlayerVisual::updateAnimation(float delta)
{
	_time += delta;

	// 呼吸效果
	float breath = (sinf(_time * 3.0f) + 1.0f) / 2.0f;
	float scale = 0.97f + breath * 0.06f;
	_bodyContainer->setScale(scale);

	// 光晕脉冲
	float alpha = 0.2f + breath * 0.6f;
	_glow->setOpacity(alpha * 255);
	float glowScale = 1.1f + breath * 0.5f;
	_glow->setScale(glowScale);

	// 瞳孔平滑移动
	_pupilOffset = _pupilOffset.lerp(_targetPupilOffset, 10.0f * delta);
	_leftPupil->setPosition(Vec2(-EYE_X_OFFSET + _pupilOffset.x, EYE_Y_OFFSET + _pupilOffset.y));
	_rightPupil->setPosition(Vec2(EYE_X_OFFSET + _pupilOffset.x, EYE_Y_OFFSET + _pupilOffset.y));
}

// ========== 纹理生成方法（直接使用 Texture2D）==========
//发光脉冲
Texture2D* PlayerVisual::createGlowTexture()
{
	int size = GameConfig::TILE_SIZE;
	int dataSize = size * size * 4;
	unsigned char* data = new unsigned char[dataSize];
	memset(data, 0, dataSize);

	for (int x = 0; x < size; x++)
	{
		for (int y = 0; y < size; y++)
		{
			float dx = x - size / 2.0f;
			float dy = y - size / 2.0f;
			float dist = sqrtf(dx*dx + dy*dy);
			float alpha = (1.0f - dist / (size / 2.0f)) * 0.5f;
			if (alpha > 0)
			{
				int idx = (y * size + x) * 4;
				data[idx] = 255;
				data[idx + 1] = 255;
				data[idx + 2] = 255;
				data[idx + 3] = (unsigned char)(alpha * 255);
			}
		}
	}

	Texture2D* texture = new Texture2D();
	texture->initWithData(data, dataSize, Texture2D::PixelFormat::RGBA8888, size, size, Size(size, size));
	delete[] data;
	return texture;
}
//圆形外轮廓
Texture2D* PlayerVisual::createCircleTexture(const Color4F& color)
{
	int size = 32;
	int dataSize = size * size * 4;
	unsigned char* data = new unsigned char[dataSize];
	memset(data, 0, dataSize);

	for (int x = 0; x < size; x++)
	{
		for (int y = 0; y < size; y++)
		{
			float dx = x - size / 2.0f;
			float dy = y - size / 2.0f;
			float dist = sqrtf(dx*dx + dy*dy);
			if (dist < size / 2.0f)
			{
				int idx = (y * size + x) * 4;
				data[idx] = (unsigned char)(color.r * 255);
				data[idx + 1] = (unsigned char)(color.g * 255);
				data[idx + 2] = (unsigned char)(color.b * 255);
				data[idx + 3] = 255;
			}
		}
	}

	Texture2D* texture = new Texture2D();
	texture->initWithData(data, dataSize, Texture2D::PixelFormat::RGBA8888, size, size, Size(size, size));
	delete[] data;
	return texture;
}
//圆形内轮廓
Texture2D* PlayerVisual::createCircleGradientTexture()
{
	int size = 28;
	int dataSize = size * size * 4;
	unsigned char* data = new unsigned char[dataSize];
	memset(data, 0, dataSize);

	for (int x = 0; x < size; x++)
	{
		for (int y = 0; y < size; y++)
		{
			float dx = x - size / 2.0f;
			float dy = y - size / 2.0f;
			float dist = sqrtf(dx*dx + dy*dy);
			if (dist < size / 2.0f)
			{
				float brightness = 1.0f - dist / (size / 2.0f) * 0.5f;
				int idx = (y * size + x) * 4;
				unsigned char val = (unsigned char)(brightness * 255);
				data[idx] = val;
				data[idx + 1] = val;
				data[idx + 2] = val;
				data[idx + 3] = 153;
			}
		}
	}

	Texture2D* texture = new Texture2D();
	texture->initWithData(data, dataSize, Texture2D::PixelFormat::RGBA8888, size, size, Size(size, size));
	delete[] data;
	return texture;
}
//眼睛
Texture2D* PlayerVisual::createEyeTexture()
{
	int size = 13;
	int dataSize = size * size * 4;
	unsigned char* data = new unsigned char[dataSize];
	memset(data, 0, dataSize);

	for (int x = 0; x < size; x++)
	{
		for (int y = 0; y < size; y++)
		{
			float dx = x - size / 2.0f;
			float dy = y - size / 2.0f;
			float dist = sqrtf(dx*dx + dy*dy);
			if (dist < size / 2.0f)
			{
				int idx = (y * size + x) * 4;
				data[idx] = 255;
				data[idx + 1] = 255;
				data[idx + 2] = 255;
				data[idx + 3] = 255;
			}
		}
	}

	Texture2D* texture = new Texture2D();
	texture->initWithData(data, dataSize, Texture2D::PixelFormat::RGBA8888, size, size, Size(size, size));
	delete[] data;
	return texture;
}
//眼珠
Texture2D* PlayerVisual::createPupilTexture()
{
	int size = 5;
	int dataSize = size * size * 4;
	unsigned char* data = new unsigned char[dataSize];
	memset(data, 0, dataSize);

	for (int i = 0; i < dataSize; i += 4)
	{
		data[i] = 0;
		data[i + 1] = 0;
		data[i + 2] = 0;
		data[i + 3] = 255;
	}

	Texture2D* texture = new Texture2D();
	texture->initWithData(data, dataSize, Texture2D::PixelFormat::RGBA8888, size, size, Size(size, size));
	delete[] data;
	return texture;
}

Texture2D* PlayerVisual::createMouthTexture(bool isMoving)
{
	int width = 16;
	int height = 8;
	int dataSize = width * height * 4;
	unsigned char* data = new unsigned char[dataSize];
	memset(data, 0, dataSize);

	if (isMoving)
	{
		// O形嘴
		// O形嘴 - 圆形
		int centerX = width / 2;
		int centerY = height / 2;
		int radius = 4;  // 半径3像素

		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				int dx = x - centerX;
				int dy = y - centerY;
				if (dx*dx + dy*dy <= radius*radius)
				{
					int idx = (y * width + x) * 4;
					data[idx] = 0;
					data[idx + 1] = 0;
					data[idx + 2] = 0;
					data[idx + 3] = 255;
				}
			}
		}
	}
	else
	{
		// 微笑
		for (int x = 3; x < width - 3; x++)
		{
			float t = (x - 3.0f) / (width - 7.0f);
			int y = 5 + (int)(sinf(t * M_PI) * 2);
			y = std::max(0, std::min(y, height - 1));
			int idx = (y * width + x) * 4;
			data[idx] = 0;
			data[idx + 1] = 0;
			data[idx + 2] = 0;
			data[idx + 3] = 255;
		}
	}

	Texture2D* texture = new Texture2D();
	texture->initWithData(data, dataSize, Texture2D::PixelFormat::RGBA8888, width, height, Size(width, height));
	delete[] data;
	return texture;
}

Texture2D* PlayerVisual::createStarTexture()
{
	int size = 32;
	int dataSize = size * size * 4;
	unsigned char* data = new unsigned char[dataSize];
	memset(data, 0, dataSize);

	// 绘制五角星轮廓
	int center = size / 2;
	int radius = 10;

	for (int angle = 0; angle < 360; angle += 36)
	{
		float rad = angle * M_PI / 180.0f;
		int x1 = center + (int)(radius * cos(rad));
		int y1 = center + (int)(radius * sin(rad));

		rad = (angle + 36) * M_PI / 180.0f;
		int x2 = center + (int)(radius * 0.5f * cos(rad));
		int y2 = center + (int)(radius * 0.5f * sin(rad));

		if (x1 >= 0 && x1 < size && y1 >= 0 && y1 < size)
		{
			int idx = (y1 * size + x1) * 4;
			data[idx] = 255;
			data[idx + 1] = 215;
			data[idx + 2] = 0;
			data[idx + 3] = 255;
		}
		if (x2 >= 0 && x2 < size && y2 >= 0 && y2 < size)
		{
			int idx = (y2 * size + x2) * 4;
			data[idx] = 255;
			data[idx + 1] = 215;
			data[idx + 2] = 0;
			data[idx + 3] = 255;
		}
	}

	Texture2D* texture = new Texture2D();
	texture->initWithData(data, dataSize, Texture2D::PixelFormat::RGBA8888, size, size, Size(size, size));
	delete[] data;
	return texture;
}

Texture2D* PlayerVisual::createCrownTexture()
{
	int size = 32;
	int dataSize = size * size * 4;
	unsigned char* data = new unsigned char[dataSize];
	memset(data, 0, dataSize);

	// 皇冠形状
	for (int x = 8; x <= 16; x++)
	{
		int idx = (12 * size + x) * 4;
		data[idx] = 255;
		data[idx + 1] = 215;
		data[idx + 2] = 0;
		data[idx + 3] = 255;
	}

	int points[][2] = { { 12,4 },{ 8,8 },{ 10,8 },{ 14,8 },{ 16,8 } };
	for (int i = 0; i < 5; i++)
	{
		int x = points[i][0];
		int y = points[i][1];
		int idx = (y * size + x) * 4;
		data[idx] = 255;
		data[idx + 1] = 215;
		data[idx + 2] = 0;
		data[idx + 3] = 255;
	}

	Texture2D* texture = new Texture2D();
	texture->initWithData(data, dataSize, Texture2D::PixelFormat::RGBA8888, size, size, Size(size, size));
	delete[] data;
	return texture;
}

// ========== 属性设置方法 ==========

void PlayerVisual::setPlayerColor(const Color4F& color)
{
	_playerColor = color;
	if (_body)
	{
		_body->setTexture(createCircleTexture(color));
	}
}

void PlayerVisual::setPlayerName(const std::string& name)
{
	_playerName = name;
	if (_nameLabel)
	{
		_nameLabel->setString(name);
	}
}

void PlayerVisual::setIsLocalPlayer(bool isLocal)
{
	_isLocalPlayer = isLocal;

	if (_decoration)
	{
		_decoration->removeFromParent();
		if (_isLocalPlayer)
		{
			_decoration = Sprite::createWithTexture(createCrownTexture());
			_decoration->setPosition(Vec2(0, 20));
			_decoration->setScale(0.5f);
		}
		else
		{
			_decoration = Sprite::createWithTexture(createStarTexture());
			_decoration->setPosition(Vec2(0, 20));
			_decoration->setScale(0.4f);
			_decoration->setColor(Color3B(255, 215, 0));
		}
		_bodyContainer->addChild(_decoration);
	}
}

void PlayerVisual::setMoving(bool moving)
{
	if (_isMoving == moving) return;
	_isMoving = moving;

	if (_mouth)
	{
		_mouth->setTexture(createMouthTexture(moving));
	}
}

void PlayerVisual::setDirection(float direction)
{
	_direction = direction;

	float rad = CC_DEGREES_TO_RADIANS(direction);
	_targetPupilOffset = Vec2(cosf(rad) * PUPIL_MAX_OFFSET, sinf(rad) * PUPIL_MAX_OFFSET);
}

void PlayerVisual::setPupilOffset(const Vec2& offset)
{
	_targetPupilOffset = offset;
}