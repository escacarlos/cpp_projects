#include "./Game/Game.h"
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    Game game;

    if (!game.Initialize()) {
        game.Destroy();
        return 1;
    }
    const std::string levelPath = argc > 1 ? argv[1] : "./assets/levels/level2.json";
    try {
        game.Run(levelPath);
    } catch (const std::exception& error) {
        std::cerr << "Unable to run level '" << levelPath << "': " << error.what() << '\n';
        game.Destroy();
        return 1;
    }
    game.Destroy();
    
    return 0;
}
