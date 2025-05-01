#ifndef _HID_H
#define _HID_H

enum CONTROLLER_TYPE {
    CONTROLLER_TYPE_PROCON,
    CONTROLLER_TYPE_DUALSENSE,
    CONTROLLER_TYPE_INVALID
};

struct SENSITIVITY {
    int x;
    int y;
    int z;
};

HANDLE GetControllerHandle(CONTROLLER_TYPE type);
bool SetupProcon(HANDLE handle);
SENSITIVITY GetProconSensitivity(HANDLE handle);
SENSITIVITY GetDualSenseSensitivity(HANDLE handle);

#endif


