#include "./LevelLoader.h"
#include "./Game.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TextLabelComponent.h"
#include "../Components/TransformComponent.h"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <random>
#include <stdexcept>
#include <string>
#include <nlohmann/json.hpp>

namespace {
using Json = nlohmann::json;

std::filesystem::path ResolveProjectPath(const std::string& path) {
    const std::filesystem::path filePath(path);
    if (filePath.is_absolute()) {
        return filePath;
    }
#ifdef GAMEENGINE_PROJECT_DIR
    return std::filesystem::path(GAMEENGINE_PROJECT_DIR) / filePath;
#else
    return std::filesystem::current_path() / filePath;
#endif
}

double ReadCoordinate(const Json& value) {
    if (value.is_number()) {
        return value.get<double>();
    }

    if (value.is_object() && value.contains("random")) {
        const auto& range = value.at("random");
        if (!range.is_array() || range.size() != 2) {
            throw std::runtime_error("Random coordinate must contain a two-value range");
        }
        const int minimum = range.at(0).get<int>();
        const int maximum = range.at(1).get<int>();
        if (minimum > maximum) {
            throw std::runtime_error("Random coordinate range minimum exceeds maximum");
        }
        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(minimum, maximum);
        return distribution(generator);
    }

    throw std::runtime_error("Coordinate must be a number or a random range");
}

glm::vec2 ReadVector(const Json& value, glm::vec2 fallback = glm::vec2(0)) {
    return glm::vec2(
        value.contains("x") ? ReadCoordinate(value.at("x")) : fallback.x,
        value.contains("y") ? ReadCoordinate(value.at("y")) : fallback.y
    );
}

void AddEntityComponents(Entity& entity, const Json& components) {
    if (components.contains("transform")) {
        const auto& transform = components.at("transform");
        entity.AddComponent<TransformComponent>(
            ReadVector(transform.value("position", Json::object())),
            ReadVector(transform.value("scale", Json::object()), glm::vec2(1)),
            transform.value("rotation", 0.0)
        );
    }

    if (components.contains("rigidbody")) {
        entity.AddComponent<RigidBodyComponent>(
            ReadVector(components.at("rigidbody").value("velocity", Json::object()))
        );
    }

    if (components.contains("sprite")) {
        const auto& sprite = components.at("sprite");
        entity.AddComponent<SpriteComponent>(
            sprite.at("texture_asset_id").get<std::string>(),
            sprite.at("width").get<int>(),
            sprite.at("height").get<int>(),
            sprite.value("z_index", 0),
            sprite.value("fixed", false),
            sprite.value("src_rect_x", 0),
            sprite.value("src_rect_y", 0)
        );
    }

    if (components.contains("animation")) {
        const auto& animation = components.at("animation");
        entity.AddComponent<AnimationComponent>(
            animation.value("num_frames", 1),
            animation.value("speed_rate", 1)
        );
    }

    if (components.contains("boxcollider")) {
        const auto& collider = components.at("boxcollider");
        entity.AddComponent<BoxColliderComponent>(
            collider.at("width").get<int>(),
            collider.at("height").get<int>(),
            ReadVector(collider.value("offset", Json::object()))
        );
    }

    if (components.contains("health")) {
        entity.AddComponent<HealthComponent>(
            components.at("health").value("health_percentage", 100)
        );
    }

    if (components.contains("projectile_emitter")) {
        const auto& emitter = components.at("projectile_emitter");
        entity.AddComponent<ProjectileEmitterComponent>(
            ReadVector(emitter.value("projectile_velocity", Json::object())),
            static_cast<int>(emitter.value("repeat_frequency", 0.0) * 1000),
            static_cast<int>(emitter.value("projectile_duration", 10.0) * 1000),
            emitter.value("hit_percentage_damage", 10),
            emitter.value("friendly", false)
        );
    }

    if (components.contains("camera_follow")) {
        entity.AddComponent<CameraFollowComponent>();
    }

    if (components.contains("keyboard_controller")) {
        const auto& controls = components.at("keyboard_controller");
        entity.AddComponent<KeyboardControlledComponent>(
            ReadVector(controls.value("up_velocity", Json::object())),
            ReadVector(controls.value("right_velocity", Json::object())),
            ReadVector(controls.value("down_velocity", Json::object())),
            ReadVector(controls.value("left_velocity", Json::object()))
        );
    }

    if (components.contains("text_label")) {
        const auto& label = components.at("text_label");
        const auto color = label.value("color", Json::array({0, 0, 0}));
        if (!color.is_array() || color.size() < 3) {
            throw std::runtime_error("Text label color must contain at least three channels");
        }
        entity.AddComponent<TextLabelComponent>(
            ReadVector(label.value("position", Json::object())),
            label.value("text", std::string()),
            label.at("asset_id").get<std::string>(),
            SDL_Color{
                color.at(0).get<Uint8>(),
                color.at(1).get<Uint8>(),
                color.at(2).get<Uint8>(),
                color.size() > 3 ? color.at(3).get<Uint8>() : static_cast<Uint8>(255)
            },
            label.value("fixed", true)
        );
    }
}
}

LevelLoader::LevelLoader() {
    Logger::Log("LevelLoader constructor called!");
}

LevelLoader::~LevelLoader() {
    Logger::Log("LevelLoader destructor called!");
}

void LevelLoader::LoadLevel(
    const std::unique_ptr<Registry>& registry,
    const std::unique_ptr<AssetStore>& assetStore,
    SDL_Renderer* renderer,
    const std::string& levelPath
) {
    const auto resolvedLevelPath = ResolveProjectPath(levelPath);
    std::ifstream levelFile(resolvedLevelPath);
    if (!levelFile) {
        throw std::runtime_error("Unable to open level file: " + resolvedLevelPath.string());
    }

    const Json level = Json::parse(levelFile);

    for (const auto& asset : level.at("assets")) {
        const auto type = asset.at("type").get<std::string>();
        const auto id = asset.at("id").get<std::string>();
        const auto path = asset.at("file").get<std::string>();
        if (type == "texture") {
            assetStore->AddTexture(renderer, id, ResolveProjectPath(path).string());
        } else if (type == "font") {
            assetStore->AddFont(id, ResolveProjectPath(path).string(), asset.at("font_size").get<int>());
        } else {
            throw std::runtime_error("Unsupported asset type '" + type + "' in " + resolvedLevelPath.string());
        }
    }

    const auto& tilemap = level.at("tilemap");
    const auto& rows = tilemap.at("tiles");
    const int tileSize = tilemap.at("tile_size").get<int>();
    const double scale = tilemap.value("scale", 1.0);
    if (tileSize <= 0 || scale <= 0) {
        throw std::runtime_error("Tile size and scale must be positive in " + resolvedLevelPath.string());
    }
    if (!rows.is_array() || rows.empty() || !rows.front().is_array() || rows.front().empty()) {
        throw std::runtime_error("Tilemap must contain at least one tile in " + resolvedLevelPath.string());
    }

    const auto numCols = rows.front().size();
    const auto numRows = rows.size();
    const auto textureAssetId = tilemap.at("texture_asset_id").get<std::string>();
    for (std::size_t y = 0; y < numRows; ++y) {
        const auto& row = rows.at(y);
        if (!row.is_array() || row.size() != numCols) {
            throw std::runtime_error("Tilemap rows must have the same number of tiles in " + resolvedLevelPath.string());
        }

        for (std::size_t x = 0; x < numCols; ++x) {
            const auto tileIndex = row.at(x).get<std::string>();
            if (tileIndex.size() != 2 ||
                !std::isdigit(static_cast<unsigned char>(tileIndex[0])) ||
                !std::isdigit(static_cast<unsigned char>(tileIndex[1]))) {
                throw std::runtime_error("Invalid two-digit tile index in " + resolvedLevelPath.string());
            }
            const int sourceY = (tileIndex[0] - '0') * tileSize;
            const int sourceX = (tileIndex[1] - '0') * tileSize;
            Entity tile = registry->CreateEntity();
            tile.AddComponent<TransformComponent>(
                glm::vec2(x * scale * tileSize, y * scale * tileSize),
                glm::vec2(scale, scale),
                0.0
            );
            tile.AddComponent<SpriteComponent>(
                textureAssetId,
                tileSize,
                tileSize,
                0,
                false,
                sourceX,
                sourceY
            );
        }
    }

    Game::mapWidth = static_cast<int>(numCols * tileSize * scale);
    Game::mapHeight = static_cast<int>(numRows * tileSize * scale);

    for (const auto& entityData : level.at("entities")) {
        Entity entity = registry->CreateEntity();
        if (entityData.contains("tag")) {
            entity.Tag(entityData.at("tag").get<std::string>());
        }
        if (entityData.contains("group")) {
            entity.Group(entityData.at("group").get<std::string>());
        }
        AddEntityComponents(entity, entityData.value("components", Json::object()));
    }
}
