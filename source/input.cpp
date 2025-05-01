#include "input.h"
#include "keyboard.h"
#include <set>
#include <thread>
#include <unordered_map>
#include "hid.h"
#include <atomic>

#include <xinput.h>
#pragma comment (lib, "xinput.lib")

static constexpr const int PRO_CON_RECONNECT_TIMEOUT = 3000;
static constexpr const int DUAL_SENSE_RECONNECT_TIMEOUT = 3000;

static INPUT_STATE currentInputs;
static INPUT_STATE lastInputs;

static HANDLE proconHandle = nullptr;
static SENSITIVITY proconAcceSensitivity;

static HANDLE dualSenseHandle = nullptr;
static SENSITIVITY dualSenseAcceSensitivity;

static std::thread* connectProcon;
static std::thread* connectDualSense;
static std::atomic<bool> connectingProcon = false;
static std::atomic<bool> connectingDualSense = false;
static int lastProconConnectTime = 0;
static int lastDualSenseConnectTime = 0;

static bool xInputConnected = false;

static std::unordered_map<std::wstring, std::wstring> pcLabels;
static std::unordered_map<std::wstring, std::wstring> proconLabels;
static std::unordered_map<std::wstring, std::wstring> dualsenseLabels;
static std::unordered_map<std::wstring, std::wstring> xinputLabels;

void InitInput() {

    pcLabels = {
        {L"↑", L"[W]"},
        {L"↓", L"[S]"},
        {L"←", L"[A]"},
        {L"→", L"[D]"},
        {L"{AnalogLeft}", L"[A][D]"},
        {L"{OK}", L"[Enter]"},
        {L"{Cancel}", L"[Back]"},
        {L"{Start}", L"[Space]"},
        {L"{Select}", L"[Z]"}
    };
    proconLabels = {
        {L"{AnalogLeft}", L"[左スティック]"},
        {L"{OK}", L"🅰"},
        {L"{Cancel}", L"🅱"},
        {L"{Start}", L"⊕"},
        {L"{Select}", L"⊖"}
    };
    dualsenseLabels = {
        {L"{AnalogLeft}", L"[左スティック]"},
        {L"{OK}", L"[○]"},
        {L"{Cancel}", L"[✕]"},
        {L"{Start}", L"[option]"},
        {L"{Select}", L"[create]"}
    };
    xinputLabels = {
        {L"{AnalogLeft}", L"[左スティック]"},
        {L"{OK}", L"🅰"},
        {L"{Cancel}", L"🅱"},
        {L"{Start}", L"[Start]"},
        {L"{Select}", L"[Back]"}
    };
}

void UnitInput() {
    if (connectProcon) {
        if (connectProcon->joinable()) {
            connectProcon->join();
        }
        delete connectProcon;
        connectProcon = nullptr;
    }
    if (proconHandle) {
        CloseHandle(proconHandle);
        proconHandle = nullptr;
    }
}

void ConnectProcon() {
    proconHandle = GetControllerHandle(CONTROLLER_TYPE_PROCON);
    if (proconHandle != INVALID_HANDLE_VALUE) {
        SetupProcon(proconHandle);
        proconAcceSensitivity = GetProconSensitivity(proconHandle);
        
    }
    else {
        proconHandle = nullptr;
    }
    connectingProcon.store(false, std::memory_order_release);
}

bool HasProcon()
{
    return proconHandle && proconHandle != INVALID_HANDLE_VALUE;
}

bool HasDualSense()
{
    return dualSenseHandle && dualSenseHandle != INVALID_HANDLE_VALUE;
}

bool HasXInpput()
{
    return xInputConnected;
}

std::wstring GetInputLabel(const std::wstring& placeholder)
{
    if (HasProcon()) {
        return (proconLabels.count(placeholder)) ? proconLabels[placeholder] : placeholder;
    }
    else if (HasDualSense()) {
        return (dualsenseLabels.count(placeholder)) ? dualsenseLabels[placeholder] : placeholder;
    }
    else if (xInputConnected) {
        return (xinputLabels.count(placeholder)) ? xinputLabels[placeholder] : placeholder;
    }
    return (pcLabels.count(placeholder)) ? pcLabels[placeholder] : placeholder;
}

void ConnectDualSense() {
    dualSenseHandle = GetControllerHandle(CONTROLLER_TYPE_DUALSENSE);
    if (dualSenseHandle != INVALID_HANDLE_VALUE) {
        // プロコンと違って特にセットアップ要らないらしい
        dualSenseAcceSensitivity = GetDualSenseSensitivity(dualSenseHandle);
    }
    else {
        dualSenseHandle = nullptr;
    }
    connectingDualSense.store(false, std::memory_order_release);
}

void UpdateInput()
{
    memcpy(&lastInputs, &currentInputs, sizeof(INPUT_STATE));
    memset(&currentInputs, 0, sizeof(INPUT_STATE));

    if (connectProcon && !connectingProcon.load(std::memory_order_relaxed)) {
        delete connectProcon;
        connectProcon = nullptr;
    }
    if (connectDualSense && !connectingDualSense.load(std::memory_order_relaxed)) {
        delete connectDualSense;
        connectDualSense = nullptr;
    }
    //if (proconHandle) {
    //    BYTE report[256];
    //    DWORD bytesRead;
    //    if (ReadFile(proconHandle, report, sizeof(report), &bytesRead, NULL)) {
    //        if (report[0] == 0x30) {

    //            int acceX = (report[14] << 8) | report[13];
    //            int acceY = (report[16] << 8) | report[15];
    //            int acceZ = (report[18] << 8) | report[17];

    //            if (acceX > 0x7FFF) acceX -= 0x10000;
    //            if (acceY > 0x7FFF) acceY -= 0x10000;
    //            if (acceZ > 0x7FFF) acceZ -= 0x10000;

    //            currentInputs.analog[ANALOG_STATE_ACCE_Z] = -(float)acceX / proconAcceSensitivity.x;
    //            currentInputs.analog[ANALOG_STATE_ACCE_X] = (float)acceY / proconAcceSensitivity.y;
    //            currentInputs.analog[ANALOG_STATE_ACCE_Y] = -(float)acceZ / proconAcceSensitivity.z;


    //            int lStickH = report[6] | ((report[7] & 0x0f) << 8) - 0x7f5;
    //            int lStickV = (report[7] >> 4) | (report[8] << 4) - 0x7db;
    //            float lStickHNormalization = 0.0f;
    //            float lStickVNormalization = 0.0f;
    //            if (lStickH < 0) {
    //                lStickHNormalization = (float)lStickH / 0x61b;
    //            }
    //            else {
    //                lStickHNormalization = (float)lStickH / 0x62f;
    //            }
    //            if (lStickV < 0) {
    //                lStickVNormalization = (float)lStickV / 0x645;
    //            }
    //            else {
    //                lStickVNormalization = (float)lStickV / 0x643;
    //            }

    //            currentInputs.analog[ANALOG_STATE_LEFT_X] = lStickHNormalization;
    //            currentInputs.analog[ANALOG_STATE_LEFT_Y] = lStickVNormalization;
    //            

    //            currentInputs.buttons[BUTTON_STATE_0] = currentInputs.buttons[BUTTON_STATE_0] |
    //                (bool)((report[5] & 0x02)) |
    //                (bool)((report[5] & 0x01)) << 1 |
    //                (bool)((report[5] & 0x08)) << 2 |
    //                (bool)((report[5] & 0x04)) << 3 |
    //                (bool)((report[3] & 0x08)) << 4 |
    //                (bool)((report[3] & 0x04)) << 5 |
    //                (bool)((report[4] & 0x02)) << 6 |
    //                (bool)((report[4] & 0x01)) << 7;
    //        }
    //    }
    //    else {
    //        // 接続ロスト
    //        CloseHandle(proconHandle);
    //        proconHandle = nullptr;
    //    }
    //}
    //else {
    //    // 再接続を試みる
    //    if (!connectProcon && timeGetTime() - lastProconConnectTime > PRO_CON_RECONNECT_TIMEOUT) {
    //        lastProconConnectTime = timeGetTime();
    //        connectingProcon.store(true, std::memory_order_relaxed);
    //        connectProcon = new std::thread(ConnectProcon);
    //        connectProcon->detach();
    //    }
    //}
    if (dualSenseHandle) {
        BYTE report[256];
        DWORD bytesRead;
        if (ReadFile(dualSenseHandle, report, sizeof(report), &bytesRead, NULL)) {
            if (report[0] == 0x01) {

                int acceX = (report[23] << 8) | report[22];
                int acceY = (report[25] << 8) | report[24];
                int acceZ = (report[27] << 8) | report[26];

                if (acceX > 0x7FFF) acceX -= 0x10000;
                if (acceY > 0x7FFF) acceY -= 0x10000;
                if (acceZ > 0x7FFF) acceZ -= 0x10000;


                currentInputs.analog[ANALOG_STATE_ACCE_X] = -(float)acceX / dualSenseAcceSensitivity.x;
                currentInputs.analog[ANALOG_STATE_ACCE_Y] = -(float)acceY / dualSenseAcceSensitivity.y;
                currentInputs.analog[ANALOG_STATE_ACCE_Z] = (float)acceZ / dualSenseAcceSensitivity.z;

                if (report[1] > 0x80) {
                    currentInputs.analog[ANALOG_STATE_LEFT_X] = (float)(report[1] - 0x80) / 127.0f;
                }
                else {
                    currentInputs.analog[ANALOG_STATE_LEFT_X] = (float)(report[1] - 0x80) / 128.0f;
                }
                if (report[2] > 0x80) {
                    currentInputs.analog[ANALOG_STATE_LEFT_Y] = -(float)(report[2] - 0x80) / 127.0f;
                }
                else {
                    currentInputs.analog[ANALOG_STATE_LEFT_Y] = -(float)(report[2] - 0x80) / 128.0f;
                }

                unsigned int dpad = report[8] & 0x0f;
                bool up = (dpad == 0 || dpad == 1 || dpad == 7);
                bool down = (dpad == 3 || dpad == 4 || dpad == 5);
                bool left = (dpad == 5 || dpad == 6 || dpad == 7);
                bool right = (dpad == 1 || dpad == 2 || dpad == 3);
                bool square = (report[8] & 0x10);
                bool cross = (report[8] & 0x20);
                bool circle = (report[8] & 0x40);
                bool triangle = (report[8] & 0x80);
                bool start = (report[9] & 0x20);
                bool select = (report[9] & 0x10);


                currentInputs.buttons[BUTTON_STATE_0] = currentInputs.buttons[BUTTON_STATE_0] |
                    up |
                    down << 1 |
                    left << 2 |
                    right << 3 |
                    circle << 4 |
                    cross << 5 |
                    start << 6 |
                    select << 7;

            }
        }
        else {
            // 接続ロスト
            CloseHandle(dualSenseHandle);
            dualSenseHandle = nullptr;
        }
    }
    else {
        // 再接続を試みる
        if (!connectDualSense && timeGetTime() - lastDualSenseConnectTime > DUAL_SENSE_RECONNECT_TIMEOUT) {
            lastDualSenseConnectTime = timeGetTime();
            connectingDualSense.store(true, std::memory_order_relaxed);
            connectDualSense = new std::thread(ConnectDualSense);
            connectDualSense->detach();
        }
    }

    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));

    DWORD dwResult = XInputGetState(0, &state);

    if (dwResult == ERROR_SUCCESS) {
        xInputConnected = true;

        currentInputs.buttons[BUTTON_STATE_0] = currentInputs.buttons[BUTTON_STATE_0] |
            (bool)((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)) |
            (bool)((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)) << 1 |
            (bool)((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)) << 2 |
            (bool)((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)) << 3 |
            (bool)((state.Gamepad.wButtons & XINPUT_GAMEPAD_A)) << 4 |
            (bool)((state.Gamepad.wButtons & XINPUT_GAMEPAD_B)) << 5 |
            (bool)((state.Gamepad.wButtons & XINPUT_GAMEPAD_START)) << 6 |
            (bool)((state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)) << 7;

        if (state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        {
            currentInputs.analog[ANALOG_STATE_LEFT_X] = (float)state.Gamepad.sThumbLX / 32767.0f;
        }
        else if(state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) 
        {
            currentInputs.analog[ANALOG_STATE_LEFT_X] = (float)state.Gamepad.sThumbLX / 32768.0f;
        }
        else {
            currentInputs.analog[ANALOG_STATE_LEFT_X] = 0.0f;
        }

        if (state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        {
            currentInputs.analog[ANALOG_STATE_LEFT_Y] = (float)state.Gamepad.sThumbLY / 32767.0f;
        }
        else if (state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        {
            currentInputs.analog[ANALOG_STATE_LEFT_Y] = (float)state.Gamepad.sThumbLY / 32768.0f;
        }
        else {
            currentInputs.analog[ANALOG_STATE_LEFT_Y] = 0.0f;
        }
    }
    else {
        xInputConnected = false;
    }

    currentInputs.buttons[BUTTON_STATE_0] = currentInputs.buttons[BUTTON_STATE_0] |
        Keyboard_IsKeyDown(KK_W) |
        Keyboard_IsKeyDown(KK_S) << 1 |
        Keyboard_IsKeyDown(KK_A) << 2 |
        Keyboard_IsKeyDown(KK_D) << 3 |
        Keyboard_IsKeyDown(KK_ENTER) << 4 |
        Keyboard_IsKeyDown(KK_BACK) << 5 |
        Keyboard_IsKeyDown(KK_SPACE) << 6 |
        Keyboard_IsKeyDown(KK_Z) << 7;

}

bool IsInputDown(const BUTTON_STATE alias, const unsigned int mask)
{
    if (alias < 0 || alias >= BUTTON_STATE_MAX) return false;
    return currentInputs.buttons[alias] & mask;
}

bool IsInputTrigger(const BUTTON_STATE alias, const unsigned int mask)
{
    if (alias < 0 || alias >= BUTTON_STATE_MAX) return false;

    return (currentInputs.buttons[alias] & mask) && !(lastInputs.buttons[alias] & mask);
}

float GetInputAnalogValue(const ANALOG_STATE alias)
{
    return currentInputs.analog[alias];
}



