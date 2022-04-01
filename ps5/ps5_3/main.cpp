#include "bintree.h"


int main(void)
{
	BinaryTree <int, int> tree;

	auto first_val = tree->First();
	auto findnext_first_val = tree->FindPrev(first_val);

	auto last_val = tree->Last();
	auto findnext_last_val = tree->FindNext(last_val);

	if (findnext_first_val.IsNotNull())
	{
		return 1;
	}


	if (findnext_last_val.IsNotNull())
	{
		return 1;
	}


	return 0;
}
