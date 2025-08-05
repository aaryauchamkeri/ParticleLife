#include <SDL.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <unordered_set>
#include "ThreadPool.h"

const int SCREEN_WIDTH = 1800;
const int SCREEN_HEIGHT = 1000;
const int NUM_PARTICLES = 5500;
const int NUM_TYPES = 6;
const float TIME_STEP = 0.125f/2.0f;
const float FORCE_SCALE = 100.0f;
const float DAMPING = 0.6f;
const float MAX_SPEED = 2.5f;
const int PARTICLE_SIZE = 2;

struct Particle {
    float x, y;
    float vx, vy;
    int type;

    bool operator==(const Particle& other) const {
        return x == other.x && y == other.y && vx == other.vx && vy == other.vy && type == other.type;
    }
};

std::vector<Particle> particles;
float force_matrix[NUM_PARTICLES][NUM_PARTICLES] = {
    {-0.2f, 0.5f, -0.3f, 0.1f, -0.3f, 0.0f},
    {0.1f, -0.1f, 0.7f, -0.6f, 0.4f, 0.0f},
    {-0.5f, 0.3f, -0.2f, 0.4f, -0.2f, 0.0f},
    { 0.2f, -0.7f, -0.1f, -0.3f, 0.7f, 0.0f},
    {0.4f, -0.2f, 0.5f, -0.1f, -0.2f, 0.0f},
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f} // chaos particle
};

std::vector<std::unordered_set<int>> grid = std::vector<std::unordered_set<int>>((SCREEN_WIDTH / 100) * (SCREEN_HEIGHT / 100));

void init_particles() {
    for (int i = 0; i < NUM_PARTICLES; ++i) {
        float x = (float)(rand() % SCREEN_WIDTH); // column
        float y = (float)(rand() % SCREEN_HEIGHT); // row
        particles.push_back({
            x,y,
            0.0f, 0.0f,
            rand() % NUM_TYPES
        });
        int grid_x = (int)(x / 100);
        int grid_y = (int)(y / 100);
        grid[grid_y + grid_x].insert(particles.size() - 1);
    }
}

void apply_forces(int i) {
    Particle& a = particles[i];
    float ax = 0, ay = 0;
    int grid_x = (int)(a.x / 100);
    int grid_y = (int)(a.y / 100);

    for(int f = -1; f <= 1; f++) {
        for(int s = -1; s <= 1; s++) {
            if((grid_x + f) + (grid_y + s) >= 0 && 
                (grid_x + f + grid_y + s) < (int)(SCREEN_WIDTH / 100) * (int)(SCREEN_HEIGHT / 100)) {
                for(const int& ind : grid[grid_y + s + grid_x + f]) {
                    Particle& b = particles[ind];
                    if(b == a) continue; // skip self
                    float dx = b.x - a.x;
                    float dy = b.y - a.y;
                    float dist_sq = dx * dx + dy * dy;
                    if (dist_sq > 0 && dist_sq < 8000) {
                        float dist = std::sqrt(dist_sq);
                        float f = force_matrix[a.type][b.type] / (dist); // gravity like force
                        if(dist <= 5.0f) { // if the particles are very close we need to repulse
                            f -= 150.0f / (dist_sq + 1e-3f); 
                        }
                        ax += f * dx;
                        ay += f * dy;
                    }
                }
            }
        }
    }



    // for (size_t j = 0; j < particles.size(); ++j) {
    //     if (&particles[j] == &a) continue;
    //     Particle& b = particles[j];

    //     float dx = b.x - a.x;
    //     float dy = b.y - a.y;
    //     float dist_sq = dx * dx + dy * dy;
    //     if (dist_sq > 0 && dist_sq < 8000) {
    //         float dist = std::sqrt(dist_sq);
    //         float f = force_matrix[a.type][b.type] / (dist); // gravity like force
    //         if(dist <= 5.0f) { // if the particles are very close we need to repulse
    //             f -= 150.0f / (dist_sq + 1e-3f); 
    //         }
    //         ax += f * dx;
    //         ay += f * dy;
    //     }
    // }

    a.vx = (a.vx + ax * TIME_STEP) * DAMPING;
    a.vy = (a.vy + ay * TIME_STEP) * DAMPING;

    float speed = std::sqrt(a.vx * a.vx + a.vy * a.vy);
    if (speed > MAX_SPEED) { // basically just setting the speed to max speed
        a.vx *= MAX_SPEED / speed;
        a.vy *= MAX_SPEED / speed;
    }

    a.x += a.vx;
    a.y += a.vy;

    int new_grid_x = (int)(a.x / 100);
    int new_grid_y = (int)(a.y / 100);
    if (new_grid_x != grid_x || new_grid_y != grid_y) {
        grid[grid_y + grid_x].erase(i);
        grid[new_grid_y + new_grid_x].insert(i);
    }

    if (a.x < 0) a.x += SCREEN_WIDTH;
    if (a.x >= SCREEN_WIDTH) a.x -= SCREEN_WIDTH;
    if (a.y < 0) a.y += SCREEN_HEIGHT;
    if (a.y >= SCREEN_HEIGHT) a.y -= SCREEN_HEIGHT;
}

void draw_particles(SDL_Renderer* renderer) {
    for (const auto& p : particles) {
        switch (p.type) {
            case 0: SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); break;
            case 1: SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); break;
            case 2: SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); break;
            case 3: SDL_SetRenderDrawColor(renderer, 125, 0, 255, 255); break;
            case 4: SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); break;
            case 5: SDL_SetRenderDrawColor(renderer, 125, 125, 125, 155); break; // chaos particle
        }
        SDL_Rect rect = { (int)p.x, (int)p.y, PARTICLE_SIZE, PARTICLE_SIZE };
        SDL_RenderFillRect(renderer, &rect);
        // SDL_RenderDrawPoint(renderer, (int)p.x, (int)p.y);
    }
}

int main() {
    srand((unsigned)time(nullptr));
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Particle Life",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    ThreadPool threadPool(8);

    init_particles(); 

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        for(int i = 0; i < particles.size(); ++i) {
            threadPool.submitTask([i]() {
                apply_forces(i);
            });
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_particles(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(2);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
