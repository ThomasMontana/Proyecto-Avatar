#include "avatar.h"
#include "../util/paths.h"
#include <string>
#include "../draw/draw.h"
#include <SDL2/SDL.h> // Necesario para SDL_GetTicks()
#include <cstdio>
#include <cstdlib>

using namespace std;

#define MS_POR_FRAME 40 // Aumenta este valor si quieres que vaya aún más lento

static AnimacionDraw g_animReposo;
static AnimacionDraw g_animEscuchando;
static AnimacionDraw g_animHablando;

static EstadoAvatar g_estadoActual = ESTADO_REPOSO;
static int g_frameActual = 0;
static Uint32 g_ultimoTiempo = 0; // Guarda el tiempo del último cambio de frame

bool avatar_init(void) {
    bool ok = true;

    const string rReposo      = resourcePath("animations/AnimacionReposo");
    const string rEscuchando  = resourcePath("animations/escuchando");
    const string rHablando    = resourcePath("animations/hablando");

    ok &= draw_cargarAnimacion((rReposo + ".png").c_str(), (rReposo + ".json").c_str(), &g_animReposo);
    ok &= draw_cargarAnimacion((rEscuchando + ".png").c_str(), (rEscuchando + ".json").c_str(), &g_animEscuchando);
    ok &= draw_cargarAnimacion((rHablando + ".png").c_str(), (rHablando + ".json").c_str(), &g_animHablando);

    if (!ok) {
        printf("[Avatar Error] Falló al cargar alguna de las animaciones.\n");
        return false;
    }

    g_estadoActual = ESTADO_REPOSO;
    g_frameActual = 0;
    g_ultimoTiempo = SDL_GetTicks();
    return true;
}

void avatar_setEstado(EstadoAvatar nuevoEstado) {
    if (g_estadoActual != nuevoEstado) {
        g_estadoActual = nuevoEstado;
        g_frameActual = 0;
        g_ultimoTiempo = SDL_GetTicks();
    }
}

void avatar_update(void) {
    const AnimacionDraw* animActual = &g_animReposo;

    switch (g_estadoActual) {
        case ESTADO_ESCUCHANDO:
            animActual = &g_animEscuchando;
            break;
        case ESTADO_HABLANDO:
            animActual = &g_animHablando;
            break;
        case ESTADO_REPOSO:
        default:
            animActual = &g_animReposo;
            break;
    }

    Uint32 tiempoActual = SDL_GetTicks();

    // Solo avanza el frame si pasaron al menos MS_POR_FRAME milisegundos
    if (tiempoActual - g_ultimoTiempo >= MS_POR_FRAME) {
        if (animActual->cantidadFrames > 0) {
            g_frameActual = (g_frameActual + 1) % animActual->cantidadFrames;
        } else {
            g_frameActual = 0;
        }
        g_ultimoTiempo = tiempoActual;
    }

    draw_update(animActual, g_frameActual, 8);
    draw_dibujar();
}

void avatar_finish(void) {
    if (g_animReposo.frames) free(g_animReposo.frames);
    if (g_animEscuchando.frames) free(g_animEscuchando.frames);
    if (g_animHablando.frames) free(g_animHablando.frames);
}