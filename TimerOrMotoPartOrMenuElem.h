#pragma once

/**
 * MotoComponent
 * 
 * Базовая единица физики. Представляет материальную точку 
 * с положением, скоростью и силами.
 */
class MotoComponent {
public:
    // Положение (Fixed-point 16.16)
    int xF16;
    int yF16;
    int angleF16;

    // Скорости
    int velX;
    int velY;
    int angularVelocity;

    // Силы
    int forceX;
    int forceY;
    int torque;

    MotoComponent();
    void setToZeros();
};