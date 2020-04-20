#pragma once
#include <any>
#include <queue>
#include <thread>
#include <unordered_map>

#include "MessageQueue.hpp"

// TODO - Replace the Guid to true Guid.
#include <string>
typedef std::string Guid;


// Only support basic functionality.
// Need to support multi-thread
template<class L, class E, void (L::*F)(const E&)>
class Dispatcher
{
public:
	typedef L ListenerType;
	typedef E EventType;

	Dispatcher() {};
	virtual ~Dispatcher() { m_Listeners.clear(); }

	void addListener(ListenerType* pListener)
	{
		m_Listeners.insert(std::make_pair(pListener->UniqueID, pListener));
	}
	void removeListener(ListenerType* pListener)
	{
		m_Listeners.remove(pListener->UniqueID);
	}
	void dispatch(const EventType& event)
	{
		for (auto iter : m_Listeners)
		{
			auto pListener(iter.second);
			if (pListener)
			{
				(pListener->*F)(event);
			}
		}
	}

protected:
	std::unordered_map<Guid, ListenerType*> m_Listeners;
};

class DispatcherAsync
{
public:
	DispatcherAsync(){}

	template <typename F>
	void BeginInvoke(F&& f)
	{
		msgQ.Put(f);
		msgQ.append(f);
	}

	template <typename F>
	void Invoke(F&& f)
	{
		msgQ.Put(f);
		return;
	}

	static MessageQueue msgQ;
};