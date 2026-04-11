#ifndef __JSON_HELPER_H__
#define __JSON_HELPER_H__

#include "cocos2d.h"
#include "json/rapidjson.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"
#include <string>
#include <vector>

class JsonHelper
{
public:
	// 基本转换
	static std::string toString(const rapidjson::Document& doc);
	static std::string toString(const rapidjson::Value& value);
	static rapidjson::Document parse(const std::string& json);

	// 从字符串获取值
	static bool getBool(const rapidjson::Value& obj, const char* key, bool defaultValue = false);
	static int getInt(const rapidjson::Value& obj, const char* key, int defaultValue = 0);
	static float getFloat(const rapidjson::Value& obj, const char* key, float defaultValue = 0.0f);
	static double getDouble(const rapidjson::Value& obj, const char* key, double defaultValue = 0.0);
	static std::string getString(const rapidjson::Value& obj, const char* key, const std::string& defaultValue = "");

	// 检查键是否存在
	static bool hasKey(const rapidjson::Value& obj, const char* key);

	// 创建常用 JSON 对象
	static rapidjson::Document createDocument();
	static rapidjson::Value createValue(rapidjson::Document::AllocatorType& allocator);

	// 数组操作
	template<typename T>
	static std::vector<T> getArray(const rapidjson::Value& obj, const char* key)
	{
		std::vector<T> result;
		if (!obj.HasMember(key) || !obj[key].IsArray())
			return result;

		const auto& arr = obj[key];
		for (rapidjson::SizeType i = 0; i < arr.Size(); i++)
		{
			if constexpr (std::is_same_v<T, int>)
				result.push_back(arr[i].GetInt());
			else if constexpr (std::is_same_v<T, float>)
				result.push_back(arr[i].GetFloat());
			else if constexpr (std::is_same_v<T, double>)
				result.push_back(arr[i].GetDouble());
			else if constexpr (std::is_same_v<T, std::string>)
				result.push_back(arr[i].GetString());
			else if constexpr (std::is_same_v<T, bool>)
				result.push_back(arr[i].GetBool());
		}
		return result;
	}

	// 数组设置
	static void setIntArray(rapidjson::Value& obj, const char* key,
		const std::vector<int>& arr, rapidjson::Document::AllocatorType& allocator);
	static void setFloatArray(rapidjson::Value& obj, const char* key,
		const std::vector<float>& arr, rapidjson::Document::AllocatorType& allocator);
	static void setStringArray(rapidjson::Value& obj, const char* key,
		const std::vector<std::string>& arr, rapidjson::Document::AllocatorType& allocator);

	// 调试输出
	static void printJson(const rapidjson::Document& doc);
	static void printJson(const rapidjson::Value& value);
};

#endif // __JSON_HELPER_H__