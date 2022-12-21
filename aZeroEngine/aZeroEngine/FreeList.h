#pragma once
#include <deque>
#include <mutex>

class FreeList
{
private:
	std::deque<int>queue;
	int maxValues = -1;
	std::mutex mutex;
public:
	FreeList() = default;

	FreeList(int _startValue, int _endValue)
	{
		if (_startValue > _endValue)
			throw;

		for (int i = _startValue; i < _endValue; i++)
		{
			queue.push_back(i);
		}

		maxValues = _endValue - _startValue;
	}

	void Init(int _startValue, int _endValue)
	{
		if (_startValue > _endValue)
			throw;

		for (int i = _startValue; i < _endValue; i++)
		{
			queue.push_back(i);
		}

		maxValues = _endValue - _startValue;
	}

	int GetValue()
	{
		std::unique_lock<std::mutex>lock(mutex);
		int element = queue.front();
		queue.pop_front();
		return element;
	}

	void FreeValue(int _value)
	{
		std::unique_lock<std::mutex>lock(mutex);
		queue.push_front(_value);
	}

	int ValuesLeft()
	{
		std::unique_lock<std::mutex>lock(mutex);
		return (int)queue.size();
	}

	int GetMaxValues()
	{
		return maxValues;
	}
};