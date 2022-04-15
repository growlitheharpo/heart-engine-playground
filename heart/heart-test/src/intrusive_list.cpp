/* Copyright (C) 2022 James Keats
*
* This file is part of Heart, a collection of game engine technologies.
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#include <heart/memory/intrusive_list.h>

#include <gtest/gtest.h>

struct SimpleListEntryType
{
	char label = 0;
	HeartIntrusiveListLink link;
};

using SimpleHeartIntrusiveList = HeartIntrusiveList<SimpleListEntryType, &SimpleListEntryType::link>;

TEST(IntrusiveList, EmptyListIterators)
{
	SimpleHeartIntrusiveList list;

	EXPECT_TRUE(list.begin() == list.end());
}

TEST(IntrusiveList, EmptyListSize)
{
	SimpleHeartIntrusiveList list;

	EXPECT_EQ(list.Size(), 0);
	EXPECT_TRUE(list.IsEmpty());
}

TEST(IntrusiveList, PushBack)
{
	SimpleListEntryType a {'a'};

	SimpleHeartIntrusiveList list;
	list.PushBack(&a);

	EXPECT_EQ(list.Size(), 1);
}

TEST(IntrusiveList, PushFront)
{
	SimpleListEntryType a {'a'};

	SimpleHeartIntrusiveList list;
	list.PushFront(&a);

	EXPECT_EQ(list.Size(), 1);
}

TEST(IntrusiveList, Contains)
{
	SimpleListEntryType a {'a'};
	SimpleListEntryType b {'b'};

	SimpleHeartIntrusiveList list;
	list.PushBack(&a);

	EXPECT_EQ(list.Size(), 1);

	EXPECT_TRUE(list.Contains(&a));
	EXPECT_FALSE(list.Contains(&b));
}

TEST(IntrusiveList, Front)
{
	SimpleListEntryType a {'a'};
	SimpleListEntryType b {'b'};

	SimpleHeartIntrusiveList list;
	list.PushBack(&a);
	list.PushBack(&b);

	SimpleListEntryType* front = list.Front();
	EXPECT_EQ(front, &a);

	const SimpleHeartIntrusiveList& listRef = list;
	const SimpleListEntryType* constFront = listRef.Front();
	EXPECT_EQ(constFront, &a);
}

TEST(IntrusiveList, Back)
{
	SimpleListEntryType a {'a'};
	SimpleListEntryType b {'b'};

	SimpleHeartIntrusiveList list;
	list.PushBack(&a);
	list.PushBack(&b);

	SimpleListEntryType* front = list.Back();
	EXPECT_EQ(front, &b);

	const SimpleHeartIntrusiveList& listRef = list;
	const SimpleListEntryType* constFront = listRef.Back();
	EXPECT_EQ(constFront, &b);
}

TEST(IntrusiveList, PopBack)
{
	SimpleListEntryType a {'a'};
	SimpleListEntryType b {'b'};

	SimpleHeartIntrusiveList list;
	list.PushBack(&a);
	list.PushBack(&b);

	EXPECT_EQ(list.Size(), 2);
	EXPECT_TRUE(list.Contains(&a));
	EXPECT_TRUE(list.Contains(&b));

	SimpleListEntryType* entry = list.PopBack();
	EXPECT_EQ(entry, &b);
	EXPECT_FALSE(list.Contains(&b));
	EXPECT_TRUE(list.Contains(&a));
}

TEST(IntrusiveList, PopFront)
{
	SimpleListEntryType a {'a'};
	SimpleListEntryType b {'b'};

	SimpleHeartIntrusiveList list;
	list.PushBack(&a);
	list.PushBack(&b);

	EXPECT_EQ(list.Size(), 2);
	EXPECT_TRUE(list.Contains(&a));
	EXPECT_TRUE(list.Contains(&b));

	SimpleListEntryType* entry = list.PopFront();
	EXPECT_EQ(entry, &a);
	EXPECT_FALSE(list.Contains(&a));
	EXPECT_TRUE(list.Contains(&b));
}

TEST(IntrusiveList, RemovePointer)
{
	SimpleListEntryType a {'a'};

	SimpleHeartIntrusiveList list;
	list.PushBack(&a);

	EXPECT_TRUE(list.Contains(&a));

	list.Remove(&a);

	EXPECT_FALSE(list.Contains(&a));
}

TEST(IntrusiveList, RemoveIterator)
{
	SimpleListEntryType a {'a'};

	SimpleHeartIntrusiveList list;
	list.PushBack(&a);

	EXPECT_TRUE(list.Contains(&a));

	auto iter = list.begin();
	list.Remove(iter);

	EXPECT_FALSE(list.Contains(&a));
}

TEST(IntrusiveList, Clear)
{
	SimpleListEntryType a {'a'};
	SimpleListEntryType b {'b'};
	SimpleListEntryType c {'c'};

	SimpleHeartIntrusiveList list;
	list.PushBack(&a);
	list.PushBack(&b);
	list.PushBack(&c);

	EXPECT_EQ(list.Size(), 3);
	list.Clear();
	EXPECT_EQ(list.Size(), 0);
	EXPECT_FALSE(list.Contains(&a));
	EXPECT_FALSE(list.Contains(&b));
	EXPECT_FALSE(list.Contains(&c));
}

TEST(IntrusiveList, BasicIteration)
{
	SimpleListEntryType a {'a'};
	SimpleListEntryType b {'b'};
	SimpleListEntryType c {'c'};

	int count = 0;

	SimpleHeartIntrusiveList list;
	list.PushBack(&a);
	list.PushBack(&b);
	list.PushBack(&c);

	for (SimpleListEntryType& entry : list)
	{
		if (count == 0)
		{
			EXPECT_EQ(&entry, &a);
		}
		else if (count == 1)
		{
			EXPECT_EQ(&entry, &b);
		}
		else if (count == 2)
		{
			EXPECT_EQ(&entry, &c);
		}

		++count;
	}

	EXPECT_EQ(count, 3);
}

TEST(IntrusiveList, IteratorComparison)
{
	SimpleListEntryType a {'a'};
	SimpleListEntryType b {'b'};
	SimpleListEntryType c {'c'};

	int count = 0;

	SimpleHeartIntrusiveList list;
	list.PushBack(&a);
	list.PushBack(&b);
	list.PushBack(&c);

	auto iteratorA = list.begin();

	auto iteratorB = list.begin();
	++iteratorB;

	auto iteratorC = list.begin();
	++iteratorC;
	++iteratorC;

	auto iteratorD = list.begin();
	++iteratorD;
	++iteratorD;
	++iteratorD;

	EXPECT_LT(iteratorA, iteratorB);
	EXPECT_LT(iteratorA, iteratorC);
	EXPECT_LT(iteratorB, iteratorC);

	EXPECT_LT(iteratorA, list.end());
	EXPECT_LT(iteratorB, list.end());
	EXPECT_LT(iteratorC, list.end());
	EXPECT_EQ(iteratorD, list.end());
}

TEST(IntrusiveList, AddBothEnds)
{
	SimpleListEntryType a {'a'};
	SimpleListEntryType b {'b'};
	SimpleListEntryType c {'c'};

	SimpleHeartIntrusiveList list;

	// Add b first
	list.PushFront(&b);

	// Add a in front of b and c after b
	list.PushFront(&a);
	list.PushBack(&c);

	int count = 0;
	for (SimpleListEntryType& entry : list)
	{
		if (count == 0)
		{
			EXPECT_EQ(&entry, &a);
		}
		else if (count == 1)
		{
			EXPECT_EQ(&entry, &b);
		}
		else if (count == 2)
		{
			EXPECT_EQ(&entry, &c);
		}

		++count;
	}
}

TEST(IntrusiveList, Removal)
{
	SimpleListEntryType a {'a'};
	SimpleListEntryType b {'b'};
	SimpleListEntryType c {'c'};

	SimpleHeartIntrusiveList list;
	list.PushBack(&a);
	list.PushBack(&b);
	list.PushBack(&c);

	EXPECT_EQ(list.Size(), 3);

	list.Remove(&b);

	EXPECT_EQ(list.Size(), 2);

	int count = 0;
	for (SimpleListEntryType& entry : list)
	{
		if (count == 0)
		{
			EXPECT_EQ(&entry, &a);
		}
		else if (count == 1)
		{
			EXPECT_EQ(&entry, &c);
		}

		++count;
	}

	EXPECT_EQ(count, 2);
}

TEST(IntrusiveList, MoveConstructor)
{
	SimpleListEntryType a {'a'};
	SimpleListEntryType b {'b'};
	SimpleListEntryType c {'c'};

	SimpleHeartIntrusiveList listA;
	listA.PushBack(&a);
	listA.PushBack(&b);
	listA.PushBack(&c);

	EXPECT_EQ(listA.Size(), 3);

	SimpleHeartIntrusiveList listB(std::move(listA));

	EXPECT_EQ(listA.Size(), 0);
	EXPECT_EQ(listB.Size(), 3);
	EXPECT_FALSE(listA.Contains(&a));
	EXPECT_FALSE(listA.Contains(&b));
	EXPECT_FALSE(listA.Contains(&c));
	EXPECT_TRUE(listB.Contains(&a));
	EXPECT_TRUE(listB.Contains(&b));
	EXPECT_TRUE(listB.Contains(&c));

	int count = 0;
	for (SimpleListEntryType& entry : listB)
	{
		if (count == 0)
		{
			EXPECT_EQ(&entry, &a);
		}
		else if (count == 1)
		{
			EXPECT_EQ(&entry, &b);
		}
		else if (count == 2)
		{
			EXPECT_EQ(&entry, &c);
		}

		++count;
	}
	EXPECT_EQ(count, 3);

	count = 0;
	for (SimpleListEntryType& entry : listA)
	{
		++count;
	}
	EXPECT_EQ(count, 0);
}

TEST(IntrusveList, MoveAssignment)
{
	SimpleListEntryType a {'a'};
	SimpleListEntryType b {'b'};
	SimpleListEntryType c {'c'};
	SimpleListEntryType d {'d'};

	SimpleHeartIntrusiveList listA;
	listA.PushBack(&a);
	listA.PushBack(&b);
	listA.PushBack(&c);

	EXPECT_EQ(listA.Size(), 3);

	SimpleHeartIntrusiveList listB;
	listB.PushBack(&d);

	EXPECT_EQ(listB.Size(), 1);
	EXPECT_TRUE(listA.Contains(&a));
	EXPECT_TRUE(listB.Contains(&d));

	listB = std::move(listA);

	EXPECT_EQ(listA.Size(), 0);
	EXPECT_EQ(listB.Size(), 3);
	EXPECT_FALSE(listA.Contains(&a));
	EXPECT_FALSE(listA.Contains(&b));
	EXPECT_FALSE(listA.Contains(&c));
	EXPECT_TRUE(listB.Contains(&a));
	EXPECT_TRUE(listB.Contains(&b));
	EXPECT_TRUE(listB.Contains(&c));

	int count = 0;
	for (SimpleListEntryType& entry : listB)
	{
		if (count == 0)
		{
			EXPECT_EQ(&entry, &a);
		}
		else if (count == 1)
		{
			EXPECT_EQ(&entry, &b);
		}
		else if (count == 2)
		{
			EXPECT_EQ(&entry, &c);
		}

		++count;
	}
	EXPECT_EQ(count, 3);

	count = 0;
	for (SimpleListEntryType& entry : listA)
	{
		++count;
	}
	EXPECT_EQ(count, 0);
}
