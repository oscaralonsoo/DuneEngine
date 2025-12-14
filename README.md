# Dune Engine

Dune Engine is a 3D game engine developed using **SDL3** and **OpenGL**, created as an academic project with the goal of learning how to build a game engine and later use it to develop a game.  
It follows an **editor-based workflow inspired by Unity**, allowing visual interaction with scenes, GameObjects and assets.

The engine is currently in development, but it already includes several solid features with a promising future direction.

🔗 **GitHub repository**  
https://github.com/oscaralonsoo/DuneEngine

🌐 **Engine website**  
https://oscaralonsoo.github.io/DuneEngine/

---

## Team Members

- **Javier Gómez González** — [@javiergg14](https://github.com/javiergg14)
- **Oscar Alonso Camenforte** — [@oscaralonsoo](https://github.com/oscaralonsoo)
- **Toni Llovera Roca** — [@ToniLlovera](https://github.com/ToniLlovera)

---

## Platform Support

- Windows  
- macOS  
- Linux  

(Cross-platform support via **CMake**)

---

## How to Use the Engine

### Running the Engine

- Open the executable.
- An OpenGL window (800×600) is created with depth test enabled.
- SDL3, DevIL and Assimp are initialized.
- GPU and OpenGL version information is printed in the console.
- The scene **StreetEnvironment.fbx** is automatically loaded at startup.

---

### Camera Controls (Editor Mode)

| Action | Key / Mouse |
|------|------------|
| Move camera | W / A / S / D (Right Mouse Button) |
| Increase movement speed | SHIFT |
| Look around | Right Mouse Button + Mouse |
| Zoom | Mouse Wheel (Right Mouse Button) |
| Orbit around object | ALT + Right Mouse Button |
| Focus object | F |
| Exit | ESC |

---

## Scene Interaction

- **Left click** to select GameObjects using raycasting and AABB tests.
- Drag & drop while holding left click to reparent objects.
- GameObjects can be:
  - Renamed
  - Deleted
  - Duplicated
  - Reparented
- Creation of empty GameObjects.
- Creation of primitive meshes:
  - Quad
  - Cube
  - Sphere
  - Plane
  - Cylinder
  - Cone
  - Torus
  - Capsule

---

## Gizmos & Inspector

- **W / E / R** to switch between Translate, Rotate and Scale gizmos.
- Transform values can be edited directly in the Inspector.
- Textures can be applied by drag & drop:
  - Onto objects in the Scene
  - Or directly in the Inspector

---

## Asset Management

### Supported Formats

**Models**
- `.fbx`, `.obj`, `.dae`, `.gltf`, `.glb`, `.3ds`, `.ply`, `.blend`

**Textures**
- `.png`, `.jpg`, `.jpeg`, `.tga`, `.bmp`, `.psd`, `.gif`, `.hdr`, `.pic`

### Features

- Project / Assets window with folder navigation.
- Asset search and file visualization.
- Right-click context menus for creating folders, renaming and deleting assets.
- Deleting an asset also removes its associated files from `/Library`.
- Import settings are stored in `.meta` files.
- The `/Library` folder is regenerated on startup from `/Assets` and `.meta` data.
- Resource reference counting ensures assets are loaded only once in memory.

---

## Play, Pause & Stop

- Located in the **Scene Panel**, above the Scene view.

**Play**
- Switches to Game View.
- Uses the Main Camera.
- Editor camera movement is disabled.

**Pause**
- Pauses the simulation.

**Stop**
- Stops the simulation and restores the initial state.

Additional controls:
- Simulation speed control
- Step-by-step simulation
- Wireframe view toggle

---

## Rendering & Debug

- AABB bounding boxes are always visible in Editor Mode.
- Raycast debug visualization is always active in Scene View.
- Frustum Culling is applied and visualized.
- Octree is used as an acceleration structure for selection and culling.
- Wireframe rendering can be toggled from the Scene Panel.

---

## Assignment Requirements Implemented

- Automatic loading of `StreetEnvironment.fbx`
- Full GameObject editing via Inspector
- Hierarchy editing (create, delete, reparent, empty objects)
- Transform editing (position, rotation, scale)
- Mesh and texture drag & drop
- Configurable Camera component
- Mouse-based GameObject selection
- AABB bounding volumes
- Frustum Culling with debug visualization
- Octree acceleration structure
- Scene serialization and loading
- Custom Library system
- Play / Pause / Stop simulation
- Asset management with reference counting
- Wireframe visualization

---

## Additional Features

- Asset explorer with search functionality
- Folder navigation and file preview
- Asset deletion with Library cleanup
- Import options saved in `.meta` files
- Simulation speed and step control

---

## Dependencies

- **SDL3** — Window and input handling
- **GLAD** — OpenGL function loader
- **GLM** — Mathematics library
- **Assimp** — Model loading
- **DevIL** — Image loading

---

## Notes for the Professor

The workload was distributed evenly among the team members based on individual interests.  
Project organization and task management were handled using **Trello**.
