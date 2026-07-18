#include "dialogos.h"
#include <map>
#include <algorithm>

using namespace std;

string procesarRespuestaUniversitaria(string textoUsuario) {
    // 1. Normalizar: Convertir todo a minúsculas para asegurar coincidencias
    transform(textoUsuario.begin(), textoUsuario.end(), textoUsuario.begin(), 
                   [](unsigned char c){ return tolower(c); });

    // 2. Base de datos de conocimiento (Palabra clave -> Respuesta)
    map<string,string> respuestas = {
        {"horario", "Los horarios de cursada del primer cuatrimestre están publicados en la cartelera digital.\n"},
        {"cursa", "Los horarios de cursada del primer cuatrimestre están publicados en la cartelera digital.\n"},
        {"final", "Las fechas de exámenes finales de julio ya se pueden consultar en el SIU Guaraní.\n"},
        {"examen", "Las fechas de exámenes finales de julio ya se pueden consultar en el SIU Guaraní.\n"},
        {"aula", "Las aulas de Algoritmia y Probabilidad se encuentran en el Pabellón B, segundo piso, Aula 204.\n"},
        {"piso", "Las aulas de Algoritmia y Probabilidad se encuentran en el Pabellón B, segundo piso, Aula 204.\n"},
        {"inscripcion", "El periodo de inscripción a materias comienza la segunda semana de agosto.\n"},
        {"anotarme", "El periodo de inscripción a materias comienza la segunda semana de agosto.\n"},
        {"hola", "¡Hola! Bienvenido al asistente de la universidad. ¿En qué te puedo ayudar hoy?\n"},
        {"buen", "¡Hola! Bienvenido al asistente de la universidad. ¿En qué te puedo ayudar hoy?\n"}
    };

    // 3. Buscar coincidencia de palabras clave
    for (const auto& [palabraClave, respuestaAutomatica] : respuestas) {
        if (textoUsuario.find(palabraClave) != string::npos) {
            return respuestaAutomatica; // Retorna la respuesta inmediatamente si la encuentra
        }
    }

    // 4. Respuesta segura por defecto si no entendió
    return "Disculpa, no logré identificar tu consulta. Puedes preguntarme por: horarios, exámenes, inscripción o aulas.\n";
}