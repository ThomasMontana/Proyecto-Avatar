#pragma once
#include <string>

using namespace std;

// Funciones del módulo de audio 
void audio_init();
void audio_update();
string audio_obtenerTexto();
void audio_hablar(string textoRespuesta);
void audio_finish();