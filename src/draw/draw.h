#ifndef DRAW_H
#define DRAW_H

#include <SDL2/SDL_image.h>
#include <stdbool.h>

// Coordenadas recortadas de cada frame dentro del sprite sheet
typedef struct {
    int x, y, w, h;
} FrameSprite;

// Estructura contenedora de los datos de la animación
typedef struct {
    SDL_Texture* textura;
    FrameSprite* frames;
    int cantidadFrames;
} AnimacionDraw;

// --- Funciones solicitadas ---

// Inicializa la ventana y el renderer de SDL3
bool draw_init(const char* titulo, int ancho, int alto);

// Carga las animaciones PNG y lee los archivos JSON de Piskel
bool draw_cargarAnimacion(const char* pngPath, const char* jsonPath, AnimacionDraw* animOut);

// Prepara y calcula las coordenadas del frame actual
void draw_update(const AnimacionDraw* anim, int frameIndex, int escala);

// Renderiza en la ventana la escena preparada
void draw_dibujar(void);

// Cierra SDL3 y libera el renderer y la ventana
void draw_finish(void);

#endif 