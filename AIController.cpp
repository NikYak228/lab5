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
    std::cout << "[AI] PID Control Logic Initialized" << std::endl;
}

void AIController::update(GamePhysics* p) {
    static int frameCount = 0;
    static int stuckCounter = 0;
    frameCount++;
    
    // Reset Inputs
    p->isInputAcceleration = false;
    p->isInputBreak = false;
    p->isInputBack = false;
    p->isInputForward = false;

    if (p->isCrashed || !p->levelLoader || !p->levelLoader->gameLevel) return;
    if (p->motorcycleParts.size() < 3) return;

    int sIdx = p->primaryStateIndex;
    auto* chassis = p->motorcycleParts[0]->motoComponents[sIdx].get();
    auto* frontWheel = p->motorcycleParts[1]->motoComponents[sIdx].get();
    auto* rearWheel = p->motorcycleParts[2]->motoComponents[sIdx].get();
    
    if (!chassis || !frontWheel || !rearWheel) return;

    // --- 1. SENSORS (PHYSICS STATE) ---
    // ... (coords calculation same as before) ...
    double fx = (double)frontWheel->xF16 / 65536.0;
    double fy = (double)frontWheel->yF16 / 65536.0;
    double rx = (double)rearWheel->xF16 / 65536.0;
    double ry = (double)rearWheel->yF16 / 65536.0;
    
    double dx = fx - rx;
    double dy = fy - ry; 
    double bikePitch = std::atan2(dy, dx); 
    
    // Manual Angular Velocity Calculation
    static double lastPitch = 0.0;
    double calculatedAngVel = bikePitch - lastPitch;
    
    // Handle wrap-around (if pitch jumps from PI to -PI)
    if (calculatedAngVel > M_PI) calculatedAngVel -= 2.0 * M_PI;
    if (calculatedAngVel < -M_PI) calculatedAngVel += 2.0 * M_PI;
    
    lastPitch = bikePitch;
    
    double velX = (double)chassis->velX / 65536.0;
    
    // --- 2. TARGET CALCULATION ---
    double targetPitch = 0.0;
    
    if (p->inAir) {
        targetPitch = -0.15; 
    } else {
        int mapX = frontWheel->xF16 >> 1;
        int lookAhead = 100 << 15; 
        int yCurr = p->levelLoader->gameLevel->getTrackHeightAt(mapX, 0);
        int yNext = p->levelLoader->gameLevel->getTrackHeightAt(mapX + lookAhead, yCurr);
        double slopeDy = (double)(yNext - yCurr) / 65536.0;
        double slopeDx = (double)(lookAhead) / 65536.0;
        double terrainSlope = std::atan2(slopeDy, slopeDx);
        targetPitch = terrainSlope;
        if (terrainSlope < -0.5) targetPitch += 0.2; 
        if (terrainSlope > 0.5) targetPitch -= 0.2;
    }
    
    // --- 3. PID CONTROL ---
    double error = targetPitch - bikePitch;
    
    // Coefficients
    double Kp = 3.0;  // Increased for better responsiveness
    double Kd = 15.0; // Reduced to prevent jitter/violent corrections
    
    double output = (Kp * error) - (Kd * calculatedAngVel);
    
    // Clamp Output to sane values (-5.0 to 5.0) to prevent overflow logic
    if (output > 5.0) output = 5.0;
    if (output < -5.0) output = -5.0;
    
    std::string action = "";
    
    // --- 4. ACTUATION (RESTORED TO "IDEAL" SIGNS) ---
    // ... (rest is same) ...
    
    // --- 4. ACTUATION (RESTORED TO "IDEAL" SIGNS) ---
    double deadzone = 0.1;
    
    if (output > deadzone) {
        p->isInputBack = true; 
        action += "BACK";
    } else if (output < -deadzone) {
        p->isInputForward = true; 
        action += "FWD";
    }
    
    p->isInputAcceleration = true; // Default Gas
    
    // Protection (Restored to "Ideal" mapping)
    if (output < -1.5) {
        // This was FWD_LOOP_PROT in your ideal log
        if (velX < 20.0) {
             p->isInputAcceleration = true; 
        } else {
             p->isInputAcceleration = false; 
        }
        if (output < -2.5 && velX > 20.0) p->isInputBreak = true; 
        action += "_LOOP_PROT";
    }
    
    if (output > 1.5) {
        // This was BACK_ENDO_PROT
        p->isInputAcceleration = true; 
        p->isInputBreak = false;
        action += "_ENDO_PROT";
    }
    
    // Stuck
    if (std::abs(velX) < 2.0 && !p->inAir) stuckCounter++;
    else stuckCounter = 0;
    if (stuckCounter > 60) {
        p->isInputAcceleration = true;
        p->isInputBack = true;
        action = "STUCK";
    }

    // Logging
    if (frameCount % 15 == 0) {
        std::cout << "[AI] F:" << frameCount 
                  << " Pitch:" << std::fixed << std::setprecision(2) << bikePitch 
                  << " Tgt:" << targetPitch 
                  << " Err:" << error 
                  << " Out:" << output 
                  << " Act:" << action << std::endl;
    }
}