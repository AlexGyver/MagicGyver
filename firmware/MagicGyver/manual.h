#pragma once
#include <StepperCore.h>

template <GS_driverType _DRV>
class ManualStepper {
   public:
    ManualStepper(Stepper<_DRV>* st) : stepper(st) {}
    void start(int16_t speed) {
        stepper->dir = (speed > 0) ? 1 : -1;
        if (speed) {
            prd = 1000000ul / abs(speed);
            stepper->enable();
        } else {
            prd = 0;
            stepper->disable();
        }
    }
    void stop() {
        prd = 0;
        stepper->disable();
    }
    void tick() {
        if (prd) {
            uint32_t us = micros();
            if (us - tmr >= prd) {
                tmr = us;
                stepper->step();
            }
        }
    }
    bool running() {
        return prd;
    }

   private:
    Stepper<_DRV>* stepper;
    uint32_t tmr, prd;
};