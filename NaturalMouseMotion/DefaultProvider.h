#pragma once

#ifdef __linux__
#include "X11/X.h"
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#elif _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef max
#undef min
#else
#error Unsupported architecture
#endif

#include <algorithm>
#include <thread>
#include <chrono>

#include "MotionNature.h"
#include "FlowTemplates.h"

namespace NaturalMouseMotion
{
namespace DefaultProvider
{

// defauts
static constexpr int TIME_TO_STEPS_DIVIDER{8};
static constexpr int MIN_STEPS{10};
static constexpr int EFFECT_FADE_STEPS{15};
static constexpr int REACTION_TIME_BASE_MS{20};
static constexpr int REACTION_TIME_VARIATION_MS{120};
static constexpr int DEFAULT_SLOPE_DIVIDER{10};

/**
 * Provides random mistakes in the trajectory of the moving mouse.
 */
struct DefaultNoiseProvider
{
	DefaultNoiseProvider(double noisinessDivider = 2.0) : noisinessDivider(noisinessDivider)
	{
	}

	Point<double> operator()(RandomZeroToOneFunc random, double xStepSize, double yStepSize) const
	{
		if (std::abs(xStepSize - 0.0) < SMALL_DELTA && std::abs(yStepSize - 0.0) < SMALL_DELTA)
		{
			return {0.0, 0.0};
		}
		double noiseX = 0.0;
		double noiseY = 0.0;
		auto stepSize = std::hypot(xStepSize, yStepSize);
		auto noisiness = std::max(0.0, (8 - stepSize)) / 50;
		if (random() < noisiness)
		{
			noiseX = (random() - 0.5) * std::max(0.0, (8 - stepSize)) / noisinessDivider;
			noiseY = (random() - 0.5) * std::max(0.0, (8 - stepSize)) / noisinessDivider;
		}
		return {noiseX, noiseY};
	}

private:
	static constexpr double SMALL_DELTA{1.0E-5};
	double noisinessDivider;
};

/**
 * Overshoots provide a realistic way to simulate user trying to reach the destination with mouse, but miss.
 */
struct DefaultOvershootManager : public OvershootManager
{
	DefaultOvershootManager(RandomZeroToOneFunc random) : random(random)
	{
	}

	size_t getOvershoots(const Flow *flow, time_type mouseMovementMs, double distance) override
	{
		if (distance < minDistanceForOvershoots)
		{
			return 0;
		}
		return overshoots;
	}

	Point<int> getOvershootAmount(double distanceToRealTargetX, double distanceToRealTargetY, time_type mouseMovementMs, size_t overshootsRemaining) override
	{
		auto distanceToRealTarget = std::hypot(distanceToRealTargetX, distanceToRealTargetY);
		auto randomModifier = distanceToRealTarget / overshootRandomModifierDivider;
		int x = (int)(random() * randomModifier - randomModifier / 2.0) * overshootsRemaining;
		int y = (int)(random() * randomModifier - randomModifier / 2.0) * overshootsRemaining;
		return {x, y};
	}

	time_type deriveNextMouseMovementTimeMs(time_type mouseMovementMs, size_t overshootsRemaining) override
	{
		return std::max(static_cast<time_type>((mouseMovementMs / overshootSpeedupDivider)), minOvershootMovementMs);
	}

	static constexpr double OVERSHOOT_SPEEDUP_DIVIDER{1.8};
	static constexpr int MIN_OVERSHOOT_MOVEMENT_MS{40};
	static constexpr int OVERSHOOT_RANDOM_MODIFIER_DIVIDER{20};
	static constexpr int MIN_DISTANCE_FOR_OVERSHOOTS{10};
	static constexpr size_t DEFAULT_OVERSHOOT_AMOUNT{3};

	time_type minOvershootMovementMs{MIN_OVERSHOOT_MOVEMENT_MS};
	time_type minDistanceForOvershoots{MIN_DISTANCE_FOR_OVERSHOOTS};
	double overshootRandomModifierDivider{OVERSHOOT_RANDOM_MODIFIER_DIVIDER};
	double overshootSpeedupDivider{OVERSHOOT_SPEEDUP_DIVIDER};
	size_t overshoots{DEFAULT_OVERSHOOT_AMOUNT};
	RandomZeroToOneFunc random;
};

/**
 * Provider to defines how the MouseMotion trajectory is being deviated or arced.
 */
constexpr double pi = 3.141592653589793;

struct SinusoidalDeviationProvider
{
	SinusoidalDeviationProvider(double slopeDivider = 10) : slopeDivider(slopeDivider){};

	Point<double> operator()(double totalDistanceInPixels, double completionFraction) const
	{
		auto deviationFunctionResult = (1 - std::cos(completionFraction * pi * 2)) / 2;
		auto deviationX = totalDistanceInPixels / slopeDivider;
		auto deviationY = totalDistanceInPixels / slopeDivider;
		return {deviationFunctionResult * deviationX, deviationFunctionResult * deviationY};
	}

private:
	double slopeDivider;
};

/**
 * SpeedManager controls how long does it take to complete a movement and within that
 * time how slow or fast the cursor is moving at a particular moment, the flow of movement.
 */
struct DefaultSpeedManager
{
	DefaultSpeedManager(std::vector<Flow> flows, RandomZeroToOneFunc random, time_type mouseMovementSpeedMs = 500) : flows(flows), random(random), mouseMovementTimeMs(mouseMovementSpeedMs)
	{
	}

	/**
	 * Get the SpeedFlow object, which contains Flow and planned time for mouse movement in ms.
	 **/
	std::pair<const Flow *, time_type> operator()(double distance) const
	{
		if (flows.empty())
			return {nullptr, 0};

		time_type time = mouseMovementTimeMs + static_cast<time_type>(random() * mouseMovementTimeMs);

		// pick a random flow
		auto &flow = flows[static_cast<size_t>(random() * flows.size()) % flows.size()];
		auto timePerBucket = time / static_cast<double>(flow.getFlowCharacteristics().size());
		for (auto bucket : flow.getFlowCharacteristics())
		{
			if (std::abs(bucket - 0) < SMALL_DELTA)
			{
				time += (time_type)timePerBucket;
			}
		}
		return {&flow, time};
	}

private:
	static constexpr double SMALL_DELTA{1.0E-5};
	std::vector<Flow> flows;
	time_type mouseMovementTimeMs;
	RandomZeroToOneFunc random;
};

/*
 * Basic system calls
 */
struct DefaultSystemCalls : public SystemCalls
{
#ifdef __linux__
	Display *display;
	std::vector<Window> root_windows;
	int screen;

	DefaultSystemCalls()
	{
		display = XOpenDisplay(nullptr);
		screen = XDefaultScreen(display);

	    int number_of_screens = XScreenCount(display);
	    for (int i = 0; i < number_of_screens; i++) {
	    	root_windows.push_back(XRootWindow(display, i));
	    }
	}

	~DefaultSystemCalls()
	{
		XCloseDisplay(display);
	}

	Dimension getScreenSize() override
	{
		Screen *scr = ScreenOfDisplay(display, screen);
		return {scr->width, scr->height};
	}

	void setMousePosition(int x, int y) override
	{
		XSelectInput(display, root_windows[screen], KeyReleaseMask);
		XWarpPointer(display, None, root_windows[screen], 0, 0, 0, 0, x, y);
		XFlush(display);
	}

	Point<int> getMousePosition() override
	{
	    Window window_returned;
	    int root_x, root_y;
	    int win_x, win_y;
	    unsigned int mask_return;

	    for (auto& r: root_windows)
	    {
	        if (XQueryPointer(display, r, &window_returned,
	                &window_returned, &root_x, &root_y, &win_x, &win_y,
	                &mask_return))
	        {
	            return {root_x, root_y};
	        }
	    }
        // No mouse found
        return {0, 0};
	}
#elif _WIN32
	Dimension getScreenSize() override
	{
		return {GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
	}

	void setMousePosition(int x, int y) override
	{
		SetCursorPos(x, y);
	}

	Point<int> getMousePosition()
	{
		POINT pt;
		GetCursorPos(&pt);
		return {pt.y, pt.y};
	}
#else
	#error unsupported OS
#endif

	time_type currentTimeMillis() override
	{
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		auto duration = now.time_since_epoch();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		return millis;
	}

	void sleep(time_type time) override
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(time));
	}
};

/**
 * Source of randomness
 */
struct DefaultRandomProvider
{
	DefaultRandomProvider()
	{
		std::random_device rnd_device;
		mersenne_engine = std::mt19937(rnd_device());
	}

	double operator()()
	{
		return dist(mersenne_engine);
	}

private:
	std::mt19937 mersenne_engine;
	std::uniform_real_distribution<double> dist{0.0f, 1.0f};
};

/*
 * Simple console printer
 */
struct DefaultPrinter
{
    void operator()(const std::string str) const
    {
        printf("%s\n", str.c_str());
    }
};

} // namespace DefaultProvider
} // namespace NaturalMouseMotion
