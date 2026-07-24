# Proyecto-Avatar

Avatar virtual desarrollado para la Facultad de Morón. El objetivo es contar con un asistente universitario con una interfaz gráfica animada (el "avatar"), capaz de escuchar consultas por voz, interpretarlas y responder preguntas frecuentes de los alumnos (horarios, exámenes, inscripciones, ubicación de aulas, etc.).

## Estado actual

El proyecto está en una etapa temprana de prototipo. Existen tres módulos con distintos niveles de avance:

- **Diálogo**: funcional, basado en reglas simples.
- **Renderizado gráfico (SDL3)**: funcional a nivel de infraestructura (ventana, texturas, eventos), pero aún no integrado con el resto de la lógica.
- **Audio/reconocimiento de voz**: sin implementar todavía.

## Dependencias/Requisitos

### Programas / herramientas (build tooling)

- CMake ≥ 3.10 - sistema de generación de build.
- Un compilador de C++17 (MSVC, GCC o Clang).
- Un gestor de paquetes C++ capaz de resolver `find_package(... CONFIG REQUIRED)`, como vcpkg (o Conan). *(El repositorio no incluye actualmente un `vcpkg.json`/`conanfile.txt`, así que esto es una suposición a partir de cómo está escrito `CMakeLists.txt`; falta confirmarlo o agregar el manifiesto correspondiente.)*

### Librerías C++ (declaradas en CMakeLists.txt vía find_package)

- SDL3 - ventana, renderer, eventos de teclado (usada en draw.cpp).
- SDL3_image - carga de texturas/imágenes (IMG_LoadTexture en draw.cpp).
- nlohmann_json - parseo de JSON (previsiblemente para leer Avatar.json, aunque ningún .cpp actual la usa todavía).

Ejemplo de instalación con vcpkg:

```bash
vcpkg install sdl3 sdl3-image nlohmann-json
```

## Estructura del proyecto

```
Proyecto UM AVATAR/
├── main.cpp         # Punto de entrada del programa
├── dialog.cpp/.h     # Lógica de procesamiento de diálogo
├── draw.cpp/.h       # Renderizado gráfico con SDL3 (clase Renderizador)
├── audio.cpp/.h      # Módulo de audio (pendiente de implementación)
├── Avatar.png        # Spritesheet del avatar
├── Avatar.json       # Atlas de sprites (formato Piskel/TexturePacker)
└── CMakeLists.txt    # Configuración de build
```

## Recursos

`Avatar.png` + `Avatar.json` forman un spritesheet de 224×256 px con 53 frames de 32×32 px cada uno, exportado con [Piskel](https://github.com/piskelapp/piskel/), pensado para animar al avatar en pantalla.

## Módulos

### `main.cpp`

Punto de entrada de la aplicación. Implementa un bucle principal que:

1. Obtiene un texto (por ahora simulado con una cadena fija, a la espera de la integración con el módulo de audio).
2. Evalúa si el usuario pidió salir ("salir", "chau").
3. Envía el texto al módulo de diálogo y muestra la respuesta por consola.

### `dialog.cpp`

Contiene `procesarRespuestaUniversitaria()`, que normaliza el texto del usuario y busca coincidencias contra un diccionario de palabras clave (horarios, finales, aulas, inscripción, saludos) para devolver una respuesta predefinida. Si no encuentra ninguna coincidencia, devuelve un mensaje por defecto.

### `draw.cpp`

Implementa la clase `Renderizador`, encargada de la capa gráfica con **SDL3** y **SDL3_image**:

- Inicialización de ventana y renderer.
- Manejo de eventos de teclado y de cierre de ventana.
- Carga y liberación de texturas.
- Dibujo de sprites (recorte de región origen/destino, útil para animar el avatar usando `Avatar.json`).

### `audio.cpp`

Actualmente vacío. Es el módulo previsto para capturar y/o reconocer voz y alimentar con texto real al módulo de diálogo, reemplazando el texto simulado de `main.cpp`.

## Compilación

> **Nota:** `CMakeLists.txt` actualmente solo compila `main.cpp` y hace referencia a un `Avatar.cpp` que no existe en el repositorio, por lo que CMake fallará en la etapa de generación (`configure`) hasta corregirlo. Para que el build incluya toda la funcionalidad (diálogo y renderizado), falta agregar `dialog.cpp` y `draw.cpp` a `add_executable`, y corregir o eliminar la referencia a `Avatar.cpp`. Ver [Próximos pasos](#próximos-pasos).

Con las dependencias de la sección [Dependencias/Requisitos](#dependenciasrequisitos) instaladas:

```bash
cmake -B build -S "Proyecto UM AVATAR"
cmake --build build
```

## Ejecución

Tras compilar, el binario `avatar_test` queda en el directorio de build (p. ej. `build/` o `build/Debug/` según el generador usado), junto con `Avatar.png` y `Avatar.json`, que se copian automáticamente ahí como parte del build:

```bash
./build/avatar_test
```

## Próximos pasos

- Corregir el `CMakeLists.txt` para que compile todos los módulos existentes (ver nota en [Compilación](#compilación)).
- Implementar `audio.cpp` (captura y/o reconocimiento de voz).
- Integrar el `Renderizador` (`draw.cpp`) con el bucle principal para mostrar el avatar animado en pantalla.
- Reemplazar el texto simulado en `main.cpp` por la entrada real de audio.
