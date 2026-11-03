#ifndef LEVELLOADER_H
#define LEVELLOADER_H

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <memory>
#include <string>

class LevelLoader {
    public:
        LevelLoader();
        ~LevelLoader();
        void LoadLevel(
            const std::unique_ptr<Registry>& registry,
            const std::unique_ptr<AssetStore>& assetStore,
            SDL_Renderer* renderer,
            const std::string& levelPath
        );
};

#endif
