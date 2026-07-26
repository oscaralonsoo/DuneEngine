# DuneEngine

Visualizador de geometría en **OpenGL 3.3** con carga de modelos mediante **Assimp**, texturas con **DevIL** y una cámara tipo **Unity/FPS**.  
Soporta **drag & drop** de modelos y texturas, selección por *raycast* y transformaciones simples sobre el objeto seleccionado.

Repositorio: [https://github.com/oscaralonsoo/DuneEngine](https://github.com/oscaralonsoo/DuneEngine)

---

## 👥 Integrantes del grupo

| Nombre | GitHub |
|--------|--------|
| **Javier Gómez González** | [@javiergg14](https://github.com/javiergg14) |
| **Oscar Alonso Camenforte** | [@oscaralonsoo](https://github.com/oscaralonsoo) |
| **Toni Llovera Roca** | [@ToniLlovera](https://github.com/ToniLlovera) |

---

## 🚀 Cómo usar el motor

1. **Abrir el ejecutable**  
   - Se crea una ventana OpenGL (800×600) con *depth test* activado.  
   - Se inicializan SDL3, DevIL y Assimp.  
   - Se muestran logs de versión y GPU en consola.

2. **Cargar modelos y texturas**
   - Al iniciar se carga automáticamente la *Baker House* con su textura (`Baker_house.png`).
   - Puedes **arrastrar y soltar (drag & drop)**:
     - **Modelos**: `.fbx`, `.obj`, `.dae`, `.gltf`, `.glb`, `.3ds`, `.ply`, `.blend`.
     - **Texturas**: `.png`, `.jpg/jpeg`, `.tga`, `.bmp`, `.psd`, `.gif`, `.hdr`, `.pic`.
   - Los modelos se instancian **frente a la cámara** (~2.5 unidades).  
   - Las texturas se aplican al objeto bajo el cursor o al seleccionado.

3. **Selección**
   - Click izquierdo para seleccionar un objeto.  
   - Se usa *raycast* con prueba AABB para determinar el objeto bajo el cursor.

4. **Render**
   - Los modelos se renderizan con su *shader* asociado.  
   - Cada objeto puede usar materiales del modelo o una textura de **override** si se ha arrastrado una imagen sobre él.

---

## 🎮 Controles

### Cámara
| Acción | Tecla / Ratón |
|--------|----------------|
| Mover cámara | **W / A / S / D** (con botón derecho pulsado) |
| Aumentar velocidad de movimiento | **Mantener SHIFT** |
| Mirar alrededor | **Botón derecho + mover ratón** |
| Zoom | **Rueda del ratón** (con botón derecho pulsado) |
| Orbitar alrededor del objeto | **ALT + Botón derecho** |
| Centrar cámara en objeto | **F** |
| Salir | **ESC** |

### Selección y transformación
| Acción | Tecla |
|--------|-------|
| Seleccionar objeto | **Click izquierdo** |
| Escalar | **Flechas ↑ / ↓** |
| Rotar (eje Y) | **Flechas ← / →** |
| Mover (X/Y) | **W / A / S / D** |
| Mover (Z) | **Q / E** |

### Visualización
| Acción | Tecla |
|--------|-------|
| Modo wireframe | **1** |
| Modo sólido | **2** |

---

## 🧩 Funcionalidades principales

- **Carga de modelos** mediante *Assimp* (`fbx`, `obj`, `gltf`, etc.).  
- **Carga de texturas** mediante *DevIL* (`png`, `jpg`, `hdr`, etc.).  
- **Drag & Drop** de modelos y texturas directamente sobre la ventana.  
- **Selección de objetos** por *raycast* y prueba de colisión AABB.  
- **Cámara** con modo libre FPS, zoom con rueda, órbita y enfoque al objeto.  
- **Transformaciones** básicas de traslación, rotación y escala.  
- **Shader system** con carga desde archivo y *uniform helpers*. 
- **Interfaz de usuario (UI)** con **ImGui**, que incluye:
  - Ventanas acoplables (DockSpace)  
  - **Jerarquía** de escena  
  - **Inspector** con transformaciones, información del mesh y textura  
  - **Consola** con logs filtrables y scroll automático  
  - **Panel de configuración** con información de GPU, FPS y escena  
  - **Toolbar** para añadir primitivas (como cubos)  
  - **Menú superior** (File / View / Help) con enlaces a la documentación del GitHub  
- **Logs** de GPU, versión OpenGL y errores de carga. 

---

## 🌟 Funcionalidades extra

- **Modelos con materiales o con textura única:**  
  Los modelos pueden usar sus materiales originales o una textura de *override* si se arrastra una imagen sobre ellos.  
  Esto permite visualizar un modelo con su material o con una textura simple, ideal para depurar UVs.

- **Colocación automática** de nuevos modelos frente a la cámara al soltarlos.  

---

## ⚙️ Dependencias principales

- [**SDL3**](https://github.com/libsdl-org/SDL) — Ventana e input  
- [**GLAD**](https://glad.dav1d.de/) — Carga de funciones OpenGL  
- [**GLM**](https://github.com/g-truc/glm) — Matemáticas  
- [**Assimp**](https://github.com/assimp/assimp) — Carga de modelos  
- [**DevIL**](https://github.com/DentonW/DevIL) — Carga de imágenes  
- [**ImGui**](https://github.com/ocornut/imgui) — Interfaz gráfica para herramientas y depuración

---

## 🧱 Estado actual del proyecto

Actualmente, **la arquitectura y modularización del proyecto están en proceso de completarse**.  
Una vez finalizada esta etapa, **se realizará la migración del código** para adaptarlo a la nueva estructura, facilitando su mantenimiento, escalabilidad y futuras ampliaciones del motor.


