#include "./AssetStore.h"
#include "../Logger/Logger.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL_image.h>
#include <stdexcept>

AssetStore::AssetStore() {
    Logger::Log("AssetStore constructor called!");
}

AssetStore::~AssetStore() {
    ClearAssets();
    Logger::Log("AssetStore destructor called!");
}

void AssetStore::ClearAssets() {
    for (auto texture: textures) {
        SDL_DestroyTexture(texture.second);
    }
    textures.clear();

    for (auto font: fonts) {
        TTF_CloseFont(font.second);
    }
    fonts.clear();
}

void AssetStore::AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath) {
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if (!surface) {
        throw std::runtime_error("Unable to load texture '" + filePath + "': " + IMG_GetError());
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        throw std::runtime_error("Unable to create texture '" + assetId + "': " + SDL_GetError());
    }

    // Add the texture to the map
    const auto inserted = textures.emplace(assetId, texture);
    if (!inserted.second) {
        SDL_DestroyTexture(texture);
        throw std::runtime_error("Duplicate texture asset id: " + assetId);
    }

    Logger::Log("Texture added to the AssetStore with id " + assetId);
}

SDL_Texture* AssetStore::GetTexture(const std::string& assetId) {
    return textures.at(assetId);
}

void AssetStore::AddFont(const std::string& assetId, const std::string& filePath, int fontSize) {
    TTF_Font* font = TTF_OpenFont(filePath.c_str(), fontSize);
    if (!font) {
        throw std::runtime_error("Unable to load font '" + filePath + "': " + TTF_GetError());
    }
    const auto inserted = fonts.emplace(assetId, font);
    if (!inserted.second) {
        TTF_CloseFont(font);
        throw std::runtime_error("Duplicate font asset id: " + assetId);
    }
}

TTF_Font* AssetStore::GetFont(const std::string& assetId) {
    return fonts.at(assetId);
}
