#pragma once

#include <string>

class IFactory
{
public:
	virtual bool setIntProperty(void*, std::string, int) = 0;
	virtual bool setStringProperty(void*, std::string, std::string&) = 0;
	virtual bool setObjectProperty(void*, std::string, void*) = 0;

	virtual void* CreateObject() = 0;
	virtual std::string GetClassName() = 0;
	virtual IFactory* NextClass() = 0;
};

template <typename T>
class FactoryT : public IFactory
{
public:
	std::map<std::string, void (T::*)(int)> *getIntMap()
	{
		static std::map<std::string, void (T::*)(int)> IntMap;
		return &IntMap;
	};
	std::map<std::string, void (T::*)(std::string)> *getStrMap()
	{
		static std::map<std::string, void (T::*)(std::string)> StrMap;
		return &StrMap;
	};
	std::map<std::string, void (T::*)(void*)> *getObjMap()
	{
		static std::map<std::string, void (T::*)(void*)> ObjMap;
		return &ObjMap;
	};
	bool setIntProperty(void* obj, std::string name, int value)
	{
		typename std::map<std::string, void (T::*)(int) >::iterator iter;
		iter = getIntMap()->find(name);
		if (iter != getIntMap()->end()) {
			((T*)obj->*(iter->second))(value);
			return true;
		}
		else
			return false;
	}
	bool setStringProperty(void* obj, std::string name, std::string value)
	{
		typename std::map<std::string, void (T::*)(std::string) >::iterator iter;
		iter = getStrMap()->find(name);
		if (iter != getStrMap()->end()) {
			((T*)obj->*(iter->second))(value);
			return true;
		}
		else
			return false;
	}
	bool setObjectProperty(void* obj, std::string name, void* value)
	{
		typename std::map<std::string, void (T::*)(void*)>::iterator iter;
		iter = getObjMap()->find(name);
		if (iter != getObjMap()->end()) {
			((T*)obj->*(iter->second))(value);
			return true;
		}
		else
			return false;
	}
	void* createObject()
	{
		return new T();
	}
	virtual std::string getClassName() {
		return std::string("FactoryT");
	}
	virtual IFactory* nextClass() {
		return NULL;
	}
};