# syntax=docker/dockerfile:1

# =============================================================================
# STAGE 1: builder - compila el proyecto con todas las dependencias de dev
# =============================================================================
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git pkg-config ca-certificates \
    libsdl2-dev libsdl2-image-dev \
    portaudio19-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Traer whisper.cpp fijado a un commit concreto (reproducible, no depende
# de que el host ya tenga el submódulo clonado)
ARG WHISPER_CPP_REF=v1.9.3
RUN git clone --depth 1 --branch ${WHISPER_CPP_REF} \
    https://github.com/ggml-org/whisper.cpp external/whisper.cpp

# Copiar el resto del código fuente
COPY CMakeLists.txt ./
COPY src/ ./src/
COPY resources/ ./resources/

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j"$(nproc)"

# =============================================================================
# STAGE 2: runtime - solo las librerías compartidas necesarias para ejecutar
# =============================================================================
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libsdl2-2.0-0 libsdl2-image-2.0-0 \
    libportaudio2 \
    libgomp1 \
    libasound2-plugins pulseaudio-utils \
    speech-dispatcher speech-dispatcher-espeak-ng espeak-ng \
    && rm -rf /var/lib/apt/lists/*

# Redirige el dispositivo ALSA "default" hacia PulseAudio del host en vez de
# pelear por acceso directo al hardware (evita los errores de dmix/dsnoop
# y "Invalid sample rate" al correr dentro de un contenedor).
COPY docker/asound.conf /etc/asound.conf

# Usuario no-root: /dev/snd normalmente exige pertenecer al grupo "audio".
# El docker-compose.yml fuerza el UID/GID del host en runtime (necesario
# para el socket de PulseAudio), lo que pisa el UID interno de "avatar" acá
# creado - por eso /home/avatar queda con un dueño que no coincide y
# speech-dispatcher no puede escribir su config/caché ahí. En vez de pelear
# con UIDs, usamos /tmp como HOME: es escribible por cualquier UID (sticky
# bit, permisos 1777) sin importar quién termine corriendo el proceso.
RUN groupadd -r avatar && useradd -r -g avatar -G audio avatar
ENV HOME=/tmp

WORKDIR /app
COPY --from=builder /build/build/bin/avatar_app ./
COPY --from=builder /build/resources ./resources

# whisper.cpp compila como librerías dinámicas (libwhisper.so, libggml*.so,
# libparakeet.so) que quedan junto al binario en el stage builder. Sin este
# paso el ejecutable falla en runtime con "cannot open shared object file",
# porque la imagen runtime nunca las tuvo.
COPY --from=builder /build/build/bin/*.so* /usr/local/lib/
RUN ldconfig

# El modelo de Whisper (~148MB) NO se copia en la imagen: se monta como
# volumen en tiempo de ejecución (ver docker-compose.yml / README-docker.md).
# resourcePath() en src/util/paths.cpp ya sabe buscarlo vía
# AVATAR_RESOURCES_DIR o en ./resources junto al binario.
ENV AVATAR_RESOURCES_DIR=/app/resources

USER avatar
ENTRYPOINT ["./avatar_app"]
