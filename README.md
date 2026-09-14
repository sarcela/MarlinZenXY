# Mesa Zen XY

Firmware personalizado basado en Marlin para una mesa de molienda / acabado tipo Zen, con movimiento en dos ejes, sin hotend y sin cama caliente. Este fork está orientado a control de recorrido XY con lógica de automación y carga aleatoria de archivos G desde SD.

## Descripción del proyecto

Este firmware ya no está pensado para impresora 3D convencional. La máquina objetivo tiene:

- dos ejes activos: X e Y
- sin extrusor
- sin hotend
- sin cama caliente
- control de secuencias automatizadas desde SD
- carga aleatoria de archivos `PATTERN*.g` y `ERRASER*.g`

## Estado actual

- El código base sigue siendo Marlin 2.1.x
- Se han desactivado las funciones de extrusión y temperatura del hotend/cama en la configuración
- La lógica personalizada de carga aleatoria queda como comportamiento principal del sistema

## Compilación

```bash
py -m platformio run -e mega2560
```

## Estructura relevante

- [random_auto_g_file_loader.cpp](random_auto_g_file_loader.cpp): cargador de archivos aleatorios desde SD
- [Marlin/src/MarlinCore.cpp](Marlin/src/MarlinCore.cpp): integración del bucle principal con la lógica de la mesa
- [Marlin/Configuration.h](Marlin/Configuration.h): configuración del firmware para hardware sin extrusión ni calentamiento
- [Marlin/Configuration_adv.h](Marlin/Configuration_adv.h): ajustes avanzados del sistema

## Licencia

Este proyecto conserva la licencia original de Marlin, pero se ha adaptado para una aplicación de mesa Zen específica. Mantener la licencia original es compatible con la obra derivada.
