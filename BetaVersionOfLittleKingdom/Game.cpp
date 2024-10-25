#include <iostream>
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <tinyxml2.h>
#include <sstream>
#include <set>
#include "player.h"

using namespace tinyxml2;

const int TILE_SIZE = 64;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int playerSpeed = 25;
bool collision = false;

struct Tileset {
    SDL_Texture* texture;
    int tileWidth;
    int tileHeight;
    int imageWidth;
    int imageHeight;
    int firstGid;
};

struct Camera {
    int x, y;
    int width, height;

    Camera(int width, int height) : x(0), y(0), width(width), height(height) {}
};

player Player;
bool quit = false;
bool flipRunning = false; // Flip state for running animation
int currentFrame = -1; // Current frame of the player animation
player::AnimationState state = player::IDLE; // Player animation state
float timeSinceLastFrame = 0.0f; // Time tracker for animation
Uint32 lastTime = SDL_GetTicks();

void log(XMLElement* c) {
    std::cout << c << std::endl;
}

std::vector<Tileset> tilesets;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
bool isFullscreen = false;

int init(const int& screenWidth, const int& screenHeight);
void destroy();
bool loadTMX(const std::string& filePath, std::vector<std::vector<std::vector<int>>>& tileMaps, int& mapWidth, int& mapHeight);
bool loadTSX(const std::string& filePath, Tileset& tileset);
void renderMap(SDL_Renderer* renderer, const std::vector<std::vector<std::vector<int>>>& tileMaps, const Camera& camera, float scale);


int main(int argc, char* argv[]) {
    if (init(SCREEN_WIDTH, SCREEN_HEIGHT) != 0) {
        return 1;
    }


    std::vector<std::vector<std::vector<int>>> tileMaps;
    int mapWidth = 0;
    int mapHeight = 0;

    std::string tmxFilePath = "D:/Assets/Tiny Swords/Tiny Swords (Update 010)/Terrain/Ground/Little_Kingdom.tmx";
    if (!loadTMX(tmxFilePath, tileMaps, mapWidth, mapHeight)) {
        destroy();
        return 1;
    }
    Player.loadPlayerSpriteSheet(renderer, "D:/Assets/Tiny Swords/Tiny Swords (Update 010)/Factions/Knights/Troops/Warrior/Blue/Warrior_Blue.png");

    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT);
    camera.x = (mapWidth * TILE_SIZE - camera.width) / 2;
    camera.y = (mapHeight * TILE_SIZE - camera.height) / 2;

    bool quit = false;
    SDL_Event e;

    // Timing variables for deltaTime
    Uint32 lastTime = SDL_GetTicks();
    float deltaTime = 0.0f;

    while (!quit) {
        // Event handling
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_w) {
                    if (!collision) {
                        camera.y -= 12;
                    }
                    state = player::RUNNING;
                }
                if (e.key.keysym.sym == SDLK_a) {
                    if (!collision) {
                        camera.x -= 12;
                    }
                    state = player::RUNNING;
                    flipRunning = true;
                }
                if (e.key.keysym.sym == SDLK_s) {
                    if (!collision) {
                        camera.y += 12;
                    }
                    state = player::RUNNING;
                }
                if (e.key.keysym.sym == SDLK_d) {
                    if (!collision) {
                        camera.x += 12;
                    }
                    state = player::RUNNING;
                    flipRunning = false;
                }
            }
            if (e.type == SDL_KEYUP) {
                if (e.key.keysym.sym == SDLK_d || e.key.keysym.sym == SDLK_a || e.key.keysym.sym == SDLK_s || e.key.keysym.sym == SDLK_w) {
                    state = player::IDLE;
                    currentFrame = 0;
                }
            }
        }

        // Calculate deltaTime (time since the last frame)
        Uint32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f; // Convert milliseconds to seconds
        lastTime = currentTime;

        // Update animations with deltaTime
        Player.updatePlayerAnimationFrame(state, currentFrame, timeSinceLastFrame, deltaTime);

        // Restrict camera movement to map bounds
        camera.x = std::max(0, std::min(camera.x, (mapWidth * TILE_SIZE) - camera.width));
        camera.y = std::max(0, std::min(camera.y, (mapHeight * TILE_SIZE) - camera.height));

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render the map
        float scale = 1.0f;
        if (isFullscreen) {
            int windowWidth, windowHeight;
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);
            scale = static_cast<float>(windowWidth) / (mapWidth * TILE_SIZE);
        }
        int windowWidth = 0;
        int windowHeight = 0;
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        renderMap(renderer, tileMaps, camera, scale);
        Player.renderPlayerAnimation(renderer, Player.playerSpriteSheetTexture, currentFrame, (state == player::IDLE) ? 0 : 1, windowWidth / 2 - TILE_SIZE, windowHeight / 2 - TILE_SIZE, flipRunning);

        // Present updated screen
        SDL_RenderPresent(renderer);

    }

    // Clean up and close SDL
    destroy();
    return 0;
}

int init(const int& screenWidth, const int& screenHeight) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        SDL_Quit();
        return 1;
    }

    window = SDL_CreateWindow("Tile Map", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    return 0;
}

void destroy() {
    for (const auto& tileset : tilesets) {
        if (tileset.texture) {
            SDL_DestroyTexture(tileset.texture);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

bool loadTMX(const std::string& filePath, std::vector<std::vector<std::vector<int>>>& tileMaps, int& mapWidth, int& mapHeight) {
    XMLDocument doc;

    if (doc.LoadFile(filePath.c_str()) != XML_SUCCESS) {
        return false;
    }

    XMLElement* mapElement = doc.FirstChildElement("map");
    if (!mapElement) {
        return false;
    }

    mapElement->QueryIntAttribute("width", &mapWidth);
    mapElement->QueryIntAttribute("height", &mapHeight);

    //for tileset
    for (XMLElement* tilesetElement = mapElement->FirstChildElement("tileset"); tilesetElement; tilesetElement = tilesetElement->NextSiblingElement("tileset")) {
        Tileset tileset;
        tileset.firstGid = tilesetElement->IntAttribute("firstgid");
        const char* source = tilesetElement->Attribute("source");
        if (source) {
            if (!loadTSX(source, tileset)) {
                return false;
            }
        }
        tilesets.push_back(tileset);
    }

    //for layer
    for (XMLElement* layerElement = mapElement->FirstChildElement("layer"); layerElement; layerElement = layerElement->NextSiblingElement("layer")) {
        XMLElement* dataElement = layerElement->FirstChildElement("data");
        if (!dataElement) {
            return false;
        }

        std::vector<std::vector<int>> layerMap;
        int tileCount = 0;
        for (XMLElement* tileElement = dataElement->FirstChildElement("tile"); tileElement; tileElement = tileElement->NextSiblingElement("tile")) {
            if (tileCount % mapWidth == 0) {
                layerMap.push_back(std::vector<int>());
            }

            int gid = tileElement->IntAttribute("gid");
            layerMap.back().push_back(gid);
            tileCount++;
        }

        if (tileCount != mapWidth * mapHeight) {
            return false;
        }

        tileMaps.push_back(layerMap);
    }

    return true;
}

bool loadTSX(const std::string& filePath, Tileset& tileset) {
    XMLDocument doc;
    std::string actualPath = "D:/Assets/GameFiles/maps/" + filePath;

    if (doc.LoadFile(actualPath.c_str()) != XML_SUCCESS) {
        return false; // File load failure
    }

    // Get the tileset element
    XMLElement* tilesetElement = doc.FirstChildElement("tileset");
    if (!tilesetElement) {
        return false; // No tileset found
    }

    // Load tileset properties
    tileset.tileWidth = tilesetElement->IntAttribute("tilewidth");
    tileset.tileHeight = tilesetElement->IntAttribute("tileheight");

    // Load image properties
    XMLElement* imageElement = tilesetElement->FirstChildElement("image");
    if (imageElement) {
        const char* imagePath = imageElement->Attribute("source");
        if (imagePath) {
            std::string actualImagePath = "D:/Assets/GameFiles/Images/" + std::string(imagePath);
            tileset.texture = IMG_LoadTexture(renderer, actualImagePath.c_str());
            if (!tileset.texture) {
                return false;
            }
            imageElement->QueryIntAttribute("width", &tileset.imageWidth);
            imageElement->QueryIntAttribute("height", &tileset.imageHeight);
        }
    }

    return true; // Success
}

void renderMap(SDL_Renderer* renderer, const std::vector<std::vector<std::vector<int>>>& tileMaps, const Camera& camera, float scale) {
    for (size_t layer = 0; layer < tileMaps.size(); ++layer) {

        const auto& layerMap = tileMaps[layer];

        for (size_t y = 0; y < layerMap.size(); ++y) {
            const auto& row = layerMap[y];

            for (size_t x = 0; x < row.size(); ++x) {
                int localId = row[x];

                if (localId > 0) {

                    const Tileset* tileset = nullptr;
                    for (const auto& ts : tilesets) {
                        if (localId >= ts.firstGid) {
                            tileset = &ts;
                        }
                    }

                    if (tileset && tileset->texture) {
                        int tileWidth = tileset->tileWidth;
                        int tileHeight = tileset->tileHeight;
                        int imageWidth = tileset->imageWidth;
                        int imageHeight = tileset->imageHeight;

                        int cols = imageWidth / tileWidth;
                        int rows = imageHeight / tileHeight;


                        // Regular static tile rendering
                        int tileIndex = localId - tileset->firstGid;
                        int srcX = (tileIndex % cols) * tileWidth;
                        int srcY = (tileIndex / cols) * tileHeight;

                        int GID = localId - tileIndex;
                        SDL_Rect srcRect = { srcX, srcY, tileWidth, tileHeight };
                        SDL_Rect dstRect = {
                        static_cast<int>(x * TILE_SIZE - camera.x),
                        static_cast<int>(y * TILE_SIZE - camera.y),
                        tileWidth,
                        tileHeight
                        };

                        SDL_RenderCopy(renderer, tileset->texture, &srcRect, &dstRect);

                    }
                }
            }
        }
    }
}
