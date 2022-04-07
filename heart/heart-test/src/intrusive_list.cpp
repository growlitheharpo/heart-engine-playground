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

TEST(IntrusiveList, BasicIteration)
{
	SimpleListEntryType a {'a'};
	SimpleListEntryType b {'b'};
	SimpleListEntryType c {'c'};

	int count = 0;

	SimpleHeartIntrusiveList list;
	list.AddTail(&a);
	list.AddTail(&b);
	list.AddTail(&c);

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
	list.AddTail(&a);
	list.AddTail(&b);
	list.AddTail(&c);

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
	list.AddHead(&b);

	// Add a in front of b and c after b
	list.AddHead(&a);
	list.AddTail(&c);

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
	list.AddTail(&a);
	list.AddTail(&b);
	list.AddTail(&c);

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
