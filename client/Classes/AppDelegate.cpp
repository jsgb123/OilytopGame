#include "AppDelegate.h"
#include "LoginScene.h"


#if USE_AUDIO_ENGINE && USE_SIMPLE_AUDIO_ENGINE
#error "Don't use AudioEngine and SimpleAudioEngine at the same time. Please just select one in your game!"
#endif

#if USE_AUDIO_ENGINE
#include "audio/include/AudioEngine.h"
using namespace cocos2d::experimental;
#elif USE_SIMPLE_AUDIO_ENGINE
#include "audio/include/SimpleAudioEngine.h"
using namespace CocosDenshion;
#endif

USING_NS_CC;

static cocos2d::Size designResolutionSize = cocos2d::Size(1024, 600);

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate()
{
#if USE_AUDIO_ENGINE
	AudioEngine::end();
#elif USE_SIMPLE_AUDIO_ENGINE
	SimpleAudioEngine::end();
#endif
}

void AppDelegate::initGLContextAttrs()
{
	GLContextAttrs glContextAttrs = { 8, 8, 8, 8, 24, 8, 0 };

	GLView::setGLContextAttrs(glContextAttrs);
}

static int register_all_packages()
{
	return 0;
}

bool AppDelegate::applicationDidFinishLaunching() {
	auto director = Director::getInstance();
	if (!director)
	{
		CCLOG("Director::getInstance() failed!");
		return false;
	}
	auto glview = director->getOpenGLView();
	if (!glview) {
		glview = GLViewImpl::createWithRect("OilyTop", cocos2d::Rect(0, 0, designResolutionSize.width, designResolutionSize.height));
		if (!glview)
		{
			CCLOG("GLViewImpl::createWithRect failed!");
			return false;
		}
		director->setOpenGLView(glview);
	}
	glview->setDesignResolutionSize(designResolutionSize.width,
		designResolutionSize.height,
		ResolutionPolicy::SHOW_ALL);

	director->setDisplayStats(false);//左下角信息

	director->setAnimationInterval(1.0f / 60);

	auto frameSize = glview->getFrameSize();

	float scaleX = (float)frameSize.width / designResolutionSize.width;
	float scaleY = (float)frameSize.height / designResolutionSize.height;
	float scale = std::min(scaleX, scaleY);
	director->setContentScaleFactor(scale);

	register_all_packages();

	auto scene = LoginScene::createScene();

	director->runWithScene(scene);

	return true;
}

void AppDelegate::applicationDidEnterBackground() {
	Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
	AudioEngine::pauseAll();
#elif USE_SIMPLE_AUDIO_ENGINE
	SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
	SimpleAudioEngine::getInstance()->pauseAllEffects();
#endif
}

void AppDelegate::applicationWillEnterForeground() {
	Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
	AudioEngine::resumeAll();
#elif USE_SIMPLE_AUDIO_ENGINE
	SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
	SimpleAudioEngine::getInstance()->resumeAllEffects();
#endif
	auto scene = Director::getInstance()->getRunningScene();
	if (scene)
	{
		// 发送自定义事件
		Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_COME_TO_FOREGROUND);
	}
}