#include "GamePhysics.h"

#include "BikePart.h"
#include "LevelLoader.h"
#include "MathF16.h"
#include "AIController.h"
#include "LevelGenerator.h"
#include "Logger.h"
#include <algorithm>
#include <iostream>
#include <cmath>

// --- Конструктор ---
GamePhysics::GamePhysics(LevelLoader *levelLoader) {
  LOG_PHYS("GamePhysics: Конструктор вызван.");
  primaryStateIndex = 0;
  secondaryStateIndex = 1;

  // Инициализация частей мотоцикла
  for (int i = 0; i < 6; ++i) {
    motoComponents[i] = std::make_unique<MotoComponent>();
  }
  
  // Сброс флагов и параметров
  restartCounter = 0;
  gameMode = 0;
  useSpriteGraphics = false;
  isRenderMotoWithSprites = false;
  
  // Сброс ввода
  isInputAcceleration = false;
  isInputBreak = false;
  isInputBack = false;
  isInputForward = false;
  isInputUp = false;
  isInputDown = false;
  isInputLeft = false;
  isInputRight = false;
  
  inAir = false;
  trackStarted = false;
  isEnableLookAhead = true;
  camShiftX = 0;
  camShiftY = 0;
  cameraShiftLimit = 655360;

  cameraParams = {{45875}, {32768}, {52428}};
  this->levelLoader = levelLoader;
  
  // Инициализация AI системы и генератора
  aiController = std::make_unique<AIController>();
  levelGenerator = std::make_unique<LevelGenerator>();
  currentLevelMode = LevelMode::MANUAL;
  aiRestartTimer = 0;
  
  resetSmth(true);
  isGenerateInputAI = false;
  snapshotMotoState();
  isCrashed = false;
}

// --- Управление настройками ---

int GamePhysics::getRenderMode() {
  if (useSpriteGraphics && isRenderMotoWithSprites) {
    return 3;
  } else if (isRenderMotoWithSprites) {
    return 1;
  } else {
    return useSpriteGraphics ? 2 : 0;
  }
}

void GamePhysics::setRenderFlags(int flags) {
  useSpriteGraphics = false;
  isRenderMotoWithSprites = false;
  if ((flags & 2) != 0) {
    useSpriteGraphics = true;
  }

  if ((flags & 1) != 0) {
    isRenderMotoWithSprites = true;
  }
}

void GamePhysics::setMode(int mode) {
  gameMode = mode;
  switch (mode) {
  case 1:
  default:
    wheelBase = 1310;
    gravityF16 = 1638400; // Стандартная гравитация
    setMotoLeague(1);
  }
}

void GamePhysics::setMotoLeague(int league) {
  curentMotoLeague = league;
  
  // Базовые настройки подвески
  suspensionFront = 45875;
  suspensionBack = 13107;
  forkLength = 39321;
  massFactor = 1600000;
  dampingF16 = 262144;
  frictionCoeff = 6553;
  
  switch (league) {
  case 0:
  default:
    torqueFront = 18000;
    torqueRear = 18000;
    maxAngularVelocity = 1200000;   // GOD MODE: Безопасная макс. скорость
    maxTorque = 200000000;          // GOD MODE: Крутящий момент
    accelStep = 20000000;           // GOD MODE: Ускорение
    brakeDamping = 327;
    brakeForce = 0;
    airControlForce = 32768;
    airControlLimit = 327680;
    springStiffness = 24660800; // Жесткие пружины
    break;
  case 1:
    torqueFront = 32768;
    torqueRear = 32768;
    maxAngularVelocity = 2500000;   // Увеличена для петли
    maxTorque = 120000000; 
    accelStep = 8000000;            // Быстрый набор скорости
    brakeDamping = 6553;
    brakeForce = 26214;
    airControlForce = 26214;
    airControlLimit = 327680;
    springStiffness = 19660800;
    break;

  case 2:
    // --- Двигатель на максимуме ---
    maxAngularVelocity = 2200000;
    maxTorque = 90000000;
    accelStep = 5500000;

    // --- Шасси и управление ---
    massFactor = 1350000; // Легкий байк
    airControlForce = 45000; // Высокий контроль в воздухе

    // --- Остальные параметры ---
    torqueFront = 32768;
    torqueRear = 32768;
    brakeDamping = 6553;
    brakeForce = 26214;
    airControlLimit = 327680;
    springStiffness = 21626880;
    suspensionFront = 45875;
    suspensionBack = 13107;
    forkLength = 39321;
    frictionCoeff = 6553;
    dampingF16 = 262144;
    curentMotoLeague = league;
    break;
  case 3: // Демонстрационный режим "Наглядная Подвеска"
    maxAngularVelocity = 1400000;
    maxTorque = 90000000;
    accelStep = 2400000;

    // --- Шасси и Подвеска ---
    massFactor = 1850000;   // Тяжелее
    springStiffness = 18000000; // Мягкие пружины
    dampingF16 = 340000;    // Сильное демпфирование
    frictionCoeff = 12500; // Сильное торможение двигателем

    // --- Управление в воздухе ---
    airControlForce = 26000;

    // Остальные параметры
    curentMotoLeague = league;
    suspensionFront = 45875;
    suspensionBack = 13107;
    forkLength = 39321;
    frictionCoeff = 6553;
    torqueFront = 32768;
    torqueRear = 32768;
    brakeDamping = 6553;
    brakeForce = 26214;
    airControlLimit = 327680;
    break;
  }
}

void GamePhysics::resetSmth(bool unused) {

  (void)unused;
  restartCounter = 0;

  setupBike(levelLoader->getStartPosX(), levelLoader->getStartPosY());

  engineTorqueF16 = 0;
  wheelBalance = 0;
  isCrashed = false;
  isGroundColliding = false;
  inAir = false;
  trackStarted = false;
  atStart = false;
  cameraToggle = false;

  levelLoader->gameLevel->setSegmentRange(
      motorcycleParts[2]->motoComponents[5]->xF16 + 98304 - const175_1_half[0],
      motorcycleParts[1]->motoComponents[5]->xF16 - 98304 + const175_1_half[0]);

  // Запоминаем базовое расстояние между колесами
  initialWheelSeparationF16 = calcVectorLengthF16(
      motorcycleParts[1]->motoComponents[primaryStateIndex]->xF16 -
          motorcycleParts[2]->motoComponents[primaryStateIndex]->xF16,
      motorcycleParts[1]->motoComponents[primaryStateIndex]->yF16 -
          motorcycleParts[2]->motoComponents[primaryStateIndex]->yF16);
}

void GamePhysics::shiftBikeVertical(bool up) {
  int var2 = (up ? 65536 : -65536) << 1;

  for (int var3 = 0; var3 < 6; ++var3) {
    for (int var4 = 0; var4 < 6; ++var4) {
      motorcycleParts[var3]->motoComponents[var4]->yF16 += var2;
    }
  }
}

// --- Инициализация байка ---
void GamePhysics::setupBike(int startX, int startY) {
  if (motorcycleParts.empty()) {
    motorcycleParts = std::vector<std::unique_ptr<BikePart>>(6);
  }

  if (springComponents.empty()) {
    springComponents = std::vector<std::unique_ptr<MotoComponent>>(10);
  }

  int var4 = 0;
  int8_t connectionTypeLocal = 0;
  int var6 = 0;
  int var7 = 0;

  int i;
  for (i = 0; i < 6; ++i) {
    short angleOffset = 0;
    switch (i) {
    case 0: // Шасси
      connectionTypeLocal = 1;
      var4 = 360448;
      var6 = 0;
      var7 = 0;
      break;
    case 1: // Переднее колесо
      connectionTypeLocal = 0;
      var4 = 98304;
      var6 = 229376;
      var7 = 0;
      break;
    case 2: // Заднее колесо
      connectionTypeLocal = 0;
      var4 = 360448;
      var6 = -229376;
      var7 = 0;
      angleOffset = 21626;
      break;
    case 3: // Амортизатор задний
      connectionTypeLocal = 1;
      var4 = 229376;
      var6 = 131072;
      var7 = 196608;
      break;
    case 4: // Амортизатор передний
      connectionTypeLocal = 1;
      var4 = 229376;
      var6 = -131072;
      var7 = 196608;
      break;
    case 5: // Голова водителя
      connectionTypeLocal = 2;
      var4 = 294912;
      var6 = 0;
      var7 = 327680;
    }

    if (motorcycleParts[i] == nullptr) {
      motorcycleParts[i] = std::make_unique<BikePart>();
    }

    motorcycleParts[i]->reset();
    motorcycleParts[i]->connectionLengthF16 = const175_1_half[connectionTypeLocal];
    motorcycleParts[i]->connectionType = connectionTypeLocal;
    // Расчет прочности подвески на основе массы
    motorcycleParts[i]->suspensionStrength =
        (int)((int64_t)((int)(281474976710656L / (int64_t)var4 >> 16)) *
                  (int64_t)massFactor >>
              16);
    // Начальные координаты
    motorcycleParts[i]->motoComponents[primaryStateIndex]->xF16 = startX + var6;
    motorcycleParts[i]->motoComponents[primaryStateIndex]->yF16 =
        startY + var7 + 500000;
    motorcycleParts[i]->motoComponents[5]->xF16 = startX + var6;
    motorcycleParts[i]->motoComponents[5]->yF16 = startY + var7 + 500000;
    motorcycleParts[i]->angleOffsetF16 = angleOffset;
  }

  for (i = 0; i < 10; ++i) {
    if (springComponents[i] == nullptr) {
      springComponents[i] = std::make_unique<MotoComponent>();
    }

    springComponents[i]->setToZeros();
    springComponents[i]->xF16 = springStiffness;
    springComponents[i]->angleF16 = dampingF16;
  }

  // Настройка параметров пружин (жесткость, длина и т.д.)
  springComponents[0]->yF16 = 229376;
  springComponents[1]->yF16 = 229376;
  springComponents[2]->yF16 = 236293;
  springComponents[3]->yF16 = 236293;
  springComponents[4]->yF16 = 262144;
  springComponents[5]->yF16 = 219814;
  springComponents[6]->yF16 = 219814;
  springComponents[7]->yF16 = 185363;
  springComponents[8]->yF16 = 185363;
  springComponents[9]->yF16 = 327680;
  
  springComponents[5]->angleF16 = (int)((int64_t)dampingF16 * 45875L >> 16);
  springComponents[6]->xF16 = (int)(6553L * (int64_t)springStiffness >> 16);
  springComponents[5]->xF16 = (int)(6553L * (int64_t)springStiffness >> 16);
  springComponents[9]->xF16 = (int)(72089L * (int64_t)springStiffness >> 16);
  springComponents[8]->xF16 = (int)(72089L * (int64_t)springStiffness >> 16);
  springComponents[7]->xF16 = (int)(72089L * (int64_t)springStiffness >> 16);
}

void GamePhysics::setRenderMinMaxX(int minX, int maxX) {
  levelLoader->setMinMaxX(minX, maxX);
}

// --- Обработка ввода ---

void GamePhysics::processPointerReleased() {
  isInputUp = isInputDown = isInputRight = isInputLeft = false;
}

void GamePhysics::applyUserInput(int xDir, int yDir) {
  if (!isGenerateInputAI) {
    isInputUp = isInputDown = isInputRight = isInputLeft = false;
    if (xDir > 0) {
      isInputUp = true;
    } else if (xDir < 0) {
      isInputDown = true;
    }

    if (yDir > 0) {
      isInputRight = true;
      return;
    }

    if (yDir < 0) {
      isInputLeft = true;
    }
  }
}

void GamePhysics::enableGenerateInputAI() {
  resetSmth(true);
  isGenerateInputAI = true;
}

void GamePhysics::disableGenerateInputAI() { isGenerateInputAI = false; }

void GamePhysics::setInputFromAI() {
  // Вызываем логику AI
  if (aiController) {
    aiController->update(this);
  } else {
    static int warnCount = 0;
    if (warnCount++ % 300 == 0) { 
      std::cout << "[GamePhysics] WARNING: isGenerateInputAI=true но aiController is null!" << std::endl;
    }
  }
}

void GamePhysics::setLevelMode(LevelMode mode, int levelId) {
    currentLevelMode = mode;
    isGenerateInputAI = (mode != LevelMode::MANUAL);
    
    std::cout << "[GamePhysics] setLevelMode: mode=" << static_cast<int>(mode) 
              << ", levelId=" << levelId 
              << ", isGenerateInputAI=" << (isGenerateInputAI ? "true" : "false") << std::endl;
    
    if (levelGenerator) {
        int modeInt = static_cast<int>(mode);
        levelGenerator->loadLevel(levelLoader, modeInt, levelId);
    }
    
    resetSmth(true); 
    
    // Восстанавливаем флаг AI после сброса
    isGenerateInputAI = (mode != LevelMode::MANUAL);
    aiRestartTimer = 0;
}

// --- Физика Байка (Engine, Control) ---

void GamePhysics::updateBikePhysics() {
  if (!isCrashed) {
    // Рассчитываем вектор между колесами
    int vecX = motorcycleParts[1]->motoComponents[primaryStateIndex]->xF16 -
               motorcycleParts[2]->motoComponents[primaryStateIndex]->xF16;
    int vecY = motorcycleParts[1]->motoComponents[primaryStateIndex]->yF16 -
               motorcycleParts[2]->motoComponents[primaryStateIndex]->yF16;
    int vecLen = calcVectorLengthF16(vecX, vecY);

    if (vecLen != 0) {
      double normalized_x = (double)vecX / vecLen;
      double normalized_y = (double)vecY / vecLen;
      vecX = (int)(normalized_x * 65536.0);
      vecY = (int)(normalized_y * 65536.0);
    }

    // Применяем ускорение
    if (isInputAcceleration && engineTorqueF16 >= -maxTorque) {
      engineTorqueF16 -= accelStep;
    }

    // Применяем торможение
    if (isInputBreak) {
      engineTorqueF16 = 0;
      // Демпфирование угловой скорости колес
      motorcycleParts[1]->motoComponents[primaryStateIndex]->angularVelocity =
          (int)((int64_t)motorcycleParts[1]
                        ->motoComponents[primaryStateIndex]
                        ->angularVelocity *
                    (int64_t)(65536 - brakeDamping) >>
                16);
      motorcycleParts[2]->motoComponents[primaryStateIndex]->angularVelocity =
          (int)((int64_t)motorcycleParts[2]
                        ->motoComponents[primaryStateIndex]
                        ->angularVelocity *
                    (int64_t)(65536 - brakeDamping) >>
                16);
      
      // Остановка колес при малой скорости
      if (motorcycleParts[1]
              ->motoComponents[primaryStateIndex]
              ->angularVelocity < 6553) {
        motorcycleParts[1]->motoComponents[primaryStateIndex]->angularVelocity = 0;
      }

      if (motorcycleParts[2]
              ->motoComponents[primaryStateIndex]
              ->angularVelocity < 6553) {
        motorcycleParts[2]->motoComponents[primaryStateIndex]->angularVelocity = 0;
      }
    }

    // Настройка жесткости подвески (динамическая, зависит от нагрузки)
    motorcycleParts[0]->suspensionStrength = (int)(11915L * (int64_t)massFactor >> 16);
    motorcycleParts[4]->suspensionStrength = (int)(18724L * (int64_t)massFactor >> 16);
    motorcycleParts[3]->suspensionStrength = (int)(18724L * (int64_t)massFactor >> 16);
    motorcycleParts[1]->suspensionStrength = (int)(43690L * (int64_t)massFactor >> 16);
    motorcycleParts[2]->suspensionStrength = (int)(11915L * (int64_t)massFactor >> 16);
    motorcycleParts[5]->suspensionStrength = (int)(14563L * (int64_t)massFactor >> 16);
    
    // Перенос веса при наклонах (ввод Back/Forward)
    if (isInputBack) {
      motorcycleParts[0]->suspensionStrength = (int)(18724L * (int64_t)massFactor >> 16);
      motorcycleParts[4]->suspensionStrength = (int)(14563L * (int64_t)massFactor >> 16);
      motorcycleParts[3]->suspensionStrength = (int)(18724L * (int64_t)massFactor >> 16);
      motorcycleParts[1]->suspensionStrength = (int)(43690L * (int64_t)massFactor >> 16);
      motorcycleParts[2]->suspensionStrength = (int)(10082L * (int64_t)massFactor >> 16);
    } else if (isInputForward) {
      motorcycleParts[0]->suspensionStrength = (int)(18724L * (int64_t)massFactor >> 16);
      motorcycleParts[4]->suspensionStrength = (int)(18724L * (int64_t)massFactor >> 16);
      motorcycleParts[3]->suspensionStrength = (int)(14563L * (int64_t)massFactor >> 16);
      motorcycleParts[1]->suspensionStrength = (int)(26214L * (int64_t)massFactor >> 16);
      motorcycleParts[2]->suspensionStrength = (int)(11915L * (int64_t)massFactor >> 16);
    }

    // Управление в воздухе (вращение байка)
    if (isInputBack || isInputForward) {
      int perpX = -vecY;
      // int perpY = vecX; // unused locally, but logic implied
      
      MotoComponent *comp;
      int scale;
      int force;
      int forceX;
      int forceY;
      int forceX2;
      int forceY2;

      // Логика вращения НАЗАД
      if (isInputBack && wheelBalance > -airControlLimit) {
        scale = 65536;
        if (wheelBalance < 0) {
          scale = (int)(((int64_t)(airControlLimit - (wheelBalance < 0 ? -wheelBalance : wheelBalance)) << 32) /
                        (int64_t)airControlLimit >> 16);
        }

        force = (int)((int64_t)airControlForce * (int64_t)scale >> 16);
        forceX = (int)((int64_t)perpX * (int64_t)force >> 16);
        forceY = (int)((int64_t)vecX * (int64_t)force >> 16);
        forceX2 = (int)((int64_t)vecX * (int64_t)force >> 16);
        forceY2 = (int)((int64_t)vecY * (int64_t)force >> 16);
        
        // Обновляем угол наклона водителя
        if (tiltAngleF16 > 32768) {
          tiltAngleF16 = tiltAngleF16 - 1638 < 0 ? 0 : tiltAngleF16 - 1638;
        } else {
          tiltAngleF16 = tiltAngleF16 - 3276 < 0 ? 0 : tiltAngleF16 - 3276;
        }

        // Применяем силы к частям тела (голова, плечи)
        comp = motorcycleParts[4]->motoComponents[primaryStateIndex].get();
        comp->velX -= forceX;
        comp = motorcycleParts[4]->motoComponents[primaryStateIndex].get();
        comp->velY -= forceY;
        comp = motorcycleParts[3]->motoComponents[primaryStateIndex].get();
        comp->velX += forceX;
        comp = motorcycleParts[3]->motoComponents[primaryStateIndex].get();
        comp->velY += forceY;
        comp = motorcycleParts[5]->motoComponents[primaryStateIndex].get();
        comp->velX -= forceX2;
        comp = motorcycleParts[5]->motoComponents[primaryStateIndex].get();
        comp->velY -= forceY2;
      }

      // Логика вращения ВПЕРЕД
      if (isInputForward && wheelBalance < airControlLimit) {
        scale = 65536;
        if (wheelBalance > 0) {
          scale = (int)(((int64_t)(airControlLimit - wheelBalance) << 32) /
                           (int64_t)airControlLimit >> 16);
        }

        force = (int)((int64_t)airControlForce * (int64_t)scale >> 16);
        forceX = (int)((int64_t)perpX * (int64_t)force >> 16);
        forceY = (int)((int64_t)vecX * (int64_t)force >> 16);
        forceX2 = (int)((int64_t)vecX * (int64_t)force >> 16);
        forceY2 = (int)((int64_t)vecY * (int64_t)force >> 16);
        
        if (tiltAngleF16 > 32768) {
          tiltAngleF16 = tiltAngleF16 + 1638 < 65536 ? tiltAngleF16 + 1638 : 65536;
        } else {
          tiltAngleF16 = tiltAngleF16 + 3276 < 65536 ? tiltAngleF16 + 3276 : 65536;
        }

        comp = motorcycleParts[4]->motoComponents[primaryStateIndex].get();
        comp->velX += forceX;
        comp = motorcycleParts[4]->motoComponents[primaryStateIndex].get();
        comp->velY += forceY;
        comp = motorcycleParts[3]->motoComponents[primaryStateIndex].get();
        comp->velX -= forceX;
        comp = motorcycleParts[3]->motoComponents[primaryStateIndex].get();
        comp->velY -= forceY;
        comp = motorcycleParts[5]->motoComponents[primaryStateIndex].get();
        comp->velX += forceX2;
        comp = motorcycleParts[5]->motoComponents[primaryStateIndex].get();
        comp->velY += forceY2;
      }
      return;
    }

    // Возврат водителя в нейтральное положение
    if (tiltAngleF16 < 26214) {
      tiltAngleF16 += 3276;
      return;
    }
    if (tiltAngleF16 > 39321) {
      tiltAngleF16 -= 3276;
      return;
    }
    tiltAngleF16 = 32768;
  }
}

// --- Основной цикл обновления ---

int GamePhysics::updatePhysics() {
  // 1. Управление Дзен-генератором
  if (currentLevelMode == LevelMode::ZEN_ENDLESS && !isCrashed && levelGenerator) {
    levelGenerator->updateZen(levelLoader, getCamPosX());
    checkAndShiftWorld();
  }

  // 2. Авто-рестарт при аварии AI
  if (isCrashed && currentLevelMode != LevelMode::MANUAL) {
    if (++aiRestartTimer > 60) {
      setLevelMode(currentLevelMode, 0); 
      return 0;
    }
  }

  // 3. Обработка ввода
  static int inputLogCounter = 0;
  if (isGenerateInputAI) {
    setInputFromAI();
    if (++inputLogCounter % 60 == 0) {
      // Логируем редко, чтобы не засорять консоль
      // std::cout << "AI Inputs..." << std::endl;
    }
  } else {
    isInputAcceleration = isInputUp;
    isInputBreak = isInputDown;
    isInputBack = isInputLeft;
    isInputForward = isInputRight;
  }
  
  updateBikePhysics();
  
  int physicsResult = runPhysicsLoop(wheelBase);
  
  if (physicsResult != 5 && !isGroundColliding) {
    if (isCrashed) {
      return 3;
    } else if (isTrackStarted()) {
      trackStarted = false;
      return 4;
    } else {
      return physicsResult;
    }
  } else {
    return 5; // Crash/Fail
  }
}

bool GamePhysics::isTrackStarted() {
  return motorcycleParts[1]->motoComponents[primaryStateIndex]->xF16 <
         levelLoader->getStartFlagX();
}

bool GamePhysics::isTrackFinished() {
  return motorcycleParts[1]->motoComponents[secondaryStateIndex]->xF16 >
             levelLoader->getFinishFlagX() ||
         motorcycleParts[2]->motoComponents[secondaryStateIndex]->xF16 >
             levelLoader->getFinishFlagX();
}

int GamePhysics::runPhysicsLoop(int maxStep) {
  bool wasInAir = inAir;
  int currentStep = 0;
  int targetStep = maxStep;
  int failsafeCounter = 0;

  // Бесконечный цикл с goto (legacy, но работает как бинарный поиск времени столкновения)
restart:
  if (++failsafeCounter > 2000) {
    return 5; // Аварийный выход
  }

  int collisionState;
  while (currentStep < maxStep) {
    // Интегрируем физику на шаг
    updateComponents(targetStep - currentStep);
    
    // Проверка столкновения
    if (!wasInAir && !inAir && isTrackFinished()) {
      collisionState = 3; // Финиш
    } else {
      collisionState = updateLevelCollision(secondaryStateIndex);
    }

    // Если взлетели
    if (!wasInAir && inAir) {
      if (collisionState != 3) {
        return 2;
      }
      return 1;
    }

    // Если нет столкновения
    if (collisionState == 0) {
      targetStep = (currentStep + targetStep) >> 1; // Уменьшаем шаг
      goto restart;
    }

    // Обработка результата
    if (collisionState == 3) {
      inAir = true;
      targetStep = (currentStep + targetStep) >> 1;
    } else {
      // Столкновение колеса
      int wheelCollisionResult;
      if (collisionState == 1) {
        do {
          updateWheelPhysics(secondaryStateIndex);
          wheelCollisionResult = updateLevelCollision(secondaryStateIndex);
          if (wheelCollisionResult == 0) {
            return 5;
          }
        } while (wheelCollisionResult != 2);
      }

      // Фиксируем шаг и меняем буферы
      currentStep = targetStep;
      targetStep = maxStep;
      primaryStateIndex = primaryStateIndex == 1 ? 0 : 1;
      secondaryStateIndex = secondaryStateIndex == 1 ? 0 : 1;
    }
  }

  return 0;
}

// --- Применение сил (Helper Functions) ---

// Проверка: находится ли байк в зоне петли (для отключения гравитации)
bool GamePhysics::checkLoopZone(int componentIndex, int& bikeVel) {
  int loopCenterXF16 = 2200 << 1;
  int loopRadiusF16 = 500 << 1; 
  bool inLoopZone = false;
  bikeVel = 0;

  if (motorcycleParts.size() > 1 && motorcycleParts[1]) {
    int bikeX = motorcycleParts[1]->motoComponents[componentIndex]->xF16;
    bikeVel = calcVectorLengthF16(
        motorcycleParts[1]->motoComponents[componentIndex]->velX,
        motorcycleParts[1]->motoComponents[componentIndex]->velY);
    int dxLoop = bikeX - loopCenterXF16;
    int absDxLoop = dxLoop < 0 ? -dxLoop : dxLoop;
    inLoopZone = absDxLoop < loopRadiusF16;
  }
  return inLoopZone;
}

void GamePhysics::applyGravity(int componentIndex, bool inLoopZone, int bikeVel) {
  // Отключаем гравитацию только если скорость достаточна, чтобы "прилипнуть"
  bool fastEnough = bikeVel > 200000;
  int effectiveGravity = (inLoopZone && fastEnough) ? 0 : gravityF16;

  for (int i = 0; i < 6; ++i) {
    BikePart *part = motorcycleParts[i].get();
    MotoComponent *component = part->motoComponents[componentIndex].get();
    
    // Сброс сил
    component->forceX = 0;
    component->forceY = 0;
    component->torque = 0;
    
    // Применение гравитации
    component->forceY -= (int)(((int64_t)effectiveGravity << 32) /
                                   (int64_t)part->suspensionStrength >>
                               16);
  }
}

void GamePhysics::applyInternalConstraints(int componentIndex) {
    if (!isCrashed) {
    applySpringConstraint(motorcycleParts[0].get(), springComponents[1].get(),
                          motorcycleParts[2].get(), componentIndex, 65536);
    applySpringConstraint(motorcycleParts[0].get(), springComponents[0].get(),
                          motorcycleParts[1].get(), componentIndex, 65536);
    applySpringConstraint(motorcycleParts[2].get(), springComponents[6].get(),
                          motorcycleParts[4].get(), componentIndex, 131072);
    applySpringConstraint(motorcycleParts[1].get(), springComponents[5].get(),
                          motorcycleParts[3].get(), componentIndex, 131072);
  }

  applySpringConstraint(motorcycleParts[0].get(), springComponents[2].get(),
                        motorcycleParts[3].get(), componentIndex, 65536);
  applySpringConstraint(motorcycleParts[0].get(), springComponents[3].get(),
                        motorcycleParts[4].get(), componentIndex, 65536);
  applySpringConstraint(motorcycleParts[3].get(), springComponents[4].get(),
                        motorcycleParts[4].get(), componentIndex, 65536);
  applySpringConstraint(motorcycleParts[5].get(), springComponents[8].get(),
                        motorcycleParts[3].get(), componentIndex, 65536);
  applySpringConstraint(motorcycleParts[5].get(), springComponents[7].get(),
                        motorcycleParts[4].get(), componentIndex, 65536);
  applySpringConstraint(motorcycleParts[5].get(), springComponents[9].get(),
                        motorcycleParts[0].get(), componentIndex, 65536);
}

void GamePhysics::applyEngineTorque(int componentIndex) {
  MotoComponent *rearWheel =
      motorcycleParts[2]->motoComponents[componentIndex].get();
      
  engineTorqueF16 =
      (int)((int64_t)engineTorqueF16 * (int64_t)(65536 - frictionCoeff) >> 16);
      
  rearWheel->torque = engineTorqueF16;
  
  if (rearWheel->angularVelocity > maxAngularVelocity) {
    rearWheel->angularVelocity = maxAngularVelocity;
  }

  if (rearWheel->angularVelocity < -maxAngularVelocity) {
    rearWheel->angularVelocity = -maxAngularVelocity;
  }
}

void GamePhysics::applyAerodynamics(int componentIndex, bool inLoopZone) {
  int avgVelX = 0;
  int avgVelY = 0;

  for (int i = 0; i < 6; ++i) {
    avgVelX += motorcycleParts[i]->motoComponents[componentIndex]->velX;
    avgVelY += motorcycleParts[i]->motoComponents[componentIndex]->velY;
  }

  avgVelX = (int)(((int64_t)avgVelX << 32) / 393216L >> 16);
  avgVelY = (int)(((int64_t)avgVelY << 32) / 393216L >> 16);
  int relativeSpeed = 0;

  for (int i = 0; i < 6; ++i) {
    int dx = motorcycleParts[i]->motoComponents[componentIndex]->velX - avgVelX;
    int dy = motorcycleParts[i]->motoComponents[componentIndex]->velY - avgVelY;
    relativeSpeed = calcVectorLengthF16(dx, dy);
    
    // Отключаем сопротивление воздуха в петле, чтобы не тормозить
    if (!inLoopZone && relativeSpeed > 1700000) {
      int normX = (int)(((int64_t)dx << 32) / (int64_t)relativeSpeed >> 16);
      int normY = (int)(((int64_t)dy << 32) / (int64_t)relativeSpeed >> 16);
      motorcycleParts[i]->motoComponents[componentIndex]->velX -= normX;
      motorcycleParts[i]->motoComponents[componentIndex]->velY -= normY;
    }
  }

  // Расчет баланса колес (для контроля полета)
  int wheelYSign =
      motorcycleParts[2]->motoComponents[componentIndex]->yF16 -
                  motorcycleParts[0]->motoComponents[componentIndex]->yF16 >= 0 ? 1 : -1;
  int wheelXSign =
      motorcycleParts[2]->motoComponents[componentIndex]->velX -
                  motorcycleParts[0]->motoComponents[componentIndex]->velX >= 0 ? 1 : -1;
  wheelBalance = (wheelYSign * wheelXSign > 0) ? relativeSpeed : -relativeSpeed;
}

void GamePhysics::applyForces(int componentIndex) {
    int bikeVel = 0;
    bool inLoopZone = checkLoopZone(componentIndex, bikeVel);
    
    applyGravity(componentIndex, inLoopZone, bikeVel);
    applyInternalConstraints(componentIndex);
    applyEngineTorque(componentIndex);
    applyAerodynamics(componentIndex, inLoopZone);
}

// --- Утилиты ---

int GamePhysics::calcVectorLengthF16(int xF16, int yF16) {
  if (xF16 == 0 && yF16 == 0)
    return 0;
  double length = sqrt(pow((double)xF16, 2) + pow((double)yF16, 2));
  return (int)length;
}

void GamePhysics::applySpringConstraint(BikePart *partA, MotoComponent *spring,
                                        BikePart *partB, int componentIndex,
                                        int stiffnessF16) {
  MotoComponent *compA = partA->motoComponents[componentIndex].get();
  MotoComponent *compB = partB->motoComponents[componentIndex].get();
  int deltaX = compA->xF16 - compB->xF16;
  int deltaY = compA->yF16 - compB->yF16;
  int distanceF16;
  if (((distanceF16 = calcVectorLengthF16(deltaX, deltaY)) < 0
           ? -distanceF16
           : distanceF16) >= 3) {
    deltaX = (int)(((int64_t)deltaX << 32) / (int64_t)distanceF16 >> 16);
    deltaY = (int)(((int64_t)deltaY << 32) / (int64_t)distanceF16 >> 16);
    int stretchF16 = distanceF16 - spring->yF16;
    int forceX =
        (int)((int64_t)deltaX *
                  (int64_t)((int)((int64_t)stretchF16 * (int64_t)spring->xF16 >>
                                  16)) >>
              16);
    int forceY =
        (int)((int64_t)deltaY *
                  (int64_t)((int)((int64_t)stretchF16 * (int64_t)spring->xF16 >>
                                  16)) >>
              16);
    int velDiffX = compA->velX - compB->velX;
    int velDiffY = compA->velY - compB->velY;
    int damping =
        (int)((int64_t)((int)((int64_t)deltaX * (int64_t)velDiffX >> 16) +
                        (int)((int64_t)deltaY * (int64_t)velDiffY >> 16)) *
                  (int64_t)spring->angleF16 >>
              16);
    forceX += (int)((int64_t)deltaX * (int64_t)damping >> 16);
    forceY += (int)((int64_t)deltaY * (int64_t)damping >> 16);
    forceX = (int)((int64_t)forceX * (int64_t)stiffnessF16 >> 16);
    forceY = (int)((int64_t)forceY * (int64_t)stiffnessF16 >> 16);
    compA->forceX -= forceX;
    compA->forceY -= forceY;
    compB->forceX += forceX;
    compB->forceY += forceY;
  }
}

void GamePhysics::blendComponentState(int dstIndex, int srcIndex,
                                      int scaleF16) {
  for (int i = 0; i < 6; ++i) {
    MotoComponent *src = motorcycleParts[i]->motoComponents[dstIndex].get();
    MotoComponent *dst = motorcycleParts[i]->motoComponents[srcIndex].get();
    dst->xF16 = (int)((int64_t)src->velX * (int64_t)scaleF16 >> 16);
    dst->yF16 = (int)((int64_t)src->velY * (int64_t)scaleF16 >> 16);
    int scaledSuspension =
        (int)((int64_t)scaleF16 *
                  (int64_t)motorcycleParts[i]->suspensionStrength >>
              16);
    dst->velX = (int)((int64_t)src->forceX * (int64_t)scaledSuspension >> 16);
    dst->velY = (int)((int64_t)src->forceY * (int64_t)scaledSuspension >> 16);
  }
}

void GamePhysics::combineComponentState(int targetIndex, int baseIndex,
                                        int deltaIndex) {
  for (int i = 0; i < 6; ++i) {
    MotoComponent *target =
        motorcycleParts[i]->motoComponents[targetIndex].get();
    MotoComponent *base = motorcycleParts[i]->motoComponents[baseIndex].get();
    MotoComponent *delta = motorcycleParts[i]->motoComponents[deltaIndex].get();
    target->xF16 = base->xF16 + (delta->xF16 >> 1);
    target->yF16 = base->yF16 + (delta->yF16 >> 1);
    target->velX = base->velX + (delta->velX >> 1);
    target->velY = base->velY + (delta->velY >> 1);
  }
}

void GamePhysics::updateComponents(int deltaF16) {
  applyForces(primaryStateIndex);
  blendComponentState(primaryStateIndex, 2, deltaF16);
  combineComponentState(4, primaryStateIndex, 2);
  applyForces(4);
  blendComponentState(4, 3, deltaF16 >> 1);
  combineComponentState(4, primaryStateIndex, 3);
  combineComponentState(secondaryStateIndex, primaryStateIndex, 2);
  combineComponentState(secondaryStateIndex, secondaryStateIndex, 3);

  for (int i = 1; i <= 2; ++i) {
    MotoComponent *wheelBase =
        motorcycleParts[i]->motoComponents[primaryStateIndex].get();
    MotoComponent *wheelInterp =
        motorcycleParts[i]->motoComponents[secondaryStateIndex].get();
    wheelInterp->angleF16 =
        wheelBase->angleF16 +
        (int)((int64_t)deltaF16 * (int64_t)wheelBase->angularVelocity >> 16);
    wheelInterp->angularVelocity =
        wheelBase->angularVelocity +
        (int)((int64_t)deltaF16 *
                  (int64_t)((int)((int64_t)motorcycleParts[i]->angleOffsetF16 *
                                      (int64_t)wheelBase->torque >>
                                  16)) >>
              16);
  }

  enforceMinimumWheelBase(primaryStateIndex);
  enforceMinimumWheelBase(secondaryStateIndex);

  // Притяжение к трассе в зоне петли для прохождения петли
  if (!isCrashed && levelLoader && levelLoader->gameLevel) {
    int loopCenterXF16 = 2200 << 16;
    int loopRadius = 300;
    int loopRadiusF16 = loopRadius << 16;
    const int loopStartXF16 = loopCenterXF16 - loopRadiusF16 - (50 << 16);
    const int loopEndXF16 = loopCenterXF16 + loopRadiusF16 + (50 << 16);
    const int loopExitXF16 = loopCenterXF16 + (loopRadius << 16) +
                             (100 << 16); // Точка выхода из петли

    for (int i = 1; i <= 2; ++i) {
      MotoComponent *wheel =
          motorcycleParts[i]->motoComponents[primaryStateIndex].get();
      int bikeX = wheel->xF16;

      // Притяжение работает только в зоне петли и до точки выхода
      bool inLoopZone = (bikeX >= loopStartXF16 && bikeX <= loopEndXF16);
      bool pastExit = (bikeX > loopExitXF16);

      // Отключаем притяжение после выхода из петли
      if (inLoopZone && !pastExit) {
        // В зоне петли - очень сильное притяжение к трассе
        int wheelX = bikeX >> 1;
        if (wheelX >= 0 &&
            wheelX <
                10000) { // Проверка                    // Передаем Y подсказку
          int trackY = (levelLoader->gameLevel->getTrackHeightAt(
                           wheelX, wheel->yF16 >> 1))
                       << 1;
          int wheelRadius = const175_1_half[0];
          int targetY = trackY + wheelRadius;

          int distanceToTrack = targetY - wheel->yF16;
          // Притягиваем только если колесо выше трассы и не слишком далеко
          if (distanceToTrack > 0 && distanceToTrack < 300000) {
            // Сильное притяжение к трассе в петле
            int pullStrength = 57344; // 0.875 в F16 - сильное притяжение
            int pullY =
                (int)((int64_t)distanceToTrack * (int64_t)pullStrength >> 16);
            wheel->yF16 += pullY;
            // Убираем скорость падения в петле
            if (wheel->velY < 0) {
              wheel->velY =
                  (int)((int64_t)wheel->velY * 8192L >> 16); // уменьшаем
            }
            // Добавляем скорость в направлении трассы
            if (distanceToTrack > 65536) {
              wheel->velY += 24576; // Притяжение вверх
            }
          }
        }
      }
    }
  }
}

void GamePhysics::enforceMinimumWheelBase(int stateIndex) {
  if (initialWheelSeparationF16 <= 0) {
    return;
  }

  MotoComponent *front = motorcycleParts[1]->motoComponents[stateIndex].get();
  MotoComponent *rear = motorcycleParts[2]->motoComponents[stateIndex].get();
  int dx = front->xF16 - rear->xF16;
  int dy = front->yF16 - rear->yF16;
  int distF16 = calcVectorLengthF16(dx, dy);
  if (distF16 == 0) {
    return;
  }

  int minAllowed = (int)((int64_t)initialWheelSeparationF16 * 99 / 100); // 99% базы
  if (distF16 >= minAllowed) {
    return;
  }

  // Раздвигаем колёса
  int correction = minAllowed - distF16;
  int normX = (int)(((int64_t)dx << 32) / distF16 >> 16);
  int normY = (int)(((int64_t)dy << 32) / distF16 >> 16);
  int corrX = (int)((int64_t)normX * correction >> 16);
  int corrY = (int)((int64_t)normY * correction >> 16);

  front->xF16 += corrX;
  front->yF16 += corrY;
  rear->xF16 -= corrX;
  rear->yF16 -= corrY;

  front->velX += corrX;
  front->velY += corrY;
  rear->velX -= corrX;
  rear->velY -= corrY;
  engineTorqueF16 = 0; 
}

int GamePhysics::updateLevelCollision(int componentIndex) {
  int8_t collisionType = 2;
  int maxWheelX =
      std::max({motorcycleParts[1]->motoComponents[componentIndex]->xF16,
                motorcycleParts[2]->motoComponents[componentIndex]->xF16,
                motorcycleParts[5]->motoComponents[componentIndex]->xF16});
  int minWheelX =
      std::min({motorcycleParts[1]->motoComponents[componentIndex]->xF16,
                motorcycleParts[2]->motoComponents[componentIndex]->xF16,
                motorcycleParts[5]->motoComponents[componentIndex]->xF16});
  levelLoader->updateVisibleRange(
      minWheelX - const175_1_half[0], maxWheelX + const175_1_half[0],
      motorcycleParts[5]->motoComponents[componentIndex]->yF16);
  int axleDX = motorcycleParts[1]->motoComponents[componentIndex]->xF16 -
               motorcycleParts[2]->motoComponents[componentIndex]->xF16;
  int axleDY = motorcycleParts[1]->motoComponents[componentIndex]->yF16 -
               motorcycleParts[2]->motoComponents[componentIndex]->yF16;
  int axleLen = calcVectorLengthF16(axleDX, axleDY);
  
  if (axleLen == 0) {
    axleDX = 65536; 
    axleDY = 0;
    axleLen = 65536;
  } else {
    axleDX = (int)(((int64_t)axleDX << 32) / (int64_t)axleLen >> 16);
    axleDY = (int)(((int64_t)axleDY << 32) / (int64_t)axleLen >> 16);
  }
  int axleNormalX = -axleDY;
  int axleNormalY = axleDX;

  for (int partIdx = 0; partIdx < 6; ++partIdx) {
    if (partIdx != 4 && partIdx != 3) {
      MotoComponent *component =
          motorcycleParts[partIdx]->motoComponents[componentIndex].get();
      if (partIdx == 0) {
        component->xF16 += (int)((int64_t)axleNormalX * 65536L >> 16);
        component->yF16 += (int)((int64_t)axleNormalY * 65536L >> 16);
      }

      int collisionResult = levelLoader->detectCollision(
          component, motorcycleParts[partIdx]->connectionType);
      if (partIdx == 0) {
        component->xF16 -= (int)((int64_t)axleNormalX * 65536L >> 16);
        component->yF16 -= (int)((int64_t)axleNormalY * 65536L >> 16);
      }

      collisionNormalX = levelLoader->collisionNormalX;
      collisionNormalY = levelLoader->collisionNormalY;

      // Смерть только при видимом соприкосновении головы (часть 5) с поверхностью
      if (partIdx == 5 && collisionResult != 2) {
        isGroundColliding = true;

        int headX = component->xF16 >> 1;
        int visibleStartX = levelLoader->visibleStartX;
        int visibleEndX = levelLoader->visibleEndX;

        bool headVisible =
            (headX >= visibleStartX - 200 && headX <= visibleEndX + 200);

        int wheel1Y = motorcycleParts[1]->motoComponents[componentIndex]->yF16;
        int wheel2Y = motorcycleParts[2]->motoComponents[componentIndex]->yF16;
        int avgWheelY = (wheel1Y + wheel2Y) >> 1;

        bool isInverted = component->yF16 < (avgWheelY - 65536);

        if (collisionResult == 0 && headVisible && isInverted) {
          LOG_PHYS("CRASH DETECTED! HeadY=%d AvgWheelY=%d", component->yF16, avgWheelY);
          isCrashed = true;
          return 2; // Allow overlap to trigger crash
        }
      }

      if (partIdx == 1 && collisionResult != 2) {
        trackStarted = true;
      }

      if (collisionResult == 1) {
        activePartIndex = partIdx;
        collisionType = 1;
      } else if (collisionResult == 0) {
        activePartIndex = partIdx;
        collisionType = 0;
        if (partIdx != 5) {
          break;
        }
      }
    }
  }

  return collisionType;
}

void GamePhysics::updateWheelPhysics(int componentIndex) {
  BikePart *part = motorcycleParts[activePartIndex].get();
  MotoComponent *component = part->motoComponents[componentIndex].get();
  component->xF16 += (int)((int64_t)collisionNormalX * 3276L >> 16);
  component->yF16 += (int)((int64_t)collisionNormalY * 3276L >> 16);

  int suspensionFrontLocal;
  int suspensionBackLocal;
  int forkLengthLocal;
  int torqueFrontLocal;
  int torqueBackLocal;
  
  if (isInputBreak && (activePartIndex == 2 || activePartIndex == 1) &&
      component->angularVelocity < 6553) {
    suspensionFrontLocal = suspensionFront - brakeForce;
    suspensionBackLocal = 13107;
    forkLengthLocal = 39321;
    torqueFrontLocal = 26214 - brakeForce;
    torqueBackLocal = 26214 - brakeForce;
  } else {
    suspensionFrontLocal = suspensionFront;
    suspensionBackLocal = suspensionBack;
    forkLengthLocal = forkLength;
    torqueFrontLocal = torqueFront;
    torqueBackLocal = torqueRear;
  }

  int normalLen = calcVectorLengthF16(collisionNormalX, collisionNormalY);
  if (normalLen == 0) {
    collisionNormalX = 0;
    collisionNormalY = 65536; 
  } else {
    collisionNormalX = (int)(((int64_t)collisionNormalX << 32) / (int64_t)normalLen >> 16);
    collisionNormalY = (int)(((int64_t)collisionNormalY << 32) / (int64_t)normalLen >> 16);
  }
  int velX = component->velX;
  int velY = component->velY;
  int normalVel = -((int)((int64_t)velX * (int64_t)collisionNormalX >> 16) +
                    (int)((int64_t)velY * (int64_t)collisionNormalY >> 16));
  int tangentVel = -((int)((int64_t)velX * (int64_t)(-collisionNormalY) >> 16) +
                     (int)((int64_t)velY * (int64_t)collisionNormalX >> 16));
  int newAngularVel =
      (int)((int64_t)suspensionFrontLocal *
                (int64_t)component->angularVelocity >>
            16) -
      (int)((int64_t)suspensionBackLocal *
                (int64_t)((int)(((int64_t)tangentVel << 32) /
                                    (int64_t)part->connectionLengthF16 >>
                                16)) >>
            16);
  int newTangentVel =
      (int)((int64_t)torqueFrontLocal * (int64_t)tangentVel >> 16) -
      (int)((int64_t)forkLengthLocal *
                (int64_t)((int)((int64_t)component->angularVelocity *
                                    (int64_t)part->connectionLengthF16 >>
                                16)) >>
            16);
  int normalPush = -((int)((int64_t)torqueBackLocal * (int64_t)normalVel >> 16));
  int deltaVelX =
      (int)((int64_t)(-newTangentVel) * (int64_t)(-collisionNormalY) >> 16);
  int deltaVelY =
      (int)((int64_t)(-newTangentVel) * (int64_t)collisionNormalX >> 16);
  int deltaNormalX =
      (int)((int64_t)(-normalPush) * (int64_t)collisionNormalX >> 16);
  int deltaNormalY =
      (int)((int64_t)(-normalPush) * (int64_t)collisionNormalY >> 16);
  component->angularVelocity = newAngularVel;
  component->velX = deltaVelX + deltaNormalX;
  component->velY = deltaVelY + deltaNormalY;

  enforceMinimumWheelBase(componentIndex);
}

void GamePhysics::setEnableLookAhead(bool value) { isEnableLookAhead = value; }

void GamePhysics::setMinimalScreenWH(int minWH) {
  cameraShiftLimit =
      (int)(((int64_t)((int)(655360L * (int64_t)(minWH << 16) >> 16)) << 32) /
                8388608L >>
            16);
}

int GamePhysics::getCamPosX() {
  if (isEnableLookAhead) {
    camShiftX =
        (int)(((int64_t)motoComponents[0]->velX << 32) / 1572864L >> 16) +
        (int)((int64_t)camShiftX * 57344L >> 16);
  } else {
    camShiftX = 0;
  }

  camShiftX = camShiftX < cameraShiftLimit ? camShiftX : cameraShiftLimit;
  camShiftX = camShiftX < -cameraShiftLimit ? -cameraShiftLimit : camShiftX;
  return (motoComponents[0]->xF16 + camShiftX) << 2 >> 16;
}

int GamePhysics::getCamPosY() {
  if (isEnableLookAhead) {
    camShiftY =
        (int)(((int64_t)motoComponents[0]->velY << 32) / 1572864L >> 16) +
        (int)((int64_t)camShiftY * 57344L >> 16);
  } else {
    camShiftY = 0;
  }

  camShiftY = camShiftY < cameraShiftLimit ? camShiftY : cameraShiftLimit;
  camShiftY = camShiftY < -cameraShiftLimit ? -cameraShiftLimit : camShiftY;
  return (motoComponents[0]->yF16 + camShiftY) << 2 >> 16;
}

int GamePhysics::getGroundHeight() {
  int frontBackX = motoComponents[1]->xF16 < motoComponents[2]->xF16
                       ? motoComponents[2]->xF16
                       : motoComponents[1]->xF16;
  return isCrashed ? levelLoader->getProgressAt(motoComponents[0]->xF16)
                   : levelLoader->getProgressAt(frontBackX);
}

void GamePhysics::snapshotMotoState() {
  for (int var2 = 0; var2 < 6; ++var2) {
    motorcycleParts[var2]->motoComponents[5]->xF16 =
        motorcycleParts[var2]->motoComponents[primaryStateIndex]->xF16;
    motorcycleParts[var2]->motoComponents[5]->yF16 =
        motorcycleParts[var2]->motoComponents[primaryStateIndex]->yF16;
    motorcycleParts[var2]->motoComponents[5]->angleF16 =
        motorcycleParts[var2]->motoComponents[primaryStateIndex]->angleF16;
  }

  motorcycleParts[0]->motoComponents[5]->velX =
      motorcycleParts[0]->motoComponents[primaryStateIndex]->velX;
  motorcycleParts[0]->motoComponents[5]->velY =
      motorcycleParts[0]->motoComponents[primaryStateIndex]->velY;
  motorcycleParts[2]->motoComponents[5]->angularVelocity =
      motorcycleParts[2]->motoComponents[primaryStateIndex]->angularVelocity;
  // }
}

void GamePhysics::setMotoComponents() {
  for (int i = 0; i < 6; ++i) {
    motoComponents[i]->xF16 = motorcycleParts[i]->motoComponents[5]->xF16;
    motoComponents[i]->yF16 = motorcycleParts[i]->motoComponents[5]->yF16;
    motoComponents[i]->angleF16 =
        motorcycleParts[i]->motoComponents[5]->angleF16;
  }

  motoComponents[0]->velX = motorcycleParts[0]->motoComponents[5]->velX;
  motoComponents[0]->velY = motorcycleParts[0]->motoComponents[5]->velY;
  motoComponents[2]->angularVelocity =
      motorcycleParts[2]->motoComponents[5]->angularVelocity;
}

// --- Отрисовка (Rendering) ---

void GamePhysics::renderMotoFork(GameCanvas *canvas) {
  canvas->setColor(128, 128, 128);
  canvas->drawLineF16(motoComponents[3]->xF16, motoComponents[3]->yF16,
                      motoComponents[1]->xF16, motoComponents[1]->yF16);
}

void GamePhysics::renderRiderSkeleton(GameCanvas *gameCanvas, int dirXF16,
                                      int dirYF16, int rotXF16, int rotYF16) {
  int var7;
  int var8 = motoComponents[0]->xF16;
  int var9 = motoComponents[0]->yF16;

  int xF16 = 0, yF16 = 0, x2F16 = 0, y2F16 = 0, x3F16 = 0, y3F16 = 0, x4F16 = 0,
      y4F16 = 0;
  int x5F16 = 0, y5F16 = 0, x6F16 = 0, y6F16 = 0, circleXF16 = 0,
      circleYF16 = 0;

  std::vector<std::vector<int>> baseCoords;
  std::vector<std::vector<int>> targetCoords;

  if (tiltAngleF16 < 32768) {
    baseCoords = hardcodedArr5;
    targetCoords = hardcodedArr4;
    var7 = (int)((int64_t)tiltAngleF16 * 131072L >> 16);
  } else {
    baseCoords = hardcodedArr4;
    targetCoords = hardcodedArr6;
    var7 = (int)((int64_t)(tiltAngleF16 - 32768) * 131072L >> 16);
  }

  for (std::size_t i = 0; i < baseCoords.size(); ++i) {
    int finalX =
        (int)((int64_t)baseCoords[i][0] * (int64_t)(65536 - var7) >> 16) +
        (int)((int64_t)targetCoords[i][0] * (int64_t)var7 >> 16);
    int finalY =
        (int)((int64_t)baseCoords[i][1] * (int64_t)(65536 - var7) >> 16) +
        (int)((int64_t)targetCoords[i][1] * (int64_t)var7 >> 16);

    int rotatedX = var8 + (int)((int64_t)rotXF16 * (int64_t)finalX >> 16) +
                   (int)((int64_t)dirXF16 * (int64_t)finalY >> 16);
    int rotatedY = var9 + (int)((int64_t)rotYF16 * (int64_t)finalX >> 16) +
                   (int)((int64_t)dirYF16 * (int64_t)finalY >> 16);

    switch (i) {
    case 0:
      x2F16 = rotatedX;
      y2F16 = rotatedY;
      break;
    case 1:
      x3F16 = rotatedX;
      y3F16 = rotatedY;
      break;
    case 2:
      x4F16 = rotatedX;
      y4F16 = rotatedY;
      break;
    case 3:
      circleXF16 = rotatedX;
      circleYF16 = rotatedY;
      break;
    case 4:
      x5F16 = rotatedX;
      y5F16 = rotatedY;
      break;
    case 5:
      xF16 = rotatedX;
      yF16 = rotatedY;
      break;
    case 7:
      x6F16 = rotatedX;
      y6F16 = rotatedY;
      break;
    }
  }

  gameCanvas->setColor(0, 0, 0);
  gameCanvas->drawLineF16(xF16, yF16, x2F16, y2F16);
  gameCanvas->drawLineF16(x2F16, y2F16, x3F16, y3F16);
  gameCanvas->setColor(0, 0, 128);
  gameCanvas->drawLineF16(x3F16, y3F16, x4F16, y4F16);
  gameCanvas->drawLineF16(x4F16, y4F16, x5F16, y5F16);
  gameCanvas->drawLineF16(x5F16, y5F16, x6F16, y6F16);

  int head_radius = 65536;
  gameCanvas->setColor(156, 0, 0);
  gameCanvas->drawCircle(circleXF16 << 2 >> 16, circleYF16 << 2 >> 16,
                         (head_radius + head_radius) << 2 >> 16);
}

void GamePhysics::renderBikeWireframe(GameCanvas *gameCanvas, int dirXF16,
                                      int dirYF16, int rotXF16, int rotYF16) {
  int var7 = motoComponents[2]->xF16;
  int var8 = motoComponents[2]->yF16;
  int var9 = var7 + (int)((int64_t)rotXF16 * (int64_t)32768 >> 16);
  int var10 = var8 + (int)((int64_t)rotYF16 * (int64_t)32768 >> 16);
  int var11 = var7 - (int)((int64_t)rotXF16 * (int64_t)32768 >> 16);
  int var12 = var8 - (int)((int64_t)rotYF16 * (int64_t)32768 >> 16);
  int var13 = motoComponents[0]->xF16 + (int)((int64_t)dirXF16 * 32768L >> 16);
  int var14 = motoComponents[0]->yF16 + (int)((int64_t)dirYF16 * 32768L >> 16);
  int var15 = var13 - (int)((int64_t)dirXF16 * 131072L >> 16);
  int var16 = var14 - (int)((int64_t)dirYF16 * 131072L >> 16);
  int var17 = var15 + (int)((int64_t)rotXF16 * 65536L >> 16);
  int var18 = var16 + (int)((int64_t)rotYF16 * 65536L >> 16);
  int var19 = var15 + (int)((int64_t)dirXF16 * 49152L >> 16) +
              (int)((int64_t)rotXF16 * 49152L >> 16);
  int var20 = var16 + (int)((int64_t)dirYF16 * 49152L >> 16) +
              (int)((int64_t)rotYF16 * 49152L >> 16);
  int var21 = var15 + (int)((int64_t)rotXF16 * 32768L >> 16);
  int var22 = var16 + (int)((int64_t)rotYF16 * 32768L >> 16);
  int var23 = motoComponents[1]->xF16;
  int var24 = motoComponents[1]->yF16;
  int var25 = motoComponents[4]->xF16 - (int)((int64_t)dirXF16 * 49152L >> 16);
  int var26 = motoComponents[4]->yF16 - (int)((int64_t)dirYF16 * 49152L >> 16);
  int var27 = var25 - (int)((int64_t)rotXF16 * 32768L >> 16);
  int var28 = var26 - (int)((int64_t)rotYF16 * 32768L >> 16);
  int var29 = var25 - (int)((int64_t)dirXF16 * 131072L >> 16) +
              (int)((int64_t)rotXF16 * 16384L >> 16);
  int var30 = var26 - (int)((int64_t)dirYF16 * 131072L >> 16) +
              (int)((int64_t)rotYF16 * 16384L >> 16);
  int var31 = motoComponents[3]->xF16;
  int var32 = motoComponents[3]->yF16;
  int var33 = var31 + (int)((int64_t)rotXF16 * 32768L >> 16);
  int var34 = var32 + (int)((int64_t)rotYF16 * 32768L >> 16);
  int var35 = var31 + (int)((int64_t)rotXF16 * 114688L >> 16) -
              (int)((int64_t)dirXF16 * 32768L >> 16);
  int var36 = var32 + (int)((int64_t)rotYF16 * 114688L >> 16) -
              (int)((int64_t)dirYF16 * 32768L >> 16);
  gameCanvas->setColor(50, 50, 50);
  gameCanvas->drawCircle(var21 << 2 >> 16, var22 << 2 >> 16,
                         (32768 + 32768) << 2 >> 16);
  if (!isCrashed) {
    gameCanvas->drawLineF16(var9, var10, var17, var18);
    gameCanvas->drawLineF16(var11, var12, var15, var16);
  }

  gameCanvas->drawLineF16(var13, var14, var15, var16);
  gameCanvas->drawLineF16(var13, var14, var31, var32);
  gameCanvas->drawLineF16(var19, var20, var33, var34);
  gameCanvas->drawLineF16(var33, var34, var35, var36);
  if (!isCrashed) {
    gameCanvas->drawLineF16(var31, var32, var23, var24);
    gameCanvas->drawLineF16(var35, var36, var23, var24);
  }

  gameCanvas->drawLineF16(var17, var18, var27, var28);
  gameCanvas->drawLineF16(var19, var20, var25, var26);
  gameCanvas->drawLineF16(var25, var26, var29, var30);
  gameCanvas->drawLineF16(var27, var28, var29, var30);
}

void GamePhysics::renderGame(GameCanvas *gameCanvas) {
  gameCanvas->drawSkyGradient();
  if (isCrashed) {
    int x1 = motoComponents[3]->xF16;
    int x2 = motoComponents[4]->xF16;
    if (x2 < x1)
      std::swap(x1, x2);
    levelLoader->gameLevel->setSegmentRange(x1, x2);
  }
  int xxF16 = motoComponents[3]->xF16 - motoComponents[4]->xF16;
  int yyF16 = motoComponents[3]->yF16 - motoComponents[4]->yF16;
  int maxAbs;
  if ((maxAbs = calcVectorLengthF16(xxF16, yyF16)) != 0) {
    xxF16 = (int)(((int64_t)xxF16 << 32) / (int64_t)maxAbs >> 16);
    yyF16 = (int)(((int64_t)yyF16 << 32) / (int64_t)maxAbs >> 16);
  }
  int var5 = -yyF16;

  gameCanvas->setColor(50, 50, 50);

  gameCanvas->drawBrakeDisc(
      motoComponents[1]->xF16 << 2 >> 16, motoComponents[1]->yF16 << 2 >> 16,
      const175_1_half[0] << 2 >> 16, MathF16::atan2F16(xxF16, yyF16));

  if (!isCrashed) { 
    renderMotoFork(gameCanvas);
  }

  renderBikeWireframe(gameCanvas, xxF16, yyF16, var5, xxF16);
  renderRiderSkeleton(gameCanvas, xxF16, yyF16, var5, xxF16);

  int wheelRadius = GamePhysics::const175_1_half[0] << 2 >> 16;
  gameCanvas->drawWheel(motoComponents[2]->xF16 << 2 >> 16,
                        motoComponents[2]->yF16 << 2 >> 16, wheelRadius);
  gameCanvas->drawWheel(motoComponents[1]->xF16 << 2 >> 16,
                        motoComponents[1]->yF16 << 2 >> 16, wheelRadius);

  levelLoader->renderTrackNearestLine(gameCanvas);
}

void GamePhysics::enforceGroundCollision() {
  if (!levelLoader || !levelLoader->gameLevel) {
    return;
  }

  const int loopCenterXF16 = 2200 << 1;
  const int loopRadiusF16 = 350 << 1;
  const int loopMarginF16 = 500 << 1;

  for (int i = 1; i <= 2; ++i) {
    MotoComponent *wheel =
        motorcycleParts[i]->motoComponents[primaryStateIndex].get();
    if (!wheel)
      continue;

    int phys_wheel_x = wheel->xF16;
    int phys_wheel_y = wheel->yF16;

    int dxLoop = phys_wheel_x - loopCenterXF16;
    int absDxLoop = dxLoop < 0 ? -dxLoop : dxLoop;
    bool inLoopZone = absDxLoop < loopRadiusF16 + loopMarginF16;

    int wheel_radius = GamePhysics::const175_1_half[0];

    int trackX = phys_wheel_x >> 1;
    if (trackX < 0 || trackX > 10000) {
      continue; 
    }

    int phys_track_y =
        (levelLoader->gameLevel->getTrackHeightAt(trackX, phys_wheel_y >> 1))
        << 1;
    int target_y = phys_track_y + wheel_radius;

    if (inLoopZone) {
      int distanceToTrack = target_y - phys_wheel_y;
      if (distanceToTrack > 0) {
        int pullStrength = 65536; 
        int pullY =
            (int)((int64_t)distanceToTrack * (int64_t)pullStrength >> 16);
        wheel->yF16 += pullY;
        if (wheel->velY < 0) {
          wheel->velY = (int)((int64_t)wheel->velY * 16384L >> 16); 
        }
      } else if (distanceToTrack < -65536) {
        int pullStrength = 16384; 
        int pullY =
            (int)((int64_t)distanceToTrack * (int64_t)pullStrength >> 16);
        wheel->yF16 += pullY;
      }
    } else {
      if (phys_wheel_y < target_y) {
        wheel->yF16 = target_y;
        if (wheel->velY < 0) {
          wheel->velY = 0;
        }
      }
    }
  }
}

void GamePhysics::checkAndShiftWorld() {
    if (motorcycleParts.empty() || !motorcycleParts[0]) return;
    
    int currentX = motorcycleParts[0]->motoComponents[primaryStateIndex]->xF16;
    
    const int THRESHOLD = 1000000000; 
    const int SHIFT_AMOUNT = 499990528; 
    
    if (currentX > THRESHOLD) {
        LOG_PHYS("СДВИГ МИРА! currentX=%d Shift=%d", currentX, SHIFT_AMOUNT);
        
        for(auto& part : motorcycleParts) {
            if(!part) continue;
            for(auto& comp : part->motoComponents) {
                if(comp) comp->xF16 -= SHIFT_AMOUNT;
            }
        }
        
        int levelShift = SHIFT_AMOUNT >> 1;
        
        if (levelLoader) {
            levelLoader->shiftLevel(levelShift);
        }
        
        int generatorShift = SHIFT_AMOUNT >> 14;
        
        if (levelGenerator) {
            levelGenerator->shiftGeneration(generatorShift);
        }
    }
}

GamePhysics::~GamePhysics() = default;