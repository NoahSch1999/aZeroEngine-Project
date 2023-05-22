#pragma once
#include <list>

class SelectionList
{
private:
	std::list<int> selection;

public:
	SelectionList() = default;
	~SelectionList() = default;

	void Add(int _id)
	{
		if (Selected(_id))
			return;

		selection.push_back(_id);
	}

	void Remove(int _id)
	{
		std::list<int>::iterator remove = std::remove(selection.begin(), selection.end(), _id);
		selection.erase(remove);
	}

	bool Selected(int _id)
	{
		std::list<int>::iterator found = std::find(selection.begin(), selection.end(), _id);
		return found != selection.end();
	}

	int GetRoot()
	{
		if (selection.empty())
			return -1;

		return selection.front();
	}

	void Clear()
	{
		selection.clear();
	}

	bool Empty()
	{
		return selection.empty();
	}

	int Count()
	{
		return selection.size();
	}

	std::list<int>::iterator Begin()
	{
		return selection.begin();
	}

	std::list<int>::iterator End()
	{
		return selection.end();
	}
};