#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include <random>
#include <iomanip> // Include iomanip header
#include <sstream>
#include <algorithm>
#include <iostream>
#include <omp.h>  // Include OpenMP header

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int INITIAL_PARTICLES = 5000;
const int TRAIL_LENGTH = 20;
const int NUM_ORBITS = 5;
const float ORBIT_SPEED = 0.02f;
const float ROAM_SPEED = 1.0f;
const float CAPTURE_RADIUS = 100.0f;
const float ABSORPTION_RADIUS = 5.0f;
const float ESCAPE_PROBABILITY = 0.005f;
const float CAPTURE_PROBABILITY = 0.05f;

struct OrbitPoint {
    float x, y;
    float radius;
    int absorbed_count;
};

struct Particle {
    float x, y;
    float dx, dy;
    float angle;
    float orbitRadius;
    int orbitIndex;
    bool isOrbiting;
    SDL_Color color;
    std::vector<SDL_Point> trail;

    Particle(float x, float y, float dx, float dy, SDL_Color color)
        : x(x), y(y), dx(dx), dy(dy), angle(0), orbitRadius(0), orbitIndex(-1),
          isOrbiting(false), color(color) {}
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

bool updateParticle(Particle& p, std::vector<OrbitPoint>& orbits, std::mt19937& gen) {
    std::uniform_real_distribution<> prob_dis(0.0, 1.0);

    if (p.isOrbiting) {
        // Check for escape
        if (prob_dis(gen) < ESCAPE_PROBABILITY) {
            p.isOrbiting = false;
            p.dx = ROAM_SPEED * (prob_dis(gen) * 2 - 1);
            p.dy = ROAM_SPEED * (prob_dis(gen) * 2 - 1);
        } else {
            // Update orbital motion
            p.angle += ORBIT_SPEED;
            if (p.angle > 2 * M_PI) p.angle -= 2 * M_PI;
            OrbitPoint& orbit = orbits[p.orbitIndex];
            p.x = orbit.x + p.orbitRadius * cos(p.angle);
            p.y = orbit.y + p.orbitRadius * sin(p.angle);

            // Check for absorption
            if (p.orbitRadius < ABSORPTION_RADIUS) {
                orbit.absorbed_count++;
                return false;  // Particle is absorbed
            }

            // Gradually decrease orbit radius
            p.orbitRadius = std::max(p.orbitRadius - 0.01f, 0.0f);
        }
    } else {
        // Roaming motion
        p.x += p.dx;
        p.y += p.dy;

        // Bounce off screen edges
        if (p.x < 0 || p.x >= SCREEN_WIDTH) p.dx = -p.dx;
        if (p.y < 0 || p.y >= SCREEN_HEIGHT) p.dy = -p.dy;

        // Check for capture
        for (size_t i = 0; i < orbits.size(); ++i) {
            float dx = p.x - orbits[i].x;
            float dy = p.y - orbits[i].y;
            float distance = sqrt(dx*dx + dy*dy);
            if (distance < CAPTURE_RADIUS && prob_dis(gen) < CAPTURE_PROBABILITY) {
                p.isOrbiting = true;
                p.orbitIndex = i;
                p.orbitRadius = distance;
                p.angle = atan2(dy, dx);
                p.color = getRandomColor();  // Change color on capture
                break;
            }
        }
    }

    // Update trail
    p.trail.insert(p.trail.begin(), SDL_Point{static_cast<int>(p.x), static_cast<int>(p.y)});
    if (p.trail.size() > TRAIL_LENGTH) {
        p.trail.pop_back();
    }

    return true;  // Particle survives
}

void drawParticle(SDL_Renderer* renderer, const Particle& p) {
    for (size_t i = 0; i < p.trail.size(); ++i) {
        int alpha = 255 * (1 - static_cast<float>(i) / TRAIL_LENGTH);
        SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, alpha);
        SDL_RenderDrawPoint(renderer, p.trail[i].x, p.trail[i].y);
    }
}

void drawOrbit(SDL_Renderer* renderer, const OrbitPoint& orbit) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    for (int i = 0; i < 360; i++) {
        float angle = i * M_PI / 180;
        int x = static_cast<int>(orbit.x + orbit.radius * cos(angle));
        int y = static_cast<int>(orbit.y + orbit.radius * sin(angle));
        SDL_RenderDrawPoint(renderer, x, y);
    }
}

int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Particle Absorbing Screensaver - FPS: 0", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    std::vector<OrbitPoint> orbits;
    std::vector<Particle> particles;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> pos_dis(0, 1);
    std::uniform_real_distribution<> vel_dis(-ROAM_SPEED, ROAM_SPEED);
    std::uniform_real_distribution<> radius_dis(50, 150);

    // Create orbit points
    for (int i = 0; i < NUM_ORBITS; ++i) {
        float x = SCREEN_WIDTH * (i + 1) / (NUM_ORBITS + 1);
        float y = SCREEN_HEIGHT / 2 + (i % 2 == 0 ? -1 : 1) * SCREEN_HEIGHT / 4;
        float radius = radius_dis(gen);
        orbits.push_back({x, y, radius, 0});
    }

    double startTime = SDL_GetTicks();

    // Create initial particles
    #pragma omp parallel for
    for (int i = 0; i < INITIAL_PARTICLES; ++i) {
        float x = pos_dis(gen) * SCREEN_WIDTH;
        float y = pos_dis(gen) * SCREEN_HEIGHT;
        float dx = vel_dis(gen);
        float dy = vel_dis(gen);
        SDL_Color color = getRandomColor();
        #pragma omp critical
        particles.emplace_back(x, y, dx, dy, color);
    }

    double endTime = SDL_GetTicks();
    double generationTime = endTime - startTime;
    std::cout << "Time to generate particles: " << generationTime << " ms" << std::endl;

    int frameCount = 0;
    double currentTime = startTime;

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Update and draw orbits
        for (const auto& orbit : orbits) {
            drawOrbit(renderer, orbit);
        }

        // Update and draw particles in parallel
        #pragma omp parallel
        {
            #pragma omp for
            for (size_t i = 0; i < particles.size(); ++i) {
                if (!updateParticle(particles[i], orbits, gen)) {
                    #pragma omp critical
                    particles.erase(particles.begin() + i);
                    --i;  // Adjust index after removal
                }
            }
        }

        for (const auto& particle : particles) {
            drawParticle(renderer, particle);
        }

        // Add new particles if needed
        #pragma omp parallel
        {
            #pragma omp single
            {
                while (particles.size() < INITIAL_PARTICLES) {
                    float x = pos_dis(gen) * SCREEN_WIDTH;
                    float y = pos_dis(gen) * SCREEN_HEIGHT;
                    float dx = vel_dis(gen);
                    float dy = vel_dis(gen);
                    SDL_Color color = getRandomColor();
                    #pragma omp critical
                    particles.emplace_back(x, y, dx, dy, color);
                }
            }
        }

        SDL_RenderPresent(renderer);

        frameCount++;
        
        double now = SDL_GetTicks();
        if (now - currentTime >= 1000) {
            double fps = frameCount / ((now - currentTime) / 1000.0);
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(2) << fps; // Set precision to 2
            std::string fps_string = stream.str();
            std::string title = "Particle Absorbing Screensaver - FPS: " + fps_string;
            SDL_SetWindowTitle(window, title.c_str()); // Update window title
            std::cout << "FPS: " << fps << std::endl;
            currentTime = now;
            frameCount = 0;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}