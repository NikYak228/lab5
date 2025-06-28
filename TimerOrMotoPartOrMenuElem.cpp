#include "TimerOrMotoPartOrMenuElem.h"

TimerOrMotoPartOrMenuElem::TimerOrMotoPartOrMenuElem() {
    setToZeros();
}

void TimerOrMotoPartOrMenuElem::setToZeros() {
    xF16 = yF16 = angleF16 = 0;
    field_382 = field_383 = field_384 = 0;
    field_385 = field_386 = field_387 = 0;
}