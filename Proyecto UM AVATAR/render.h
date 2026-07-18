#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <map>

class Renderizador {
private:
    SDL_Window* ventana = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::map<SDL_Keycode, bool> teclasPresionadas;
    bool salir = false;

public:
    bool inicializar(const std::string& titulo, int ancho, int alto);
    void procesarEventos();
    bool debeSalir() const;
    bool estaPresionada(SDL_Keycode tecla) const;

    SDL_Texture* cargarTextura(const std::string& path);
    void destruirTextura(SDL_Texture* textura);

    void limpiarPantalla(Uint8 r, Uint8 g, Uint8 b);
    void dibujarTextura(SDL_Texture* textura,
                         int origenX, int origenY, int origenW, int origenH,
                         int destinoX, int destinoY, int destinoW, int destinoH);
    void presentar();

    void cerrar();
    ~Renderizador();
};
