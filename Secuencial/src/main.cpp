/**
 * Universidad del Valle de Guatemala
 * Programación Paralela y Distribuida sección 10
 * Abner Ivan Garcia Alegria 21285
 * Oscar Esteban Donis Martinez 21610
 * pROYECTO 1
 */

#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include <random>
#include <iomanip> // Include iomanip header
#include <sstream>
#include <algorithm>
#include <iostream>

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
// Estructura para almacenar un punto de orbita
struct OrbitPoint {
    float x, y; // COORDENADAS
    float radius; // RADIO
    int absorbed_count; // CANTIDAD DE ABSORBIDOS
};
// Estructura para almacenar una particula
struct Particle {
    float x, y; // COORDENADAS
    float dx, dy; // VELOCIDADES
    float angle; // ANGULO
    float orbitRadius; // RADIO DE ORBITA
    int orbitIndex; // INDICE DE ORBITA
    bool isOrbiting; // ESTA EN ORBITA
    SDL_Color color; // COLOR
    std::vector<SDL_Point> trail; // ESTELA
    // CONSTRUCTOR DE PARTICULA
    Particle(float x, float y, float dx, float dy, SDL_Color color)
        : x(x), y(y), dx(dx), dy(dy), angle(0), orbitRadius(0), orbitIndex(-1),
          isOrbiting(false), color(color) {}
};
// FUNCION PARA OBTENER UN COLOR ALEATORIO
SDL_Color getRandomColor() {
    static std::random_device rd; // DISPOSITIVO ALEATORIO
    static std::mt19937 gen(rd()); // GENERADOR ALEATORIO
    static std::uniform_int_distribution<> dis(0, 255); // DISTRIBUCION ALEATORIA

    return SDL_Color{static_cast<Uint8>(dis(gen)),
                     static_cast<Uint8>(dis(gen)),
                     static_cast<Uint8>(dis(gen)),
                     255};
}
// FUNCION PARA ACTUALIZAR UNA PARTICULA
bool updateParticle(Particle& p, std::vector<OrbitPoint>& orbits, std::mt19937& gen) {
    std::uniform_real_distribution<> prob_dis(0.0, 1.0); // DISTRIBUCION ALEATORIA
    // CHEQUEAR SI ESTA EN ORBITA
    if (p.isOrbiting) {
        // CHEQUEAR PROBABILIDAD DE ESCAPE
        if (prob_dis(gen) < ESCAPE_PROBABILITY) {
            p.isOrbiting = false; // NO ESTA EN ORBITA
            p.dx = ROAM_SPEED * (prob_dis(gen) * 2 - 1); // VELOCIDAD DE MOVIMIENTO
            p.dy = ROAM_SPEED * (prob_dis(gen) * 2 - 1); // VELOCIDAD DE MOVIMIENTO
        } else {
            // ACTUALIZAR ANGULO
            p.angle += ORBIT_SPEED;
            if (p.angle > 2 * M_PI) p.angle -= 2 * M_PI; // ANGULO DE ORBITA
            OrbitPoint& orbit = orbits[p.orbitIndex]; // OBTENER ORBITA
            p.x = orbit.x + p.orbitRadius * cos(p.angle); // COORDENADA X
            p.y = orbit.y + p.orbitRadius * sin(p.angle); // COORDENADA Y

            // CHEQUEAR RADIO DE ABSORCION
            if (p.orbitRadius < ABSORPTION_RADIUS) {
                orbit.absorbed_count++; // AUMENTAR CANTIDAD DE ABSORBIDOS
                return false; // PARTICULA MUERE
            }

            // REDUCIR RADIO DE ORBITA
            p.orbitRadius = std::max(p.orbitRadius - 0.01f, 0.0f);
        }
    } else {
        // MOVER PARTICULA
        p.x += p.dx;
        p.y += p.dy; // MOVER PARTICULA

        //  REBOTAR EN LOS BORDES
        if (p.x < 0 || p.x >= SCREEN_WIDTH) p.dx = -p.dx;
        if (p.y < 0 || p.y >= SCREEN_HEIGHT) p.dy = -p.dy;

        // CHEQUEAR CAPTURA
        for (size_t i = 0; i < orbits.size(); ++i) {
            float dx = p.x - orbits[i].x; // DIFERENCIA EN X
            float dy = p.y - orbits[i].y; // DIFERENCIA EN Y
            float distance = sqrt(dx*dx + dy*dy);  // DISTANCIA
            if (distance < CAPTURE_RADIUS && prob_dis(gen) < CAPTURE_PROBABILITY) {
                p.isOrbiting = true; // ESTA EN ORBITA
                p.orbitIndex = i; // INDICE DE ORBITA
                p.orbitRadius = distance; // RADIO DE ORBITA
                p.angle = atan2(dy, dx); // ANGULO
                p.color = getRandomColor();  // COLOR ALEATORIO
                break;
            }
        }
    }

    // AGREGAR PUNTO A LA ESTELA
    p.trail.insert(p.trail.begin(), SDL_Point{static_cast<int>(p.x), static_cast<int>(p.y)}); // AGREGAR PUNTO
    if (p.trail.size() > TRAIL_LENGTH) {
        p.trail.pop_back(); // ELIMINAR PUNTO MAS ANTIGUO
    }

    return true;  // PARTICULA VIVA
}
// FUNCION PARA DIBUJAR UNA PARTICULA
void drawParticle(SDL_Renderer* renderer, const Particle& p) {
    for (size_t i = 0; i < p.trail.size(); ++i) {
        int alpha = 255 * (1 - static_cast<float>(i) / TRAIL_LENGTH); // TRANSPARENCIA
        SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, alpha); // COLOR
        SDL_RenderDrawPoint(renderer, p.trail[i].x, p.trail[i].y); // DIBUJAR PUNTO
    }
}
// FUNCION PARA DIBUJAR UNA ORBITA
void drawOrbit(SDL_Renderer* renderer, const OrbitPoint& orbit) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // COLOR
    for (int i = 0; i < 360; i++) {
        float angle = i * M_PI / 180; // ANGULO
        int x = static_cast<int>(orbit.x + orbit.radius * cos(angle)); // COORDENADA X
        int y = static_cast<int>(orbit.y + orbit.radius * sin(angle)); // COORDENADA Y 
        SDL_RenderDrawPoint(renderer, x, y); // DIBUJAR PUNTO
    }
}
// FUNCION PRINCIPAL
int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_VIDEO); // INICIAR SDL
    SDL_Window* window = SDL_CreateWindow("Particle Absorbing Screensaver - FPS: 0", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN); // CREAR VENTANA
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // CREAR RENDERIZADOR

    std::vector<OrbitPoint> orbits; // VECTOR DE ORBITAS
    std::vector<Particle> particles; // VECTOR DE PARTICULAS
    std::random_device rd; // DISPOSITIVO ALEATORIO
    std::mt19937 gen(rd()); // GENERADOR ALEATORIO
    std::uniform_real_distribution<> pos_dis(0, 1); // DISTRIBUCION ALEATORIA
    std::uniform_real_distribution<> vel_dis(-ROAM_SPEED, ROAM_SPEED); // DISTRIBUCION ALEATORIA
    std::uniform_real_distribution<> radius_dis(50, 150); // DISTRIBUCION ALEATORIA

    // CREAR ORBITAS
    for (int i = 0; i < NUM_ORBITS; ++i) {
        float x = SCREEN_WIDTH * (i + 1) / (NUM_ORBITS + 1); // COORDENADA X
        float y = SCREEN_HEIGHT / 2 + (i % 2 == 0 ? -1 : 1) * SCREEN_HEIGHT / 4; // COORDENADA Y
        float radius = radius_dis(gen); // RADIO
        orbits.push_back({x, y, radius, 0}); // AGREGAR ORBITA
    }

    double startTime = SDL_GetTicks(); // INICIAR CRONOMETRO

    // CREAR PARTICULAS INICIALES
    for (int i = 0; i < INITIAL_PARTICLES; ++i) {
        float x = pos_dis(gen) * SCREEN_WIDTH; // COORDENADA X
        float y = pos_dis(gen) * SCREEN_HEIGHT; // COORDENADA Y
        float dx = vel_dis(gen); // VELOCIDAD DE MOVIMIENTO
        float dy = vel_dis(gen); // VELOCIDAD DE MOVIMIENTO
        particles.emplace_back(x, y, dx, dy, getRandomColor()); // AGREGAR PARTICULA
    }

    double endTime = SDL_GetTicks(); // DETENER CRONOMETRO
    double generationTime = endTime - startTime; // CALCULAR TIEMPO DE GENERACION
    std::cout << "Time to generate particles: " << generationTime << " ms" << std::endl; // MOSTRAR TIEMPO DE GENERACION

    int frameCount = 0; // CONTADOR DE CUADROS
    double currentTime = startTime; // TIEMPO ACTUAL

    bool quit = false; // BANDERA DE SALIDA
    SDL_Event e; // EVENTO
    // CICLO PRINCIPAL
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true; // SALIR
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // COLOR DE FONDO
        SDL_RenderClear(renderer); // LIMPIAR PANTALLA

        // DIBUJAR ORBITAS Y PARTICULAS
        for (const auto& orbit : orbits) {
            drawOrbit(renderer, orbit); // DIBUJAR ORBITA
        }

        // ACTUALIZAR Y DIBUJAR PARTICULAS
        particles.erase(std::remove_if(particles.begin(), particles.end(),
            [&](Particle& p) { return !updateParticle(p, orbits, gen); }),
            particles.end()); // ACTUALIZAR PARTICULAS
        // DIBUJAR PARTICULAS
        for (const auto& particle : particles) {
            drawParticle(renderer, particle); // DIBUJAR PARTICULA
        }

        // AGREGAR PARTICULAS
        while (particles.size() < INITIAL_PARTICLES) {
            float x = pos_dis(gen) * SCREEN_WIDTH; // COORDENADA X
            float y = pos_dis(gen) * SCREEN_HEIGHT; // COORDENADA Y
            float dx = vel_dis(gen); // VELOCIDAD DE MOVIMIENTO
            float dy = vel_dis(gen); // VELOCIDAD DE MOVIMIENTO
            particles.emplace_back(x, y, dx, dy, getRandomColor()); // AGREGAR PARTICULA
        }

        SDL_RenderPresent(renderer); // ACTUALIZAR PANTALLA
        SDL_Delay(16);  // APROXIMADAMENTE 60 FPS

        frameCount++; // AUMENTAR CONTADOR DE CUADROS

        // CALCULAR FPS
        double now = SDL_GetTicks();
        if (now - currentTime >= 1000) {
            double fps = frameCount / ((now - currentTime) / 1000.0); // CALCULAR FPS
            std::ostringstream stream; // Create a stringstream
            stream << std::fixed << std::setprecision(2) << fps; // PRECISION DE 2 DECIMALES
            std::string fps_string = stream.str(); // OBETENER FPS COMO STRING
            std::string title = "Particle Absorbing Screensaver - FPS: " + fps_string; // TITULO DE LA VENTANA
            SDL_SetWindowTitle(window, title.c_str()); // ACTUALIZAR TITULO
            std::cout << "FPS: " << fps << std::endl; // MOSTRAR FPS
            currentTime = now; // ACTUALIZAR TIEMPO ACTUAL 
            frameCount = 0; // REINICIAR CONTADOR DE CUADROS
        }
    }

    SDL_DestroyRenderer(renderer); // DESTRUIR RENDERIZADOR
    SDL_DestroyWindow(window); // DESTRUIR VENTANA
    SDL_Quit(); // CERRAR SDL

    return 0; // SALIR
}