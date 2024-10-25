#pragma once

#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <string>

class player {
private:
    const int PLAYER_WIDTH = 192;
    const int PLAYER_HEIGHT = 192;
    const int TOTAL_FRAMES_IDLE = 6;
    const int TOTAL_FRAMES_RUNNING = 6;
    const float FRAME_DURATION = 0.1f; // Duration of each frame in seconds (100 ms)

public:

    SDL_Texture* playerSpriteSheetTexture;
    enum AnimationState {
        IDLE,
        RUNNING
    };

    SDL_Texture* loadPlayerSpriteSheet(SDL_Renderer* renderer, const std::string& imagePath) {
        playerSpriteSheetTexture = IMG_LoadTexture(renderer, imagePath.c_str());

        if (!playerSpriteSheetTexture) {
            std::cerr << "Failed to load texture! IMG_Error: " << IMG_GetError() << std::endl;
        }

        return playerSpriteSheetTexture;
    }

    void renderPlayerAnimation(SDL_Renderer* renderer, SDL_Texture* playerSpriteSheetTexture, int currentFrame, int row, int x, int y, bool flip) {
        SDL_Rect srcRect;
        srcRect.x = currentFrame * PLAYER_WIDTH;
        srcRect.y = row * PLAYER_HEIGHT;
        srcRect.w = PLAYER_WIDTH;
        srcRect.h = PLAYER_HEIGHT;

        SDL_Rect destRect;
        destRect.x = x;
        destRect.y = y;
        destRect.w = PLAYER_WIDTH;
        destRect.h = PLAYER_HEIGHT;
        SDL_RendererFlip flipType = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

        SDL_RenderCopyEx(renderer, playerSpriteSheetTexture, &srcRect, &destRect, 0, NULL, flipType);
    }


    void updatePlayerAnimationFrame(AnimationState state, int& currentFrame, float& timeSinceLastFrame, float deltaTime) {
        timeSinceLastFrame += deltaTime;

        if (timeSinceLastFrame >= FRAME_DURATION) {
            timeSinceLastFrame -= FRAME_DURATION;

            if (state == IDLE) {
                currentFrame = (currentFrame + 1) % TOTAL_FRAMES_IDLE;
            }
            else if (state == RUNNING) {
                currentFrame = (currentFrame + 1) % TOTAL_FRAMES_RUNNING;
            }
        }
    }
};
