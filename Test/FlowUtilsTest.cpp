#include <vector>
#include <numeric>
#include <cmath>
#include "Flow.h"
#include "FlowUtils.h"
#include "gtest/gtest.h"

using NaturalMouseMotion::Flow;
using NaturalMouseMotion::FlowCharacteristicsContainer;
using NaturalMouseMotion::FlowModifierFunc;
using NaturalMouseMotion::FlowUtils;

static constexpr double SMALL_DELTA = 10e-6;

#define EXPECT_ARRAY_EQ(a, b) { \
    EXPECT_EQ(a.size(), b.size()); \
    size_t element_count = std::min(a.size(), b.size()); \
    for(int i = 0; i < element_count; ++i) { EXPECT_NEAR(a[i], b[i], SMALL_DELTA); } \
} \

double average(FlowCharacteristicsContainer v)
{
    auto n = v.size();
    double average = 0.0;
    if (n != 0) {
         average = std::accumulate(v.begin(), v.end(), 0.0) / n;
    }
    return average;
}

TEST(FlowUtilsTest, testStretchFlow_3to9) {
    FlowCharacteristicsContainer flow = {1, 2, 3};
    auto result = FlowUtils::stretchFlow(flow, 9);
    EXPECT_ARRAY_EQ(result, FlowCharacteristicsContainer({1.0, 1.25, 1.5, 1.75, 2.0, 2.25, 2.5, 2.75, 3.0}));


    double sum = std::accumulate(result.begin(), result.end(), 0.0);
    EXPECT_NEAR(sum, average(flow) * 9.0, SMALL_DELTA);
}

TEST(FlowUtilsTest, testStretchFlow_1to9) {
    FlowCharacteristicsContainer flow = {1};
    auto result = FlowUtils::stretchFlow(flow, 9);
    EXPECT_ARRAY_EQ(result, FlowCharacteristicsContainer({1, 1, 1, 1, 1, 1, 1, 1, 1}));

    double sum = std::accumulate(result.begin(), result.end(), 0.0);
    EXPECT_NEAR(sum, average(flow) * 9, SMALL_DELTA);
}

TEST(FlowUtilsTest, testStretchFlow_3to5) {
    FlowCharacteristicsContainer flow = {1, 2, 3};
    auto result = FlowUtils::stretchFlow(flow, 5);
    EXPECT_ARRAY_EQ(result, FlowCharacteristicsContainer({1.0, 1.5, 2.0, 2.5, 3}));

    double sum = std::accumulate(result.begin(), result.end(), 0.0);
    EXPECT_NEAR(sum, average(flow) * 5, SMALL_DELTA);
}

TEST(FlowUtilsTest, testStretchFlow_3to5_withModifier) {
    FlowCharacteristicsContainer flow = {1, 2, 3};
    FlowModifierFunc modifier = [](double& d) { d *= 2.0; };
    auto result = FlowUtils::stretchFlow(flow, 5, modifier);
    EXPECT_ARRAY_EQ(result, FlowCharacteristicsContainer({2.0, 3.0, 4.0, 5.0, 6.0}));

    double sum = std::accumulate(result.begin(), result.end(), 0.0);
    EXPECT_NEAR(sum, average(flow) * 2 * 5, SMALL_DELTA);
}

TEST(FlowUtilsTest, testStretchFlow_3to6_withModifier) {
    FlowCharacteristicsContainer flow = {1, 2, 3};
    FlowModifierFunc modifier = [](double& d) { d = std::floor(d); };
    auto result = FlowUtils::stretchFlow(flow, 6, modifier);
    EXPECT_ARRAY_EQ(result, FlowCharacteristicsContainer({
            1, 1, 1, 2, 2, 2,
        }));
}

TEST(FlowUtilsTest, testStretchFlow_2to9) {
    FlowCharacteristicsContainer flow = {1, 2};
    auto result = FlowUtils::stretchFlow(flow, 9);
    EXPECT_ARRAY_EQ(result, FlowCharacteristicsContainer({1.0, 1.125, 1.25, 1.375, 1.5, 1.625, 1.75, 1.875, 2.0}));

    double sum = std::accumulate(result.begin(), result.end(), 0.0);
    EXPECT_NEAR(sum, average(flow) * 9, SMALL_DELTA);
}

TEST(FlowUtilsTest, testStretchFlow_2to8) {
    FlowCharacteristicsContainer flow = {1, 2};
    auto result = FlowUtils::stretchFlow(flow, 8);

    EXPECT_ARRAY_EQ(result, FlowCharacteristicsContainer({
            1.0, 1.142857, 1.285714, 1.428571,
            1.571428, 1.714285, 1.857142, 2.0
        }));

    double sum = std::accumulate(result.begin(), result.end(), 0.0);
    EXPECT_NEAR(sum, average(flow) * 8, SMALL_DELTA);
}

TEST(FlowUtilsTest, testStretchFlow_3to6) {
    FlowCharacteristicsContainer flow = {1, 2, 3};
    auto result = FlowUtils::stretchFlow(flow, 6);
    EXPECT_ARRAY_EQ(result, FlowCharacteristicsContainer({
            1.047619, 1.428571, 1.809523,
            2.190476, 2.571428, 2.952380
        }));

    double sum = std::accumulate(result.begin(), result.end(), 0.0);
    EXPECT_NEAR(sum, average(flow) * 6, SMALL_DELTA);
}


TEST(FlowUtilsTest, testStretchFlow_3to18) {
    FlowCharacteristicsContainer flow = {1.1, 1.2, 1.3};
    auto result = FlowUtils::stretchFlow(flow, 18);
    EXPECT_ARRAY_EQ(result, FlowCharacteristicsContainer({
            1.102795, 1.113978, 1.125161, 1.136774,
            1.148602, 1.159784, 1.170967, 1.183010,
            1.194408, 1.205591, 1.216989, 1.229032,
            1.240215, 1.251397, 1.263225, 1.274838,
            1.286021, 1.297204}));

    double sum = std::accumulate(result.begin(), result.end(), 0.0);
    EXPECT_NEAR(sum, average(flow) * 18, SMALL_DELTA);
}


TEST(FlowUtilsTest, testReduceFlow_5to3) {
    FlowCharacteristicsContainer flow = {1, 1.5, 2, 2.5, 3};
    auto result = FlowUtils::reduceFlow(flow, 3);
    EXPECT_ARRAY_EQ(result, FlowCharacteristicsContainer({1.2, 2, 2.8}));

    double sum = std::accumulate(result.begin(), result.end(), 0.0);
    EXPECT_NEAR(sum, average(flow) * 3, SMALL_DELTA);
}

TEST(FlowUtilsTest, testReduceFlow_10to3) {
    FlowCharacteristicsContainer flow = {5, 5, 4, 4, 3, 3, 2, 2, 1, 1};
    auto result = FlowUtils::reduceFlow(flow, 3);
    EXPECT_ARRAY_EQ(result, FlowCharacteristicsContainer({4.6, 3.0, 1.4}));

    double sum = std::accumulate(result.begin(), result.end(), 0.0);
    EXPECT_NEAR(sum, average(flow) * 3, SMALL_DELTA);
}

TEST(FlowUtilsTest, testReduceFlow_10to1) {
    FlowCharacteristicsContainer flow = {5, 5, 4, 4, 3, 3, 2, 2, 1, 1};
    auto result = FlowUtils::reduceFlow(flow, 1);
    EXPECT_ARRAY_EQ(result, FlowCharacteristicsContainer({ 3.0 }));

    double sum = std::accumulate(result.begin(), result.end(), 0.0);
    EXPECT_NEAR(sum, average(flow) * 1, SMALL_DELTA);
}