#pragma once

#include <list>
#include "MotionNature.h"

using namespace NaturalMouseMotion;

struct MockRandomProvider
{
	MockRandomProvider(const std::vector<double> rands): rands(rands)
	{
	}

	double operator()()
	{
		return rands[i++ % rands.size()];
	}

private:
    size_t i{0};
	std::vector<double> rands;
};

struct MockSystemCalls: public SystemCalls
{
    int screenWidth;
    int screenHeight;
    std::list<Point<int>> mousePos {};

    MockSystemCalls(int screenWidth, int screenHeight): screenWidth(screenWidth), screenHeight(screenHeight)
    {
        mousePos.push_back({0,0});
    }

	time_type currentTimeMillis() override
    {
        return 0;
    }
	void sleep(time_type /* time */) override
    {
        // do nothing
    }
	Dimension getScreenSize() override
    {
        return {screenWidth, screenHeight};
    }
	void setMousePosition(int x, int y) override
    {
        mousePos.push_back({x,y});
    }
	Point<int> getMousePosition() override
    {
        auto lastPos = mousePos.back();
        return {lastPos.x, lastPos.y};
    }
};
