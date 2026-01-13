#include "TimerOrMotoPartOrMenuElem.h"

MotoComponent::MotoComponent() {
    setToZeros();
}

void MotoComponent::setToZeros() {
    xF16 = yF16 = angleF16 = 0;
    velX = velY = angularVelocity = 0;
    forceX = forceY = torque = 0;
}