#include <cstdio>
#include <algorithm>
#include <string>
#include "dialogos.h" 

int main() {
    bool continuar = true;
    printf("=== ASISTENTE UNIVERSITARIO ACTIVO ===\n");

    while (continuar) {
        // 1. Se obtiene el texto del módulo externo audio
        std::string textoEscuchado = "Busco el aula de algoritmia"; // Ejemplo simulado

        // 2. Evaluamos la salida antes de procesar el diálogo
        std::string textoEvaluar = textoEscuchado;
        std::transform(textoEvaluar.begin(), textoEvaluar.end(), textoEvaluar.begin(), [](unsigned char c){ return std::tolower(c); });
        
        if (textoEvaluar.find("salir") != std::string::npos || textoEvaluar.find("chau") != std::string::npos) {
            printf("\n[Asistente Universitario]: ¡Hasta luego! Éxitos en tus estudios.\n");
            continuar = false; 
        } 
        else if (!textoEscuchado.empty()) {
            // 3. Pasamos el texto al módulo de diálogos y capturamos su retorno
            std::string respuestaAsistente = procesarRespuestaUniversitaria(textoEscuchado);
            
            // 4. Imprimimos el resultado (o lo mandamos a SDL en el futuro)
            printf("\n[Asistente Universitario]: %s", respuestaAsistente.c_str());
            printf("-------------------------------------\n");
        }
    }

    return 0;
}