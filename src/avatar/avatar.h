#ifndef AVATAR_H
#define AVATAR_H

#include <stdbool.h>

// Estados del avatar correspondientes a las animaciones de la imagen
typedef enum {
    ESTADO_REPOSO,
    ESTADO_ESCUCHANDO,
    ESTADO_HABLANDO
} EstadoAvatar;

// Inicializa las 3 animaciones desde sus archivos PNG y JSON
bool avatar_init(void);

// Actualiza el frame actual según el tiempo transcurrido y dibuja el avatar
void avatar_update(void);

// Cambia el estado actual (REPOSO, ESCUCHANDO, HABLANDO)
void avatar_setEstado(EstadoAvatar nuevoEstado);

// Libera las texturas y la memoria asignada
void avatar_finish(void);

#endif 