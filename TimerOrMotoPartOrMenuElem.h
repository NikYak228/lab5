#pragma once

class MotoComponent {
public:
    int xF16;
    int yF16;
    int angleF16;
    int velX;
    int velY;
    int angularVelocity;
    int forceX;
    int forceY;
    int torque;

    MotoComponent();
    void setToZeros();
};
