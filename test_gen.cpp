#include "LevelGenerator.h"
#include "LevelLoader.h"
#include "GameLevel.h"
#include "Logger.h"
#include <iostream>
#include <vector>
#include <cmath>

// Stub or Real? We will link real files.
// But we need to initialize Logger.

int main() {
    Logger::init("test_gen.log");
    
    LevelLoader loader;
    LevelGenerator generator;
    // GameLevel is managed by LevelLoader
    GameLevel* level = new GameLevel();
    
    // Wire them up
    loader.gameLevel = level;
    
    // 1. Test Zen Mode
    std::cout << "Testing Zen Mode Generation..." << std::endl;
    
    // loadLevel(loader, mode, levelId)
    // MODE_ZEN_ENDLESS = 1
    generator.loadLevel(&loader, 1, 0); 
    
    // Check initial points
    std::cout << "Initial points: " << level->pointsCount << std::endl;
    
    // Simulate moving forward by updating Zen
    // updateZen checks (lastX - cameraX < 4000)
    // We need to advance cameraX to force generation.
    // lastX starts around 0 or 500.
    
    int cameraX = 0;
    for (int step = 0; step < 100; ++step) {
        cameraX += 1000; // Move camera forward
        generator.updateZen(&loader, cameraX);
    }
    
    int points = level->pointsCount;
    std::cout << "Generated " << points << " points after simulation." << std::endl;
    
    bool wallFound = false;
    
    for (int i = 0; i < points - 1; ++i) {
        int x1 = level->getPointX(i);
        int y1 = level->getPointY(i);
        int x2 = level->getPointX(i+1);
        int y2 = level->getPointY(i+1);
        
        int dx = x2 - x1;
        int dy = y2 - y1;
        
        // Check for backwards or vertical wall
        if (dx <= 0) {
            std::cout << "WALL FOUND (dx<=0) at index " << i 
                      << ": (" << x1 << "," << y1 << ") -> (" << x2 << "," << y2 << ")" << std::endl;
            wallFound = true;
        } else {
             double slope = (double)dy / dx;
             // dy positive means DOWN. dy negative means UP.
             // "Wall" is usually UP (negative dy) and steep.
             // Or maybe DOWN (positive dy) if it's a cliff?
             
             if (std::abs(slope) > 5.0) { // Very steep ( > 78 degrees)
                 std::cout << "STEEP SLOPE found at index " << i 
                           << ": (" << x1 << "," << y1 << ") -> (" << x2 << "," << y2 << ") Slope: " << slope << std::endl;
                 // wallFound = true; 
             }
        }
    }
    
    if (!wallFound) {
        std::cout << "No vertical/backward walls found." << std::endl;
    }
    
    Logger::close();
    return 0;
}
