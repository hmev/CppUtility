#pragma once

#include <string>
#include <functional>
#include <map>
#include <Any>
#include "FunctionTraits.hpp"
#include "NonCopyable.hpp"

class MessageBus : NonCopyable
{
public:
	template <typename F>
	void Attach(F&& f, const std::string& strChannel = "")
	{
		auto func = to_function(std::forward<F>(f));
		Add(strChannel, std::move(func));
	}

	template <typename R>
	void SendReq(const std::string& strChannel = "")
	{
		using func_type = std::function<R()>;
		std::string strMsgType = strChannel + typeid(func_type).name();
		auto range = m_map.equal_range(strMsgType);
		for (Iterator iter = range.first; iter != range.second; iter++)
		{
			(std::any_cast<func_type>(iter->second))();			
		}
	}

	template <typename R, typename... Args>
	void Remove(const std::string& strChannel = "")
	{
		using func_type = std::function<R(Args...)>; // typename function_traits<void(CArgs)>::stl_function_type;
		std::string strMsgType = strChannel + typeid(func_type).name();
		int count = m_map.count(strMsgType);
		auto range = m_map.equal_range(strMsgType);
		m_map.erase(range.first, range.second);
	}

private:
	template <typename F>
	void Add(const std::string& strChannel, F&& f)
	{
		std::string strMsgType = strChannel + typeid(F).name();
		m_map.emplace(std::move(strMsgType), std::forward<F>(f));
	}

private:
	std::multimap<std::string, std::any> m_map;
	typedef std::multimap<std::string, std::any>::iterator Iterator;
};