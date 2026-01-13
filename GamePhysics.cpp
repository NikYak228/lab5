#include "GamePhysics.h"

#include "BikePart.h"
#include "LevelLoader.h"
#include "MathF16.h"
#include "AIController.h"
#include "LevelGenerator.h"
#include "Logger.h"
#include <algorithm>
#include <iostream>

GamePhysics::GamePhysics(LevelLoader *levelLoader) {
  LOG_PHYS("GamePhysics constructor called.");
  primaryStateIndex = 0;
  secondaryStateIndex = 1;

  for (int i = 0; i < 6; ++i) {
    motoComponents[i] = std::make_unique<MotoComponent>();
  }
  // ... (rest of constructor is implicitly preserved by logical flow, but I need to be careful with replace)
  // Since I am replacing the top of the file, I will just rewrite the constructor part I see.
  
  restartCounter = 0;
  gameMode = 0;
  useSpriteGraphics = false;
  isRenderMotoWithSprites = false;
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
  
  // Инициализация AI системы
  aiController = std::make_unique<AIController>();
  levelGenerator = std::make_unique<LevelGenerator>();
  currentLevelMode = LevelMode::MANUAL;
  aiRestartTimer = 0;
  
  resetSmth(true);
  isGenerateInputAI = false;
  snapshotMotoState();
  isCrashed = false;
}

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
    gravityF16 = 1638400;
    setMotoLeague(1);
  }
}

void GamePhysics::setMotoLeague(int league) {
  curentMotoLeague = league;
  suspensionFront = 45875;
  suspensionBack = 13107;
  forkLength = 39321;
  massFactor = 1600000;
  dampingF16 = 262144;
  frictionCoeff = 6553;
  switch (league) {
  case 0:
  default:
    motoParam1 = 18000;
    motoParam2 = 18000;
    motoParam3 = 1200000;   // GOD MODE: Safe Max Speed (No tunneling)
    motoParam4 = 200000000; // GOD MODE: Torque
    motoParam5 = 20000000;  // GOD MODE: Accel
    motoParam6 = 327;
    motoParam7 = 0;
    motoParam8 = 32768;
    motoParam9 = 327680;
    motoParam10 = 24660800; // Harder springs
    break;
  case 1:
    motoParam1 = 32768;
    motoParam2 = 32768;
    motoParam3 = 2500000;   // Еще больше увеличена максимальная скорость для
                            // прохождения петли
    motoParam4 = 120000000; // Еще больше увеличена максимальная мощность
    motoParam5 =
        8000000; // Еще больше увеличено ускорение для быстрого набора скорости
    motoParam6 = 6553;
    motoParam7 = 26214;
    motoParam8 = 26214;
    motoParam9 = 327680;
    motoParam10 = 19660800;
    break;

  case 2:
    // --- Двигатель на максимуме ---
    motoParam3 = 2200000;  // Очень высокая макс. скорость вращения
    motoParam4 = 90000000; // Огромный потолок мощности
    motoParam5 = 5500000;  // Очень резкое ускорение

    // --- Шасси и управление ---
    massFactor = 1350000; // Слегка уменьшаем массу, чтобы байк был "легче"
    motoParam8 = 45000;   // Очень высокий контроль в воздухе для трюков

    // --- Остальные параметры ---
    motoParam1 = 32768;
    motoParam2 = 32768;
    motoParam6 = 6553;
    motoParam7 = 26214;
    motoParam9 = 327680;
    motoParam10 = 21626880;
    suspensionFront = 45875;
    suspensionBack = 13107;
    forkLength = 39321;
    frictionCoeff = 6553;
    dampingF16 = 262144;
    curentMotoLeague = league;
    break;
  case 3: // Демонстрационный режим "Наглядная Подвеска"
    // --- Двигатель: тяга есть, но без мгновенного переворота ---
    motoParam3 = 1400000;  // максимум угловой скорости колеса
    motoParam4 = 90000000; // потолок крутящего момента
    motoParam5 = 2400000;  // шаг прироста момента — резво, но мягче

    // --- Шасси и Подвеска ---
    massFactor = 1850000;   // тяжелее для устойчивости
    motoParam10 = 18000000; // мягкие пружины
    dampingF16 = 340000;    // демпфер поглощает раскачку
    frictionCoeff =
        12500; // сильнее торможение двигателя — меньше chance перекрутить

    // --- Управление в воздухе ---
    motoParam8 = 26000; // ещё плавнее контроль в полёте

    // Остальные параметры
    curentMotoLeague = league;
    suspensionFront = 45875;
    suspensionBack = 13107;
    forkLength = 39321;
    frictionCoeff = 6553;
    motoParam1 = 32768;
    motoParam2 = 32768;
    motoParam6 = 6553;
    motoParam7 = 26214;
    motoParam9 = 327680;
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
  // НЕ сбрасываем isGenerateInputAI здесь - он устанавливается в setLevelMode
  // isGenerateInputAI = false; // УДАЛЕНО - сохраняем значение из setLevelMode
  atStart = false;
  cameraToggle = false;

  levelLoader->gameLevel->setSegmentRange(
      motorcycleParts[2]->motoComponents[5]->xF16 + 98304 - const175_1_half[0],
      motorcycleParts[1]->motoComponents[5]->xF16 - 98304 + const175_1_half[0]);

  // Запоминаем базовое расстояние между колесами (для контроля схлопывания
  // рамы)
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

void GamePhysics::setupBike(int startX, int startY) {
  if (motorcycleParts.empty()) {
    motorcycleParts = std::vector<std::unique_ptr<BikePart>>(6);
  }

  if (springComponents.empty()) {
    springComponents = std::vector<std::unique_ptr<MotoComponent>>(10);
  }

  int var4 = 0;
  int8_t var5 = 0;
  int var6 = 0;
  int var7 = 0;

  int i;
  for (i = 0; i < 6; ++i) {
    short var8 = 0;
    switch (i) {
    case 0:
      var5 = 1;
      var4 = 360448;
      var6 = 0;
      var7 = 0;
      break;
    case 1:
      var5 = 0;
      var4 = 98304;
      var6 = 229376;
      var7 = 0;
      break;
    case 2:
      var5 = 0;
      var4 = 360448;
      var6 = -229376;
      var7 = 0;
      var8 = 21626;
      break;
    case 3:
      var5 = 1;
      var4 = 229376;
      var6 = 131072;
      var7 = 196608;
      break;
    case 4:
      var5 = 1;
      var4 = 229376;
      var6 = -131072;
      var7 = 196608;
      break;
    case 5:
      var5 = 2;
      var4 = 294912;
      var6 = 0;
      var7 = 327680;
    }

    if (motorcycleParts[i] == nullptr) {
      motorcycleParts[i] = std::make_unique<BikePart>();
    }

    motorcycleParts[i]->reset();
    motorcycleParts[i]->connectionLengthF16 = const175_1_half[var5];
    motorcycleParts[i]->connectionType = var5;
    motorcycleParts[i]->suspensionStrength =
        (int)((int64_t)((int)(281474976710656L / (int64_t)var4 >> 16)) *
                  (int64_t)massFactor >>
              16);
    motorcycleParts[i]->motoComponents[primaryStateIndex]->xF16 = startX + var6;
    motorcycleParts[i]->motoComponents[primaryStateIndex]->yF16 =
        startY + var7 + 500000;
    motorcycleParts[i]->motoComponents[5]->xF16 = startX + var6;
    motorcycleParts[i]->motoComponents[5]->yF16 = startY + var7 + 500000;
    motorcycleParts[i]->angleOffsetF16 = var8;
  }

  for (i = 0; i < 10; ++i) {
    if (springComponents[i] == nullptr) {
      springComponents[i] = std::make_unique<MotoComponent>();
    }

    springComponents[i]->setToZeros();
    springComponents[i]->xF16 = motoParam10;
    springComponents[i]->angleF16 = dampingF16;
  }

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
  springComponents[6]->xF16 = (int)(6553L * (int64_t)motoParam10 >> 16);
  springComponents[5]->xF16 = (int)(6553L * (int64_t)motoParam10 >> 16);
  springComponents[9]->xF16 = (int)(72089L * (int64_t)motoParam10 >> 16);
  springComponents[8]->xF16 = (int)(72089L * (int64_t)motoParam10 >> 16);
  springComponents[7]->xF16 = (int)(72089L * (int64_t)motoParam10 >> 16);
}

void GamePhysics::setRenderMinMaxX(int minX, int maxX) {
  levelLoader->setMinMaxX(minX, maxX);
}

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
  // Вызываем логику AI вместо старой заглушки
  if (aiController) {
    aiController->update(this);
  } else {
    static int warnCount = 0;
    if (warnCount++ % 300 == 0) { // Логируем каждые 5 секунд
      std::cout << "[GamePhysics] WARNING: isGenerateInputAI=true but aiController is null!" << std::endl;
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
    
    resetSmth(true); // Рестарт физики байка
    
    // ВАЖНО: Восстанавливаем флаг AI после resetSmth, так как он мог быть сброшен
    isGenerateInputAI = (mode != LevelMode::MANUAL);
    
    std::cout << "[GamePhysics] After resetSmth: isGenerateInputAI=" 
              << (isGenerateInputAI ? "true" : "false") << std::endl;
    
    aiRestartTimer = 0;
}

void GamePhysics::updateBikePhysics() {
  if (!isCrashed) {
    int var1 = motorcycleParts[1]->motoComponents[primaryStateIndex]->xF16 -
               motorcycleParts[2]->motoComponents[primaryStateIndex]->xF16;
    int var2 = motorcycleParts[1]->motoComponents[primaryStateIndex]->yF16 -
               motorcycleParts[2]->motoComponents[primaryStateIndex]->yF16;
    int var3 = calcVectorLengthF16(var1, var2);

    if (var3 != 0) {
      double normalized_x = (double)var1 / var3;
      double normalized_y = (double)var2 / var3;
      var1 = (int)(normalized_x * 65536.0);
      var2 = (int)(normalized_y * 65536.0);
    }

    if (isInputAcceleration && engineTorqueF16 >= -motoParam4) {
      engineTorqueF16 -= motoParam5;
    }

    if (isInputBreak) {
      engineTorqueF16 = 0;
      motorcycleParts[1]->motoComponents[primaryStateIndex]->angularVelocity =
          (int)((int64_t)motorcycleParts[1]
                        ->motoComponents[primaryStateIndex]
                        ->angularVelocity *
                    (int64_t)(65536 - motoParam6) >>
                16);
      motorcycleParts[2]->motoComponents[primaryStateIndex]->angularVelocity =
          (int)((int64_t)motorcycleParts[2]
                        ->motoComponents[primaryStateIndex]
                        ->angularVelocity *
                    (int64_t)(65536 - motoParam6) >>
                16);
      if (motorcycleParts[1]
              ->motoComponents[primaryStateIndex]
              ->angularVelocity < 6553) {
        motorcycleParts[1]->motoComponents[primaryStateIndex]->angularVelocity =
            0;
      }

      if (motorcycleParts[2]
              ->motoComponents[primaryStateIndex]
              ->angularVelocity < 6553) {
        motorcycleParts[2]->motoComponents[primaryStateIndex]->angularVelocity =
            0;
      }
    }

    motorcycleParts[0]->suspensionStrength =
        (int)(11915L * (int64_t)massFactor >> 16);
    motorcycleParts[4]->suspensionStrength =
        (int)(18724L * (int64_t)massFactor >> 16);
    motorcycleParts[3]->suspensionStrength =
        (int)(18724L * (int64_t)massFactor >> 16);
    motorcycleParts[1]->suspensionStrength =
        (int)(43690L * (int64_t)massFactor >> 16);
    motorcycleParts[2]->suspensionStrength =
        (int)(11915L * (int64_t)massFactor >> 16);
    motorcycleParts[5]->suspensionStrength =
        (int)(14563L * (int64_t)massFactor >> 16);
    if (isInputBack) {
      motorcycleParts[0]->suspensionStrength =
          (int)(18724L * (int64_t)massFactor >> 16);
      motorcycleParts[4]->suspensionStrength =
          (int)(14563L * (int64_t)massFactor >> 16);
      motorcycleParts[3]->suspensionStrength =
          (int)(18724L * (int64_t)massFactor >> 16);
      motorcycleParts[1]->suspensionStrength =
          (int)(43690L * (int64_t)massFactor >> 16);
      motorcycleParts[2]->suspensionStrength =
          (int)(10082L * (int64_t)massFactor >> 16);
    } else if (isInputForward) {
      motorcycleParts[0]->suspensionStrength =
          (int)(18724L * (int64_t)massFactor >> 16);
      motorcycleParts[4]->suspensionStrength =
          (int)(18724L * (int64_t)massFactor >> 16);
      motorcycleParts[3]->suspensionStrength =
          (int)(14563L * (int64_t)massFactor >> 16);
      motorcycleParts[1]->suspensionStrength =
          (int)(26214L * (int64_t)massFactor >> 16);
      motorcycleParts[2]->suspensionStrength =
          (int)(11915L * (int64_t)massFactor >> 16);
    }

    if (isInputBack || isInputForward) {
      int var4 = -var2;
      MotoComponent *var10000;
      int var6;
      int var7;
      int var8;
      int var9;
      int var10;
      int var11;
      if (isInputBack && wheelBalance > -motoParam9) {
        var6 = 65536;
        if (wheelBalance < 0) {
          var6 =
              (int)(((int64_t)(motoParam9 - (wheelBalance < 0 ? -wheelBalance
                                                              : wheelBalance))
                     << 32) /
                        (int64_t)motoParam9 >>
                    16);
        }

        var7 = (int)((int64_t)motoParam8 * (int64_t)var6 >> 16);
        var8 = (int)((int64_t)var4 * (int64_t)var7 >> 16);
        var9 = (int)((int64_t)var1 * (int64_t)var7 >> 16);
        var10 = (int)((int64_t)var1 * (int64_t)var7 >> 16);
        var11 = (int)((int64_t)var2 * (int64_t)var7 >> 16);
        if (tiltAngleF16 > 32768) {
          tiltAngleF16 = tiltAngleF16 - 1638 < 0 ? 0 : tiltAngleF16 - 1638;
        } else {
          tiltAngleF16 = tiltAngleF16 - 3276 < 0 ? 0 : tiltAngleF16 - 3276;
        }

        var10000 = motorcycleParts[4]->motoComponents[primaryStateIndex].get();
        var10000->velX -= var8;
        var10000 = motorcycleParts[4]->motoComponents[primaryStateIndex].get();
        var10000->velY -= var9;
        var10000 = motorcycleParts[3]->motoComponents[primaryStateIndex].get();
        var10000->velX += var8;
        var10000 = motorcycleParts[3]->motoComponents[primaryStateIndex].get();
        var10000->velY += var9;
        var10000 = motorcycleParts[5]->motoComponents[primaryStateIndex].get();
        var10000->velX -= var10;
        var10000 = motorcycleParts[5]->motoComponents[primaryStateIndex].get();
        var10000->velY -= var11;
      }

      if (isInputForward && wheelBalance < motoParam9) {
        var6 = 65536;
        if (wheelBalance > 0) {
          var6 = (int)(((int64_t)(motoParam9 - wheelBalance) << 32) /
                           (int64_t)motoParam9 >>
                       16);
        }

        var7 = (int)((int64_t)motoParam8 * (int64_t)var6 >> 16);
        var8 = (int)((int64_t)var4 * (int64_t)var7 >> 16);
        var9 = (int)((int64_t)var1 * (int64_t)var7 >> 16);
        var10 = (int)((int64_t)var1 * (int64_t)var7 >> 16);
        var11 = (int)((int64_t)var2 * (int64_t)var7 >> 16);
        if (tiltAngleF16 > 32768) {
          tiltAngleF16 =
              tiltAngleF16 + 1638 < 65536 ? tiltAngleF16 + 1638 : 65536;
        } else {
          tiltAngleF16 =
              tiltAngleF16 + 3276 < 65536 ? tiltAngleF16 + 3276 : 65536;
        }

        var10000 = motorcycleParts[4]->motoComponents[primaryStateIndex].get();
        var10000->velX += var8;
        var10000 = motorcycleParts[4]->motoComponents[primaryStateIndex].get();
        var10000->velY += var9;
        var10000 = motorcycleParts[3]->motoComponents[primaryStateIndex].get();
        var10000->velX -= var8;
        var10000 = motorcycleParts[3]->motoComponents[primaryStateIndex].get();
        var10000->velY -= var9;
        var10000 = motorcycleParts[5]->motoComponents[primaryStateIndex].get();
        var10000->velX += var10;
        var10000 = motorcycleParts[5]->motoComponents[primaryStateIndex].get();
        var10000->velY += var11;
      }
      return;
    }

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

int GamePhysics::updatePhysics() {
  // 1. Управление Дзен-генератором
  if (currentLevelMode == LevelMode::ZEN_ENDLESS && !isCrashed && levelGenerator) {
    // getCamPosX возвращает логические координаты, подходящие для генератора
    levelGenerator->updateZen(levelLoader, getCamPosX());
    checkAndShiftWorld();
  }

  // 2. Авто-рестарт при аварии AI
  if (isCrashed && currentLevelMode != LevelMode::MANUAL) {
    if (++aiRestartTimer > 60) { // Ждем 1 секунду (60 кадров)
      setLevelMode(currentLevelMode, 0); // Перезагружаем уровень
      return 0; // Прерываем кадр
    }
  }

  // 3. Обработка ввода
  static int inputLogCounter = 0;
  if (isGenerateInputAI) {
    // AI Управление - AI сам устанавливает флаги
    setInputFromAI();
    
    // Логирование флагов управления (каждые 60 кадров)
    if (++inputLogCounter % 60 == 0) {
      std::cout << "[GamePhysics] Input flags: ACCEL=" << isInputAcceleration
                << " BRAKE=" << isInputBreak
                << " BACK=" << isInputBack
                << " FORWARD=" << isInputForward << std::endl;
    }
  } else {
    // Ручное управление из клавиатуры
    isInputAcceleration = isInputUp;
    isInputBreak = isInputDown;
    isInputBack = isInputLeft;
    isInputForward = isInputRight;
  }
  
  updateBikePhysics();
  int physicsResult;
  physicsResult = runPhysicsLoop(wheelBase);
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
    return 5;
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

restart:
  if (++failsafeCounter > 2000) {
    // Если цикл выполняется слишком долго, принудительно выходим
    return 5; // Возвращаем статус "авария"
  }

  int collisionState;
  while (currentStep < maxStep) {
    updateComponents(targetStep - currentStep);
    // Проверяем завершение трассы только если мотоцикл на земле (не в воздухе)
    // Это предотвращает преждевременное завершение при прыжке через финиш
    if (!wasInAir && !inAir && isTrackFinished()) {
      collisionState = 3;
    } else {
      collisionState = updateLevelCollision(secondaryStateIndex);
    }

    if (!wasInAir && inAir) {
      if (collisionState != 3) {
        return 2;
      }
      return 1;
    }

    if (collisionState == 0) {
      targetStep = (currentStep + targetStep) >> 1;
      goto restart;
    }

    if (collisionState == 3) {
      inAir = true;
      targetStep = (currentStep + targetStep) >> 1;
    } else {
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

      currentStep = targetStep;
      targetStep = maxStep;
      primaryStateIndex = primaryStateIndex == 1 ? 0 : 1;
      secondaryStateIndex = secondaryStateIndex == 1 ? 0 : 1;
    }
  }

  // Убрали проверку на схлопывание рамы - смерть только от соприкосновения
  // головы с трассой Проверка головы будет в updateLevelCollision

  return 0;
}

void GamePhysics::applyForces(int componentIndex) {
  // Определяем, находимся ли мы в зоне петли для уменьшения гравитации
  // scale is 14 bits (16384), not 16 bits (65536) for xF16
  // scale is 1 bit (x2) for xF16
  int loopCenterXF16 = 2200 << 1;
  int loopRadiusF16 = 500 << 1; // увеличенная зона влияния
  bool inLoopZone = false;
  int bikeVel = 0;

  if (motorcycleParts.size() > 1 && motorcycleParts[1]) {
    int bikeX = motorcycleParts[1]->motoComponents[componentIndex]->xF16;
    bikeVel = calcVectorLengthF16(
        motorcycleParts[1]->motoComponents[componentIndex]->velX,
        motorcycleParts[1]->motoComponents[componentIndex]->velY);
    int dxLoop = bikeX - loopCenterXF16;
    int absDxLoop = dxLoop < 0 ? -dxLoop : dxLoop;
    inLoopZone = absDxLoop < loopRadiusF16;
  }

  // REALISM: Only disable gravity if moving fast enough to stick
  // If stopped (vel approx 0), gravity applies -> fall.
  bool fastEnough = bikeVel > 200000;

  // В зоне петли ОТКЛЮЧАЕМ гравитацию для гарантии прохождения, НО только если
  // едем быстро
  int effectiveGravity = (inLoopZone && fastEnough) ? 0 : gravityF16;

  for (int i = 0; i < 6; ++i) {
    BikePart *part = motorcycleParts[i].get();
    MotoComponent *component = part->motoComponents[componentIndex].get();
    component->forceX = 0;
    component->forceY = 0;
    component->torque = 0;
    component->forceY -= (int)(((int64_t)effectiveGravity << 32) /
                                   (int64_t)part->suspensionStrength >>
                               16);
  }

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

  MotoComponent *rearWheel =
      motorcycleParts[2]->motoComponents[componentIndex].get();
  engineTorqueF16 =
      (int)((int64_t)engineTorqueF16 * (int64_t)(65536 - frictionCoeff) >> 16);
  rearWheel->torque = engineTorqueF16;
  if (rearWheel->angularVelocity > motoParam3) {
    rearWheel->angularVelocity = motoParam3;
  }

  if (rearWheel->angularVelocity < -motoParam3) {
    rearWheel->angularVelocity = -motoParam3;
  }

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
    // Отключаем сопротивление воздуха в петле
    if (!inLoopZone && relativeSpeed > 1700000) {
      int normX = (int)(((int64_t)dx << 32) / (int64_t)relativeSpeed >> 16);
      int normY = (int)(((int64_t)dy << 32) / (int64_t)relativeSpeed >> 16);
      motorcycleParts[i]->motoComponents[componentIndex]->velX -= normX;
      motorcycleParts[i]->motoComponents[componentIndex]->velY -= normY;
    }
  }

  int wheelYSign =
      motorcycleParts[2]->motoComponents[componentIndex]->yF16 -
                  motorcycleParts[0]->motoComponents[componentIndex]->yF16 >=
              0
          ? 1
          : -1;
  int wheelXSign =
      motorcycleParts[2]->motoComponents[componentIndex]->velX -
                  motorcycleParts[0]->motoComponents[componentIndex]->velX >=
              0
          ? 1
          : -1;
  wheelBalance = (wheelYSign * wheelXSign > 0) ? relativeSpeed : -relativeSpeed;
}

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

  // Проверяем расстояние между передним и задним колесом в актуальном состоянии
  MotoComponent *front = motorcycleParts[1]->motoComponents[stateIndex].get();
  MotoComponent *rear = motorcycleParts[2]->motoComponents[stateIndex].get();
  int dx = front->xF16 - rear->xF16;
  int dy = front->yF16 - rear->yF16;
  int distF16 = calcVectorLengthF16(dx, dy);
  if (distF16 == 0) {
    return;
  }

  int minAllowed = (int)((int64_t)initialWheelSeparationF16 * 99 /
                         100); // 99% базы - жестче рама
  // Убрали проверку на схлопывание - смерть только от соприкосновения головы с
  // трассой
  if (distF16 >= minAllowed) {
    return;
  }

  // Раздвигаем колёса вдоль оси, чтобы вернуть минимальный клиренс
  int correction = minAllowed - distF16;
  int normX = (int)(((int64_t)dx << 32) / distF16 >> 16);
  int normY = (int)(((int64_t)dy << 32) / distF16 >> 16);
  int corrX = (int)((int64_t)normX * correction >> 16);
  int corrY = (int)((int64_t)normY * correction >> 16);

  front->xF16 += corrX;
  front->yF16 += corrY;
  rear->xF16 -= corrX;
  rear->yF16 -= corrY;

  // Чуть корректируем скорости по оси, чтобы не схлопываться обратно
  front->velX += corrX;
  front->velY += corrY;
  rear->velX -= corrX;
  rear->velY -= corrY;
  engineTorqueF16 = 0; // сбрасываем момент, чтобы не схлопываться снова
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
  // Защита от деления на ноль при прыжке с петли (когда колеса могут быть в
  // одной точке)
  if (axleLen == 0) {
    // Если колеса в одной точке, используем единичный вектор по умолчанию
    axleDX = 65536; // 1.0 в F16
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

      // Смерть только при видимом соприкосновении головы (часть 5) с
      // поверхностью
      if (partIdx == 5 && collisionResult != 2) {
        isGroundColliding = true;

        // Проверяем, что голова видна на экране (в пределах видимой области)
        int headX = component->xF16 >> 1;
        // int headY = component->yF16 >> 1; // Unused
        int visibleStartX = levelLoader->visibleStartX;
        int visibleEndX = levelLoader->visibleEndX;

        // Голова видна, если находится в видимой области по X
        bool headVisible =
            (headX >= visibleStartX - 200 && headX <= visibleEndX + 200);

        // --- FIX: Проверка переворота ---
        // Получаем Y координат колес для определения ориентации
        // Используем сырые F16 координаты
        int wheel1Y = motorcycleParts[1]->motoComponents[componentIndex]->yF16;
        int wheel2Y = motorcycleParts[2]->motoComponents[componentIndex]->yF16;
        int avgWheelY = (wheel1Y + wheel2Y) >> 1;

        // В нашей системе координат Y растет вверх (земля ~ 220, верх ~ 800)
        // Если байк на колесах: HeadY > WheelY (Голова выше)
        // Если байк перевернут: HeadY < WheelY (Голова ниже)
        // Добавляем небольшой допуск (например 5 единиц в F16 ~= 327680)
        // чтобы не умирать при 90 градусах (езда по стене)
        bool isInverted = component->yF16 < (avgWheelY - 65536);

        // Смерть только при жесткой коллизии (0), видимой голове И перевороте
        if (collisionResult == 0 && partIdx == 5) {
             LOG_PHYS("Head Collision! Visible=%d Inverted=%d Y=%d WheelY=%d", 
                      headVisible, isInverted, component->yF16, avgWheelY);
        }

        if (collisionResult == 0 && headVisible && isInverted) {
          LOG_PHYS("CRASH DETECTED! HeadY=%d AvgWheelY=%d", component->yF16, avgWheelY);
          isCrashed = true;
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
        // Не прерываем цикл, если это не голова - смерть только от головы
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
  int torqueFront;
  int torqueBack;
  if (isInputBreak && (activePartIndex == 2 || activePartIndex == 1) &&
      component->angularVelocity < 6553) {
    suspensionFrontLocal = suspensionFront - motoParam7;
    suspensionBackLocal = 13107;
    forkLengthLocal = 39321;
    torqueFront = 26214 - motoParam7;
    torqueBack = 26214 - motoParam7;
  } else {
    suspensionFrontLocal = suspensionFront;
    suspensionBackLocal = suspensionBack;
    forkLengthLocal = forkLength;
    torqueFront = motoParam1;
    torqueBack = motoParam2;
  }

  int normalLen = calcVectorLengthF16(collisionNormalX, collisionNormalY);
  // Защита от деления на ноль при некорректной нормали коллизии
  if (normalLen == 0) {
    // Если нормаль нулевая, используем единичный вектор вверх по умолчанию
    collisionNormalX = 0;
    collisionNormalY = 65536; // 1.0 в F16 (направление вверх)
  } else {
    collisionNormalX =
        (int)(((int64_t)collisionNormalX << 32) / (int64_t)normalLen >> 16);
    collisionNormalY =
        (int)(((int64_t)collisionNormalY << 32) / (int64_t)normalLen >> 16);
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
      (int)((int64_t)torqueFront * (int64_t)tangentVel >> 16) -
      (int)((int64_t)forkLengthLocal *
                (int64_t)((int)((int64_t)component->angularVelocity *
                                    (int64_t)part->connectionLengthF16 >>
                                16)) >>
            16);
  int normalPush = -((int)((int64_t)torqueBack * (int64_t)normalVel >> 16));
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

  // Дополнительная защита от схлопывания сразу после коллизии
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

  // Переменные для хранения координат точек тела водителя
  int xF16 = 0, yF16 = 0, x2F16 = 0, y2F16 = 0, x3F16 = 0, y3F16 = 0, x4F16 = 0,
      y4F16 = 0;
  int x5F16 = 0, y5F16 = 0, x6F16 = 0, y6F16 = 0, circleXF16 = 0,
      circleYF16 = 0;

  std::vector<std::vector<int>> baseCoords;
  std::vector<std::vector<int>> targetCoords;

  // Выбираем массивы с координатами для модели водителя
  // определяет позу водителя в зависимости от наклона
  if (tiltAngleF16 < 32768) {
    baseCoords = hardcodedArr5;
    targetCoords = hardcodedArr4;
    var7 = (int)((int64_t)tiltAngleF16 * 131072L >> 16);
  } else {
    baseCoords = hardcodedArr4;
    targetCoords = hardcodedArr6;
    var7 = (int)((int64_t)(tiltAngleF16 - 32768) * 131072L >> 16);
  }

  // вычисляем итоговые координаты для каждой точки тела
  for (std::size_t i = 0; i < baseCoords.size(); ++i) {
    // Линейная интерполяция: pos = a * (1-t) + b * t
    int finalX =
        (int)((int64_t)baseCoords[i][0] * (int64_t)(65536 - var7) >> 16) +
        (int)((int64_t)targetCoords[i][0] * (int64_t)var7 >> 16);
    int finalY =
        (int)((int64_t)baseCoords[i][1] * (int64_t)(65536 - var7) >> 16) +
        (int)((int64_t)targetCoords[i][1] * (int64_t)var7 >> 16);

    // Поворачиваем и смещаем точку относительно центра масс
    int rotatedX = var8 + (int)((int64_t)rotXF16 * (int64_t)finalX >> 16) +
                   (int)((int64_t)dirXF16 * (int64_t)finalY >> 16);
    int rotatedY = var9 + (int)((int64_t)rotYF16 * (int64_t)finalX >> 16) +
                   (int)((int64_t)dirYF16 * (int64_t)finalY >> 16);

    // Присваиваем координаты соответствующим частям тела
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

  // Рисуем голову в виде круга
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

  // Рисуем тормозные диски (простые дуги)
  gameCanvas->drawBrakeDisc(
      motoComponents[1]->xF16 << 2 >> 16, motoComponents[1]->yF16 << 2 >> 16,
      const175_1_half[0] << 2 >> 16, MathF16::atan2F16(xxF16, yyF16));

  // Рисуем вилку мотоцикла (линия)
  if (!isCrashed) { // isCrashed - флаг, что мотоцикл не развалился
    renderMotoFork(gameCanvas);
  }

  // Рисуем "проволочную" раму, затем "палочного" водителя (чтобы голова была
  // поверх рамы при ударах)
  renderBikeWireframe(gameCanvas, xxF16, yyF16, var5, xxF16);
  renderRiderSkeleton(gameCanvas, xxF16, yyF16, var5, xxF16);

  // Отрисовка колес.
  int wheelRadius = GamePhysics::const175_1_half[0] << 2 >>
                    16; // Используем радиус из констант физики
  // Заднее колесо (индекс 2)
  gameCanvas->drawWheel(motoComponents[2]->xF16 << 2 >> 16,
                        motoComponents[2]->yF16 << 2 >> 16, wheelRadius);
  // Переднее колесо (индекс 1)
  gameCanvas->drawWheel(motoComponents[1]->xF16 << 2 >> 16,
                        motoComponents[1]->yF16 << 2 >> 16, wheelRadius);

  // Рисуем трассу в виде простой ломаной линии
  levelLoader->renderTrackNearestLine(gameCanvas);
}

void GamePhysics::enforceGroundCollision() {
  // Защита от краша - проверяем что levelLoader и gameLevel валидны
  if (!levelLoader || !levelLoader->gameLevel) {
    return;
  }

  // Притягиваем к трассе везде, включая петлю, чтобы мотоцикл проходил петлю
  // прижатым к трассе
  const int groundY_F16 = 220 << 1; // базовый уровень земли в F16
  const int loopCenterXF16 = 2200 << 1;
  const int loopRadiusF16 = 350 << 1;
  const int loopMarginF16 = 500
                            << 1; // увеличен запас вокруг петли для притяжения

  for (int i = 1; i <= 2; ++i) {
    MotoComponent *wheel =
        motorcycleParts[i]->motoComponents[primaryStateIndex].get();
    if (!wheel)
      continue;

    int phys_wheel_x = wheel->xF16;
    int phys_wheel_y = wheel->yF16;

    // Определяем, находимся ли мы в зоне петли
    int dxLoop = phys_wheel_x - loopCenterXF16;
    int absDxLoop = dxLoop < 0 ? -dxLoop : dxLoop;
    bool inLoopZone = absDxLoop < loopRadiusF16 + loopMarginF16;

    int wheel_radius = GamePhysics::const175_1_half[0];

    // Защита от краша - проверяем валидность координат
    int trackX = phys_wheel_x >> 1;
    if (trackX < 0 || trackX > 10000) {
      continue; // Выход за разумные границы
    }

    // Передаем текущий Y как подсказку, чтобы различать уровни петли (например,
    // низ vs верх или выход)
    int phys_track_y =
        (levelLoader->gameLevel->getTrackHeightAt(trackX, phys_wheel_y >> 1))
        << 1;
    int target_y = phys_track_y +
                   wheel_radius; // Y растет вверх: центр выше грунта на радиус

    // В зоне петли - сильное притяжение к трассе для прохождения петли
    if (inLoopZone) {
      // Притягиваем к трассе в петле с сильной силой для прохождения всей петли
      int distanceToTrack = target_y - phys_wheel_y;
      if (distanceToTrack > 0) {
        // Очень сильное притяжение к трассе в петле (почти мгновенное)
        int pullStrength = 65536; // 1.0 в F16 - мгновенное притяжение
        int pullY =
            (int)((int64_t)distanceToTrack * (int64_t)pullStrength >> 16);
        wheel->yF16 += pullY;
        // Корректируем скорость в направлении трассы - полностью убираем
        // скорость падения
        if (wheel->velY < 0) {
          wheel->velY = (int)((int64_t)wheel->velY * 16384L >>
                              16); // сильно уменьшаем скорость падения
        }
      } else if (distanceToTrack < -65536) {
        // Если слишком далеко от трассы в другую сторону, слегка подтягиваем
        int pullStrength = 16384; // 0.25 в F16 - слабое притяжение
        int pullY =
            (int)((int64_t)distanceToTrack * (int64_t)pullStrength >> 16);
        wheel->yF16 += pullY;
      }
    } else {
      // Вне петли - обычное притяжение к трассе
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
    // Threshold: 10,000 units in F16 (scaled by 2 for logic? no, wait)
    // MotoComponent xF16 is in raw F16 units (65536 per unit)
    // getCamPosX returns logic units (screen pixels * zoom?)
    // Let's use raw xF16 of the chassis.
    
    if (motorcycleParts.empty() || !motorcycleParts[0]) return;
    
    int currentX = motorcycleParts[0]->motoComponents[primaryStateIndex]->xF16;
    
    // 10,000 units * 65536 = 655,360,000.
    // Max int is 2,147,483,647.
    // Safe threshold: 1,000,000,000 (approx 15,000 units).
    
    const int THRESHOLD = 1000000000; // ~61k units (Safe for 32-bit signed int F14)
    // SHIFT_AMOUNT must be multiple of 16384 to match generator scale (F14 vs Units)
    // 500,000,000 / 16384 * 16384 = 499990528
    const int SHIFT_AMOUNT = 499990528; 
    
    if (currentX > THRESHOLD) {
        LOG_PHYS("SHIFTING WORLD! currentX=%d Shift=%d", currentX, SHIFT_AMOUNT);
        
        // 1. Shift all MotoComponents
        for(auto& part : motorcycleParts) {
            if(!part) continue;
            for(auto& comp : part->motoComponents) {
                if(comp) comp->xF16 -= SHIFT_AMOUNT;
            }
        }
        
        // 2. Shift Spring Components (they have xF16 too, used for physics state?)
        // Actually springComponents xF16 is used for stiffness/damping params often (see setupBike)
        // BUT some might be position?
        // In setupBike: springComponents[i]->xF16 = motoParam10; (stiffness)
        // So we should NOT shift springComponents unless they store world coordinates.
        // Looking at applySpringConstraint:
        // int stretchF16 = distanceF16 - spring->yF16;
        // int forceX = ... spring->xF16 ...
        // So spring components store PARAMETERS, not world coordinates.
        // EXCEPT maybe if they are used as anchors?
        // No, applySpringConstraint uses partA and partB positions.
        // So we SKIP springComponents.
        
        // 3. Shift Level
        // The level stores points in raw units?
        // GameLevel::addPointSimple uses x << 16 >> 3.
        // GameLevel stores (x * 8192).
        // xF16 is (x * 65536).
        // So Level scale is 1/8 of xF16 scale?
        // Let's check LevelLoader::detectCollision.
        // int wheelX = wheel->xF16 >> 1;
        // int x1 = gameLevel->pointPositions[segmentIdx][0];
        // Collision check compares wheelX (xF16/2) with x1.
        // So Level Points are stored in xF16 / 2 scale.
        
        // So shift for Level should be SHIFT_AMOUNT / 2.
        // Wait, SHIFT_AMOUNT is in xF16.
        // We need to shift level points by (SHIFT_AMOUNT >> 1).
        
        // BUT GameLevel::addPointSimple takes input, shifts it << 16 >> 3.
        // That is x * 8192.
        // wheel->xF16 >> 1 is x * 32768.
        // This discrepancy is confusing.
        // Let's trust LevelLoader::shiftLevel logic to handle the shift magnitude?
        // No, LevelLoader::shiftLevel calls GameLevel::shiftPoints(shiftX).
        // If I pass SHIFT_AMOUNT to shiftLevel, what should it be?
        
        // In detectCollision:
        // int wheelX = wheel->xF16 >> 1;
        // int x1 = gameLevel->pointPositions[i][0];
        // So pointPositions are in (F16 / 2) domain.
        
        int levelShift = SHIFT_AMOUNT >> 1;
        
        if (levelLoader) {
            levelLoader->shiftLevel(levelShift);
        }
        
        // 4. Shift Generator
        // Generator lastX is in what units?
        // addPointSimple(lastX, lastY).
        // If lastX is passed to addPointSimple, it's raw input units?
        // In loadLevel: lastX = 0.
        // addSlope: lastX += ...
        // level->addPointSimple(lastX, lastY).
        
        // addPointSimple(x, y):
        // addPoint((int)((int64_t)x << 16 >> 3), ...);
        // So x is "Logic Unit". (Screen pixel approx?)
        // xF16 is Logic Unit * 65536.
        // So x is xF16 / 65536.
        
        // The SHIFT_AMOUNT is xF16.
        // So generator shift should be SHIFT_AMOUNT / 65536?
        
        // Wait, detectCollision says:
        // wheelX = wheel->xF16 >> 1; (This is F15?)
        // x1 (from level) is compared to wheelX.
        // So level points are F15.
        // addPointSimple does: x << 16 >> 3 = x * 8192 = x * 2^13.
        // F15 is x * 2^15.
        // So addPointSimple input 'x' must be multiplied by 4 to get F15?
        // No.
        
        // Let's re-read GameLevel::addPointSimple.
        // addPoint((int)((int64_t)x << 16 >> 3));
        // x * 2^13.
        // LevelLoader uses x1 (which is from pointPositions) directly.
        // And compares with wheel->xF16 >> 1 (which is x * 2^15).
        // So x1 is F15.
        // So stored points are F15.
        // So addPointSimple produces F15?
        // x * 2^13 != F15 (x * 2^15) UNLESS input 'x' is already x * 4?
        // Or maybe my shift analysis is wrong.
        
        // Let's look at LevelLoader::prepareLevelData.
        // No conversion there.
        
        // Let's look at getTrackHeightAt.
        // x_pos is passed in.
        // It iterates pointPositions.
        
        // Let's look at Micro or setup.
        // setupBike calls levelLoader->getStartPosX().
        // getStartPosX returns startPosX << 1.
        // startPosX is stored in GameLevel (via setStartFinish).
        // setStartFinish does x << 16 >> 3.
        
        // If GameLevel stores in F13 (x*8192), and LevelLoader returns F14 (x*16384)?
        // setupBike: xF16 = startX + ...
        // If startX is F14, then xF16 is F14?
        // But physics usually runs in F16 (x*65536).
        
        // This coordinate system is a mess.
        // However, if I shift everything by consistent amounts, it should work.
        
        // SHIFT_AMOUNT is in xF16 domain (Physics Domain).
        // MotoComponents use xF16. So we subtract SHIFT_AMOUNT.
        
        // Level Points (GameLevel::pointPositions) are in whatever domain they are stored.
        // If detectCollision compares (xF16 >> 1) with PointPosition,
        // Then PointPosition is in (xF16 >> 1) domain.
        // So we must subtract (SHIFT_AMOUNT >> 1) from Level Points.
        
        // Level Generator 'lastX'.
        // addPointSimple(lastX, lastY).
        // addPointSimple converts lastX to PointPosition domain.
        // PointPosition = lastX * (2^13).
        // We know PointPosition = xF16 / 2.
        // So lastX * 2^13 = xF16 / 2 = xF16 * 2^-1.
        // lastX = xF16 * 2^-14.
        // So lastX is xF16 >> 14.
        
        // So generator shift = SHIFT_AMOUNT >> 14.
        
        // Let's verify.
        // If I shift physics by S.
        // I shift level points by S/2.
        // I shift generator by S/16384.
        
        int generatorShift = SHIFT_AMOUNT >> 14;
        
        if (levelGenerator) {
            levelGenerator->shiftGeneration(generatorShift);
        }
    }
}

GamePhysics::~GamePhysics() = default;
