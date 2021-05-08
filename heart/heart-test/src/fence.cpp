#include <heart/sync/fence.h>
#include <gtest/gtest.h>

#include <thread>

TEST(HeartFence, Synchronization)
{
	HeartFence fence1;
	HeartFence fence2;

	EXPECT_TRUE(fence1.Test(0)) << "Fence should start at 0";
	EXPECT_FALSE(fence1.Test(1)) << "Fence should start at 0";
	
	fence1.Signal(1);
	EXPECT_TRUE(fence1.Test(1)) << "Fence::Test should succeed for a value that has been signaled";
	EXPECT_TRUE(fence1.Test(0)) << "Fence::Test should succeed for past values";

	auto t = std::thread([&fence1, &fence2]() {
		fence2.Wait(1);

		EXPECT_FALSE(fence2.Test(2));
		EXPECT_FALSE(fence1.Test(2));
		fence1.Signal(2);
		fence1.Wait(3);
	});

	EXPECT_FALSE(fence2.Test(1));
	EXPECT_FALSE(fence1.Test(2));
	fence2.Signal(1);
	fence1.Wait(2);
	EXPECT_FALSE(fence1.Test(3));
	fence1.Signal(3);

	t.join();
}
