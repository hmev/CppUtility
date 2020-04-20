#pragma once

template <typename Key, typename Value>
class OrderedList
{
public:
	struct Node
	{
		Key k;
		Value v;
		Node* left;
		Node* right;

		Node(const Key& _k, const Value& _v) : k(_k), v(_v) {}
	};

	Node* root;

public:
	OrderedList() { root = nullptr; }
	OrderedList(Node* root) { root = nullptr; }

	Node* insert(const Key& k, const Value& v)
	{
		if (root == nullptr)
		{
			return root = new Node(k, v);
		}
		else
		{
			return insertImpl(root, k, v);
		}
	}

	Node* find(const Key& k)
	{
		if (root == nullptr)
		{
			return nullptr;
		}
		else 
		{
			return findImpl(root, k);
		}
	}

	Node* findNotLess(const Key& k);
	
	bool remove(const Key& k)
	{
		if (root == nullptr)
		{
			return false;
		}
		else
		{
			return removeImpl(root, k);
		}
	}

private:
	bool insertImpl(Node* root, const Key& k, const Value& v)
	{
		if (k == root->k)
		{
			// to do;
		}
		else if (k < root->k)
		{
			if (root->left != nullptr)
			{
				insertImpl(root->left, k, v);
			}
			else
			{
				root->left = new Node(k, v);
				return root->left;
			}
		}
		else
		{
			if (root->right != nullptr)
			{
				insertImpl(root->right, k, v);
			}
			else
			{
				root->right = new Node(k, v);
				return root->right;
			}
		}
	}

	Node* findImpl(Node* root, const Key& k)
	{
		if (k == root->k)
		{
			return root;
		}
		else if (k < root->k)
		{
			return findImpl(root->left, k);
		}
		else
		{
			return findImpl(root->right, k);
		}
	}

	bool removeImpl(Node* root, const Key& k)
	{
		if (k == root->k)
		{
			rotate();
		}
		else if ()
	}
};