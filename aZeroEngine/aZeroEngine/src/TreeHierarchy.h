#pragma once
#include <vector>
#include <unordered_map>

class TreeHierearchy
{
public:
	struct Node
	{
		Node(int _id)
			:id(_id)
		{

		}

		int id = -1;
		Node* parent = nullptr;
		std::vector<Node*> children;
	};

	TreeHierearchy() = default;
	~TreeHierearchy() = default;


private:
	std::unordered_map<int, Node> nodes;

public:

	void Add(int _id)
	{
		if (nodes.count(_id) == 0)
		{
			nodes.emplace(_id, Node(_id));
		}
	}

	void Remove(int _id)
	{
		if (nodes.count(_id) > 0)
		{
			Node* node = &nodes.at(_id);
			if (node->parent)
			{
				UnParent(node->parent->id, _id);
				node->parent = nullptr;
				for (Node* child : node->children)
				{
					child->parent = nullptr;
				}
			}
			nodes.erase(_id);
		}
	}

	void Parent(int _parentID, int _childID)
	{
		if (nodes.count(_parentID) > 0)
		{
			if (nodes.count(_childID) > 0)
			{
				Node* parentNode = &nodes.at(_parentID);
				Node* childNode = &nodes.at(_childID);

				// Disables circular parenting
				if (IsChildInBranch(childNode, parentNode->id))
					return;

				if (childNode->parent) // Unparents if already parented to something else
				{
					UnParent(childNode->parent->id, childNode->id);
				}

				parentNode->children.push_back(childNode);
				childNode->parent = parentNode;
			}
		}
	}

	void UnParent(int _parentID, int _childID)
	{
		if (nodes.count(_parentID) > 0)
		{
			if (nodes.count(_childID) > 0)
			{
				Node* parentNode = &nodes.at(_parentID);
				Node* childNode = &nodes.at(_childID);
				for (int i = 0; i < parentNode->children.size(); i++)
				{
					if (parentNode->children[i]->id == childNode->id)
					{
						parentNode->children.erase(parentNode->children.begin() + i);
						childNode->parent = nullptr;
						return;
					}
				}
			}
		}
	}

	Node* GetNode(int _id)
	{
		if (nodes.count(_id) > 0)
			return &nodes.at(_id);
		else
			return nullptr;
	}

	bool IsChildInBranch(Node* _node, int _idToLookFor)
	{
		for (Node* child : _node->children)
		{
			if (child->id == _idToLookFor)
				return true;

			if (IsChildInBranch(child, _idToLookFor))
				return true;
		}

		return false;
	}

};