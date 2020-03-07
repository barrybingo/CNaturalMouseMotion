#include <vector>
#include "MovementFactory.h"
#include "MockStructs.h"
#include "gtest/gtest.h"

using namespace NaturalMouseMotion;

static constexpr double SMALL_DELTA = 10e-6;
static constexpr int SCREEN_WIDTH = 500;
static constexpr int SCREEN_HEIGHT = 500;

#define EXPECT_ARRAY_EQ(a, b) { \
    EXPECT_EQ(a.size(), b.size()); \
    size_t element_count = std::min(a.size(), b.size()); \
    for(size_t i = 0; i < element_count; ++i) { EXPECT_NEAR(a[i], b[i], SMALL_DELTA); } \
} \


struct ConstantSpeedManager
{
	ConstantSpeedManager(time_type time): time(time)
	{}

	std::pair<Flow *, time_type> operator()(double /* distance */)
	{
        return {&flow, time};
	}

private:
	time_type time;
    Flow flow{std::vector<double>({100})};
};

struct NoOvershootManager : public OvershootManager
{
	int getOvershoots(const Flow* /* flow */, time_type /* mouseMovementMs */, double /* distance */) override
	{
		return 0;
	}

	Point<int> getOvershootAmount(double /* distanceToRealTargetX */, double /* distanceToRealTargetY */, time_type /* mouseMovementMs */, int /* overshootsRemaining */) override
	{
		return {0, 0};
	}

	time_type deriveNextMouseMovementTimeMs(time_type /* mouseMovementMs */, int /* overshootsRemaining*/) override
	{
		return 0;
	}
};


struct MultiOvershootManager : public OvershootManager
{
    MultiOvershootManager(std::list<Point<int>> points): points(points)
    {}

	int getOvershoots(const Flow* /* flow */, time_type /* mouseMovementMs */, double /* distance */) override
	{
		return static_cast<int>(points.size());
	}

	Point<int> getOvershootAmount(double /* distanceToRealTargetX */, double /* distanceToRealTargetY */, time_type /* mouseMovementMs */, int /* overshootsRemaining */) override
	{
        if (points.empty())
            return {0, 0};
        else
        {
            auto pt = points.front();
            points.pop_front();
            return pt;
        }
	}

	time_type deriveNextMouseMovementTimeMs(time_type mouseMovementMs, int /* overshootsRemaining */) override
	{
		return mouseMovementMs / 2;
	}

private:
    std::list<Point<int>> points;
};

FlowCharacteristicsContainer SingleElementArray_100 = FlowCharacteristicsContainer(1, {100.0});

TEST(MovementFactoryTest, testSingleMovement)
{
    MotionNature nature;
    nature.getFlowWithTime = GetFlowWithTimeFunc{ConstantSpeedManager(100)};
    nature.overshootManager = std::shared_ptr<OvershootManager>(new NoOvershootManager());
    nature.systemCalls = std::shared_ptr<SystemCalls>(new MockSystemCalls(SCREEN_WIDTH, SCREEN_HEIGHT));

    MovementFactory factory{nature, 50, 51};

    auto movements = factory.createMovements({100, 100});
    ASSERT_EQ(1, movements.size());
    EXPECT_EQ(50, movements.front().destX);
    EXPECT_EQ(51, movements.front().destY);
    EXPECT_EQ(100, movements.front().time);
    EXPECT_NEAR(-50, movements.front().xDistance, SMALL_DELTA);
    EXPECT_NEAR(-49, movements.front().yDistance, SMALL_DELTA);
    EXPECT_ARRAY_EQ(SingleElementArray_100, movements.front().flow->getFlowCharacteristics());
}

TEST(MovementFactoryTest, testMultipleMovement)
{
    MotionNature nature;
    nature.getFlowWithTime = GetFlowWithTimeFunc{ConstantSpeedManager(100)};
    nature.overshootManager = std::shared_ptr<OvershootManager>(new MultiOvershootManager({{5,5}, {-5,-5}}));
    nature.systemCalls = std::shared_ptr<SystemCalls>(new MockSystemCalls(SCREEN_WIDTH, SCREEN_HEIGHT));

    MovementFactory factory{nature, 50, 150};

    auto movements = factory.createMovements({100, 100});
    ASSERT_EQ(3, movements.size());

    Movement first = movements.front();
    movements.pop_front();
    EXPECT_EQ(55, first.destX);
    EXPECT_EQ(155, first.destY);
    EXPECT_EQ(100, first.time);
    EXPECT_NEAR(-45, first.xDistance, SMALL_DELTA);
    EXPECT_NEAR(55, first.yDistance, SMALL_DELTA);
    EXPECT_NEAR(std::hypot(first.xDistance, first.yDistance), first.distance, SMALL_DELTA);
    EXPECT_ARRAY_EQ(SingleElementArray_100, first.flow->getFlowCharacteristics());

    Movement second = movements.front();
    movements.pop_front();
    EXPECT_EQ(45, second.destX);
    EXPECT_EQ(145, second.destY);
    EXPECT_EQ(50, second.time);
    EXPECT_NEAR(-10, second.xDistance, SMALL_DELTA);
    EXPECT_NEAR(-10, second.yDistance, SMALL_DELTA);
    EXPECT_NEAR(std::hypot(second.xDistance, second.yDistance), second.distance, SMALL_DELTA);
    EXPECT_ARRAY_EQ(SingleElementArray_100, second.flow->getFlowCharacteristics());


    Movement third = movements.front();
    movements.pop_front();
    EXPECT_EQ(50, third.destX);
    EXPECT_EQ(150, third.destY);
    EXPECT_EQ(50, third.time);
    EXPECT_NEAR(5, third.xDistance, SMALL_DELTA);
    EXPECT_NEAR(5, third.yDistance, SMALL_DELTA);
    EXPECT_NEAR(std::hypot(third.xDistance, third.yDistance), third.distance, SMALL_DELTA);
    EXPECT_ARRAY_EQ(SingleElementArray_100, third.flow->getFlowCharacteristics());
}

TEST(MovementFactoryTest, testZeroOffsetOvershootsRemovedFromEnd)
{
    MotionNature nature;
    nature.getFlowWithTime = GetFlowWithTimeFunc{ConstantSpeedManager(64)};
    nature.overshootManager = std::shared_ptr<OvershootManager>(new MultiOvershootManager({{5,5}, {0,0}, {1,1}, {0,0}, {0,0}}));
    nature.systemCalls = std::shared_ptr<SystemCalls>(new MockSystemCalls(SCREEN_WIDTH, SCREEN_HEIGHT));

    MovementFactory factory{nature, 50, 150};

    auto movements = factory.createMovements({100, 100});
    ASSERT_EQ(4, movements.size()); // 3 overshoots and 1 final approach to destination

    Movement first = movements.front();
    movements.pop_front();
    EXPECT_EQ(55, first.destX);
    EXPECT_EQ(155, first.destY);
    EXPECT_EQ(64, first.time);
    EXPECT_NEAR(-45, first.xDistance, SMALL_DELTA);
    EXPECT_NEAR(55, first.yDistance, SMALL_DELTA);
    EXPECT_NEAR(std::hypot(first.xDistance, first.yDistance), first.distance, SMALL_DELTA);
    EXPECT_ARRAY_EQ(SingleElementArray_100, first.flow->getFlowCharacteristics());

    Movement second = movements.front(); // 0-offset in the middle is not removed, this one actually hits destination.
    movements.pop_front();
    EXPECT_EQ(50, second.destX);
    EXPECT_EQ(150, second.destY);
    EXPECT_EQ(32, second.time);
    EXPECT_NEAR(-5, second.xDistance, SMALL_DELTA);
    EXPECT_NEAR(-5, second.yDistance, SMALL_DELTA);
    EXPECT_NEAR(std::hypot(second.xDistance, second.yDistance), second.distance, SMALL_DELTA);
    EXPECT_ARRAY_EQ(SingleElementArray_100, second.flow->getFlowCharacteristics());

    Movement third = movements.front();
    movements.pop_front();
    EXPECT_EQ(51, third.destX);
    EXPECT_EQ(151, third.destY);
    EXPECT_EQ(16, third.time);
    EXPECT_NEAR(1, third.xDistance, SMALL_DELTA);
    EXPECT_NEAR(1, third.yDistance, SMALL_DELTA);
    EXPECT_NEAR(std::hypot(third.xDistance, third.yDistance), third.distance, SMALL_DELTA);
    EXPECT_ARRAY_EQ(SingleElementArray_100, third.flow->getFlowCharacteristics());

    Movement fourth = movements.front();
    movements.pop_front();
    EXPECT_EQ(50, fourth.destX);
    EXPECT_EQ(150, fourth.destY);
    EXPECT_EQ(32, fourth.time);
    EXPECT_NEAR(-1, fourth.xDistance, SMALL_DELTA);
    EXPECT_NEAR(-1, fourth.yDistance, SMALL_DELTA);
    EXPECT_NEAR(std::hypot(fourth.xDistance, fourth.yDistance), fourth.distance, SMALL_DELTA);
    EXPECT_ARRAY_EQ(SingleElementArray_100, fourth.flow->getFlowCharacteristics());
}

TEST(MovementFactoryTest, testZeroOffsetOvershootsRemovedFromEndIfAllZero)
{
    MotionNature nature;
    nature.getFlowWithTime = GetFlowWithTimeFunc{ConstantSpeedManager(100)};
    nature.overshootManager = std::shared_ptr<OvershootManager>(new MultiOvershootManager({{0,0}, {0,0}, {0,0}}));
    nature.systemCalls = std::shared_ptr<SystemCalls>(new MockSystemCalls(SCREEN_WIDTH, SCREEN_HEIGHT));

    MovementFactory factory{nature, 50, 150};

    auto movements = factory.createMovements({100, 100});
    ASSERT_EQ(1, movements.size());

    EXPECT_EQ(50, movements.front().destX);
    EXPECT_EQ(150, movements.front().destY);
    EXPECT_EQ(50, movements.front().time);
    EXPECT_NEAR(-50, movements.front().xDistance, SMALL_DELTA);
    EXPECT_NEAR(50, movements.front().yDistance, SMALL_DELTA);
    EXPECT_ARRAY_EQ(SingleElementArray_100, movements.front().flow->getFlowCharacteristics());
}
