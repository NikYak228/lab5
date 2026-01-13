#pragma once

#include <vector>
#include <memory>

#include "TimerOrMotoPartOrMenuElem.h"

class BikePart {
public:
    bool isActive;
    int connectionLengthF16;
    int connectionType;
    int suspensionStrength;
    int angleOffsetF16;
    std::vector<std::unique_ptr<MotoComponent>> motoComponents = std::vector<std::unique_ptr<MotoComponent>>(6);

    inline BikePart()
    {
        for (int i = 0; i < 6; ++i) {
            motoComponents[i] = std::make_unique<MotoComponent>();
        }
        reset();
    }

    inline ~BikePart()
    {
    }

    inline void reset()
    {
        connectionLengthF16 = suspensionStrength = angleOffsetF16 = 0;
        isActive = true;
        for (int i = 0; i < 6; ++i) {
            motoComponents[i]->setToZeros();
        }
    }
};
