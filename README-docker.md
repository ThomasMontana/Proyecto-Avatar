# Dockerización del Avatar Virtual

## Por qué esto no es un "docker run" trivial

Esta app abre una **ventana** (SDL2) y captura del **micrófono** (PortAudio).
Docker aísla al contenedor del hardware y del servidor gráfico del host por
diseño — así que hay que exponerle explícitamente:

1. El socket X11 del host, para que la ventana se pueda dibujar.
2. El dispositivo de audio (`/dev/snd`), para que PortAudio vea el micrófono.

Esto hace que la portabilidad entre distros Linux dependa de que el host
tenga X11 (o XWayland) corriendo — lo cual es cierto en casi cualquier
escritorio Linux (Ubuntu, Fedora, Debian, Arch, Mint, etc.), que es
justamente el "cualquier entorno" que buscás.

## 1. Build de la imagen

```bash
docker compose build
```

Esto compila todo (SDL2, PortAudio, whisper.cpp, nlohmann-json) dentro del
contenedor `builder` y arma una imagen `runtime` liviana que sólo lleva las
librerías compartidas necesarias para ejecutar, no las de desarrollo.

## 2. El modelo de Whisper (~148 MB) se monta, no se hornea en la imagen

Bakear un binario de 148 MB en la imagen la infla y la hace lenta de mover
entre máquinas. En vez de eso, se descarga una vez en el host y se monta
como volumen (ya está en el `docker-compose.yml`):

```bash
git clone --depth 1 https://github.com/ggml-org/whisper.cpp /tmp/whisper.cpp
bash /tmp/whisper.cpp/models/download-ggml-model.sh base
mkdir -p resources/models
cp /tmp/whisper.cpp/models/ggml-base.bin resources/models/
```

## 3. Habilitar acceso a la ventana X11 (una vez por sesión del host)

```bash
xhost +local:docker
```

(Esto abre el socket X11 al contenedor. Es seguro en un equipo de escritorio
personal; en un entorno compartido conviene restringir a un usuario/UID
específico en vez de `+local:docker` global — ver `man xhost`.)

## 4. Ejecutar

```bash
docker compose up
```

Al cerrar la ventana, `docker compose down` limpia el contenedor.

## 5. Notas de portabilidad entre distros

- **X11 vs Wayland**: en distros que corren Wayland puro (algunas instalaciones
  recientes de Fedora/Ubuntu), `xhost`/`/tmp/.X11-unix` funcionan igual porque
  hay una capa de compatibilidad XWayland corriendo por defecto. Si el host
  no tiene XWayland, la ventana no va a aparecer — es una limitación del
  host, no del contenedor.
- **Audio (PulseAudio/PipeWire vs ALSA)**: el compose puentea el socket de
  PulseAudio del host (`$XDG_RUNTIME_DIR/pulse`) en vez de pasar `/dev/snd`
  crudo — casi todas las distros de escritorio actuales (Mint, Ubuntu,
  Fedora, etc.) corren PulseAudio o PipeWire-con-compatibilidad-Pulse por
  debajo, así que este es el camino que funciona en más máquinas sin ajustes.
  Ese socket sólo confía en el usuario dueño de la sesión, por eso hay que
  exportar tu UID/GID antes de levantar el contenedor:
  ```bash
  export UID
  export GID=$(id -g)
  docker compose up
  ```
  Si tu escritorio usa JACK en vez de PulseAudio/PipeWire, este enfoque no
  aplica — es un caso menos común y necesitaría puentear el socket de JACK
  en su lugar.
- **Arquitectura de CPU**: la imagen se compila desde código fuente para la
  arquitectura del host (`docker compose build` compila localmente), así que
  funciona igual en x86_64 o ARM64 sin cambios — no hace falta buildx a menos
  que quieras compilar para una arquitectura distinta a la de la máquina que
  hace el build.

## 6. Reconstruir tras cambios de código

```bash
docker compose build --no-cache
docker compose up
```
