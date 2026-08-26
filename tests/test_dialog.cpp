#include <catch2/catch_test_macros.hpp>
#include "dialog/dialog.h"
#include "util/paths.h"

#include <cstdlib>

#ifndef TEST_FIXTURES_DIR
#define TEST_FIXTURES_DIR "."
#endif

namespace {

// Inicializa dialog_init() apuntando al fixture propio de tests (no al
// resources/dialog/preguntas.json real), así los casos no dependen del
// contenido de producción.
void inicializarDialogoDePrueba() {
    setenv("AVATAR_RESOURCES_DIR", TEST_FIXTURES_DIR "/resources", 1);
    paths_init(nullptr);
    dialog_init();
}

} // namespace

TEST_CASE("dialog_preguntar encuentra respuesta por substring sin importar mayusculas", "[dialog]") {
    inicializarDialogoDePrueba();

    dialog_preguntar("Decime HOLA por favor");

    REQUIRE(dialog_obtenerRespuesta() == "Hola!");
}

TEST_CASE("dialog_preguntar devuelve el mensaje por defecto si no hay coincidencia", "[dialog]") {
    inicializarDialogoDePrueba();

    dialog_preguntar("esto no matchea ninguna clave del fixture");

    REQUIRE(dialog_obtenerRespuesta() == "Disculpa, no entendí tu consulta.\n");
}

TEST_CASE("dialog_obtenerRespuesta consume la respuesta", "[dialog]") {
    inicializarDialogoDePrueba();

    dialog_preguntar("hola");

    REQUIRE(dialog_obtenerRespuesta() == "Hola!");
    REQUIRE(dialog_obtenerRespuesta() == "");
}

TEST_CASE("una nueva pregunta sobrescribe la respuesta anterior no consumida", "[dialog]") {
    inicializarDialogoDePrueba();

    dialog_preguntar("hola");
    dialog_preguntar("chau");

    REQUIRE(dialog_obtenerRespuesta() == "Chau!");
}

TEST_CASE("dialog_preguntar matchea claves de varias palabras", "[dialog]") {
    inicializarDialogoDePrueba();

    dialog_preguntar("che, como estas hoy?");

    REQUIRE(dialog_obtenerRespuesta() == "Bien, gracias.");
}
