#include <iostream>
#include <chrono>
#include <thread>
#include <SDL2/SDL.h>

#include "audio/audio.h"
#include "util/paths.h"
#include "dialog/dialog.h"
#include "avatar/avatar.h"
#include "draw/draw.h"

using namespace std;

int main(int argc, char* argv[]) {
    // Resuelve el directorio de recursos de forma portable (no depende del cwd)
    paths_init(argv[0]);

    // ------------------------------------------------------------------
    // 1. INICIALIZACIÓN DE MÓDULOS
    // ------------------------------------------------------------------
    
    // Inicializa la ventana gráfica y el renderizador SDL (800x600 px)
    if (!draw_init("Asistente Virtual - Avatar", 800, 600)) return 1;

    // Carga los recursos y sprites del avatar (animaciones)
    if (!avatar_init()) { 
        draw_finish(); 
        return 1; 
    }

    // Inicializa captura/salida de audio (PortAudio + Whisper)
    audio_init();

    // Inicializa el motor de diálogo/conversación
    dialog_init();

    // Bandera de control para mantener el bucle activo
    bool ejecutando = true;
    SDL_Event evento;

    // ------------------------------------------------------------------
    // 2. BUCLE PRINCIPAL (MAIN LOOP)
    // ------------------------------------------------------------------
    while (ejecutando) {

        // A. Procesamiento de eventos de ventana (Cerrar ventana)
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) ejecutando = false;
        }

        // B. Actualización interna de módulos
        
        audio_update();   // Procesa buffers de audio en segundo plano
        dialog_update();  // Actualiza estados internos del diálogo

        // C. Verificación de entrada de voz del usuario
        avatar_setEstado(ESTADO_REPOSO);
        string textoUsuario = audio_obtenerTexto();
        

        // Si se detectó e interpretó voz válida del usuario:
        if (!textoUsuario.empty()) {
            cout << "\n[Usuario]: " << textoUsuario << endl;
            
            // Enviar la consulta al módulo de diálogo y obtener respuesta
            dialog_preguntar(textoUsuario);
            string textoRespuesta = dialog_obtenerRespuesta();

            // Si hay una respuesta generada:
            if (!textoRespuesta.empty()) {
                cout << "[Asistente]: " << textoRespuesta << endl;
                
                // Cambiar la animación a "HABLANDO"
                avatar_setEstado(ESTADO_HABLANDO);
                audio_hablar(textoRespuesta);

                // Estimar tiempo de habla según la longitud del texto (mínimo 2000 ms)
                int duracionMs = textoRespuesta.length() * 90; 
                if (duracionMs < 2000) duracionMs = 2000;

                // --- SUB-BUCLE DE ANIMACIÓN AL HABLAR ---
                // Mantiene al avatar en ESTADO_HABLANDO durante la duración calculada
                auto inicioHabla = chrono::steady_clock::now();
                while (chrono::duration_cast<chrono::milliseconds>(
                           chrono::steady_clock::now() - inicioHabla).count() < duracionMs) {
                    
                    // Continuar atendiendo eventos de ventana para evitar cierres o bloqueos
                    while (SDL_PollEvent(&evento)) {
                        if (evento.type == SDL_QUIT) ejecutando = false;
                    }

                    // Renderizar el frame de la animación de voz y pausar (~60 FPS)
                    avatar_update();
                    this_thread::sleep_for(chrono::milliseconds(16));
                }

                // Restaurar el estado a "ESCUCHANDO" tras terminar de hablar
                avatar_setEstado(ESTADO_REPOSO);
            }
        }

        // D. Renderizado continuo del frame actual en pantalla
        avatar_update();

        // Control de tasa de refresco a ~60 FPS
        this_thread::sleep_for(chrono::milliseconds(16));
    }

    // ------------------------------------------------------------------
    // 3. LIMPIEZA Y CIERRE DE RECURSOS
    // ------------------------------------------------------------------
    avatar_finish();
    draw_finish();
    audio_finish();

    return 0;
}