#pragma once
#include <Arduino.h>

#include "config.h"
#include "stepper.h"

#define GH_NO_MQTT
#include <GyverHub.h>
GyverHub hub("MyDevices", "MagicScreen", "");

File trace_file;
bool trace_f;
uint32_t node_am, node_cur;

void build() {
    hub.Menu(F("Manual,CNC"));

    switch (hub.menu) {
        // Manual
        case 0: {
            static uint16_t st_speed = 300;
            static uint8_t st_steps = 10;

            hub.BeginWidgets();

            hub.WidgetSize(50);
            hub.Slider(&st_speed, GH_UINT16, F("JOY SPEED"), 0, 4000, 10);
            hub.Slider(&st_steps, GH_UINT8, F("STEPS"), 0, 100);

            GHpos pos;
            hub.Joystick(&pos, true, true);
            if (pos.changed()) {
                mx.start(pos.x * st_speed / 255);
                my.start(-pos.y * st_speed / 255);
            }

            GHpos pos2;
            hub.Dpad(&pos2, F("MOVE"));
            if (pos2.changed()) {
                if (pos2.x || pos2.y) { // != 0
                    int32_t pos[2] = {st_steps * pos2.x, st_steps * pos2.y};
                    planner.setTarget(pos, RELATIVE);
                }
            }

            hub.WidgetSize(50);
            if (hub.Spinner(&data.lashx, GH_UINT8, F("backlash x"))) {
                memory.update();
                planner.setBacklash(0, data.lashx);
            }
            if (hub.Spinner(&data.lashy, GH_UINT8, F("backlash y"))) {
                memory.update();
                planner.setBacklash(1, data.lashy);
            }

            hub.WidgetSize(100);
            hub.Display_(F("pos"), String("x:") + planner.getCurrent(0) + "\ny:" + planner.getCurrent(1), F("Position"));

            hub.WidgetSize(25);
            if (hub.Button(0, "Zero")) planner.reset();
            if (hub.Button(0, "Tare")) {
                data.width = planner.getCurrent(0);
                data.height = planner.getCurrent(1);
                memory.update();
                hub.refresh();
            }
            if (hub.Button(0, "Home")) {
                planner.home();
            }
            if (hub.Button(0, "Stop")) {
                planner.brake();
                mx.stop();
                my.stop();
            }

            hub.WidgetSize(100);
            hub.Label(String(data.width) + 'x' + (data.height), F("real size"));

            hub.WidgetSize(50);
            if (hub.Input(&data.spp, GH_UINT8, F("step per pix"))) {
                memory.update();
                hub.refresh();
            }
            hub.Label(String(data.width / data.spp) + 'x' + (data.height / data.spp), F("virtual size"));
        } break;

        // CNC
        case 1: {
            static String fs_paths;
            static uint8_t sel_file;

            hub.BeginWidgets();
            if (hub.buildUI()) GH_listDir(fs_paths);
            hub.Select(&sel_file, fs_paths, F("File"));

            hub.WidgetSize(25);
            if (hub.Button(0, "Home")) {
                if (!trace_f) {
                    planner.home();
                }
            }
            if (hub.Button(0, "Play")) {
                if (trace_f) planner.resume();
                else {
                    trace_file.close();
                    trace_file = GH_FS.open(GH_listIdx(fs_paths, sel_file), "r");
                    if (!trace_file) hub.sendNotice(F("Open file error"));
                    else {
                        node_cur = 0;
                        node_am = trace_file.parseInt();
                        trace_file.read();  // dummy for skip \n
                        trace_f = 1;
                    }
                }
            }
            if (hub.Button(0, "Pause")) {
                planner.pause();
                hub.sendUpdate(F("stat"), F("Pause"));
            }
            if (hub.Button(0, "Stop")) {
                trace_file.close();
                planner.stop();
                trace_f = 0;
                hub.sendUpdate(F("stat"), F("Stop"));
            }

            hub.WidgetSize(50);
            if (hub.Slider(&data.max_spd, GH_UINT16, F("MAX SPEED"), 0, 4000, 10)) {
                memory.update();
                planner.setMaxSpeed(data.max_spd);
            }
            hub.Label_(F("stat"), trace_f ? F("Run") : F("Stop"), F("Status"), GH_DEFAULT, 30);
        } break;
    }
}

void hub_init() {
    hub.onBuild(build);
    hub.begin();
}

void auto_power() {
    static GHtimer tmr;
    static int32_t x, y;
    if ((x != planner.getCurrent(0) || y != planner.getCurrent(1))) {
        x = planner.getCurrent(0);
        y = planner.getCurrent(1);
        planner.enable();
        tmr.start(3000);
    } else if (tmr) {
        planner.disable();
        tmr.stop();
    }
}

void hub_tick() {
    // tickers
    memory.tick();
    hub.tick();
    planner.tick();
    auto_power();

    // next node
    if (trace_f && planner.ready() && trace_file) {
        if (node_cur % 16 == 0) {
            hub.sendUpdate(F("stat"), String(F("Run, ")) + (node_cur * 100ul / node_am) + '%');
        }
        if (++node_cur >= node_am) {
            trace_f = 0;
            trace_file.close();
            hub.sendUpdate(F("stat"), F("Done"));
            return;
        }
        int32_t pos[2];

        char buf[10];
        int ret0 = trace_file.readBytesUntil(',', buf, 10);
        buf[ret0] = 0;
        pos[0] = atol(buf) * data.spp;
        int ret1 = trace_file.readBytesUntil('\n', buf, 10);
        buf[ret1] = 0;
        pos[1] = atol(buf) * data.spp;

        if (!ret0 || !ret1) {
            trace_f = 0;
            trace_file.close();
            hub.sendUpdate(F("stat"), F("Error"));
        } else {
            planner.setTarget(pos);
        }
        if (node_cur == 2) {
            planner.pause();
            hub.sendUpdate(F("stat"), F("Pause"));
        }
    }

    // manual control
    if (hub.menu == 0) {
        mx.tick();
        my.tick();

        static GHtimer tmr(300);
        static int32_t x, y;
        if (tmr && (x != planner.getCurrent(0) || y != planner.getCurrent(1))) {
            hub.sendUpdate(F("pos"), String(F("x:")) + planner.getCurrent(0) + F("\ny:") + planner.getCurrent(1));
            x = planner.getCurrent(0);
            y = planner.getCurrent(1);
        }
    }
}