#include "JsonHelper.h"

USING_NS_CC;

std::string JsonHelper::toString(const rapidjson::Document& doc)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	return buffer.GetString();
}

std::string JsonHelper::toString(const rapidjson::Value& value)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	value.Accept(writer);
	return buffer.GetString();
}

rapidjson::Document JsonHelper::parse(const std::string& json)
{
	rapidjson::Document doc;
	doc.Parse(json.c_str());
	return doc;
}

bool JsonHelper::getBool(const rapidjson::Value& obj, const char* key, bool defaultValue)
{
	if (obj.HasMember(key) && obj[key].IsBool())
		return obj[key].GetBool();
	return defaultValue;
}

int JsonHelper::getInt(const rapidjson::Value& obj, const char* key, int defaultValue)
{
	if (obj.HasMember(key) && obj[key].IsInt())
		return obj[key].GetInt();
	return defaultValue;
}

float JsonHelper::getFloat(const rapidjson::Value& obj, const char* key, float defaultValue)
{
	if (obj.HasMember(key) && (obj[key].IsFloat() || obj[key].IsDouble()))
		return obj[key].GetFloat();
	return defaultValue;
}

double JsonHelper::getDouble(const rapidjson::Value& obj, const char* key, double defaultValue)
{
	if (obj.HasMember(key) && (obj[key].IsFloat() || obj[key].IsDouble()))
		return obj[key].GetDouble();
	return defaultValue;
}

std::string JsonHelper::getString(const rapidjson::Value& obj, const char* key, const std::string& defaultValue)
{
	if (obj.HasMember(key) && obj[key].IsString())
		return obj[key].GetString();
	return defaultValue;
}

bool JsonHelper::hasKey(const rapidjson::Value& obj, const char* key)
{
	return obj.HasMember(key);
}

rapidjson::Document JsonHelper::createDocument()
{
	rapidjson::Document doc;
	doc.SetObject();
	return doc;
}

rapidjson::Value JsonHelper::createValue(rapidjson::Document::AllocatorType& allocator)
{
	return rapidjson::Value(rapidjson::kObjectType);
}

void JsonHelper::setIntArray(rapidjson::Value& obj, const char* key,
	const std::vector<int>& arr, rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value arrVal(rapidjson::kArrayType);
	for (int val : arr)
	{
		arrVal.PushBack(val, allocator);
	}
	obj.AddMember(rapidjson::Value(key, allocator), arrVal, allocator);
}

void JsonHelper::setFloatArray(rapidjson::Value& obj, const char* key,
	const std::vector<float>& arr, rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value arrVal(rapidjson::kArrayType);
	for (float val : arr)
	{
		arrVal.PushBack(val, allocator);
	}
	obj.AddMember(rapidjson::Value(key, allocator), arrVal, allocator);
}

void JsonHelper::setStringArray(rapidjson::Value& obj, const char* key,
	const std::vector<std::string>& arr, rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value arrVal(rapidjson::kArrayType);
	for (const auto& val : arr)
	{
		arrVal.PushBack(rapidjson::Value(val.c_str(), allocator), allocator);
	}
	obj.AddMember(rapidjson::Value(key, allocator), arrVal, allocator);
}

void JsonHelper::printJson(const rapidjson::Document& doc)
{
	std::string json = toString(doc);
	CCLOG("JSON: %s", json.c_str());
}

void JsonHelper::printJson(const rapidjson::Value& value)
{
	std::string json = toString(value);
	CCLOG("JSON: %s", json.c_str());
}