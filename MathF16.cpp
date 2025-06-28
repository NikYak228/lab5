#include "MathF16.h"
#include <cmath> 

// число пи
const double CUSTOM_PI = 3.14159265358979323846;

// конвертация из градусов в радианы
double to_radians(int angleF16) {
    return (double)angleF16 / MathF16::PiF16 * CUSTOM_PI;
}

// конвертация из радиан в формат F16
int to_f16_angle(double rad_angle) {
    return (int)(rad_angle / CUSTOM_PI * MathF16::PiF16);
}
// синус, косинус, арктангенс
int MathF16::sinF16(int angle) {
    return (int)(sin(to_radians(angle)) * 65536.0);
}

int MathF16::cosF16(int angle) {
    return (int)(cos(to_radians(angle)) * 65536.0);
}

int MathF16::atan2F16(int dy, int dx) {
    double angle_rad = atan2((double)dy, (double)dx);
    return to_f16_angle(angle_rad);
}