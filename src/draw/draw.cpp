#include "draw.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static SDL_Window* g_ventana = NULL;
static SDL_Renderer* g_renderer = NULL;
static int g_ventanaAncho = 800;
static int g_ventanaAlto = 600;

// Variables temporales para almacenar qué se dibuja en la pasada actual
static const AnimacionDraw* g_animBuffer = NULL;
static int g_frameIndexBuffer = 0;
static int g_escalaBuffer = 1;

bool draw_init(const char* titulo, int ancho, int alto) {
    g_ventanaAncho = ancho;
    g_ventanaAlto = alto;

    // En SDL2, SDL_Init devuelve < 0 en caso de error
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Error al inicializar SDL: %s\n", SDL_GetError());
        return false;
    }

    // Configuración estándar para SDL2
    g_ventana = SDL_CreateWindow(titulo, 
                                 SDL_WINDOWPOS_CENTERED, 
                                 SDL_WINDOWPOS_CENTERED, 
                                 ancho, alto, 
                                 SDL_WINDOW_SHOWN);
    if (!g_ventana) {
        printf("Error al crear la ventana: %s\n", SDL_GetError());
        return false;
    }

    // Intentar renderer acelerado por hardware primero; si no hay GPU
    // disponible (por ejemplo dentro de un contenedor sin /dev/dri pasado
    // desde el host), caer a software en vez de fallar directamente.
    g_renderer = SDL_CreateRenderer(g_ventana, -1, SDL_RENDERER_ACCELERATED);
    if (!g_renderer) {
        printf("Aviso: no se pudo crear renderer acelerado (%s), usando software.\n", SDL_GetError());
        g_renderer = SDL_CreateRenderer(g_ventana, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!g_renderer) {
        printf("Error al crear el renderer: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

// Función auxiliar en C/C++ para leer archivos de texto (JSON)
static char* leerArchivoTexto(const char* ruta) {
    FILE* f = fopen(ruta, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long largo = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buffer = (char*)malloc(largo + 1);
    if (buffer) {
        fread(buffer, 1, largo, f);
        buffer[largo] = '\0';
    }
    fclose(f);
    return buffer;
}

bool draw_cargarAnimacion(const char* pngPath, const char* jsonPath, AnimacionDraw* animOut) {
    if (!animOut) return false;

    // Cargar la textura PNG con SDL_image
    animOut->textura = IMG_LoadTexture(g_renderer, pngPath);
    if (!animOut->textura) {
        printf("Error al cargar la imagen %s: %s\n", pngPath, SDL_GetError());
        return false;
    }

    // Leer el contenido del JSON
    char* jsonTexto = leerArchivoTexto(jsonPath);
    if (!jsonTexto) {
        printf("Error al abrir el archivo JSON: %s\n", jsonPath);
        SDL_DestroyTexture(animOut->textura);
        return false;
    }

    // Contar cuántos frames contiene el JSON
    int capacidad = 0;
    const char* ptr = jsonTexto;
    while ((ptr = strstr(ptr, "\"frame\":")) != NULL) {
        capacidad++;
        ptr += 8;
    }

    if (capacidad == 0) {
        printf("No se encontraron frames validos en %s\n", jsonPath);
        free(jsonTexto);
        SDL_DestroyTexture(animOut->textura);
        return false;
    }

    animOut->frames = (FrameSprite*)malloc(sizeof(FrameSprite) * capacidad);
    animOut->cantidadFrames = 0;

    // Parsear las coordenadas {"x": ..., "y": ..., "w": ..., "h": ...}
    ptr = jsonTexto;
    while ((ptr = strstr(ptr, "\"frame\":")) != NULL) {
        const char* inicioBloque = strchr(ptr, '{');
        if (inicioBloque) {
            FrameSprite f = {0, 0, 0, 0};
            const char* px = strstr(inicioBloque, "\"x\":");
            const char* py = strstr(inicioBloque, "\"y\":");
            const char* pw = strstr(inicioBloque, "\"w\":");
            const char* ph = strstr(inicioBloque, "\"h\":");

            if (px && py && pw && ph) {
                f.x = atoi(px + 4);
                f.y = atoi(py + 4);
                f.w = atoi(pw + 4);
                f.h = atoi(ph + 4);

                animOut->frames[animOut->cantidadFrames] = f;
                animOut->cantidadFrames++;
            }
        }
        ptr += 8;
    }

    free(jsonTexto);
    return true;
}

void draw_update(const AnimacionDraw* anim, int frameIndex, int escala) {
    // Almacenamos la referencia de lo que dibuja esta iteración
    g_animBuffer = anim;
    g_frameIndexBuffer = frameIndex;
    g_escalaBuffer = escala;
}

void draw_dibujar(void) {
    if (!g_renderer) return;

    // Limpiar pantalla en negro
    SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
    SDL_RenderClear(g_renderer);

    if (g_animBuffer && g_animBuffer->textura && g_animBuffer->cantidadFrames > 0) {
        int idx = g_frameIndexBuffer;
        if (idx >= g_animBuffer->cantidadFrames) {
            idx = 0;
        }

        FrameSprite f = g_animBuffer->frames[idx];

        // Adaptación a SDL2: Uso de SDL_Rect en vez de SDL_FRect
        SDL_Rect origen = { f.x, f.y, f.w, f.h };

        int destW = f.w * g_escalaBuffer;
        int destH = f.h * g_escalaBuffer;

        SDL_Rect destino = {
            (g_ventanaAncho - destW) / 2,
            (g_ventanaAlto - destH) / 2,
            destW,
            destH
        };

        // Reemplazo de SDL_RenderTexture por SDL_RenderCopy
        SDL_RenderCopy(g_renderer, g_animBuffer->textura, &origen, &destino);
    }

    // Presentar en pantalla
    SDL_RenderPresent(g_renderer);
}

void draw_finish(void) {
    if (g_renderer) SDL_DestroyRenderer(g_renderer);
    if (g_ventana) SDL_DestroyWindow(g_ventana);
    SDL_Quit();
}