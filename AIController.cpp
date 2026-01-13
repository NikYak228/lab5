#include "AIController.h"
#include "GamePhysics.h"
#include "GameLevel.h"
#include "LevelLoader.h"
#include "BikePart.h"
#include <iostream>
#include <iomanip>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AIController::AIController() {
    std::cout << "[AI] Система PID-управления инициализирована." << std::endl;
}

void AIController::update(GamePhysics* p) {
    static int frameCount = 0;
    static int stuckCounter = 0;
    frameCount++;
    
    // Сброс ввода перед принятием решений
    p->isInputAcceleration = false;
    p->isInputBreak = false;
    p->isInputBack = false;
    p->isInputForward = false;

    // Проверки валидности
    if (p->isCrashed || !p->levelLoader || !p->levelLoader->gameLevel) return;
    if (p->motorcycleParts.size() < 3) return;

    // Получаем доступ к компонентам байка (шасси и колеса)
    int sIdx = p->primaryStateIndex; // Используем текущий индекс состояния
    auto* chassis = p->motorcycleParts[0]->motoComponents[sIdx].get();
    auto* frontWheel = p->motorcycleParts[1]->motoComponents[sIdx].get();
    auto* rearWheel = p->motorcycleParts[2]->motoComponents[sIdx].get();
    
    if (!chassis || !frontWheel || !rearWheel) return;

    // --- 1. SENSORS (СЕНСОРЫ: АНАЛИЗ ФИЗИКИ) ---
    // Конвертируем координаты F16 в double для расчетов PID
    double fx = (double)frontWheel->xF16 / 65536.0;
    double fy = (double)frontWheel->yF16 / 65536.0;
    double rx = (double)rearWheel->xF16 / 65536.0;
    double ry = (double)rearWheel->yF16 / 65536.0;
    
    // Текущий угол наклона байка (Pitch)
    double dx = fx - rx;
    double dy = fy - ry; 
    double bikePitch = std::atan2(dy, dx); 
    
    // Расчет угловой скорости (производная угла)
    static double lastPitch = 0.0;
    double calculatedAngVel = bikePitch - lastPitch;
    
    // Коррекция перехода через PI/-PI (чтобы не было резкого скачка скорости)
    if (calculatedAngVel > M_PI) calculatedAngVel -= 2.0 * M_PI;
    if (calculatedAngVel < -M_PI) calculatedAngVel += 2.0 * M_PI;
    
    lastPitch = bikePitch;
    
    double velX = (double)chassis->velX / 65536.0;
    
    // --- 2. TARGET CALCULATION (ВЫБОР ЦЕЛИ) ---
    double targetPitch = 0.0;
    
    if (p->inAir) {
        // В воздухе стараемся держать нос чуть вниз (для приземления на склон)
        targetPitch = -0.15; 
    } else {
        // На земле: адаптируемся под наклон трассы
        int mapX = frontWheel->xF16 >> 1; // Координаты для карты (F15)
        int lookAhead = 100 << 15; 
        
        // Получаем высоту под колесом и чуть впереди
        int yCurr = p->levelLoader->gameLevel->getTrackHeightAt(mapX, 0);
        int yNext = p->levelLoader->gameLevel->getTrackHeightAt(mapX + lookAhead, yCurr);
        
        double slopeDy = (double)(yNext - yCurr) / 65536.0;
        double slopeDx = (double)(lookAhead) / 65536.0;
        
        double terrainSlope = std::atan2(slopeDy, slopeDx);
        targetPitch = terrainSlope;
        
        // Ограничиваем цель, чтобы не перевернуться на крутых склонах
        if (terrainSlope < -0.5) targetPitch += 0.2; 
        if (terrainSlope > 0.5) targetPitch -= 0.2;
    }
    
    // --- 3. PID CONTROL (РЕГУЛЯТОР) ---
    double error = targetPitch - bikePitch;
    
    // Коэффициенты регулятора
    double Kp = 3.0;  // Пропорциональный (сила реакции на ошибку угла)
    double Kd = 15.0; // Дифференциальный (гашение колебаний / сопротивление вращению)
    
    double output = (Kp * error) - (Kd * calculatedAngVel);
    
    // Ограничение выхода (Clamp)
    if (output > 5.0) output = 5.0;
    if (output < -5.0) output = -5.0;
    
    std::string action = "";
    
    // --- 4. ACTUATION (ИСПОЛНИТЕЛЬНЫЕ МЕХАНИЗМЫ) ---
    double deadzone = 0.1;
    
    // Управление вращением (Back/Forward)
    if (output > deadzone) {
        p->isInputBack = true; 
        action += "BACK";
    } else if (output < -deadzone) {
        p->isInputForward = true; 
        action += "FWD";
    }
    
    // Газ всегда нажат (для дзен-режима)
    p->isInputAcceleration = true; 
    
    // --- ЗАЩИТА ОТ ПЕРЕВОРОТА (Anti-Loop / Anti-Endo) ---
    // Если байк слишком сильно наклонился назад (Front Loop Risk)
    if (output < -1.5) {
        // Сбрасываем газ или тормозим
        if (velX < 20.0) {
             p->isInputAcceleration = true; // Если почти стоим, газуем чтобы выровняться
        } else {
             p->isInputAcceleration = false; 
        }
        
        // Если летим быстро и сильно задрали нос - тормоз (активное опускание носа)
        if (output < -2.5 && velX > 20.0) p->isInputBreak = true; 
        
        action += "_LOOP_PROT";
    }
    
    // Если байк клюнул носом (Endo Risk)
    if (output > 1.5) {
        // Газ в пол, отпускаем тормоз (поднимаем нос)
        p->isInputAcceleration = true; 
        p->isInputBreak = false;
        action += "_ENDO_PROT";
    }
    
    // --- ДЕТЕКЦИЯ ЗАСТРЕВАНИЯ ---
    if (std::abs(velX) < 2.0 && !p->inAir) stuckCounter++;
    else stuckCounter = 0;
    
    if (stuckCounter > 60) { // Если стоим 1 секунду
        p->isInputAcceleration = true;
        p->isInputBack = true; // Пытаемся раскачаться назад
        action = "STUCK";
    }

    // Логгирование (каждые 15 кадров)
    if (frameCount % 15 == 0) {
        std::cout << "[AI] F:" << frameCount 
                  << " Pitch:" << std::fixed << std::setprecision(2) << bikePitch 
                  << " Tgt:" << targetPitch 
                  << " Err:" << error 
                  << " Out:" << output 
                  << " Act:" << action << std::endl;
    }
}
