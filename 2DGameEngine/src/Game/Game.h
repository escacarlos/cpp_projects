#ifndef GAME_H
#define GAME_H

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../EventBus/EventBus.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <string>

const int FPS = 60;
const int MILLISECS_PER_FRAME = 1000 / FPS;

class Game {
    private:
        bool isRunning;
        bool isDebug;
        bool sdlInitialized = false;
        bool ttfInitialized = false;
        int millisecsPreviousFrame = 0;
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        SDL_Rect camera{};
        
        std::unique_ptr<Registry> registry;
        std::unique_ptr<AssetStore> assetStore;
        std::unique_ptr<EventBus> eventBus;

    public:
        Game();
        ~Game();
        bool Initialize();
        void Run(const std::string& levelPath);
        void Setup(const std::string& levelPath);
        void ProcessInput();
        void Update();
        void Render();
        void Destroy();

        static int windowWidth;
        static int windowHeight;
        static int mapWidth;
        static int mapHeight;
};

#endif
