#include "gtest/gtest.h" // must be included before X.h in linux so put before DefaultProvider.h - see https://github.com/google/googletest/issues/371
#include "NaturalMouseMotion.h"
#include "MockStructs.h"

using namespace NaturalMouseMotion;

static constexpr double SMALL_DELTA = 10e-6;
static constexpr int SCREEN_WIDTH = 800;
static constexpr int SCREEN_HEIGHT = 500;

using MockOvershootManager = DefaultProvider::DefaultOvershootManager;

MotionNature NewMockNature()
{
	MotionNature nature;

	nature.info_printer = nullptr; //LoggerPrinterFunc{DefaultProvider::DefaultPrinter()};
	nature.debug_printer = nullptr; //LoggerPrinterFunc{DefaultProvider::DefaultPrinter()};
	nature.observer = nullptr;
    nature.timeToStepsDivider = DefaultProvider::TIME_TO_STEPS_DIVIDER;
	nature.minSteps = DefaultProvider::MIN_STEPS;
	nature.effectFadeSteps = DefaultProvider::EFFECT_FADE_STEPS;
	nature.reactionTimeBaseMs = DefaultProvider::REACTION_TIME_BASE_MS;
	nature.reactionTimeVariationMs = DefaultProvider::REACTION_TIME_VARIATION_MS;

    nature.getDeviation = [](double, double) -> Point<double>{return {0,0};};
    nature.getNoise = [](RandomZeroToOneFunc, double, double) -> Point<double>{return {0,0};};
	nature.random = RandomZeroToOneFunc{MockRandomProvider({0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1})};
    nature.systemCalls = std::shared_ptr<SystemCalls>(new MockSystemCalls(SCREEN_WIDTH, SCREEN_HEIGHT));
	nature.overshootManager = std::shared_ptr<OvershootManager>(new MockOvershootManager(nature.random));
	nature.getFlowWithTime = [](double) -> std::pair<Flow *, time_type>{
        static Flow flow{std::vector<double>({100})};
        return {&flow, 10};
        };
	return nature;
}

class MockTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_nature = NewMockNature();
}

    // void TearDown() override {}
    MotionNature mock_nature;

    void assertMousePosition(int x, int y)
    {
        auto pos = mock_nature.systemCalls->getMousePosition();
        EXPECT_EQ(x, pos.x);
        EXPECT_EQ(y, pos.y);
    }

    std::list<Point<int>> getMouseMovements()
    {

        return dynamic_cast<MockSystemCalls*>(mock_nature.systemCalls.get())->mousePos;
    }

    void mouseMove(int x, int y) {
        dynamic_cast<MockSystemCalls*>(mock_nature.systemCalls.get())->mousePos.push_back({x,y});
    }

};

TEST(MockStructs, testRandomProvider)
{
    auto random = RandomZeroToOneFunc{MockRandomProvider({0.1, 0.0, 0.4, 0.5, 0.8, 0.2})};
    EXPECT_EQ(random(), 0.1);
    EXPECT_EQ(random(), 0.0);
    EXPECT_EQ(random(), 0.4);
    EXPECT_EQ(random(), 0.5);
    EXPECT_EQ(random(), 0.8);
    EXPECT_EQ(random(), 0.2);
    EXPECT_EQ(random(), 0.1);
    EXPECT_EQ(random(), 0.0);
    EXPECT_EQ(random(), 0.4);
    EXPECT_EQ(random(), 0.5);
    EXPECT_EQ(random(), 0.8);
}

TEST_F(MockTest, linearMotionNoOvershoots)
{
    assertMousePosition(0, 0);
    dynamic_cast<MockOvershootManager*>(mock_nature.overshootManager.get())->overshoots = 0;

    Move(mock_nature, 50, 50);
    assertMousePosition(50, 50);

    auto points = getMouseMovements();
    // The chosen 5 is 'good enough value' for 0,0 -> 50,50 for this test. we don't expect it to
    // be any certain value, because it can be changed in the future how the implementation actually works,
    // but based on gut feeling anything below 5 is too low.
    EXPECT_TRUE(points.size() > 5);
    // We don't want to verify every pixel what the mouse visits
    // instead we make sure its path is linear, as this is what we can expect from this test.
    Point<int> lastPoint{0,0};
    for (auto& p :points)
    {
      EXPECT_EQ(p.x, p.y);
      EXPECT_TRUE(p.x >= lastPoint.x);
      EXPECT_TRUE(p.y >= lastPoint.y);
      lastPoint = p;
    }
}

TEST_F(MockTest, cantMoveOutOfScreenToNegative_noOverShoots)
{
    dynamic_cast<MockOvershootManager*>(mock_nature.overshootManager.get())->overshoots = 0;
    Move(mock_nature, -50, -50);

    auto points = getMouseMovements();
    for (auto& p :points)
    {
      EXPECT_TRUE(p.x >= 0 && p.y >= 0);
    }
    assertMousePosition(0, 0);
}

TEST_F(MockTest, cantMoveUpToScreenWidth_noOvershoots)
{
    // This helps to make sure that the test detects if used height instead of width or vice versa in implementation
    ASSERT_NE(SCREEN_WIDTH, SCREEN_HEIGHT);

    dynamic_cast<MockOvershootManager*>(mock_nature.overshootManager.get())->overshoots = 0;
    Move(mock_nature, SCREEN_WIDTH + 100, SCREEN_HEIGHT - 100);

    auto points = getMouseMovements();
    for (auto& p :points)
    {
      EXPECT_TRUE(p.x < SCREEN_WIDTH);
    }
    assertMousePosition(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 100);
}

TEST_F(MockTest, cantMoveUpToScreenWidth_withOvershoots)
{
    // This helps to make sure that the test detects if used height instead of width or vice versa in implementation
    ASSERT_NE(SCREEN_WIDTH, SCREEN_HEIGHT);

    dynamic_cast<MockOvershootManager*>(mock_nature.overshootManager.get())->overshoots = 100;
    Move(mock_nature, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 100);

    auto points = getMouseMovements();
    for (auto& p :points)
    {
      EXPECT_TRUE(p.x < SCREEN_WIDTH);
    }
    assertMousePosition(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 100);
}

TEST_F(MockTest, cantMoveUpToScreenHeight_noOvershoots)
{
    // This helps to make sure that the test detects if used height instead of width or vice versa in implementation
    ASSERT_NE(SCREEN_WIDTH, SCREEN_HEIGHT);

    dynamic_cast<MockOvershootManager*>(mock_nature.overshootManager.get())->overshoots = 0;
    Move(mock_nature, SCREEN_WIDTH - 100, SCREEN_HEIGHT + 100);

    auto points = getMouseMovements();
    for (auto& p :points)
    {
      EXPECT_TRUE(p.y < SCREEN_HEIGHT);
    }
    assertMousePosition(SCREEN_WIDTH - 100, SCREEN_HEIGHT - 1);
}

TEST_F(MockTest, cantMoveUpToScreenHeight_withOvershoots)
{
    // This helps to make sure that the test detects if used height instead of width or vice versa in implementation
    ASSERT_NE(SCREEN_WIDTH, SCREEN_HEIGHT);

    dynamic_cast<MockOvershootManager*>(mock_nature.overshootManager.get())->overshoots = 100;
    Move(mock_nature, SCREEN_WIDTH - 100, SCREEN_HEIGHT - 1);

    auto points = getMouseMovements();
    for (auto& p :points)
    {
      EXPECT_TRUE(p.y < SCREEN_HEIGHT);
    }
    assertMousePosition(SCREEN_WIDTH - 100, SCREEN_HEIGHT - 1);
}

TEST_F(MockTest, cantMoveOutOfScreenToNegative_withOverShoots)
{
    // setup mouse to 50,50
    mouseMove(50, 50);
    assertMousePosition(50, 50);

    dynamic_cast<MockOvershootManager*>(mock_nature.overshootManager.get())->overshoots = 100;
    Move(mock_nature, 0, 0);

    auto points = getMouseMovements();
    for (auto& p :points)
    {
      EXPECT_TRUE(p.x >= 0 && p.y >= 0);
    }
    assertMousePosition(0, 0);
}