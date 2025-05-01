#include "formatTime.h"
#include <iostream>
#include <iomanip>
#include <sstream>


std::string FormatTime(float seconds)
{
    int minutes = std::min(static_cast<int>(seconds) / 60, 59);
    int sec = static_cast<int>(seconds) % 60;
    int millis = static_cast<int>((seconds - static_cast<int>(seconds)) * 1000);

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << sec << "."
        << std::setw(3) << std::setfill('0') << millis;

    return oss.str();
}