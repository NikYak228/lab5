#pragma once

#include <vector>
#include <memory>

#include "TimerOrMotoPartOrMenuElem.h"

/**
 * BikePart
 * 
 * Представляет составную часть мотоцикла (колесо, шасси, амортизатор).
 * Хранит физические компоненты (точки) и параметры связи.
 */
class BikePart {
public:
    bool isActive;
    int connectionLengthF16; // Длина связи
    int connectionType;      // Тип (0=Колесо, 1=Шасси, 2=Голова)
    int suspensionStrength;  // Жесткость подвески
    int angleOffsetF16;      // Смещение угла
    
    // Массив физических состояний (компонентов) для этой части
    std::vector<std::unique_ptr<MotoComponent>> motoComponents = std::vector<std::unique_ptr<MotoComponent>>(6);

    inline BikePart() {
        for (int i = 0; i < 6; ++i) {
            motoComponents[i] = std::make_unique<MotoComponent>();
        }
        reset();
    }

    inline ~BikePart() {}

    // Сброс параметров части
    inline void reset() {
        connectionLengthF16 = suspensionStrength = angleOffsetF16 = 0;
        isActive = true;
        for (int i = 0; i < 6; ++i) {
            motoComponents[i]->setToZeros();
        }
    }
};