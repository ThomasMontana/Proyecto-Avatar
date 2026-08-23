#include "audio.h"
#include "../util/paths.h"
#include "whisper.h"
#include <iostream>
#include <vector>
#include <cmath> // Necesario para std::abs
#include <portaudio.h>

using namespace std;

// --- Parámetros de Configuración ---
#define SAMPLE_RATE 16000          // Whisper requiere audio a 16 kHz
#define FRAMES_PER_BUFFER 512

// --- UMBRAL DE VOLUMEN (Noise Gate) ---
#define UMBRAL_VOLUMEN 0.1f

// --- Estado Privado del Módulo (Variables Estáticas) ---
static bool inicializado = false;
static PaStream* streamAudio = nullptr;
static struct whisper_context* ctxWhisper = nullptr;

// Buffer interno para almacenar las muestras capturadas por el micrófono
static vector<float> bufferGrabacion;
static string textoTranscrito = "";

// Contador para detectar bloques de silencio consecutivos
static int framesSilencioConsecutivos = 0;

// Callback de PortAudio para capturar muestras de audio desde el micrófono
static int paCallback(const void* inputBuffer, void* outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void* userData) {
    
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if (inputBuffer == nullptr) return paContinue;

    const float* in = (const float*)inputBuffer;
    
    // 1. Encontrar el nivel de volumen pico en el buffer actual
    float maxVolumen = 0.0f;
    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        float absSample = std::abs(in[i]);
        if (absSample > maxVolumen) {
            maxVolumen = absSample;
        }
    }

    // 2. Solo guardar en el buffer si el volumen supera el umbral
    if (maxVolumen >= UMBRAL_VOLUMEN) {
        for (unsigned long i = 0; i < framesPerBuffer; ++i) {
            bufferGrabacion.push_back(in[i]);
        }
        framesSilencioConsecutivos = 0; // Reinicia el contador al detectar voz
    } else {
        // Si hay silencio pero ya juntó algo de voz previa (mínimo 0.5s de audio):
        if (bufferGrabacion.size() >= SAMPLE_RATE * 0.5) { 
            framesSilencioConsecutivos++;
        } else {
            // Si el buffer tenía muy poco audio (ruido corto), se descarta
            bufferGrabacion.clear();
            framesSilencioConsecutivos = 0;
        }
    }

    return paContinue;
}

// Inicialización de PortAudio y Whisper
void audio_init() {
    if (inicializado) return;

    // 1. Inicializar PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        cerr << "[Audio Error] No se pudo inicializar PortAudio: " << Pa_GetErrorText(err) << endl;
        return;
    }

    // Abrir stream de grabación de micrófono (1 canal mono, float 32-bit)
    err = Pa_OpenDefaultStream(&streamAudio,
                              1,          // 1 canal de entrada (Mono)
                              0,          // 0 canales de salida
                              paFloat32,  // Formato float32 requerido por Whisper
                              SAMPLE_RATE,
                              FRAMES_PER_BUFFER,
                              paCallback,
                              nullptr);

    if (err != paNoError) {
        cerr << "[Audio Error] Error al abrir el stream de entrada de PortAudio: " << Pa_GetErrorText(err) << endl;
        Pa_Terminate();
        return;
    }

    err = Pa_StartStream(streamAudio);
    if (err != paNoError) {
        cerr << "[Audio Error] Error al iniciar el stream: " << Pa_GetErrorText(err) << endl;
        Pa_CloseStream(streamAudio);
        Pa_Terminate();
        return;
    }

    // 2. Inicializar modelo de Whisper (usando la API actualizada)
    struct whisper_context_params cparams = whisper_context_default_params();
    const string rutaModelo = resourcePath("models/ggml-base.bin");
    ctxWhisper = whisper_init_from_file_with_params(rutaModelo.c_str(), cparams);

    if (ctxWhisper == nullptr) {
        cerr << "[Audio Error] No se pudo cargar el modelo de Whisper en: " << rutaModelo << endl;
        Pa_StopStream(streamAudio);
        Pa_CloseStream(streamAudio);
        Pa_Terminate();
        return;
    }

    inicializado = true;
    cout << "[Audio] Módulo de audio inicializado con PortAudio y Whisper." << endl;
}

// Actualización del módulo en el bucle principal
void audio_update() {
    if (!inicializado) return;

    // Condiciones para procesar: alcanzado el límite de 3s O detectado ~1 segundo de silencio (~30 callbacks)
    bool tiempoAgotado = (bufferGrabacion.size() >= SAMPLE_RATE * 3);
    bool silencioDetectado = (framesSilencioConsecutivos >= 30);

    if ((tiempoAgotado || silencioDetectado) && !bufferGrabacion.empty()) {
        whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
        params.print_progress   = false;
        params.print_special    = false;
        params.print_realtime   = false;
        params.print_timestamps = false;
        params.language         = "es";

        if (whisper_full(ctxWhisper, params, bufferGrabacion.data(), bufferGrabacion.size()) == 0) {
            int n_segments = whisper_full_n_segments(ctxWhisper);
            string temp = "";
            for (int i = 0; i < n_segments; ++i) {
                const char* text = whisper_full_get_segment_text(ctxWhisper, i);
                temp += text;
            }

            // Filtrar ruido/música o corchetes
            if (temp.find('[') == string::npos && temp.find('(') == string::npos) {
                textoTranscrito = temp;
            } else {
                textoTranscrito = ""; // Ignorar ruido detectado como música
            }
        }

        // Limpieza tras procesar
        bufferGrabacion.clear();
        framesSilencioConsecutivos = 0;
    }
}

// Retorna el texto obtenido tras procesar la voz con Whisper
string audio_obtenerTexto() {
    if (!inicializado || textoTranscrito.empty()) {
        return "";
    }

    string resultado = textoTranscrito;
    textoTranscrito = ""; // Limpiamos la variable tras entregarla
    return resultado;
}

// Reproducción o procesamiento del texto de respuesta
void audio_hablar(string textoRespuesta) {
    if (!inicializado || textoRespuesta.empty()) return;

    cout << "[Audio] Reproduciendo voz: \"" << textoRespuesta << "\"" << endl;

    // Ejecuta el comando de voz del sistema en español (-l es)
    string comando = "spd-say -l es \"" + textoRespuesta + "\"";
    system(comando.c_str());
}

// Cierre y liberación de recursos
void audio_finish() {
    if (!inicializado) return;

    // Cerrar PortAudio
    if (streamAudio) {
        Pa_StopStream(streamAudio);
        Pa_CloseStream(streamAudio);
        streamAudio = nullptr;
    }
    Pa_Terminate();

    // Liberar contexto de Whisper
    if (ctxWhisper) {
        whisper_free(ctxWhisper);
        ctxWhisper = nullptr;
    }

    inicializado = false;
    cout << "[Audio] Módulo de audio liberado y cerrado correctamente." << endl;
}