#include "dialog.h"
#include "../util/paths.h"
#include <iostream>
#include <fstream>
#include <map>
#include <cctype>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// Estado privado del módulo
static map<string, string> baseConocimiento;
static string respuestaActual = ""; // Solo guardamos la respuesta de la interacción actual

void dialog_init() {
    ifstream archivo(resourcePath("dialog/preguntas.json"));
    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir " << resourcePath("dialog/preguntas.json") << endl;
        return;
    }

    try {
        json datosJson;
        archivo >> datosJson;
        baseConocimiento.clear();

        // Leer el array "preguntas"
        if (datosJson.contains("preguntas") && datosJson["preguntas"].is_array()) {
            for (const auto& item : datosJson["preguntas"]) {
                if (item.contains("clave") && item.contains("respuesta")) {
                    string clave = item["clave"].get<string>();
                    string respuesta = item["respuesta"].get<string>();
                    
                    // Convertir la clave a minúsculas para facilitar búsquedas
                    for (char &c : clave) c = tolower(static_cast<unsigned char>(c));
                    
                    baseConocimiento[clave] = respuesta;
                }
            }
        }
    } catch (const exception& e) {
        cerr << "Error al leer el JSON: " << e.what() << endl;
    }

    archivo.close();
    respuestaActual = "";
}

void dialog_update() {
    // En un ciclo síncrono no requiere procesamiento continuo,
    // pero queda disponible para lógica asíncrona futura.
}

void dialog_preguntar(const string& texto) {
    // Si entra una nueva llamada, la respuesta anterior se DESCARTA/SOBREESCRIBE inmediatamente
    string min = texto;
    for (char &c : min) {
        c = tolower(static_cast<unsigned char>(c));
    }

    bool encontrada = false;
    for (const auto& [clave, resp] : baseConocimiento) {
        if (min.find(clave) != string::npos) {
            respuestaActual = resp; // Sobreescribe directamente
            encontrada = true;
            break;
        }
    }

    if (!encontrada) {
        respuestaActual = "Disculpa, no entendí tu consulta.\n";
    }
}

string dialog_obtenerRespuesta() {
    // Retorna la respuesta actual y limpia la variable para la siguiente interacción
    string respuesta = respuestaActual;
    respuestaActual = ""; // Se consume la respuesta
    return respuesta;
}


void dialog_finish(){
    
}