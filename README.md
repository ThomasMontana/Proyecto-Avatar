# Avatar Virtual - Asistente con reconocimiento de voz

Reestructuración portable del proyecto original. Pensado para compilar en
cualquier Ubuntu limpio, sin depender de vcpkg ni de rutas de una máquina
en particular.

Este documento cubre el build **nativo** (compilar directo en tu Ubuntu); para correr el proyecto dentro de un contenedor ver [README-docker.md](README-docker.md).

> Todos los comandos de este README asumen que estás parado en la raíz
> del repositorio clonado, salvo que se indique lo contrario.

## 1. Clonar el repositorio

```bash
git clone https://github.com/ThomasMontana/Proyecto-Avatar avatar
cd avatar
```

## Arquitectura

```
avatar/
├── CMakeLists.txt          # build único, en la raíz (no adentro de un módulo)
├── README.md
├── external/
│   └── whisper.cpp/        # git submodule (no tiene paquete apt)
├── src/
│   ├── main.cpp
│   ├── audio/               # captura de mic (PortAudio) + STT (whisper.cpp)
│   ├── dialog/               # motor de preguntas/respuestas (nlohmann-json)
│   ├── avatar/                # estados y animación del avatar
│   ├── draw/                  # capa de render SDL2 / SDL2_image
│   └── util/paths.{h,cpp}     # resolución portable de rutas de recursos
└── resources/
    ├── animations/            # sprites + json de cada animación
    ├── dialog/preguntas.json  # base de conocimiento
    └── models/                # acá va ggml-base.bin (no se versiona, ver abajo)
```

Cada módulo (`audio`, `dialog`, `avatar`, `draw`) vive en su propia carpeta
con su `.h`/`.cpp`, y el `CMakeLists.txt` está en la raíz del proyecto,
no adentro de un submódulo - así compila todo el árbol de una sola pasada.

## 2. Dependencias del sistema (Ubuntu / Debian, vía apt)

```bash
sudo apt update
sudo apt install -y \
    build-essential cmake git pkg-config \
    libsdl2-dev libsdl2-image-dev \
    portaudio19-dev \
    nlohmann-json3-dev \
    speech-dispatcher speech-dispatcher-espeak-ng espeak-ng
```

Detalle:
- `libsdl2-dev` / `libsdl2-image-dev`: ventana, render y carga de PNG.
- `portaudio19-dev`: captura de audio del micrófono.
- `nlohmann-json3-dev`: parseo de `preguntas.json` (sí trae un config cmake
  real, por eso el `CMakeLists.txt` puede usar `find_package` normal acá).
- `speech-dispatcher` + `speech-dispatcher-espeak-ng` + `espeak-ng`: proveen
  el comando `spd-say`, que es lo que `audio_hablar()` invoca para la
  síntesis de voz. Los tres son necesarios: `speech-dispatcher` es el
  demonio, `espeak-ng` es el motor de síntesis, y `speech-dispatcher-espeak-ng`
  es el módulo puente que los conecta - sin él, `speech-dispatcher` arranca
  pero no encuentra ningún sintetizador y reproduce un mensaje de
  diagnóstico en vez del audio real.

> Nota técnica: `libsdl2-dev` y `portaudio19-dev` instalados por apt **no**
> generan archivos `.cmake` en modo `CONFIG` (eso es específico de vcpkg).
> Por eso el `CMakeLists.txt` usa `pkg-config` (`find_package(PkgConfig)` +
> `pkg_check_modules`) en vez de `find_package(SDL2 CONFIG REQUIRED)`.

## 3. whisper.cpp (reconocimiento de voz)

No existe paquete apt para whisper.cpp, así que se agrega como submódulo
y se compila junto al proyecto. Este repo **ya tiene el submódulo
registrado** (ver `.gitmodules`), así que al clonar sólo hace falta
descargar su contenido:

```bash
git submodule update --init --recursive
```

(`git submodule add ...` es el comando que se usó una única vez para
registrar el submódulo por primera vez en el repo - no hace falta
volver a correrlo al clonar.)

## 4. Modelo de Whisper

El modelo (`ggml-base.bin`, ~148 MB) **no se versiona** en el repositorio.
Se descarga aparte y se copia a `resources/models/`:

```bash
bash external/whisper.cpp/models/download-ggml-model.sh base
mkdir -p resources/models
cp external/whisper.cpp/models/ggml-base.bin resources/models/
```

## 5. Compilación

```bash
mkdir -p build && cd build
cmake ..
cmake --build . -j"$(nproc)"
cd ..
```

El binario queda en `build/bin/avatar_app`, con `resources/` copiado al
lado automáticamente (paso POST_BUILD del CMakeLists). El `cd ..` final te
deja de nuevo en la raíz del repo, para que el siguiente paso funcione tal
cual está escrito.

## 6. Ejecución

```bash
cd build/bin
./avatar_app
cd ../..
```

(El `cd ../..` al final es para volver a la raíz del repo antes del
siguiente paso opcional; solo se ejecuta después de que cierres la ventana
del avatar.)

### Portabilidad de recursos

`src/util/paths.cpp` resuelve dónde están los recursos con esta prioridad:

1. Variable de entorno `AVATAR_RESOURCES_DIR` (si está seteada).
2. Carpeta `resources/` junto al ejecutable (caso normal en desarrollo).
3. Ruta de instalación fijada en tiempo de compilación
   (`/usr/local/share/avatar/resources` si se hace `cmake --install`).

Esto permite mover el binario a cualquier lado (otra carpeta, otra máquina,
un paquete `.deb`, un contenedor) sin tocar código: sólo hay que asegurarse
de que `resources/` viaje junto, o setear `AVATAR_RESOURCES_DIR`.

## 7. Instalación a nivel de sistema (opcional)

```bash
cd build
sudo cmake --install .
AVATAR_RESOURCES_DIR=/usr/local/share/avatar/resources avatar_app
```

## Tests

Los tests unitarios cubren los módulos de lógica pura (`dialog`, `util/paths`)
que no dependen de hardware real (mic, ventana SDL, modelo de Whisper). Usan
[Catch2](https://github.com/catchorg/Catch2) v3 y están apagados por defecto
para no afectar el build nativo normal ni la imagen Docker.

```bash
sudo apt install -y catch2

mkdir -p build && cd build
cmake .. -DAVATAR_BUILD_TESTS=ON
cmake --build . -j"$(nproc)"
ctest --output-on-failure
cd ..
```

## Notas

- No se debe commitear la carpeta `build/` (ver `.gitignore`) - el proyecto
  original traía objetos `.o` y un `CMakeCache.txt` con rutas absolutas de
  la máquina del autor, lo que rompía cualquier build en otro entorno.
- El modelo `ggml-base.bin` tampoco se commitea por su tamaño; se descarga
  con el script de whisper.cpp (paso 4).
