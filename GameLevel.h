#pragma once

#include <cstdint>
#include <vector>

class GameCanvas;

/**
 * GameLevel
 * 
 * Хранит данные уровня: точки трассы, старт, финиш.
 * Отвечает за простые операции с геометрией уровня (поиск высоты, добавление точек).
 */
class GameLevel {
private:
  int minX = 0;
  int maxX = 0;
  int segmentStart = 0;
  int segmentEnd = 0;
  int segmentExtra = 0;

public:
  int startPosX;
  int startPosY;
  int finishPosX;
  int finishPosY;
  int startFlagPoint = 0;
  int finishFlagPoint = 0;
  int pointsCount;
  int currentSegment;
  
  // Массив точек трассы [pointIndex][0=x, 1=y]
  // Хранятся в специальных единицах (обычно (x << 16) >> 3 = x * 8192)
  std::vector<std::vector<int>> pointPositions;

  GameLevel();
  ~GameLevel();
  
  void init();
  
  // Настройка старта и финиша
  void setStartFinish(int startX, int startY, int finishX, int finishY);
  int getStartPosX();
  int getStartPosY();
  int getFinishPosX();
  int getFinishPosY();
  
  // Доступ к точкам
  int getPointX(int pointNo);
  int getPointY(int pointNo);
  
  // Расчет прогресса (0..65536) от старта до финиша
  int computeProgress(int x);
  
  // Получение высоты трассы в заданной точке X
  int getTrackHeightAt(int x_pos, int current_y_hint = 0);
  
  // Установка границ рендеринга
  void setMinMaxX(int minX, int maxX);
  void setSegmentRange(int start, int end);
  void setSegmentRangeExact(int start, int end, int extra);
  
  // Отрисовка
  void renderTrackNearestGreenLine(GameCanvas *canvas);
  
  // Добавление точек
  void addPointSimple(int x, int y); // Принимает обычные координаты, конвертирует внутри
  void addPoint(int x, int y);       // Принимает сырые координаты
  
  // Сдвиг всех точек уровня (для бесконечной генерации)
  void shiftPoints(int shiftX);
};