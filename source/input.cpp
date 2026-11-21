// Reads PS3 pads by physical port so disconnects never move another player's controls.
#include "input.h"
#include <debugnet.h>
#include <io/pad.h>

static_assert(CONTROLLER_PORTS == MAX_PORT_NUM, "PS3 controller port count");
const unsigned DIGITAL2_SHIFT = 8;
const unsigned PORT_CONNECTED = 1;

struct PadState {
    bool connected;
    unsigned buttons;
    float stick[2];
    int flick[2];
    float pulseStrength, pulseLeft;
    int sentLarge, sentSmall;
};
static PadState pads[CONTROLLER_PORTS];

bool inputOpen() {
    int result = ioPadInit(CONTROLLER_PORTS);
    if (result != PAD_OK) debugNetPrintf(ERROR, "controller init failed: 0x%x\n", (unsigned)result);
    return result == PAD_OK;
}

void inputRead(Actions* actions) {
    padInfo2 info = {};
    bool haveStatus = ioPadGetInfo2(&info) == PAD_OK;
    for (int port = 0; port < CONTROLLER_PORTS; port++) {
        Actions& a = actions[port];
        a = {};
        PadState& pad = pads[port];
        bool connected = haveStatus && (info.port_status[port] & PORT_CONNECTED) && info.device_type[port] != PAD_TYPE_REMOTE;
        if (connected != pad.connected)
            debugNetPrintf(INFO, "controller %d %s\n", port, connected ? "connected" : "disconnected");
        if (!connected) {
            pad = {};
            continue;
        }
        pad.connected = a.connected = true;

        unsigned previous = pad.buttons;
        padData data = {};
        if (ioPadGetData(port, &data) == PAD_OK && data.len > PAD_BUTTON_OFFSET_ANALOG_LEFT_Y) {
            pad.buttons = data.button[PAD_BUTTON_OFFSET_DIGITAL1] | (unsigned)data.button[PAD_BUTTON_OFFSET_DIGITAL2] << DIGITAL2_SHIFT;
            pad.stick[0] = ((int)data.button[PAD_BUTTON_OFFSET_ANALOG_LEFT_X] - 128) / 128.0f;
            pad.stick[1] = ((int)data.button[PAD_BUTTON_OFFSET_ANALOG_LEFT_Y] - 128) / 128.0f;
        }
        unsigned pressed = pad.buttons & ~previous;
        auto down = [&](unsigned mask) { return (pad.buttons & mask) != 0; };
        auto press = [&](unsigned mask) { return (pressed & mask) != 0; };
        auto down2 = [&](unsigned mask) { return down(mask << DIGITAL2_SHIFT); };
        auto press2 = [&](unsigned mask) { return press(mask << DIGITAL2_SHIFT); };

        float dx = down(PAD_CTRL_RIGHT) - down(PAD_CTRL_LEFT);
        float dy = down(PAD_CTRL_DOWN) - down(PAD_CTRL_UP);
        if (dx || dy) {
            float length = sqrtf(dx * dx + dy * dy);
            a.moveX = dx / length;
            a.moveY = dy / length;
        } else {
            a.moveX = pad.stick[0];
            a.moveY = pad.stick[1];
            applyDeadzone(a.moveX, a.moveY, STICK_DEADZONE);
        }
        a.fire = down2(PAD_CTRL_R1);
        a.rocket = press2(PAD_CTRL_R2);
        a.pause = press(PAD_CTRL_START);
        a.item = press2(PAD_CTRL_TRIANGLE);
        a.autopilot = press(PAD_CTRL_SELECT);
        a.screenshot = press2(PAD_CTRL_L1);
        a.godMode = (press2(PAD_CTRL_L2) || press(PAD_CTRL_L3)) && down2(PAD_CTRL_L2) && down(PAD_CTRL_L3);
        a.cycle = press2(PAD_CTRL_SQUARE);
        a.jump = press2(PAD_CTRL_CROSS);
        a.confirm = a.jump || a.pause;
        a.jumpHeld = down2(PAD_CTRL_CROSS);
        a.selectHeld = down(PAD_CTRL_SELECT);
        a.up = press(PAD_CTRL_UP);
        a.down = press(PAD_CTRL_DOWN);
        a.left = press(PAD_CTRL_LEFT);
        a.right = press(PAD_CTRL_RIGHT);
        a.back = press2(PAD_CTRL_CIRCLE);
        for (int axis = 0; axis < 2; axis++) {
            float v = pad.stick[axis];
            int dir = v < -0.6f ? -1 : v > 0.6f ? 1 : fabsf(v) < 0.3f ? 0 : pad.flick[axis];
            if (dir != 0 && dir != pad.flick[axis]) {
                if (axis == 0) (dir < 0 ? a.left : a.right) = true;
                else (dir < 0 ? a.up : a.down) = true;
            }
            pad.flick[axis] = dir;
        }
    }
}

void inputRumblePulse(int port, float strength, float seconds) {
    PadState& pad = pads[port];
    if (pad.pulseLeft <= 0 || strength >= pad.pulseStrength) {
        pad.pulseStrength = strength;
        pad.pulseLeft = seconds;
    }
}

void inputRumbleUpdate(unsigned activePorts, float shake, float dt) {
    for (int port = 0; port < CONTROLLER_PORTS; port++) {
        PadState& pad = pads[port];
        if (!pad.connected) continue;
        bool active = (activePorts & (1u << port)) != 0;
        pad.pulseLeft = active ? fmaxf(0, pad.pulseLeft - dt) : 0;
        float pulse = pad.pulseLeft > 0 ? pad.pulseStrength : 0;
        float level = fmaxf(pulse, active && shake >= 0.1f ? shake : 0);
        int large = (int)(fminf(level, 1) * 255) / 16 * 16;
        int small = pad.pulseLeft > 0 ? 1 : 0;
        if (large == pad.sentLarge && small == pad.sentSmall) continue;
        padActParam act = {};
        act.small_motor = small;
        act.large_motor = large;
        ioPadSetActDirect(port, &act);
        pad.sentLarge = large;
        pad.sentSmall = small;
    }
}

void inputClose() {
    inputRumbleUpdate(0, 0, 0);
    ioPadEnd();
}
