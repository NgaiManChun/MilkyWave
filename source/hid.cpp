#include <windows.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <iostream>
#include <vector>
#include <map>
#include "hid.h"
#pragma comment(lib, "hid.lib")
#pragma comment(lib, "SetupAPI.lib")

#define VENDOR_NINTENDO 0x057E
#define PRODUCT_PROCON  0x2009

#define VENDOR_SONY  0x054C
#define PRODUCT_DUALSENSE  0x0CE6

static constexpr const int READ_SPI_TIMEOUT = 1000;

HANDLE GetControllerHandle(CONTROLLER_TYPE type) {
    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);

    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData = {};
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &hidGuid, i, &deviceInterfaceData); i++) {
        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);

        auto detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(requiredSize);
        detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        if (SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, detailData, requiredSize, NULL, NULL)) {

            HANDLE hDevice = CreateFile(
                detailData->DevicePath,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, //FILE_FLAG_OVERLAPPED,
                NULL
            );
            if (hDevice != INVALID_HANDLE_VALUE) {
                HIDD_ATTRIBUTES attributes;
                if (HidD_GetAttributes(hDevice, &attributes)) {
                    if (type == CONTROLLER_TYPE_PROCON && attributes.VendorID == VENDOR_NINTENDO && attributes.ProductID == PRODUCT_PROCON) {
                        free(detailData);
                        return hDevice;
                    }
                    else if (type == CONTROLLER_TYPE_DUALSENSE && attributes.VendorID == VENDOR_SONY && attributes.ProductID == PRODUCT_DUALSENSE) {
                        free(detailData);
                        return hDevice;
                    }
                }
            }
        }

        free(detailData);
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return INVALID_HANDLE_VALUE;
}

bool SetupProcon(HANDLE handle) {

    DWORD bytesWritten;

    // HID初期化要求
    {

        // Macアドレス要求
        std::vector<uint8_t> data = {
                0x01, // レポート
                0x00, // packetCount
                0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40,
                0x80, // subcommand
                0x01 // args
        };

        if (!WriteFile(handle, data.data(), 64, &bytesWritten, nullptr)) {
            return false;
        }
        

        // ハンドシェイク
        data[1] = 0x01;
        data[10] = 0x80;
        data[11] = 0x02;

        if (!WriteFile(handle, data.data(), 64, &bytesWritten, nullptr)) {
            return false;
        }


        // baudrate設定
        data[1] = 0x02;
        data[10] = 0x80;
        data[11] = 0x03;

        if (!WriteFile(handle, data.data(), 64, &bytesWritten, nullptr)) {
            return false;
        }


        // USB HID通信開始
        data[1] = 0x03;
        data[10] = 0x80;
        data[11] = 0x04;

        if (!WriteFile(handle, data.data(), 64, &bytesWritten, nullptr)) {
            return false;
        }

    }


    // フルモード起動
    {
        std::vector<uint8_t> data = {
            0x01, // レポート
            0x04, // packetCount
            0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40,
            0x03, // subcommand
            0x30 // args
        };

        if (!WriteFile(handle, data.data(), 64, &bytesWritten, nullptr)) {
            return false;
        }
    }

    // 六軸センサー起動
    {
        std::vector<uint8_t> data = {
            0x01, // レポート
            0x05, // packetCount
            0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40,
            0x40, // subcommand
            0x01 // args
        };

        if (!WriteFile(handle, data.data(), 64, &bytesWritten, nullptr)) {
            return false;
        }
    }

    

    return true;
}

SENSITIVITY GetProconSensitivity(HANDLE handle)
{
    int x = 0x4000;
    int y = 0x4000;
    int z = 0x4000;

    x /= 4;
    y /= 4;
    z /= 4;

    return {
        x, y, z
    };

    // アクセラレータ感度設定取得（デフォルト設定）
    {
        BYTE report[256];
        DWORD bytesRead;
        DWORD bytesWritten;
        std::vector<uint8_t> data = {
            0x01, // レポート
            0x06, // packetCount
            0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40,
            0x10, // subcommand
            0x26, 0x60, 0x00, 0x00, 0x06 // args
        };

        if (WriteFile(handle, data.data(), 64, &bytesWritten, nullptr)) {
            DWORD timeout = timeGetTime() + READ_SPI_TIMEOUT;
            while (timeout > timeGetTime()) {
                if (ReadFile(handle, report, sizeof(report), &bytesRead, NULL)) {
                    if (report[0] == 0x21) {

                        x = (int)((report[21] << 8) | report[20]);
                        y = (int)((report[23] << 8) | report[22]);
                        z = (int)((report[25] << 8) | report[24]);
                        break;
                    }
                }
                else {
                    break;
                }
            }
        }
    }

    // アクセラレータ感度設定取得（ユーザー設定）
    {
        BYTE report[256];
        DWORD bytesRead;
        DWORD bytesWritten;
        std::vector<uint8_t> data = {
            0x01, // レポート
            0x07, // packetCount
            0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40,
            0x10, // subcommand
            0x2e, 0x80, 0x00, 0x00, 0x06 // args
        };

        if (WriteFile(handle, data.data(), 64, &bytesWritten, nullptr)) {
            DWORD timeout = timeGetTime() + READ_SPI_TIMEOUT;
            while (timeout > timeGetTime()) {
                if (ReadFile(handle, report, sizeof(report), &bytesRead, NULL)) {
                    if (report[0] == 0x21) {

                        int x = (report[21] << 8) | report[20];
                        int y = (report[23] << 8) | report[22];
                        int z = (report[25] << 8) | report[24];

                        if (((report[21] << 8) | report[20]) != 0xffff) x = (report[21] << 8) | report[20];
                        if (((report[23] << 8) | report[22]) != 0xffff) y = (report[23] << 8) | report[22];
                        if (((report[25] << 8) | report[24]) != 0xffff) z = (report[25] << 8) | report[24];

                         break;
                    }
                }
                else {
                    break;
                }
            }
        }
    }

    x /= 4;
    y /= 4;
    z /= 4;

    return {
        x, y, z
    };
}

SENSITIVITY GetDualSenseSensitivity(HANDLE handle)
{
    int x = 0x8000;
    int y = 0x8000;
    int z = 0x8000;

    x /= 4;
    y /= 4;
    z /= 4;

    return {
        x, y, z
    };
}
