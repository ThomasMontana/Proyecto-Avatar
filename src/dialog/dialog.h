#pragma once
#include <string>

using namespace std;

// Inicializa las variables internas y la base de conocimiento
void dialog_init();

// Función de actualización si se requiere en el ciclo principal
void dialog_update();

// Recibe la pregunta del usuario y procesa la respuesta correspondiente
void dialog_preguntar(const string& texto);

// Retorna la siguiente respuesta en cola (o string vacío si no hay más)
string dialog_obtenerRespuesta();

