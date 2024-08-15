#include <SDL2/SDL.h>
#include <cmath>
#include <string>

// Define the screen width and height
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

/**
 * Main function
 * @param argc Number of command line arguments
 * @param args Command line arguments
 * @return 0 on success, 1 on failure
 */
int main(int argc, char* args[]) {
    // Initialize SDL window and renderer
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    // Catch error when SDL fails to initialize
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create SDL window with the specified title, position, width, height, and flags
    window = SDL_CreateWindow("Particle Screensaver - 0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    // Catch error when SDL fails to create a window
    if (window == nullptr) {
        SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create SDL renderer with the specified window, index, and flags
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    // Catch error when SDL fails to create a renderer
    if (renderer == nullptr) {
        SDL_Log("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Set the draw of window's background color
    SDL_SetRenderDrawColor(renderer, 122, 122, 122, 255);
    // Clear the window
    SDL_RenderClear(renderer);

    // Set the draw color to white
    SDL_RenderPresent(renderer);

    int frameCount = 0;
    double startTime = SDL_GetTicks();
    double currentTime = startTime;

    // Declares SDL event variable to handle events
    SDL_Event e;
    // Controls the main loop of the program
    bool quit = false;
    // Main loop, executed as long as the program is not closed
    while (!quit) {
        // Handle pending events
        while (SDL_PollEvent(&e) != 0) {
            // Checks if the event is a quit event
            if (e.type == SDL_QUIT) {
                // If so, sets the quit flag to true
                quit = true;
            }
            frameCount++;

            // Calculate and display FPS
            if (SDL_GetTicks() - currentTime >= 1000) {
                currentTime = SDL_GetTicks();
                std::string title = "Hello World - FPS: " + std::to_string(frameCount);
                SDL_SetWindowTitle(window, title.c_str());
                frameCount = 0;
            }
        }    
    }

    // Destroy the renderer and window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    // Quit SDL
    SDL_Quit();

    // Return 0 on success
    return 0;
}