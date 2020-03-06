#include "gtest/gtest.h" // must be included before X.h in linux so put before DefaultProvider.h - see https://github.com/google/googletest/issues/371
#include "DefaultProvider.h"
#include "MockStructs.h"

using namespace NaturalMouseMotion;

static constexpr double SMALL_DELTA = 10e-6;
static constexpr int SCREEN_WIDTH = 500;
static constexpr int SCREEN_HEIGHT = 500;

TEST(DefaultOvershootManagerTest, returnsSetOvershootNumber)
{
    auto random = RandomZeroToOneFunc{MockRandomProvider({0.1, 0.2, 0.3, 0.4, 0.5})};
    DefaultProvider::DefaultOvershootManager manager(random);

    Flow flow{FlowCharacteristicsContainer(1, {100.0})};
    auto overshoots = manager.getOvershoots(&flow, 200, 1000);
    EXPECT_EQ(3, overshoots);

    manager.overshoots = 10;
    overshoots = manager.getOvershoots(&flow, 200, 1000);
    EXPECT_EQ(10, overshoots);
}

TEST(DefaultOvershootManagerTest, overshootSizeDecreasesWithOvershootsRemaining)
{
    Point<int> overshoot1;
    Point<int> overshoot2;
    Point<int> overshoot3;

    {
      auto random = RandomZeroToOneFunc{MockRandomProvider({0.1})};
      DefaultProvider::DefaultOvershootManager manager(random);
      overshoot1 = manager.getOvershootAmount(1000, 500, 1000, 1);
    }

    {
      auto random = RandomZeroToOneFunc{MockRandomProvider({0.1})};
      DefaultProvider::DefaultOvershootManager manager(random);
      overshoot2 = manager.getOvershootAmount(1000, 500, 1000, 2);
    }

    {
      auto random = RandomZeroToOneFunc{MockRandomProvider({0.1})};
      DefaultProvider::DefaultOvershootManager manager(random);
      overshoot3 = manager.getOvershootAmount(1000, 500, 1000, 3);
    }

    EXPECT_EQ(overshoot3.x, overshoot1.x * 3);
    EXPECT_EQ(overshoot2.x, overshoot1.x * 2);
}

TEST(DefaultOvershootManagerTest, nextMouseMovementTimeIsBasedOnCurrentMouseMovementMs)
{
    auto random = RandomZeroToOneFunc{MockRandomProvider({0.1, 0.2, 0.3, 0.4, 0.5})};
    DefaultProvider::DefaultOvershootManager manager(random);

    {
      // DEFAULT VALUE
      auto nextTime = manager.deriveNextMouseMovementTimeMs(
          (time_type)(DefaultProvider::DefaultOvershootManager::OVERSHOOT_SPEEDUP_DIVIDER * 500), 3
      );
      EXPECT_EQ(500, nextTime);
    }

    {
      manager.overshootSpeedupDivider = 2;
      auto nextTime = manager.deriveNextMouseMovementTimeMs(1000, 3);
      EXPECT_EQ(500, nextTime);
    }

    {
      manager.overshootSpeedupDivider = 4;
      auto nextTime = manager.deriveNextMouseMovementTimeMs(1000, 3);
      EXPECT_EQ(250, nextTime);
    }
}

TEST(DefaultOvershootManagerTest, nextMouseMovementTimeHasMinValue)
{
    auto random = RandomZeroToOneFunc{MockRandomProvider({0.1, 0.2, 0.3, 0.4, 0.5})};
    DefaultProvider::DefaultOvershootManager manager(random);

    {
      manager.overshootSpeedupDivider = 2;
      manager.minOvershootMovementMs = 1500;
      auto nextTime = manager.deriveNextMouseMovementTimeMs(1000, 3);
      EXPECT_EQ(1500, nextTime);
    }
}