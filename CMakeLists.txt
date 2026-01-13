cmake_minimum_required(VERSION 3.20)
project(mototest LANGUAGES CXX)

# Настройка стандарта C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Генерируем базу данных компиляции для IDE

# Если мы на macOS с Homebrew, добавляем путь поиска библиотек
if(APPLE AND EXISTS "/opt/homebrew")
    list(PREPEND CMAKE_PREFIX_PATH "/opt/homebrew")
endif()

# Список исходных файлов основного приложения
set(SOURCES
    AppTime.cpp
    AIController.cpp
    Canvas.cpp
    CanvasImpl.cpp
    Command.cpp
    GameCanvas.cpp
    GameLevel.cpp
    GamePhysics.cpp
    Graphics.cpp
    LevelGenerator.cpp
    LevelLoader.cpp
    Logger.cpp
    MathF16.cpp
    Micro.cpp
    TimerOrMotoPartOrMenuElem.cpp
    main.cpp
)

# Создание исполняемого файла mototest
add_executable(mototest ${SOURCES})
target_include_directories(mototest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Создание тестовой утилиты gen_test (для отладки генератора уровней)
add_executable(gen_test 
    test_gen.cpp 
    LevelGenerator.cpp 
    LevelLoader.cpp 
    GameLevel.cpp 
    GamePhysics.cpp 
    Logger.cpp 
    MathF16.cpp 
    Canvas.cpp
    CanvasImpl.cpp
    GameCanvas.cpp
    Graphics.cpp
    Command.cpp
    AppTime.cpp
    Micro.cpp
    TimerOrMotoPartOrMenuElem.cpp
    AIController.cpp
)

# Поиск и подключение библиотеки SDL2
find_package(SDL2 CONFIG QUIET)
if(SDL2_FOUND)
    # Если найден конфиг пакет (современный способ)
    target_link_libraries(mototest PRIVATE SDL2::SDL2)
    target_link_libraries(gen_test PRIVATE SDL2::SDL2)
    if(TARGET SDL2::SDL2main)
        target_link_libraries(mototest PRIVATE SDL2::SDL2main)
        target_link_libraries(gen_test PRIVATE SDL2::SDL2main)
    endif()
else()
    # Если конфиг не найден, используем старый модуль FindSDL2
    find_package(SDL2 REQUIRED)
    target_include_directories(mototest PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(mototest PRIVATE ${SDL2_LIBRARIES})
    target_include_directories(gen_test PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(gen_test PRIVATE ${SDL2_LIBRARIES})
endif()