#pragma once
#include <GyverPlanner.h>

#include "config.h"
#include "data.h"
#include "manual.h"

Stepper<STEPPER2WIRE> step_x(MOTX_STEP, MOTX_DIR, MOTX_EN);
Stepper<STEPPER2WIRE> step_y(MOTY_STEP, MOTY_DIR, MOTY_EN);
ManualStepper<STEPPER2WIRE> mx(&step_x);
ManualStepper<STEPPER2WIRE> my(&step_y);
GPlanner<STEPPER2WIRE, 2> planner;

void step_init() {
    planner.addStepper(0, step_x);
    planner.addStepper(1, step_y);
    //planner.autoPower(1);
    planner.disable();

    planner.setAcceleration(0);
    planner.setMaxSpeed(data.max_spd);

    planner.setBacklash(0, data.lashx);
    planner.setBacklash(1, data.lashy);
}