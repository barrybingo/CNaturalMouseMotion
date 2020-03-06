#pragma once

#include <cstdint>
#include <functional>
#include <random>
#include <memory>

#include "Flow.h"
#include "Logger.h"

namespace NaturalMouseMotion
{

/**
 * Type used for screen dimensions
 **/
struct Dimension
{
	int Width;
	int Height;
};

/**
 * Simple coordinates type
 **/
template<typename T>
struct Point
{
	T x;
	T y;
};

/**
 * Type used for milliseconds
 **/
using time_type = int64_t;

/*
 * Return a double between 0.0 and 1.0
 */
using RandomZeroToOneFunc = std::function<double()>;


/**
 * Use to observe mouse movement when moving
 */
using MouseMotionObserverFunc = std::function<void(int x, int y)>;

/**
 * Provides noise or mistakes in the mouse movement
 * Implementation should be immutable.
 *
 * Noise is offset from the original trajectory, simulating user and physical errors on mouse movement.
 * Noise is accumulating, so on average it should create an equal chance of either positive or negative movement
 * on each axis, otherwise the mouse movement will always be slightly offset to single direction.
 * Deviation from DeviationProvider is different from the Noise
 * because it works like a mathematical function and is not accumulating.
 * Not every step needs to add noise, use randomness to only add noise sometimes, otherwise return Point(0, 0).
 * During the final steps of mouse movement, the effect of noise is gradually reduced, so the mouse
 * would finish on the intended pixel smoothly, thus the implementation of this class can safely ignore
 * and not know the beginning and end of the movement.
 * @param random use this to generate randomness in the offset
 * @param xStepSize the step size that is taken horizontally
 * @param yStepSize the step size that is taken vertically
 * @return a point which describes how much the mouse offset is increased or decreased this step.
 * This value must not include the parameters xStepSize and yStepSize. For no change in noise just return (0,0).
 */
using GetNoiseFunc = std::function<Point<double>(RandomZeroToOneFunc random, double xStepSize, double yStepSize)>;


/**
 * Creates arcs or deviation into mouse movement.
 * DeviationProvider implementation should be immutable.
 *
 * Gets the deviation for current trajectory. Deviation is an offset from the original straight trajectory.
 * Deviation is different from the Noise because it works like a mathematical function, the resulting
 * Point is added to single trajectory point and it will not have any effect in the next
 * mouse movement step, making it easy to implement this as a formula based on the input parameters.
 * e.g the something like 'deviation = totalDistanceInPixels / completionFraction', resulting in smooth movement.
 * (Don't actually use this formula), 'Noise' is generating an offset from the original trajectory and is accumulating.
 * As deviation should be deterministic and return same result for same parameters, it should not include Random
 * behaviour, thus Random is not included as a parameter.
 * It is recommended that deviation is decreasing when completionFraction nears 1, but MouseMotion itself
 * also makes sure that the effect of deviation is reduced when the mouse is nearing its destination.
 * @param totalDistanceInPixels the total pixels between target and mouse initial position
 * @param completionFraction the completed fraction of mouse movement total distance, value from 0...1 (0;1]
 * @return a point which describes how much the mouse is going to deviate from the straight trajectory between
 * target and initial position. This is not the final deviation of the mouse as MouseMotion will randomly decide
 * to either amplify or decrease it over the whole mouse movement, making the resulting arc stand out more or less,
 * or is flipped negatively.
 * @see
 */
using GetDeviationFunc = std::function<Point<double>(double totalDistanceInPixels, double completionFraction)>;

/**
 * SpeedManager controls how long does it take to complete a mouse movement and within that
 * time how slow or fast the cursor is moving at a particular moment, the flow.
 * Flow controls how jagged or smooth, accelerating or decelerating, the movement is.
 *
 * Get the SpeedFlow object, which contains Flow and planned time for mouse movement in ms.
 * @param distance the distance from where the cursor is now to the destination point
 * @return the SpeedFlow object, which details are a SpeedManager implementation decision.
 */
using GetFlowWithTimeFunc = std::function<std::pair<const Flow*, time_type>(double distance)>;

/**
 * Overshoots provide a realistic way to simulate user trying to reach the destination with mouse, but miss.
 * Points around the destination are produced which will be hit before the mouse hits the real destination.
 * If overshoot happens to match the target, then overshooting is cancelled and real destination will be reached.
 */
struct OvershootManager
{
	/**
	 * Get the maximum amount of overshoots the cursor does before reaching its final destination.
	 * @param flow the flow which is planned to be used to reach the target.
	 * (If returned overshoots &gt; 0, then a new flow will be calculated for each overshoot.).
	 * This flow could be analyzed if overshooting is suitable. It is not available
	 * as a parameter in overshootAmount calculation, because flow itself is calculated
	 * from the movement distance, which is dependent on the overshoot amount.
	 * @param mouseMovementMs the planned time for reaching the real target
	 * @param distance the distance between mouse position and real target
	 * @return the number of maximum overshoots used or 0 if no overshoots
	 */
	virtual size_t getOvershoots(const Flow *flow, time_type mouseMovementMs, double distance) = 0;

	/**
	 * Returns the overshoot amount which will be added to real target, thus getting the overshoot target.
	 * @param distanceToRealTargetX distance to real target X-coordinate
	 * @param distanceToRealTargetY distance to real target Y-coordinate
	 * @param mouseMovementMs the time planned for reaching the real target
	 * @param overshootsRemaining the amount of overshoots remaining, current included.
	 * Values from (n to 1), where n &gt;= 1
	 * @return the amount which will be added to real target, thus getting the overshoot target.
	 */
	virtual Point<int> getOvershootAmount(double distanceToRealTargetX, double distanceToRealTargetY, time_type mouseMovementMs, size_t overshootsRemaining) = 0;

	/**
	 * Once the mouse reaches the overshoot target, new trajectory with new speed is calculated for next target
	 * (can be real or overshoot target, if the next target is real target, the overshootsRemaining value is 0)
	 * @param mouseMovementMs the last mouse movement in ms
	 * @param overshootsRemaining the amount of overshoots remaining, including this.
	 * Values from (n to 0), where n &gt;= 0
	 * @return the next mouse movement time in ms
	 */
	virtual time_type deriveNextMouseMovementTimeMs(time_type mouseMovementMs, size_t overshootsRemaining) = 0;
};

/**
 * Abstracts ordinary static System calls away
 */
struct SystemCalls
{
	virtual time_type currentTimeMillis() = 0;
	virtual void sleep(time_type time) = 0;
	virtual Dimension getScreenSize() = 0;
	virtual void setMousePosition(int x, int y) = 0;
	virtual Point<int> getMousePosition() = 0;
};

struct MotionNature
{
	/**
	 * Loggers accosiated with nature
	 */
	LoggerPrinterFunc info_printer;
	LoggerPrinterFunc debug_printer;

	/**
	 * Observer for any movement
	 **/
	MouseMotionObserverFunc observer;

	/**
	 * Source of randomness
	 * This function must provide doubles in the range 0.0 to 1.0
	 */
	RandomZeroToOneFunc random;

	/**
	 * Time to steps is how NaturalMouseMotion calculates how many locations need to be visited between
	 * start and end point. More steps means more smooth movement. Thus increasing this divider means less
	 * steps and decreasing means more steps. The default value should be as smooth as needed for any real
	 * purpose. So unless this really is the issue, you shouldn't touch this value.
	 */
	double timeToStepsDivider;

	/**
	 * Minimum amount of steps that is taken to reach the target, this is used when calculation otherwise would
	 * lead to too few steps for smooth mouse movement, which can happen for very fast movements.
	 * The default value should cover your needs, usually no need to touch this.
	 */
	int minSteps;

	/**
	 * Effect fade decreases the noise and deviation effects linearly to 0 at the end of the mouse movement,
	 * so mouse would end up in the intended target pixel even when noise or deviation would otherwise
	 * add offset to mouse position.
	 */
	int effectFadeSteps;

	/**
	 * The minimal sleep time when overshoot or some other feature has caused mouse to miss the original target
	 * to prepare for next attempt to move the mouse to target.
	 */
	int reactionTimeBaseMs;

	/**
	 * The random sleep time when overshoot or some other feature has caused mouse to miss the original target
	 * to prepare for next attempt to move the mouse to target. Random part of this is added to the reactionTimeBaseMs.
	 */
	int reactionTimeVariationMs;

	/**
	 * Provider to defines how the MouseMotion trajectory is being deviated or arced.
	 */
	GetDeviationFunc getDeviation;

	/**
	 * Provides random mistakes in the trajectory of the moving mouse.
	 */
	GetNoiseFunc getNoise;

	/**
	 * Overshoots provide a realistic way to simulate user trying to reach the destination with mouse, but miss.
	 */
	std::shared_ptr<OvershootManager> overshootManager;

	/**
	 * System call interface
	 */
	std::shared_ptr<SystemCalls> systemCalls;

	/**
	 * SpeedManager controls how long does it take to complete a movement and within that
	 * time how slow or fast the cursor is moving at a particular moment, the flow of movement.
	 */
	GetFlowWithTimeFunc getFlowWithTime;
};

} // namespace NaturalMouseMotion