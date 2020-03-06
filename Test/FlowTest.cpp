#include <vector>
#include "Flow.h"
#include "gtest/gtest.h"

using NaturalMouseMotion::Flow;
using NaturalMouseMotion::FlowCharacteristicsContainer;

static constexpr double SMALL_DELTA = 10e-6;

TEST(FlowTest, constantCharacteristicsGetNormalizedTo100)
{
    FlowCharacteristicsContainer characteristics(100, 500.0);
    auto flow = Flow(characteristics);
    auto result = flow.getFlowCharacteristics();
    double sum = 0.0;
    for (auto &v : result)
    {
        EXPECT_NEAR(100, v, SMALL_DELTA);
        sum += v;
    }
    EXPECT_NEAR(100.0 * characteristics.size(), sum, SMALL_DELTA);
}

TEST(FlowTest, constantCharacteristicsGetNormalizedTo100withLargeArray)
{
    FlowCharacteristicsContainer characteristics(1000, 500.0);
    auto flow = Flow(characteristics);
    auto result = flow.getFlowCharacteristics();
    double sum = 0.0;
    for (auto &v : result)
    {
        EXPECT_NEAR(100, v, SMALL_DELTA);
        sum += v;
    }
    EXPECT_NEAR(100.0 * characteristics.size(), sum, SMALL_DELTA);
}

TEST(FlowTest, constantCharacteristicsGetNormalizedTo100fromLowValues)
{
    FlowCharacteristicsContainer characteristics(100, 5.0);
    auto flow = Flow(characteristics);
    auto result = flow.getFlowCharacteristics();
    double sum = 0.0;
    for (auto &v : result)
    {
        EXPECT_NEAR(100, v, SMALL_DELTA);
        sum += v;
    }
    EXPECT_NEAR(100.0 * characteristics.size(), sum, SMALL_DELTA);
}

TEST(FlowTest, characteristicsGetNormalizedToAverage100)
{
    FlowCharacteristicsContainer characteristics = {1, 2, 3, 4, 5};

    auto flow = Flow(characteristics);
    auto result = flow.getFlowCharacteristics();
    double sum = 0.0;
    for (auto &v : result)
    {
        sum += v;
    }

    EXPECT_NEAR(33.33333333, result[0], SMALL_DELTA);
    EXPECT_NEAR(66.66666666, result[1], SMALL_DELTA);
    EXPECT_NEAR(100.00000000, result[2], SMALL_DELTA);
    EXPECT_NEAR(133.33333333, result[3], SMALL_DELTA);
    EXPECT_NEAR(166.66666666, result[4], SMALL_DELTA);

    EXPECT_NEAR(100.0 * characteristics.size(), sum, SMALL_DELTA);
}

TEST(FlowTest, stepsAddUpToDistance_accelerating)
{
    auto flow = Flow({1, 2, 3, 4, 5});
    double step1 = flow.getStepSize(100, 5, 0);
    double step2 = flow.getStepSize(100, 5, 0.2);
    double step3 = flow.getStepSize(100, 5, 0.4);
    double step4 = flow.getStepSize(100, 5, 0.6);
    double step5 = flow.getStepSize(100, 5, 0.8);
    double sum = step1 + step2 + step3 + step4 + step5;
    EXPECT_NEAR(100.0, sum, SMALL_DELTA);
}

TEST(FlowTest, stepsAddUpToDistance_decelerating)
{
    auto flow = Flow({5, 4, 3, 2, 1});
    double step1 = flow.getStepSize(100, 5, 0);
    double step2 = flow.getStepSize(100, 5, 0.2);
    double step3 = flow.getStepSize(100, 5, 0.4);
    double step4 = flow.getStepSize(100, 5, 0.6);
    double step5 = flow.getStepSize(100, 5, 0.8);
    double sum = step1 + step2 + step3 + step4 + step5;
    EXPECT_NEAR(100.0, sum, SMALL_DELTA);
}

TEST(FlowTest, stepsAddUpToDistance_characteristics_not_dividable_by_steps_1)
{
    auto flow = Flow({1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5});
    double step1 = flow.getStepSize(100, 5, 0);
    double step2 = flow.getStepSize(100, 5, 0.2);
    double step3 = flow.getStepSize(100, 5, 0.4);
    double step4 = flow.getStepSize(100, 5, 0.6);
    double step5 = flow.getStepSize(100, 5, 0.8);
    double sum = step1 + step2 + step3 + step4 + step5;
    EXPECT_NEAR(100.0, sum, SMALL_DELTA);
}

TEST(FlowTest, stepsAddUpToDistance_characteristics_not_dividable_by_steps_2)
{
    auto flow = Flow({1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6});
    double step1 = flow.getStepSize(100, 5, 0);
    double step2 = flow.getStepSize(100, 5, 0.2);
    double step3 = flow.getStepSize(100, 5, 0.4);
    double step4 = flow.getStepSize(100, 5, 0.6);
    double step5 = flow.getStepSize(100, 5, 0.8);
    double sum = step1 + step2 + step3 + step4 + step5;
    EXPECT_NEAR(100.0, sum, SMALL_DELTA);
}

TEST(FlowTest, stepsAddUpToDistance_characteristics_not_dividable_by_steps_3)
{
    auto flow = Flow({1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7});
    double step1 = flow.getStepSize(100, 3, 0);
    double step2 = flow.getStepSize(100, 3, 1.0 / 3.0);
    double step3 = flow.getStepSize(100, 3, 1.0 / 3.0 * 2);
    double sum = step1 + step2 + step3;
    EXPECT_NEAR(100.0, sum, SMALL_DELTA);
}

TEST(FlowTest, stepsAddUpToDistance_characteristics_array_smaller_than_steps_not_dividable)
{
    auto flow = Flow({1, 2, 3});
    double step1 = flow.getStepSize(100, 5, 0);
    double step2 = flow.getStepSize(100, 5, 0.2);
    double step3 = flow.getStepSize(100, 5, 0.4);
    double step4 = flow.getStepSize(100, 5, 0.6);
    double step5 = flow.getStepSize(100, 5, 0.8);
    double sum = step1 + step2 + step3 + step4 + step5;
    EXPECT_NEAR(100.0, sum, SMALL_DELTA);
}

TEST(FlowTest, stepsAddUpToDistance_constantFlow)
{
    auto flow = Flow({10, 10, 10, 10, 10});
    double step1 = flow.getStepSize(500, 5, 0);
    double step2 = flow.getStepSize(500, 5, 0.2);
    double step3 = flow.getStepSize(500, 5, 0.4);
    double step4 = flow.getStepSize(500, 5, 0.6);
    double step5 = flow.getStepSize(500, 5, 0.8);
    double sum = step1 + step2 + step3 + step4 + step5;
    EXPECT_NEAR(500.0, sum, SMALL_DELTA);
}

TEST(FlowTest, stepsAddUpToDistance_constantFlow_characteristics_to_steps_not_dividable)
{
    auto flow = Flow({10, 10, 10, 10, 10, 10});
    double step1 = flow.getStepSize(500, 5, 0);
    double step2 = flow.getStepSize(500, 5, 0.2);
    double step3 = flow.getStepSize(500, 5, 0.4);
    double step4 = flow.getStepSize(500, 5, 0.6);
    double step5 = flow.getStepSize(500, 5, 0.8);
    double sum = step1 + step2 + step3 + step4 + step5;
    EXPECT_NEAR(500.0, sum, SMALL_DELTA);
}
