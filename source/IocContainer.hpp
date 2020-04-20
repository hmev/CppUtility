#pragma once

#include <any>
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include <type_traits>
//#include <boost/noncopyable.hpp>

#include "utility/Utility.hpp"

class IocContainer : NonCopyable
{
public:
	IocContainer(void) {}
	~IocContainer(void) {}

	template <class T>
	void RegisterSimple(std::string strKey)
	{
		typedef T* I;
		std::function<I()> function = Construct<I, T>::invoke;
		RegisterType(strKey, function);
	}

	template <class I, class T, typename... Ts>
	typename std::enable_if<std::is_base_of<I, T>::value>::type 
		RegisterType(std::string strKey)
	{
		std::function<I* (Ts...)> function = Construct<I*, T, Ts...>::invoke;
		RegisterType(strKey, function);
	}

	template <class I>
	I* Resolve(std::string strKey)
	{
		if (m_creatorMap.find(strKey) == m_creatorMap.end())
		{
			return nullptr;
		}

		std::any resolver = m_creatorMap[strKey];
		std::function<I* ()> function = std::any_cast<std::function<I* ()>>(resolver);

		return function();
	}

	template <class I>
	std::shared_ptr<I> ResolveShared(std::string strKey)
	{
		auto b = Resolve<I>(strKey);
		return std::shared_ptr<I>(b);
	}

	template <class I, typename... Ts>
	I* Resolve(std::string strKey, Ts... Args)
	{
		if (m_creatorMap.find(strKey) == m_creatorMap.end())
		{
			return nullptr;
		}

		std::any resolver = m_creatorMap[strKey];
		std::function<I* (Ts...)> function = std::any_cast<std::function<I* (Ts...)>>(resolver);

		return function(Args...);
	}

	template <class I, typename... Ts>
	std::shared_ptr<I> ResolveShared(std::string strKey, Ts... args)
	{
		auto b = Resolve<I, Ts...>(strKey, args...);
		return std::shared_ptr<I>(b);
	}

private:
	template <typename I, typename T, typename... Ts>	
	struct Construct
	{
		static I invoke(Ts... args) { return I(new T(args...)); }
	};

	void RegisterType(std::string strKey, std::any constructor)
	{
		if (m_creatorMap.find(strKey) != m_creatorMap.end())
		{
			throw std::exception("this key has already existed!");
		}

		m_creatorMap.insert(std::make_pair(strKey, constructor));
	}

private:
	std::unordered_map<std::string, std::any> m_creatorMap;
};