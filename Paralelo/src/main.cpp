/**
 * Universidad del Valle de Guatemala
 * Programación Paralela y Distribuida sección 10
 * Abner Ivan Garcia Alegria 21285
 * Oscar Esteban Donis Martinez 21610
 * Proyecto 1
 */

#include <SDL2/SDL.h> // Include SDL2 header
#include <vector> // Include vector header
#include <cmath> // Include cmath header
#include <random> // Include random header
#include <iomanip> // Include iomanip header
#include <sstream> // Include sstream header
#include <algorithm> // Include algorithm header
#include <iostream> // Include iostream header
#include <omp.h>  // Include OpenMP header
using namespace std;

int SCREEN_WIDTH = 800; //  ANCHO DE LA PANTALLA
int SCREEN_HEIGHT = 600;  // ALTO DE LA PANTALLA 
int INITIAL_PARTICLES = 5000; // CANTIDAD DE PARTICULAS INICIALES
int TRAIL_LENGTH = 20; // LONGITUD DE LA ESTELA
int NUM_ORBITS = 5; // CANTIDAD DE ORBITAS
float ORBIT_SPEED = 0.02f; // VELOCIDAD DE LA ORBITA
float ROAM_SPEED = 1.0f; // VELOCIDAD DE MOVIMIENTO
float CAPTURE_RADIUS = 100.0f; // RADIO DE CAPTURA
float ABSORPTION_RADIUS = 5.0f; // RADIO DE ABSORCION
float ESCAPE_PROBABILITY = 0.005f; // PROBABILIDAD DE ESCAPE
float CAPTURE_PROBABILITY = 0.05f; // PROBABILIDAD DE CAPTURA
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
    Particle(float x, float y, float dx, float dy, SDL_Color color) // CONSTRUCTOR DE PARTICULA
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

    if (p.isOrbiting) {
        // CHEQUEAR PROBABILIDAD DE ESCAPE
        if (prob_dis(gen) < ESCAPE_PROBABILITY) {
            p.isOrbiting = false; // NO ESTA EN ORBITA
            p.dx = ROAM_SPEED * (prob_dis(gen) * 2 - 1); // VELOCIDAD DE MOVIMIENTO
            p.dy = ROAM_SPEED * (prob_dis(gen) * 2 - 1); // VELOCIDAD DE MOVIMIENTO
        } else {
            // ACTUALIZAR ANGULO
            p.angle += ORBIT_SPEED; // VELOCIDAD DE ORBITA
            if (p.angle > 2 * M_PI) p.angle -= 2 * M_PI; // ANGULO DE ORBITA
            OrbitPoint& orbit = orbits[p.orbitIndex]; // OBTENER ORBITA
            p.x = orbit.x + p.orbitRadius * cos(p.angle); // COORDENADA X
            p.y = orbit.y + p.orbitRadius * sin(p.angle); // COORDENADA Y

            // CHEQUEAR RADIO DE ABSORCION
            if (p.orbitRadius < ABSORPTION_RADIUS) {
                orbit.absorbed_count++;
                return false;  // PARTICULA MUERE
            }

            // REDUCIR RADIO DE ORBITA
            p.orbitRadius = std::max(p.orbitRadius - 0.01f, 0.0f);
        }
    } else {
        // MOVER PARTICULA
        p.x += p.dx;
        p.y += p.dy;

        // REBOTAR EN LOS BORDES
        if (p.x < 0 || p.x >= SCREEN_WIDTH) p.dx = -p.dx;
        if (p.y < 0 || p.y >= SCREEN_HEIGHT) p.dy = -p.dy;

        // CHEQUEAR CAPTURA
        for (size_t i = 0; i < orbits.size(); ++i) {
            float dx = p.x - orbits[i].x; // DIFERENCIA EN X
            float dy = p.y - orbits[i].y; // DIFERENCIA EN Y
            float distance = sqrt(dx*dx + dy*dy); // DISTANCIA
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
    p.trail.insert(p.trail.begin(), SDL_Point{static_cast<int>(p.x), static_cast<int>(p.y)});
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

    string message;
    bool valid = false;

    while(!valid){
        cout << "Ingrese el ancho de la pantalla: (default: 800)\n";
        getline(cin, message);

        try
        {
            SCREEN_WIDTH = stoi(message);

            valid = true;
        }
        catch(const std::invalid_argument& e)
        {
            cerr << "Input invalido, ingrese un numero \n";
        }
        catch(const std::out_of_range& e)
        {
            cerr << "Numero muy grande, ingrese un valor no tan grande \n";
        }
        
    }
    
    valid = false;

    while(!valid){
        cout << "Ingrese el largo de la pantalla: (default: 600)\n";
        getline(cin, message);

        try
        {
            SCREEN_HEIGHT = stoi(message);

            valid = true;
        }
        catch(const std::invalid_argument& e)
        {
            cerr << "Input invalido, ingrese un numero \n";
        }
        catch(const std::out_of_range& e)
        {
            cerr << "Numero muy grande, ingrese un valor no tan grande \n";
        }
        
    }

    valid = false;

    while(!valid){
        cout << "Ingrese la cantidad de particulas iniciales: (default: 5000)\n";
        getline(cin, message);

        try
        {
            INITIAL_PARTICLES = stoi(message);

            valid = true;
        }
        catch(const std::invalid_argument& e)
        {
            cerr << "Input invalido, ingrese un numero \n";
        }
        catch(const std::out_of_range& e)
        {
            cerr << "Numero muy grande, ingrese un valor no tan grande \n";
        }
        
    }

    valid = false;

    while(!valid){
        cout << "Ingrese la cantidad de orbitas: (default: 5)\n";
        getline(cin, message);

        try
        {
            NUM_ORBITS = stoi(message);

            valid = true;
        }
        catch(const std::invalid_argument& e)
        {
            cerr << "Input invalido, ingrese un numero \n";
        }
        catch(const std::out_of_range& e)
        {
            cerr << "Numero muy grande, ingrese un valor no tan grande \n";
        }
        
    }

    valid = false;

    while(!valid){
        cout << "Ingrese la longitud de la cola: (default: 20)\n";
        getline(cin, message);

        try
        {
            TRAIL_LENGTH = stoi(message);

            valid = true;
        }
        catch(const std::invalid_argument& e)
        {
            cerr << "Input invalido, ingrese un numero \n";
        }
        catch(const std::out_of_range& e)
        {
            cerr << "Numero muy grande, ingrese un valor no tan grande \n";
        }
        
    }
    
    valid = false;

    while(!valid){
        cout << "Ingrese la velocidad de la orbita: (default: 0.02)\n";
        getline(cin, message);

        try
        {
            ORBIT_SPEED = stof(message);

            valid = true;
        }
        catch(const std::invalid_argument& e)
        {
            cerr << "Input invalido, ingrese un numero \n";
        }
        catch(const std::out_of_range& e)
        {
            cerr << "Numero muy grande, ingrese un valor no tan grande \n";
        }
        
    }

    valid = false;

    while(!valid){
        cout << "Ingrese la velocidad en la que van los destellos: (default: 1.0)\n";
        getline(cin, message);

        try
        {
            ROAM_SPEED = stof(message);

            valid = true;
        }
        catch(const std::invalid_argument& e)
        {
            cerr << "Input invalido, ingrese un numero \n";
        }
        catch(const std::out_of_range& e)
        {
            cerr << "Numero muy grande, ingrese un valor no tan grande \n";
        }
        
    }

    valid = false;

    while(!valid){
        cout << "Ingrese el radio de atrapamiento: (default: 100.0)\n";
        getline(cin, message);

        try
        {
            CAPTURE_RADIUS = stof(message);

            valid = true;
        }
        catch(const std::invalid_argument& e)
        {
            cerr << "Input invalido, ingrese un numero \n";
        }
        catch(const std::out_of_range& e)
        {
            cerr << "Numero muy grande, ingrese un valor no tan grande \n";
        }
        
    }

    valid = false;

    while(!valid){
        cout << "Ingrese el radio de absorcion: (default: 5.0)\n";
        getline(cin, message);

        try
        {
            ABSORPTION_RADIUS = stof(message);

            valid = true;
        }
        catch(const std::invalid_argument& e)
        {
            cerr << "Input invalido, ingrese un numero \n";
        }
        catch(const std::out_of_range& e)
        {
            cerr << "Numero muy grande, ingrese un valor no tan grande \n";
        }
        
    }

    valid = false;

    while(!valid){
        cout << "Ingrese el posibilidad de escape: (default: 0.005)\n";
        getline(cin, message);

        try
        {
            ESCAPE_PROBABILITY = stof(message);

            valid = true;
        }
        catch(const std::invalid_argument& e)
        {
            cerr << "Input invalido, ingrese un numero \n";
        }
        catch(const std::out_of_range& e)
        {
            cerr << "Numero muy grande, ingrese un valor no tan grande \n";
        }
        
    }

    valid = false;

    while(!valid){
        cout << "Ingrese el posibilidad de absorcion: (default: 0.05)\n";
        getline(cin, message);

        try
        {
            CAPTURE_PROBABILITY = stof(message);

            valid = true;
        }
        catch(const std::invalid_argument& e)
        {
            cerr << "Input invalido, ingrese un numero \n";
        }
        catch(const std::out_of_range& e)
        {
            cerr << "Numero muy grande, ingrese un valor no tan grande \n";
        }
        
    }


    SDL_Init(SDL_INIT_VIDEO); // INICIAR SDL
    SDL_Window* window = SDL_CreateWindow("Particle Absorbing Screensaver - FPS: 0", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN); // CREAR VENTANA
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // CREAR RENDERIZADOR
    // VECTOR DE ORBITAS
    std::vector<OrbitPoint> orbits;
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

    //  CREAR PARTICULAS
    #pragma omp parallel for // INICIAR REGION PARALELA PARA CREAR PARTICULAS
    for (int i = 0; i < INITIAL_PARTICLES; ++i) {
        float x = pos_dis(gen) * SCREEN_WIDTH; // COORDENADA X
        float y = pos_dis(gen) * SCREEN_HEIGHT; // COORDENADA Y
        float dx = vel_dis(gen); // VELOCIDAD EN X
        float dy = vel_dis(gen); // VELOCIDAD EN Y
        SDL_Color color = getRandomColor(); // COLOR
        #pragma omp critical // SECCION CRITICA PARA AGREGAR PARTICULA
        particles.emplace_back(x, y, dx, dy, color); // AGREGAR PARTICULA
    }

    double endTime = SDL_GetTicks(); // DETENER CRONOMETRO
    double generationTime = endTime - startTime; // TIEMPO DE GENERACION DE PARTICULAS
    std::cout << "Time to generate particles: " << generationTime << " ms" << std::endl; // MOSTRAR TIEMPO DE GENERACION DE PARTICULAS

    int frameCount = 0; // CONTADOR DE FRAMES
    double currentTime = startTime; // TIEMPO ACTUAL

    bool quit = false; // BANDERA DE SALIDA
    SDL_Event e; // EVENTO
    // CICLO PRINCIPAL DEL JUEGO
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true; // SALIR
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // COLOR DE FONDO
        SDL_RenderClear(renderer); // LIMPIAR PANTALLA

        // DIBUJAR ORBITAS
        for (const auto& orbit : orbits) {
            drawOrbit(renderer, orbit); // DIBUJAR ORBITA
        }

        // ACTUALIZAR PARTICULAS Y DIBUJARLAS EN PARALELO
        #pragma omp parallel
        {
            #pragma omp for // INICIAR REGION PARALELA PARA ACTUALIZAR PARTICULAS
            for (size_t i = 0; i < particles.size(); ++i) {
                if (!updateParticle(particles[i], orbits, gen)) {
                    #pragma omp critical // SECCION CRITICA PARA ELIMINAR PARTICULA
                    particles.erase(particles.begin() + i); // ELIMINAR PARTICULA
                    --i;  // DECREMENTAR INDICE
                }
            }
        }

        #pragma omp parallel
        {
            #pragma omp for // INICIAR REGION PARALELA PARA DIBUJAR PARTICULAS
            for (const auto& particle : particles) {
                drawParticle(renderer, particle); // DIBUJAR PARTICULA
            }
        }
        

        // AGREGAR PARTICULAS EN PARALELO
        #pragma omp parallel
        {
            #pragma omp single // INICIAR TAREA UNICA PARA AGREGAR PARTICULAS
            {
                while (particles.size() < INITIAL_PARTICLES) {
                    float x = pos_dis(gen) * SCREEN_WIDTH; // COORDENADA X
                    float y = pos_dis(gen) * SCREEN_HEIGHT; // COORDENADA Y
                    float dx = vel_dis(gen); // VELOCIDAD EN X
                    float dy = vel_dis(gen); // VELOCIDAD EN Y
                    SDL_Color color = getRandomColor(); // COLOR
                    #pragma omp critical // SECCION CRITICA PARA AGREGAR PARTICULA 
                    particles.emplace_back(x, y, dx, dy, color); // AGREGAR PARTICULA
                }
            }
        }

        SDL_RenderPresent(renderer); // ACTUALIZAR PANTALLA

        frameCount++; // INCREMENTAR CONTADOR DE FRAMES
        
        double now = SDL_GetTicks(); // OBTENER TIEMPO ACTUAL
        if (now - currentTime >= 1000) {
            double fps = frameCount / ((now - currentTime) / 1000.0); // CALCULAR FPS
            std::ostringstream stream; // FLUJO DE SALIDA
            stream << std::fixed << std::setprecision(2) << fps; // FORMATEAR FPS 
            std::string fps_string = stream.str(); // CONVERTIR FPS A CADENA
            std::string title = "Particle Absorbing Screensaver - FPS: " + fps_string; // TITULO DE LA VENTANA
            SDL_SetWindowTitle(window, title.c_str()); // ACTUALIZAR TITULO DE LA VENTANA
            std::cout << "FPS: " << fps << std::endl; // MOSTRAR FPS
            currentTime = now; // ACTUALIZAR TIEMPO ACTUAL
            frameCount = 0; // REINICIAR CONTADOR DE FRAMES
        }
    }

    SDL_DestroyRenderer(renderer); // DESTRUIR RENDERIZADOR
    SDL_DestroyWindow(window); // DESTRUIR VENTANA
    SDL_Quit();

    return 0;
}