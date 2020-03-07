#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>

namespace NaturalMouseMotion
{

using FlowCharacteristicsContainer = std::vector<double>;

/**
 * Flow for the mouse movement
 * Flow defines how slow or fast the cursor is moving at a particular moment, defining the characteristics
 * of movement itself not the trajectory, but how jagged or smooth, accelerating or decelerating, the movement is.
 */
class Flow
{
public:
	/**
	 * @param characteristics the characteristics array, which can be any size, contain non-negative numbers.
	 * The values in the array are translated to flow and all values are relative. For example an
	 * array of [1,2,3,4] has the same meaning as [100, 200, 300, 400] or [10, 10, 20, 20, 30, 30, 40, 40]
	 * Every array element describes a time of the movement, so that in array of n-elements every element is
	 * describing (100 / n)% of the movement. In an array of [1,2,3,4] every element is responsible for
	 * 25% of time and the movement is accelerating - in the last 25% of time the mouse cursor is 4 times faster
	 * than it was in the first 25% of the time.
	 */
	Flow(FlowCharacteristicsContainer characteristics) : buckets(normalizeBuckets(characteristics))
	{
	}

	FlowCharacteristicsContainer getFlowCharacteristics() const
	{
		return buckets;
	}

	/**
	 * This returns step size for a single axis.
	 * @param distance the total distance current movement has on current axis from beginning to target in pixels
	 * @param steps number of steps the current movement involves
	 * @param completion value between 0 and 1, the value describes movement completion in time
	 * @return the step size which should be taken next
	 */
	double getStepSize(double distance, int steps, double completion) const
	{
		auto completionStep = 1.0 / steps;
		auto bucketFrom = (completion * buckets.size());
		auto bucketUntil = ((completion + completionStep) * buckets.size());
		auto bucketContents = getBucketsContents(bucketFrom, bucketUntil);
		auto distancePerBucketContent = distance / (buckets.size() * AVERAGE_BUCKET_VALUE);
		return bucketContents * distancePerBucketContent;
	}

private:
	static constexpr int AVERAGE_BUCKET_VALUE{100};
	FlowCharacteristicsContainer buckets{};

	/**
	 * Normalizes the characteristics to have an average of AVERAGE_BUCKET_VALUE
	 * @param flowCharacteristics an array of values which describe how the mouse should move at each moment
	 * @return the normalized bucket array
	 */
	FlowCharacteristicsContainer normalizeBuckets(FlowCharacteristicsContainer flowCharacteristics)
	{
		auto buckets = FlowCharacteristicsContainer(flowCharacteristics.size());
		double sum = 0;
		for (auto &v : flowCharacteristics)
		{
			if (v < 0)
			{
				throw std::runtime_error("Invalid FlowCharacteristics");
			}
			sum += v;
		}
		if (sum == 0)
		{
			throw std::runtime_error("Invalid FlowCharacteristics. All array elements can't be 0.");
		}
		auto multiplier = static_cast<double>(AVERAGE_BUCKET_VALUE) * buckets.size() / sum;
		for (size_t i = 0; i < flowCharacteristics.size(); i++)
		{
			buckets[i] = flowCharacteristics[i] * multiplier;
		}
		return buckets;
	}

	/**
	 * Summarizes the bucket contents from bucketFrom to bucketUntil, where
	 * provided parameters may have decimal places. In that case the value
	 * from first or last bucket is just a fragment of it's full value, depending how
	 * large portion the decimal place contains. For example getBucketContents(0.6, 2.4)
	 * returns 0.4 * bucket[0] + 1 * bucket[1] + 0.4 * bucket[2]
	 * @param bucketFrom bucket from where to start reading
	 * @param bucketUntil bucket where to read
	 * @return the sum of the contents in the buckets
	 */
	double getBucketsContents(double bucketFrom, double bucketUntil) const
	{
		double sum = 0;
		for (auto i = static_cast<int>(bucketFrom); i < bucketUntil; i++)
		{
			auto value = buckets[i];
			double endMultiplier = 1;
			double startMultiplier = 0;
			if (bucketUntil < i + 1)
			{
				endMultiplier = bucketUntil - static_cast<int>(bucketUntil);
			}
			if (static_cast<int>(bucketFrom) == i)
			{
				startMultiplier = bucketFrom - static_cast<int>(bucketFrom);
			}
			value *= endMultiplier - startMultiplier;
			sum += value;
		}
		return sum;
	}
};

} // namespace NaturalMouseMotion