#pragma once

class GamePhysics;

/**
 * AIController
 * 
 * Система искусственного интеллекта для управления мотоциклом.
 * Использует PID-регулятор для поддержания баланса и ориентации.
 */
class AIController {
public:
    AIController();
    
    // Основной цикл принятия решений (вызывается каждый кадр физики)
    void update(GamePhysics* physics);

private:
    // --- Константы настройки AI ---
    static const int LOOK_AHEAD_DIST_F16 = 250 << 16; // Дистанция "взгляда" вперед (для оценки склона)
    static const int TARGET_SPEED_F16 = 480000;       // Целевая скорость
};