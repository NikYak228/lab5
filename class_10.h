#pragma once

#include <vector>
#include <memory>

#include "TimerOrMotoPartOrMenuElem.h"

class class_10 {
public:
    bool unusedBool;
    int field_257;
    int field_258;
    int field_259;
    int field_260;
    std::vector<std::unique_ptr<TimerOrMotoPartOrMenuElem>> motoComponents = std::vector<std::unique_ptr<TimerOrMotoPartOrMenuElem>>(6);

    class_10();
    ~class_10();
    void reset();
};

class_10::class_10()
{
    for (int var1 = 0; var1 < 6; ++var1) {
        motoComponents[var1] = std::make_unique<TimerOrMotoPartOrMenuElem>();
    }

    reset();
}

class_10::~class_10()
{
    //
}

void class_10::reset()
{
    field_257 = field_259 = field_260 = 0;
    unusedBool = true;
    for (int i = 0; i < 6; ++i) {
        motoComponents[i]->setToZeros();
    }
}