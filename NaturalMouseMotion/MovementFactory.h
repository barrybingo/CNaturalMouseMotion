#pragma once

#include <list>
#include "MotionNature.h"

namespace NaturalMouseMotion
{

struct Movement
{
	int destX;
	int destY;
	double distance;
	int xDistance;
	int yDistance;
	time_type time;
	const Flow *flow;

	Movement(int destX, int destY, double distance, int xDistance, int yDistance, time_type time, const Flow *flow)
		: destX(destX), destY(destY), distance(distance), xDistance(xDistance), yDistance(yDistance), time(time), flow(flow)
	{
	}
};

class MovementFactory
{
public:
	MovementFactory(MotionNature& nature, int xDest, int yDest) : xDest(xDest), yDest(yDest), nature(nature), screenSize(nature.systemCalls->getScreenSize())
	{
	}

	std::list<Movement> createMovements(Point<int> currentMousePosition)
	{
		auto movements = std::list<Movement>();
		auto lastMousePositionX = currentMousePosition.x;
		auto lastMousePositionY = currentMousePosition.y;
		auto xDistance = xDest - lastMousePositionX;
		auto yDistance = yDest - lastMousePositionY;
		auto initialDistance = std::hypot(xDistance, yDistance);
		auto flowTime = nature.getFlowWithTime(initialDistance);
		auto flow = flowTime.first;
		time_type mouseMovementMs = flowTime.second;
		auto overshoots = nature.overshootManager->getOvershoots(flow, mouseMovementMs, initialDistance);
		if (overshoots == 0)
		{
			Logger::Print(nature.debug_printer, "No overshoots for movement from (%d, %d) . (%d, %d)", currentMousePosition.x, currentMousePosition.y, xDest, yDest);
			movements.emplace_back(xDest, yDest, initialDistance, xDistance, yDistance, mouseMovementMs, flow);
			return movements;
		}
		for (auto i = overshoots; i > 0; i--)
		{
			auto overshoot = nature.overshootManager->getOvershootAmount(xDest - lastMousePositionX, yDest - lastMousePositionY, mouseMovementMs, i);
			auto currentDestinationX = limitByScreenWidth(xDest + overshoot.x);
			auto currentDestinationY = limitByScreenHeight(yDest + overshoot.y);
			xDistance = currentDestinationX - lastMousePositionX;
			yDistance = currentDestinationY - lastMousePositionY;
			auto distance = std::hypot(xDistance, yDistance);
			if (distance > 0)
			{
				flow = nature.getFlowWithTime(distance).first;
				movements.emplace_back(currentDestinationX, currentDestinationY, distance, xDistance, yDistance, mouseMovementMs, flow);
				lastMousePositionX = currentDestinationX;
				lastMousePositionY = currentDestinationY;
				mouseMovementMs = nature.overshootManager->deriveNextMouseMovementTimeMs(mouseMovementMs, i - 1);
			}
		}

		/*
		Logger::Print(nature.printer, "---------------");
		for (auto &m : movements)
		{
			Logger::Print(nature.debug_printer, "dest:(%d,%d) distance:%f time:%d distance:(%d,%d)", m.destX, m.destY, m.distance, m.time, m.xDistance, m.yDistance);
			fflush(stdout);
		}
		*/

		for (auto rit = movements.rbegin(); rit != movements.rend();)
		{
			if (rit->destX == xDest && rit->destY == yDest)
			{
				lastMousePositionX = rit->destX - rit->xDistance;
				lastMousePositionY = rit->destY - rit->yDistance;
				Logger::Print(nature.debug_printer, "Pruning 0-overshoot movement (Movement to target) from the end.");

				// https://stackoverflow.com/questions/37005449/how-to-call-erase-with-a-reverse-iterator-using-a-for-loop
				rit = decltype(rit){movements.erase(std::next(rit).base())};
			}
			else
			{
				break;
			}
		}
		xDistance = xDest - lastMousePositionX;
		yDistance = yDest - lastMousePositionY;
		auto distance = std::hypot(xDistance, yDistance);
		auto movementToTargetFlowTime = nature.getFlowWithTime(distance);
		auto finalMovementTime = nature.overshootManager->deriveNextMouseMovementTimeMs(movementToTargetFlowTime.second, 0);
		movements.emplace_back(xDest, yDest, distance, xDistance, yDistance, finalMovementTime, movementToTargetFlowTime.first);
		Logger::Print(nature.debug_printer, "%d movements returned for move (%d, %d) . (%d, %d)", movements.size(), currentMousePosition.x, currentMousePosition.y, xDest, yDest);
		return movements;
	}

private:
	int xDest;
	int yDest;
	MotionNature& nature;	
	Dimension screenSize;

	int limitByScreenWidth(int value)
	{
		return std::max(0, std::min(screenSize.Width - 1, value));
	}

	int limitByScreenHeight(int value)
	{
		return std::max(0, std::min(screenSize.Height - 1, value));
	}
};

} // namespace NaturalMouseMotion