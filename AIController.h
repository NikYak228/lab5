#ifndef AICONTROLLER_H
#define AICONTROLLER_H

class GamePhysics;

class AIController {
public:
    AIController();
    
    // Основной цикл принятия решений
    void update(GamePhysics* physics);

private:
    // Параметры AI
    static const int LOOK_AHEAD_DIST_F16 = 250 << 16; // Дистанция просмотра вперед
    static const int TARGET_SPEED_F16 = 480000;       // ~7.3 единицы скорости
};

#endif
