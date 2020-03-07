#pragma once

#include "MotionNature.h"
#include "DefaultProvider.h"

namespace NaturalMouseMotion
{
struct DefaultNature
{
    /*
    * Returns a MotionNature using all the defaults
    */
    static MotionNature NewDefaultNature()
    {
        MotionNature nature;

        nature.info_printer = nullptr; // LoggerPrinterFunc{DefaultProvider::DefaultPrinter()};
        nature.debug_printer = nullptr; // LoggerPrinterFunc{DefaultProvider::DefaultPrinter()};
        nature.observer = nullptr;
        nature.random = RandomZeroToOneFunc{DefaultProvider::DefaultRandomProvider()};
        nature.timeToStepsDivider = DefaultProvider::TIME_TO_STEPS_DIVIDER;
        nature.minSteps = DefaultProvider::MIN_STEPS;
        nature.effectFadeSteps = DefaultProvider::EFFECT_FADE_STEPS;
        nature.reactionTimeBaseMs = DefaultProvider::REACTION_TIME_BASE_MS;
        nature.reactionTimeVariationMs = DefaultProvider::REACTION_TIME_VARIATION_MS;
        nature.getDeviation = GetDeviationFunc{DefaultProvider::SinusoidalDeviationProvider()};
        nature.getNoise = GetNoiseFunc{DefaultProvider::DefaultNoiseProvider()};
        nature.overshootManager = std::shared_ptr<OvershootManager>(new DefaultProvider::DefaultOvershootManager(nature.random));
        nature.systemCalls = std::shared_ptr<SystemCalls>(new DefaultProvider::DefaultSystemCalls());
        nature.getFlowWithTime =
            GetFlowWithTimeFunc{DefaultProvider::DefaultSpeedManager(
                {
                    FlowTemplates::constantSpeed(),
                    FlowTemplates::variatingFlow(),
                    FlowTemplates::interruptedFlow(),
                    FlowTemplates::interruptedFlow2(),
                    FlowTemplates::slowStartupFlow(),
                    FlowTemplates::slowStartup2Flow(),
                    FlowTemplates::adjustingFlow(),
                    FlowTemplates::jaggedFlow(),
                    FlowTemplates::stoppingFlow(),
                },
                nature.random)};
        return nature;
    }

    /**
     * Stereotypical granny using a computer with non-optical mouse from the 90s.
     * Low speed, variating flow, lots of noise in movement.
     */
    static MotionNature NewGrannyNature()
    {
        auto grannyNature = NewDefaultNature();

        grannyNature.timeToStepsDivider = DefaultProvider::TIME_TO_STEPS_DIVIDER - 2.0;
        grannyNature.reactionTimeBaseMs = 100;
        grannyNature.getDeviation = GetDeviationFunc{DefaultProvider::SinusoidalDeviationProvider(9)};
        grannyNature.getNoise = GetNoiseFunc{DefaultProvider::DefaultNoiseProvider(1.6)};

        auto overshootManager = dynamic_cast<DefaultProvider::DefaultOvershootManager*>(grannyNature.overshootManager.get());
        overshootManager->overshoots = 3;
        overshootManager->minDistanceForOvershoots = 3;
        overshootManager->minOvershootMovementMs = 400;
        overshootManager->overshootRandomModifierDivider = DefaultProvider::DefaultOvershootManager::OVERSHOOT_RANDOM_MODIFIER_DIVIDER / 2;
        overshootManager->overshootSpeedupDivider = DefaultProvider::DefaultOvershootManager::OVERSHOOT_SPEEDUP_DIVIDER * 2;

        grannyNature.getFlowWithTime =
            GetFlowWithTimeFunc{DefaultProvider::DefaultSpeedManager(
                {
                    FlowTemplates::jaggedFlow(),
                    FlowTemplates::random(grannyNature.random),
                    FlowTemplates::interruptedFlow(),
                    FlowTemplates::interruptedFlow2(),
                    FlowTemplates::adjustingFlow(),
                    FlowTemplates::stoppingFlow(),
                },
                grannyNature.random, 1000)};
        return grannyNature;
    }

    /**
     * Robotic fluent movement.
     * Custom speed, constant movement, no mistakes, no overshoots.
     * @param motionTimeMsPer100Pixels approximate time a movement takes per 100 pixels of travelling
     */
    static MotionNature NewRobotNature(time_type motionTimeMsPer100Pixels)
    {
        auto robotNature = NewDefaultNature();
        robotNature.getDeviation = [](double, double) -> Point<double> { return {0.0, 0.0}; };
        robotNature.getNoise = [](RandomZeroToOneFunc, double, double) -> Point<double> { return {0.0, 0.0}; };

        auto overshootManager = dynamic_cast<DefaultProvider::DefaultOvershootManager*>(robotNature.overshootManager.get());
        overshootManager->overshoots = 0;

        robotNature.getFlowWithTime = [motionTimeMsPer100Pixels](double distance) -> std::pair<Flow *, time_type>{
            static auto constFlow = Flow(FlowTemplates::constantSpeed());
            double timePerPixel = motionTimeMsPer100Pixels / 100.0;
            return {&constFlow, (time_type)(timePerPixel * distance)};
        };

        return robotNature;
    }

    /**
     * Gamer with fast reflexes and quick mouse movements.
     * Quick movement, low noise, some deviation, lots of overshoots.
     */
    static MotionNature NewFastGamerNature()
    {
        auto gamerNature = NewDefaultNature();

        gamerNature.reactionTimeVariationMs = 100;

        auto overshootManager = dynamic_cast<DefaultProvider::DefaultOvershootManager*>(gamerNature.overshootManager.get());
        overshootManager->overshoots = 4;

        gamerNature.getFlowWithTime =
            GetFlowWithTimeFunc{DefaultProvider::DefaultSpeedManager(
                {
                    FlowTemplates::variatingFlow(),
                    FlowTemplates::slowStartupFlow(),
                    FlowTemplates::slowStartup2Flow(),
                    FlowTemplates::adjustingFlow(),
                    FlowTemplates::jaggedFlow(),
                },
                gamerNature.random, 250)};
        return gamerNature;
    }

    /**
     * Standard computer user with average speed and movement mistakes.
     * Medium noise, medium speed, medium noise and deviation.
     */
    static MotionNature NewAverageComputerUserNature()
    {
        auto averageUserNature = NewDefaultNature();

        averageUserNature.reactionTimeVariationMs = 110;

        auto overshootManager = dynamic_cast<DefaultProvider::DefaultOvershootManager*>(averageUserNature.overshootManager.get());
        overshootManager->overshoots = 4;

        averageUserNature.getFlowWithTime =
            GetFlowWithTimeFunc{DefaultProvider::DefaultSpeedManager(
                {
                    FlowTemplates::variatingFlow(),
                    FlowTemplates::interruptedFlow(),
                    FlowTemplates::interruptedFlow2(),
                    FlowTemplates::slowStartupFlow(),
                    FlowTemplates::slowStartup2Flow(),
                    FlowTemplates::adjustingFlow(),
                    FlowTemplates::jaggedFlow(),
                    FlowTemplates::stoppingFlow(),
                },
                averageUserNature.random, 400)};
        return averageUserNature;
    }
};

} // namespace NaturalMouseMotion