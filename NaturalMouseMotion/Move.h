#pragma once

#include "MotionNature.h"
#include "MovementFactory.h"

namespace NaturalMouseMotion
{
struct MoveImp
{
    static constexpr int SLEEP_AFTER_ADJUSTMENT_MS{2};

    /**
    * Move cursor smoothly to the destination coordinates from whereever the cursor currently is.
    * Blocking call
    *
    * @param nature the nature that defines how mouse is moved
    * @param xDest  the x-coordinate of destination
    * @param yDest  the y-coordinate of destination
    */
    static void Move(MotionNature& nature, int x, int y)
    {
        Dimension screenSize(nature.systemCalls->getScreenSize());
        Point<int> mousePosition = nature.systemCalls->getMousePosition();

        int xDest = std::max(0, std::min(screenSize.Width - 1, x));
        int yDest = std::max(0, std::min(screenSize.Height - 1, y));

        Logger::Print(nature.info_printer, "Starting to move mouse to (%d, %d), current position: (%d, %d)", xDest, yDest, mousePosition.x, mousePosition.y);


        MovementFactory movementFactory(nature, xDest, yDest);
        auto movements = movementFactory.createMovements(mousePosition);
        auto overshoots = movements.size() - 1;
        while (mousePosition.x != xDest || mousePosition.y != yDest)
        {
            if (movements.empty())
            {
                // This shouldn't usually happen, but it's possible that somehow we won't end up on the target,
                // Then just re-attempt from mouse new position. (There are known JDK bugs, that can cause sending the cursor
                // to wrong pixel)
                mousePosition = nature.systemCalls->getMousePosition();
                Logger::Print(nature.debug_printer, "Re-populating movement array. Did not end up on target pixel.");
                movements = movementFactory.createMovements(mousePosition);
            }

            Movement movement = movements.front();
            movements.pop_front();
            if (!movements.empty())
            {
                Logger::Print(nature.debug_printer, "Using overshoots (%d out of %d), aiming at (%d, %d)", overshoots - movements.size() + 1, overshoots, movement.destX, movement.destY);
            }

            double distance = movement.distance;
            time_type mouseMovementMs = movement.time;
            const Flow *flow = movement.flow;
            double xDistance = movement.xDistance;
            double yDistance = movement.yDistance;
            Logger::Print(nature.info_printer, "Movement arc length computed to %f and time predicted to %d ms", distance, mouseMovementMs);

            /* Number of steps is calculated from the movement time and limited by minimal amount of steps
            (should have at least MIN_STEPS) and distance (shouldn't have more steps than pixels travelled) */
            auto steps = (int)std::ceil(std::min(distance, std::max((double)mouseMovementMs / nature.timeToStepsDivider, (double)nature.minSteps)));

            auto startTime = nature.systemCalls->currentTimeMillis();
            time_type stepTime = mouseMovementMs / steps;

            mousePosition = nature.systemCalls->getMousePosition();
            double simulatedMouseX = mousePosition.x;
            double simulatedMouseY = mousePosition.y;

            double deviationMultiplierX = (nature.random() - 0.5) * 2;
            double deviationMultiplierY = (nature.random() - 0.5) * 2;

            double completedXDistance = 0;
            double completedYDistance = 0;
            double noiseX = 0;
            double noiseY = 0;

            for (decltype(steps) i = 0; i < steps; i++)
            {
                // All steps take equal amount of time. This is a value from 0...1 describing how far along the process is.
                double timeCompletion = i / (double)steps;

                double effectFadeStep = std::max(i - (steps - nature.effectFadeSteps) + 1, 0);
                // value from 0 to 1, when effectFadeSteps remaining steps, starts to decrease to 0 linearly
                // This is here so noise and deviation wouldn't add offset to mouse final position, when we need accuracy.
                double effectFadeMultiplier = (nature.effectFadeSteps - effectFadeStep) / nature.effectFadeSteps;

                double xStepSize = flow->getStepSize(xDistance, steps, timeCompletion);
                double yStepSize = flow->getStepSize(yDistance, steps, timeCompletion);

                completedXDistance += xStepSize;
                completedYDistance += yStepSize;
                double completedDistance = std::hypot(completedXDistance, completedYDistance);
                double completion = std::min(1.0, completedDistance / distance);
                Logger::Print(nature.debug_printer, "Step: x: %f y: %f tc: %f c: %f", xStepSize, yStepSize, timeCompletion, completion);

                auto noise = nature.getNoise(nature.random, xStepSize, yStepSize);
                auto deviation = nature.getDeviation(distance, completion);

                noiseX += noise.y;
                noiseY += noise.y;
                simulatedMouseX += xStepSize;
                simulatedMouseY += yStepSize;

                Logger::Print(nature.debug_printer, "EffectFadeMultiplier: %f", effectFadeMultiplier);
                Logger::Print(nature.debug_printer, "SimulatedMouse: [%f, %f]", simulatedMouseX, simulatedMouseY);

                time_type endTime = startTime + stepTime * (i + 1);
                auto mousePosX = roundTowards(
                    simulatedMouseX +
                        deviation.x * deviationMultiplierX * effectFadeMultiplier +
                        noiseX * effectFadeMultiplier,
                    movement.destX);

                auto mousePosY = roundTowards(
                    simulatedMouseY +
                        deviation.y * deviationMultiplierY * effectFadeMultiplier +
                        noiseY * effectFadeMultiplier,
                    movement.destY);

                mousePosX = std::max(0, std::min(screenSize.Width - 1, mousePosX));
                mousePosY = std::max(0, std::min(screenSize.Height - 1, mousePosY));

                nature.systemCalls->setMousePosition(mousePosX, mousePosY);

                // Allow other action to take place or just observe, we'll later compensate by sleeping less.
                if (nature.observer)
                {
                    nature.observer(mousePosX, mousePosY);
                }

                time_type timeLeft = endTime - nature.systemCalls->currentTimeMillis();
                nature.systemCalls->sleep(std::max(timeLeft, (time_type)0));
            }
            mousePosition = nature.systemCalls->getMousePosition();

            if (mousePosition.x != movement.destX || mousePosition.y != movement.destY)
            {
                // It's possible that mouse is manually moved or for some other reason.
                // Let's start next step from pre-calculated location to prevent errors from accumulating.
                // But print warning as this is not expected behavior.
                Logger::Print(nature.info_printer, "Mouse off from step endpoint (adjustment was done) x:(%d -> %d) y:(%d -> %d)",
                        mousePosition.x, movement.destX, mousePosition.y, movement.destY);
                nature.systemCalls->setMousePosition(movement.destX, movement.destY);
                // Let's wait a bit before getting mouse info.
                nature.systemCalls->sleep(SLEEP_AFTER_ADJUSTMENT_MS);
                mousePosition = nature.systemCalls->getMousePosition();
            }

            if (mousePosition.x != xDest || mousePosition.y != yDest)
            {
                // We are dealing with overshoot, let's sleep a bit to simulate human reaction time.
                nature.systemCalls->sleep(nature.reactionTimeBaseMs + (time_type)(nature.random() * (double)nature.reactionTimeVariationMs));
            }
            Logger::Print(nature.info_printer, "Steps completed, mouse at %d, %d", mousePosition.x, mousePosition.y);
        }
        Logger::Print(nature.info_printer, "Mouse movement to (%d, %d) completed", xDest, yDest);
    }

private:

    static int roundTowards(double value, int target)
    {
        if (target > value)
            return (int)std::ceil(value);
        else
            return (int)std::floor(value);
    }
};

// TODO this is a bit hackey and used to simplify client usage
// Might change when/if have multiple movement methods like spiraldown/movedirect/movevia/
// Then usage becomes:  NaturalMouseMotion::Move::To(nature, 200, 25);  NaturalMouseMotion::Move::Spiral(nature, 200, 25);
// But no point having a NaturalMouseMotion::Move::To() for now; so provide NaturalMouseMotion::Move() 
const auto& Move = MoveImp::Move;  // alias

} // namespace NaturalMouseMotion