# Predictive Input Buffer (recreación para Geode)

Recreación del mod **Predictive Input Buffer** a partir del log de consola de la v4.7.
No es el código original: es una reimplementación del comportamiento observado.

## Comportamiento

- Los inputs (press / release) se encolan y se aplican **N ticks después** (1 tick = 1/240 s).
- **F6** cicla las etapas dentro del nivel:

| Etapa | Ticks (todo) | Ticks (wave) |
|-------|--------------|--------------|
| 1     | 2            | 2            |
| 2     | 4            | 4            |
| 3     | 8            | 8            |
| 4     | 16           | 8            |
| OFF   | input crudo  | input crudo  |

## Cómo obtener el archivo `.geode` (la forma fácil, sin instalar nada)

1. Crea un repositorio nuevo en GitHub y sube todo el contenido de esta carpeta.
2. Ve a la pestaña **Actions** → se ejecuta *Build Geode Mod* (o lánzalo con *Run workflow*).
3. Al terminar (unos minutos), descarga el artefacto **Build Output**: dentro está `tuusuario.predictive-input-buffer.geode`.
4. Copia ese archivo a `...\Geometry Dash\geode\mods\` y abre el juego.

## Compilar en local (Windows)

1. Instala la CLI de Geode: https://docs.geode-sdk.org/getting-started/geode-cli
2. `geode sdk install` y `geode sdk install-binaries`
3. Dentro de esta carpeta: `geode build`
4. El `.geode` aparece en `build/`.

## Antes de publicar

- Cambia `tuusuario` / `TuUsuario` en `mod.json` por tu usuario.
- El código usa `GJBaseGameLayer::handleButton` y `processCommands` para GD **2.2081**.
  Si una versión nueva de Geode cambia las firmas (por ejemplo `dispatchKeyboardMSG`), ajústalas.
- Código **sin probar en el juego**: puede requerir retoques de sincronización de ticks.
- Si vas a compartirlo, respeta la licencia/permiso del autor del mod original.
