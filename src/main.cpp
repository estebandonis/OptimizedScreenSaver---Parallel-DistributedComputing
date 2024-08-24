#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include <string>
#include <random>

// Define the screen width and height
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int TRAIL_LENGTH = 20;

/**
 * Particle struct
 * Contains the x and y position of the particle, the x and y velocity of the particle, the color of the particle, and the trail of the particle
 */
struct Particle {
    // Position of the particle
    float x, y;
    // Velocity of the particle
    float dx, dy;
    // Color of the particle
    SDL_Color color;
    // Trail of the particle
    std::vector<SDL_Point> trail;

    // Constructor for the Particle struct
    Particle(float x, float y, float dx, float dy, SDL_Color color)
        : x(x), y(y), dx(dx), dy(dy), color(color) {}
};

/**
 * getRandomColor function
 * Generates a random color
 * @return SDL_Color object with random color
 */
SDL_Color getRandomColor() {
    // Generate random color
    static std::random_device rd;
    // Seed the random number generator
    static std::mt19937 gen(rd());
    // Generate random number between 0 and 255
    static std::uniform_int_distribution<> dis(0, 255);

    // Return random color
    return SDL_Color{static_cast<Uint8>(dis(gen)),
                     static_cast<Uint8>(dis(gen)),
                     static_cast<Uint8>(dis(gen)),
                     255};
}

/**
 * updateParticle function
 * Is in charge of updating the particle's position
 * @param p Particle to update
 */
void updateParticle(Particle& p) {
    // Update particle position based on velocity
    p.x += p.dx;
    p.y += p.dy;

    // Insert the current position into the trail
    p.trail.insert(p.trail.begin(), SDL_Point{static_cast<int>(p.x), static_cast<int>(p.y)});
    // Remove the last element of the trail if it exceeds the trail length
    if (p.trail.size() > TRAIL_LENGTH) {
        p.trail.pop_back();
    }
}

/**
 * drawParticle function
 * Is in charge of drawing the particle on the screen
 * @param renderer SDL_Renderer to draw the particle on
 * @param p Particle to draw
 */
void drawParticle(SDL_Renderer* renderer, const Particle& p) {
    // Draw the particle
    for (size_t i = 0; i < p.trail.size(); ++i) {
        // Calculate the alpha value based on the trail length
        int alpha = 255 * (1 - static_cast<float>(i) / TRAIL_LENGTH);
        // Set the draw color and draw the point
        SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, alpha);
        // Draw the point
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

    // Initialize frame count, start time, and current time
    int frameCount = 0;
    // Start time of the program
    double startTime = SDL_GetTicks();
    // Current time of the program
    double currentTime = startTime;

    // Generate random particle
    std::random_device rd;
    // Seed the random number generator
    std::mt19937 gen(rd());
    // Generate random number between -2 and 2
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

        // Set the color of the background
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        // Clear the screen
        SDL_RenderClear(renderer);

        // Update particle position
        updateParticle(uniqueParticle);
        // Draw the particle
        drawParticle(renderer, uniqueParticle);

        // Update the screen
        SDL_RenderPresent(renderer);
        // Delay to achieve 60 FPS
        SDL_Delay(16);  // Approx. 60 FPS

        // Calculate the frame count
        frameCount++;

        // Calculate and display FPS
        if (SDL_GetTicks() - currentTime >= 1000) {
            // Calculate the FPS
            currentTime = SDL_GetTicks();
            // Set the window title to display the FPS
            std::string title = "Hello World - FPS: " + std::to_string(frameCount);
            SDL_SetWindowTitle(window, title.c_str());
            // Reset the frame count
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