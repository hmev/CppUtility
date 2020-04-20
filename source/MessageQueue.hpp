#pragma once

#include <any>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

class MessageQueue
{
protected:
	bool IsFull() const
	{
		return m_queue.size() == m_maxSize;
	}
	bool IsEmpty() const
	{
		return m_queue.empty();
	}

public:
	MessageQueue(int maxSize) : m_maxSize(maxSize) {}
	
	template <typename T>
	void Put(const T&& x)
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		while (IsFull())
		{
			std::cout << "Buffer is full, please wait..." << std::endl;
			m_notFull.wait(m_mutex);
		}
		m_queue.push(x);
		m_notEmpty.notify_one();
	}

	template <typename T>
	void Take(T&& x)
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		while (IsEmpty())
		{
			m_notEmpty.wait(m_mutex);
		}
		x = std::any_cast<T>(m_queue.front());
		m_queue.pop();
		m_notFull.notify_one();
	}
	bool Empty()
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		return m_queue.empty();
	}
	bool Full()
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		return m_queue.size() == m_maxSize;
	}
	size_t Size()
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		return m_queue.size();
	}
	size_t Count()
	{
		return m_queue.size();
	}

private:
	std::queue<std::any> m_queue;
	std::mutex m_mutex;
	std::condition_variable_any m_notEmpty;
	std::condition_variable_any m_notFull;
	size_t m_maxSize;
};