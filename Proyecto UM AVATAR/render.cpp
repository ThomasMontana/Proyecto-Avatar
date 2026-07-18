#include "Renderizador.h"
#include <SDL3_image/SDL_image.h>
#include <cstdio>

bool Renderizador::inicializar(const std::string& titulo, int ancho, int alto) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Error al iniciar SDL: %s\n", SDL_GetError());
        return false;
    }

    ventana = SDL_CreateWindow(titulo.c_str(), ancho, alto, 0);
    if (!ventana) {
        printf("Error al crear ventana: %s\n", SDL_GetError());
        return false;
    }
    SDL_SetWindowPosition(ventana, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    renderer = SDL_CreateRenderer(ventana, nullptr);
    if (!renderer) {
        printf("Error al crear renderer: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void Renderizador::procesarEventos() {
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        if (evento.type == SDL_EVENT_QUIT) {
            salir = true;
        }
        if (evento.type == SDL_EVENT_KEY_DOWN) {
            teclasPresionadas[evento.key.key] = true;
        }
        if (evento.type == SDL_EVENT_KEY_UP) {
            teclasPresionadas[evento.key.key] = false;
        }
    }
}

bool Renderizador::debeSalir() const {
    return salir;
}

bool Renderizador::estaPresionada(SDL_Keycode tecla) const {
    auto it = teclasPresionadas.find(tecla);
    return it != teclasPresionadas.end() && it->second;
}

SDL_Texture* Renderizador::cargarTextura(const std::string& path) {
    SDL_Texture* textura = IMG_LoadTexture(renderer, path.c_str());
    if (!textura) {
        printf("Error al cargar %s: %s\n", path.c_str(), SDL_GetError());
    }
    return textura;
}

void Renderizador::destruirTextura(SDL_Texture* textura) {
    if (textura) SDL_DestroyTexture(textura);
}

void Renderizador::limpiarPantalla(Uint8 r, Uint8 g, Uint8 b) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderClear(renderer);
}

void Renderizador::dibujarTextura(SDL_Texture* textura,
                                   int origenX, int origenY, int origenW, int origenH,
                                   int destinoX, int destinoY, int destinoW, int destinoH) {
    SDL_FRect origen = { (float)origenX, (float)origenY, (float)origenW, (float)origenH };
    SDL_FRect destino = { (float)destinoX, (float)destinoY, (float)destinoW, (float)destinoH };
    SDL_RenderTexture(renderer, textura, &origen, &destino);
}

void Renderizador::presentar() {
    SDL_RenderPresent(renderer);
}

void Renderizador::cerrar() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (ventana) SDL_DestroyWindow(ventana);
    SDL_Quit();
    renderer = nullptr;
    ventana = nullptr;
}

Renderizador::~Renderizador() {
    cerrar();
}