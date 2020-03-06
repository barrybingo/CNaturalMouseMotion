#pragma once

#include <vector>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include "Flow.h"

namespace NaturalMouseMotion
{
using FlowModifierFunc = std::function<void(double &)>;

struct FlowUtils
{
    /**
     * Reduction causes loss of information, so the resulting flow is always 'good enough', but is not quaranteed
     * to be equivalent, just a shorter version of the original flow
     * @param flow the original flow
     * @param targetLength the resulting array length
     * @return the resulting flow
     */
    static FlowCharacteristicsContainer reduceFlow(FlowCharacteristicsContainer flow, size_t targetLength)
    {
        if (flow.size() <= targetLength)
        {
            throw std::runtime_error("Bad arguments");
        }

        double multiplier = targetLength / (double)flow.size();
        FlowCharacteristicsContainer result(targetLength);
        for (size_t i = 0; i < flow.size(); i++)
        {
            double index = (i * multiplier);
            double untilIndex = (i + 1) * multiplier;
            int indexInt = (int)index;
            int untilIndexInt = (int)untilIndex;
            if (indexInt != untilIndexInt)
            {
                double resultIndexPortion = 1 - (index - indexInt);
                double nextResultIndexPortion = untilIndex - untilIndexInt;
                result[indexInt] += flow[i] * resultIndexPortion;
                if (untilIndexInt < result.size())
                {
                    result[untilIndexInt] += flow[i] * nextResultIndexPortion;
                }
            }
            else
            {
                result[indexInt] += flow[i] * (untilIndex - index);
            }
        }

        return result;
    }

    /**
     * Stretch flow to longer length. Tries to fill the caps with averages.
     *
     * This is an unintuitive method, because it turns out that, for example, array size of 3
     * scales better to array size of 5 than it does to array size of 6. [1, 2, 3] can be
     * easily scaled to [1, 1.5, 2, 2.5, 3], but it's not possible without recalculating middle number (2)
     * with array size of 6, simplistic solutions quickly would run to trouble like this  [1, 1.5, 2, 2.5, 3, (3)? ]
     * or maybe: [1, 1.5, 2, 2.5, ..., 3 ]. The correct solution would correctly scale the middle numbers
     * over several indexes.
     * @param flow the original flow
     * @param targetLength the resulting flow length
     * @param modifier modifies the resulting values, you can use this to provide noise or amplify
     *                 the flow characteristics.
     * @return the resulting flow
     */
    static FlowCharacteristicsContainer stretchFlow(FlowCharacteristicsContainer flow, size_t targetLength, FlowModifierFunc modifier = nullptr)
    {
        if (targetLength < flow.size())
        {
            throw std::runtime_error("Target bucket length smaller than flow.");
        }

        int tempLength = targetLength;

        if (flow.size() != 1 && (tempLength - flow.size()) % (flow.size() - 1) != 0)
        {
            tempLength = (flow.size() - 1) * (tempLength - flow.size()) + 1;
        }

        FlowCharacteristicsContainer result(tempLength);
        int insider = flow.size() - 2;
        int stepLength = (int)((tempLength - 2) / (double)(insider + 1)) + 1;
        int countToNextStep = stepLength;
        int fillValueIndex = 0;
        for (int i = 0; i < tempLength; i++)
        {
            double fillValueBottom = flow[fillValueIndex];
            double fillValueTop = fillValueIndex + 1 < flow.size() ? flow[fillValueIndex + 1] : flow[fillValueIndex];

            double completion = (stepLength - countToNextStep) / (double)stepLength;

            result[i] = fillValueBottom * (1 - completion) + fillValueTop * completion;

            countToNextStep--;

            if (countToNextStep == 0)
            {
                countToNextStep = stepLength;
                fillValueIndex++;
            }
        }

        if (tempLength != targetLength)
        {
            result = reduceFlow(result, targetLength);
        }

        if (modifier)
        {
            std::for_each(result.begin(), result.end(), modifier);
        }
        return result;
    }
};
} // namespace NaturalMouseMotion
