# lector-wav

Lector de metadatos de archivos WAV (PCM 16 bits) en C++17, sin librerías externas.

## Qué hace
- Valida la cabecera RIFF/WAVE y recorre los chunks del archivo (sin asumir un header de 44 bytes).
- Lee frecuencia de muestreo, canales y bits por muestra.
- Carga las muestras en memoria y calcula frames y duración.

## Conceptos aplicados
Formato binario RIFF, PCM, little-endian, complemento a 2, manejo de errores con excepciones.

## Compilar y usar
```bash
g++ -std=c++17 -Wall -Wextra -o wavreader main.cpp
./wavreader
```
Ingresá la ruta del `.wav` cuando lo pida.

## Ejemplo de salida
```
Canales:  2
Frecuencia de muestreo: 44100 Hz
Bits por muestra: 16
Frames: 44100
Duración: 1.00 seg
```

## Próximos pasos
- Escritura de WAV
- Soporte de 8/24 bits y float