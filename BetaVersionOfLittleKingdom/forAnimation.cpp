#include <iostream>
#include <vector>
#include <SDL.h>

const int TILE_SIZE = 64;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

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

std::vector<Tileset> tilesets;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

struct AnimatedTile {
    int tileId; // The GID of the animated tile
    std::vector<int> frames; // List of tile IDs for animation
    float frameDuration; // Duration of each frame
    float elapsedTime; // Time elapsed since last frame
    int currentFrame; // Current frame to render
};

std::vector<AnimatedTile> animatedTiles; // List of animated tiles

void updateAnimations(float deltaTime) {
    for (auto& animatedTile : animatedTiles) {
        animatedTile.elapsedTime += deltaTime;
        if (animatedTile.elapsedTime >= animatedTile.frameDuration) {
            animatedTile.elapsedTime -= animatedTile.frameDuration;
            animatedTile.currentFrame = (animatedTile.currentFrame + 1) % animatedTile.frames.size();
        }
    }
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

                        // Check for animated tile without std::find_if
                        bool isAnimated = false;
                        int animatedTileIndex = -1;
                        for (size_t i = 0; i < animatedTiles.size(); ++i) {
                            if (animatedTiles[i].tileId == localId) {
                                isAnimated = true;
                                animatedTileIndex = i; // Save index to retrieve later
                                break;
                            }
                        }

                        if (isAnimated) {
                            // Get the animated tile data
                            auto& animTile = animatedTiles[animatedTileIndex];
                            int currentAnimatedId = animTile.frames[animTile.currentFrame];
                            int animatedTileIndex = currentAnimatedId - tileset->firstGid;

                            int srcX = (animatedTileIndex % cols) * tileWidth;
                            int srcY = (animatedTileIndex / cols) * tileHeight;

                            SDL_Rect srcRect = { srcX, srcY, tileWidth, tileHeight };

                            SDL_Rect dstRect = {
                                static_cast<int>(x * TILE_SIZE - camera.x),
                                static_cast<int>(y * TILE_SIZE - camera.y),
                                tileWidth,
                                tileHeight
                            };

                            SDL_RenderCopy(renderer, tileset->texture, &srcRect, &dstRect);
                        }
                        else {
                            // Regular static tile rendering
                            int tileIndex = localId - tileset->firstGid;

                            int srcX = (tileIndex % cols) * tileWidth;
                            int srcY = (tileIndex / cols) * tileHeight;

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
}
