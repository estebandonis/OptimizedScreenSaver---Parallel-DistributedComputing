#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include <string>
#include <random>

// Define the screen width and height
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int TRAIL_LENGTH = 20;

struct Particle {
    float x, y;
    float dx, dy;
    SDL_Color color;
    std::vector<SDL_Point> trail;

    Particle(float x, float y, float dx, float dy, SDL_Color color)
        : x(x), y(y), dx(dx), dy(dy), color(color) {}
};

SDL_Color getRandomColor() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 255);

    return SDL_Color{static_cast<Uint8>(dis(gen)),
                     static_cast<Uint8>(dis(gen)),
                     static_cast<Uint8>(dis(gen)),
                     255};
}

void updateParticle(Particle& p) {
    p.x += p.dx;
    p.y += p.dy;

    p.trail.insert(p.trail.begin(), SDL_Point{static_cast<int>(p.x), static_cast<int>(p.y)});
    if (p.trail.size() > TRAIL_LENGTH) {
        p.trail.pop_back();
    }
}

void drawParticle(SDL_Renderer* renderer, const Particle& p) {
    for (size_t i = 0; i < p.trail.size(); ++i) {
        int alpha = 255 * (1 - static_cast<float>(i) / TRAIL_LENGTH);
        SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, alpha);
        SDL_RenderDrawPoint(renderer, p.trail[i].x, p.trail[i].y);
    }
}

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

    int frameCount = 0;
    double startTime = SDL_GetTicks();
    double currentTime = startTime;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-2.0, 2.0);

    Particle uniqueParticle = Particle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, dis(gen), dis(gen), getRandomColor());

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
        }    

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        updateParticle(uniqueParticle);
        drawParticle(renderer, uniqueParticle);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);  // Approx. 60 FPS


        frameCount++;

        // Calculate and display FPS
        if (SDL_GetTicks() - currentTime >= 1000) {
            currentTime = SDL_GetTicks();
            std::string title = "Hello World - FPS: " + std::to_string(frameCount);
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
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