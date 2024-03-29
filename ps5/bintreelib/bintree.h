#include <stdio.h>
#include <cstddef>
#include <cmath>

template <class KeyClass,class ValueClass>
class BinaryTree
{
protected:
	class Node
	{
	public:
		KeyClass key;
		ValueClass value;
		Node *left,*right,*up;
		int height;
		Node() : left(nullptr),right(nullptr),up(nullptr),height(1)
		{
		}
	};
public:

	/// <autoRebalancing implementation: Question 5.1>
	bool autoRebalancing;

	class NodeHandle
	{
	friend BinaryTree <KeyClass,ValueClass>;
	private:
		Node *ptr;
	public:
		inline void Nullify(void)
		{
			ptr=nullptr;
		}
		inline bool IsNull(void) const
		{
			return ptr==nullptr;
		}
		inline bool IsNotNull(void) const
		{
			return ptr!=nullptr;
		}
		inline bool operator==(NodeHandle hd) const
		{
			return this->ptr==hd.ptr;
		}
		inline bool operator!=(NodeHandle hd) const
		{
			return this->ptr!=hd.ptr;
		}
		inline bool operator==(std::nullptr_t) const
		{
			return ptr==nullptr;
		}
		inline bool operator!=(std::nullptr_t) const
		{
			return ptr!=nullptr;
		}
	};
protected:
	///////////////////////////////////////////////////////////////////////////////////////////>

	bool CheckifBalanced(NodeHandle ndHd)              /// CheckifBalanced is a redundancy check to ensure that the tree is balanced as required in this problem
	{
		auto size_left = GetHeight(Left(ndHd));
		auto size_right = GetHeight(Right(ndHd));

		if ((size_left - size_right) > 1 || (size_right - size_left) > 1)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	Node *GetNode(NodeHandle ndHd)
	{
		if(ndHd.IsNotNull())
		{
			return ndHd.ptr;
		}
		return nullptr;
	}
	const Node *GetNode(NodeHandle ndHd) const
	{
		if(ndHd.IsNotNull())
		{
			return ndHd.ptr;
		}
		return nullptr;
	}
	static NodeHandle MakeHandle(Node *nodePtr)
	{
		NodeHandle ndHd;
		ndHd.ptr=nodePtr;
		return ndHd;
	}
	bool UpdateHeight(Node *nodePtr)
	{
		int leftHeight=1,rightHeight=1;
		if(nullptr!=nodePtr->left)
		{
			leftHeight=nodePtr->left->height+1;
		}
		if(nullptr!=nodePtr->right)
		{
			rightHeight=nodePtr->right->height+1;
		}
		int newHeight=1;
		if(leftHeight>rightHeight)
		{
			newHeight=leftHeight;
		}
		else
		{
			newHeight=rightHeight;
		}
		if(newHeight!=nodePtr->height)
		{
			nodePtr->height=newHeight;
			return true;
		}
		return false;
	}
	void UpdateHeightCascade(Node *nodePtr)
	{
		bool first=true;
		while(nullptr!=nodePtr)
		{
			auto changed=UpdateHeight(nodePtr);
			if(true!=first && true!=changed)
			{
				break;
			}
			nodePtr=nodePtr->up;
			first=false;
		}
	}

private:
	Node *root;
	long long int nElem;

public:
	BinaryTree()
	{
		root=nullptr;
		nElem=0;

		/// <autoRebalancing implementation: Question 5.1>
		autoRebalancing = false;
	}
	~BinaryTree()
	{
		CleanUp();
	}
	void CleanUp(void)
	{
		CleanUp(GetNode(RootNode()));
	}
private:
	void CleanUp(Node *nodePtr)
	{
		if(nullptr!=nodePtr)
		{
			CleanUp(nodePtr->left);
			CleanUp(nodePtr->right);
			delete nodePtr;
		}
	}
public:
	static NodeHandle Null(void)
	{
		NodeHandle ndHd;
		ndHd.ptr=nullptr;
		return ndHd;
	}
	NodeHandle RootNode(void) const
	{
		return MakeHandle(root);
	}
	NodeHandle Left(NodeHandle ndHd) const
	{
		auto nodePtr=GetNode(ndHd);
		if(nullptr!=nodePtr)
		{
			return MakeHandle(nodePtr->left);
		}
		return Null();
	}
	NodeHandle Up(NodeHandle ndHd) const
	{
		auto nodePtr=GetNode(ndHd);
		if(nullptr!=nodePtr)
		{
			return MakeHandle(nodePtr->up);
		}
		return Null();
	}
	NodeHandle Right(NodeHandle ndHd) const
	{
		auto nodePtr=GetNode(ndHd);
		if(nullptr!=nodePtr)
		{
			return MakeHandle(nodePtr->right);
		}
		return Null();
	}

	long long int GetN(void) const
	{
		return nElem;
	}
	const KeyClass &GetKey(NodeHandle ndHd) const
	{
		// This will crash if ndHd==nullptr.  Therefore, ndHd must be non-null to use this function.
		return GetNode(ndHd)->key;
	}
	ValueClass &GetValue(NodeHandle ndHd)
	{
		// This will crash if ndHd==nullptr.  Therefore, ndHd must be non-null to use this function.
		return GetNode(ndHd)->value;
	}
	const ValueClass &GetValue(NodeHandle ndHd) const
	{
		// This will crash if ndHd==nullptr.  Therefore, ndHd must be non-null to use this function.
		return GetNode(ndHd)->value;
	}
	NodeHandle FindNode(const KeyClass &key) const
	{
		auto ndHd=RootNode();
		while(nullptr!=ndHd)
		{
			if(key==GetKey(ndHd))
			{
				return ndHd;
			}
			if(key<GetKey(ndHd))
			{
				ndHd=Left(ndHd);
			}
			else
			{
				ndHd=Right(ndHd);
			}
		}
		return Null();
	}
	bool IsKeyIncluded(const KeyClass &key) const
	{
		return FindNode(key).IsNotNull();
	}
	int GetHeight(NodeHandle ndHd) const
	{
		auto nodePtr=GetNode(ndHd);
		if(nullptr!=nodePtr)
		{
			return nodePtr->height;
		}
		return 0;
	}

	NodeHandle Insert(const KeyClass &key,const ValueClass &value)
	{
		auto newNode=new Node;
		newNode->key=key;
		newNode->value=value;

		auto ndHd=RootNode();
		if(ndHd.IsNull())
		{
			root=newNode;
		}
		else
		{
			while(ndHd.IsNotNull())
			{
				if(key<GetKey(ndHd))
				{
					if(Left(ndHd)!=nullptr)
					{
						ndHd=Left(ndHd);
					}
					else
					{
						GetNode(ndHd)->left=newNode;
						newNode->up=GetNode(ndHd);
						break;
					}
				}
				else
				{
					if(Right(ndHd)!=nullptr)
					{
						ndHd=Right(ndHd);
					}
					else
					{
						GetNode(ndHd)->right=newNode;
						newNode->up=GetNode(ndHd);
						break;
					}
				}
			}
		}
		UpdateHeightCascade(newNode);
		nElem++;

		/// <autoRebalancing implementation: Question 5.3>
		Node* rebalancer = newNode->up;

		if (autoRebalancing == true)
		{
			while (rebalancer != nullptr)
			{
				Rebalance(rebalancer);
				rebalancer = rebalancer->up;
			}
		}


		return MakeHandle(newNode);
	}

	NodeHandle First(void) const
	{
		auto ndHd = RootNode();
		while (Left(ndHd).IsNotNull())
		{
			ndHd = Left(ndHd);
		}
		return ndHd;
	}
	NodeHandle FindNext(NodeHandle ndHd) const
	{
		auto rightHd=Right(ndHd);
		if(rightHd.IsNotNull())
		{
			// Has a right sub-tree.
			// The next node is the left-most of the right sub-tree.
			ndHd=Right(ndHd);
			while(Left(ndHd).IsNotNull())
			{
				ndHd=Left(ndHd);
			}
			return ndHd;
		}
		else
		{
			// Does not have a right sub-tree.
			// Go up until it goes up from the left.
			while(ndHd.IsNotNull())
			{
				auto upHd=Up(ndHd);
				if(upHd.IsNotNull() && ndHd==Left(upHd))
				{
					return upHd;
				}
				ndHd=upHd;
			}
			return Null();
		}
	}
	////////////////////////////////////////////////////////////////////////>

	/// <Unit Test: Question 6>

	NodeHandle Last(void) const // Do it in the assignment.
	{
		auto ndHd = RootNode();
		while (Right(ndHd).IsNotNull())
		{
			ndHd = Right(ndHd);
		}
		return ndHd;
	}
	
	NodeHandle FindPrev(NodeHandle ndHd) const // Do it in the assignment.
	{
		auto leftHd = Left(ndHd);
		if (leftHd.IsNotNull())
		{
			ndHd = Left(ndHd);
			while (Right(ndHd).IsNotNull())
			{
				ndHd = Right(ndHd);
			}
			return ndHd;
		}
		else
		{
			while (ndHd.IsNotNull())
			{
				auto upHd = Up(ndHd);
				if (upHd.IsNotNull() && ndHd == Right(upHd))
				{
					return upHd;
				}
				ndHd = upHd;
			}
			return Null();
		}
	}
	////////////////////////////////////////////////////////////////////////<


private:
	NodeHandle RightMostOf(NodeHandle ndHd)
	{
		while(Right(ndHd).IsNotNull())
		{
			ndHd=Right(ndHd);
		}
		return ndHd;
	}
	bool SimpleDetach(NodeHandle ndHd)
	{
		if(ndHd.IsNotNull())
		{
			auto upHd=Up(ndHd);
			auto rightHd=Right(ndHd);
			auto leftHd=Left(ndHd);
			if(rightHd.IsNull() && leftHd.IsNull())
			{
				if(upHd.IsNull()) // ndHd is a root.
				{
					root=nullptr;
				}
				else
				{
					auto upPtr=GetNode(upHd);
					if(Left(upHd)==ndHd)
					{
						upPtr->left=nullptr;
					}
					else if(Right(upHd)==ndHd)
					{
						upPtr->right=nullptr;
					}
					else
					{
						fprintf(stderr,"Error! Internal Tree Data Structure is broken.\n");
						return false;
					}
				}
				UpdateHeightCascade(GetNode(upHd));
				return true;
			}
			else if(rightHd.IsNull())
			{
				if(upHd.IsNull())
				{
					root=GetNode(leftHd);
					root->up=nullptr;
					return true;
				}
				else
				{
					// Connect upHd and leftHd
					auto upPtr=GetNode(upHd);
					auto leftPtr=GetNode(leftHd);
					if(Left(upHd)==ndHd)
					{
						upPtr->left=leftPtr;
						leftPtr->up=upPtr;
						UpdateHeightCascade(GetNode(upHd));
						return true;
					}
					else if(Right(upHd)==ndHd)
					{
						upPtr->right=leftPtr;
						leftPtr->up=upPtr;
						UpdateHeightCascade(GetNode(upHd));
						return true;
					}
					else
					{
						fprintf(stderr,"Error! Internal Tree Data Structure is broken.\n");
						return false;
					}
				}
			}
			else if(leftHd.IsNull())
			{
				if(upHd.IsNull())
				{
					root=GetNode(rightHd);
					root->up=nullptr;
					return true;
				}
				else
				{
					// Connect upHd and rightHd
					auto upPtr=GetNode(upHd);
					auto rightPtr=GetNode(rightHd);
					if(Left(upHd)==ndHd)
					{
						upPtr->left=rightPtr;
						rightPtr->up=upPtr;
						UpdateHeightCascade(GetNode(upHd));
						return true;
					}
					else if(Right(upHd)==ndHd)
					{
						upPtr->right=rightPtr;
						rightPtr->up=upPtr;
						UpdateHeightCascade(GetNode(upHd));
						return true;
					}
					else
					{
						fprintf(stderr,"Error! Internal Tree Data Structure is broken.\n");
						return false;
					}
				}
			}
			else
			{
				return false;
			}
		}
		return false;
	}
public:
	bool Delete(NodeHandle ndHd)
	{
		if(true==SimpleDetach(ndHd))
		{
			Node * rebalance_up = GetNode(ndHd)->up;
			delete GetNode(ndHd);
			--nElem; 

			//////////////////////////////////////////////////////////////////////////////////>
			/// <autoRebalancing implementation: Question 5.4>

			if (autoRebalancing == true)
			{
				Rebalance(rebalance_up);
			}
			//////////////////////////////////////////////////////////////////////////////////<

			return true;
		}
		else if(ndHd.IsNotNull())
		{
			// Right most of left. Always Simple-Detachable.
			// Also, since SimpleDetach of itself has failed, it must have a left sub-tree.
			auto RMOL=RightMostOf(Left(ndHd));

			if (true == SimpleDetach(RMOL))
			{
				// Now, RMOL needs to take position of ndHd.
				auto RMOLptr=GetNode(RMOL);
				auto upPtr=GetNode(Up(ndHd));
				auto leftPtr=GetNode(Left(ndHd));
				auto rightPtr=GetNode(Right(ndHd));

				auto upOfRMOLptr=RMOLptr->up;
				Node * rebalancer = RMOLptr->up;

				if(upOfRMOLptr==GetNode(ndHd))
				{
					upOfRMOLptr=RMOLptr;	// Now it is correct.
				}

				if(nullptr==upPtr)
				{
					root=RMOLptr;
					root->up=nullptr;
				}
				else if(upPtr->left==GetNode(ndHd))
				{
					upPtr->left=RMOLptr;
					RMOLptr->up=upPtr;
				}
				else if(upPtr->right==GetNode(ndHd))
				{
					upPtr->right=RMOLptr;
					RMOLptr->up=upPtr;
				}
				else
				{
					fprintf(stderr,"Error! Internal Tree Data Structure is broken.\n");
					return false;
				}

				RMOLptr->left=leftPtr;
				if(nullptr!=leftPtr)
				{
					leftPtr->up=RMOLptr;
				}
				RMOLptr->right=rightPtr;
				if(nullptr!=rightPtr)
				{
					rightPtr->up=RMOLptr;
				}

				UpdateHeightCascade(RMOLptr);

				delete GetNode(ndHd);
				--nElem;

				//////////////////////////////////////////////////////////////////////////////////>
				/// <autoRebalancing implementation: Question 5.4>

				if (autoRebalancing == true)
				{
					while (rebalancer != nullptr)
					{
						Rebalance(rebalancer);
						rebalancer = rebalancer->up;
					}
				}
				//////////////////////////////////////////////////////////////////////////////////<

				return true;
			}
		}
		return false; // Cannot delete a null node.
	}

	bool RotateLeft(NodeHandle ndHd)
	{
		auto nodePtr=GetNode(ndHd);
		if(nullptr!=nodePtr && nullptr!=nodePtr->right)
		{
			auto rightPtr=nodePtr->right;
			auto leftOfRight=nodePtr->right->left;

			if(nullptr==nodePtr->up)
			{
				root=rightPtr;
				rightPtr->up=nullptr;
			}
			else
			{
				rightPtr->up=nodePtr->up;
				if(nodePtr->up->left==nodePtr)
				{
					nodePtr->up->left=rightPtr;
				}
				else
				{
					nodePtr->up->right=rightPtr;
				}
			}

			rightPtr->left=nodePtr;
			nodePtr->up=rightPtr;

			nodePtr->right=leftOfRight;
			if(nullptr!=leftOfRight)
			{
				leftOfRight->up=nodePtr;
			}
			UpdateHeight(nodePtr);
			UpdateHeightCascade(rightPtr);
			return true;
		}
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////>

	/// <Right Rotation implementation: Question 4.2>

	bool RotateRight(NodeHandle ndHd)
	{
		auto nodePtr = GetNode(ndHd);
		if (nullptr != nodePtr && nullptr != nodePtr->left)
		{
			auto leftPtr = nodePtr->left;
			auto rightOfLeft = nodePtr->left->right;

			if (nullptr == nodePtr->up)
			{
				root = leftPtr;
				leftPtr->up = nullptr;
			}
			else
			{
				leftPtr->up = nodePtr->up;
				if (nodePtr->up->left == nodePtr)
				{
					nodePtr->up->left = leftPtr;
				}
				else
				{
					nodePtr->up->right = leftPtr;
				}
			}

			leftPtr->right = nodePtr;
			nodePtr->up = leftPtr;

			nodePtr->left = rightOfLeft;
			if (nullptr != rightOfLeft)
			{
				rightOfLeft->up = nodePtr;
			}
			UpdateHeight(nodePtr);
			UpdateHeightCascade(leftPtr);
			return true;
		}
		return false;
	}




	/// <Tree to Vine Implementation : Question 4.4>

	void TreeToVine()
	{
		auto vine_tail = root;

		if (nullptr == root)
		{
			return;
		}

		while (nullptr != vine_tail)
		{
			if (vine_tail->left != nullptr)
			{
				RotateRight_PointerRebalanced(vine_tail);
				vine_tail = vine_tail->up;
			}
			else
			{
				vine_tail = vine_tail->right;
			}
		}
	}

	

	/// <Vine to Tree Implementation : Question 4.5>
	/// Requires Compress Function as described in the paper
	
	void Compress(int n)
	{
		auto scanner = root;
		int shift = 0, temp = 0;

		while (shift < n)
		{
			for (int i = 0; i < temp; i++)
			{
				scanner = scanner->right;
			}

			RotateLeft_PointerRebalanced(scanner);

			/// update compression parameters
			shift = shift + 1;
			temp = 1;

			scanner = scanner->up;

		}
	}

	void VineToTree()
	{
		auto size = root->height;
		auto leaf_exp = (int)(log(size + 1) / log(2));
		auto leaf_count = size + 1 - pow(2, leaf_exp);

		Compress(leaf_count);
		size = size - leaf_count;

		while (size > 1)
		{
			Compress((int)size/2);                        
			size = (int)size / 2;
		}
	}


	/// <Rebalancing : Question 5>
	bool RotateRight_PointerRebalanced(Node* selected_node) // since pointer based rebalancing occurs, the rotate operations must read a pointer as input argument now, hence this function has been created
	{
		auto nodePtr = selected_node;
		if (nullptr != nodePtr && nullptr != nodePtr->left)
		{
			auto leftPtr = nodePtr->left;
			auto rightOfLeft = nodePtr->left->right;

			if (nullptr == nodePtr->up)
			{
				root = leftPtr;
				leftPtr->up = nullptr;
			}
			else
			{
				leftPtr->up = nodePtr->up;
				if (nodePtr->up->left == nodePtr)
				{
					nodePtr->up->left = leftPtr;
				}
				else
				{
					nodePtr->up->right = leftPtr;
				}
			}

			leftPtr->right = nodePtr;
			nodePtr->up = leftPtr;

			nodePtr->left = rightOfLeft;
			if (nullptr != rightOfLeft)
			{
				rightOfLeft->up = nodePtr;
			}
			UpdateHeight(nodePtr);
			UpdateHeightCascade(leftPtr);
			return true;
		}
		return false;
	}

	bool RotateLeft_PointerRebalanced(Node* selected_node) // since pointer based rebalancing occurs, the rotate operations must read a pointer as input argument now, hence this function has been created
	{
		auto nodePtr = selected_node;
		if (nullptr != nodePtr && nullptr != nodePtr->right)
		{
			auto rightPtr = nodePtr->right;
			auto leftOfRight = nodePtr->right->left;

			if (nullptr == nodePtr->up)
			{
				root = rightPtr;
				rightPtr->up = nullptr;
			}
			else
			{
				rightPtr->up = nodePtr->up;
				if (nodePtr->up->left == nodePtr)
				{
					nodePtr->up->left = rightPtr;
				}
				else
				{
					nodePtr->up->right = rightPtr;
				}
			}

			rightPtr->left = nodePtr;
			nodePtr->up = rightPtr;

			nodePtr->right = leftOfRight;
			if (nullptr != leftOfRight)
			{
				leftOfRight->up = nodePtr;
			}
			UpdateHeight(nodePtr);
			UpdateHeightCascade(rightPtr);
			return true;
		}
		return false;
	}


	void Rebalance(Node* selected_node)
	{
		// First we do balancing for child branches
		int depth_r = 0;
		int depth_l = 0;

		if (selected_node->right != nullptr)
		{
			depth_r = selected_node->right->height;
		}
		if (selected_node->left != nullptr)
		{
			depth_l = selected_node->left->height;
		}

		int delta_depth = (depth_r - depth_l);

		if (delta_depth == -1 || delta_depth == 0 || delta_depth == 1)
		{
			return;
		}
		else
		{
			if (depth_r > depth_l)
			{
				// Here we do balancing for grandchild branches for the criteria where (depth_r > depth_l) 
				int depth_r_r = 0;
				int depth_r_l = 0;

				if (selected_node->right->left != nullptr)
				{
					depth_r_l = selected_node->right->left->height;
				}
				if (selected_node->right->right != nullptr)
				{
					depth_r_r = selected_node->right->right->height;
				}
				// Rebalancing operation
				if (depth_r_r > depth_r_l)
				{
					RotateLeft_PointerRebalanced(selected_node);
					return;
				}
				else
				{
					RotateRight_PointerRebalanced(selected_node->right);
					RotateLeft_PointerRebalanced(selected_node);
					return;
				}
			}
			else
			{
				// Here we do balancing for grandchild branches for the criteria where (depth_r < depth_l) 
				int depth_l_r = 0;
				int depth_l_l = 0;

				if (selected_node->left->right != nullptr)
				{
					depth_l_r = selected_node->left->right->height;
				}
				if (selected_node->left->left != nullptr)
				{
					depth_l_l = selected_node->left->left->height;
				}

				// Rebalancing operation
				if (depth_l_l > depth_l_r)
				{
					RotateRight_PointerRebalanced(selected_node);
					return;
				}
				else
				{
					RotateLeft_PointerRebalanced(selected_node->left);
					RotateRight_PointerRebalanced(selected_node);
					return;
				}
			}
		}

	}
	///////////////////////////////////////////////////////////////////////////////////////////<
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////<
};


