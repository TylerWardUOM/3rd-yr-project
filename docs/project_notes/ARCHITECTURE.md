---

---
---
## Overview
- Major modules:
	- `env/`, `haptic/`, `viz/`, `world/`, `scene/`, `physics/`, `meshing/`
- Future modules:
	- `device/`
---
## Module Map
``` mermaid
graph LR
H[haptic/] --> S[scene/]
P[physics/] --> W[world/] --> S
E[env/] --> H
S --> V[viz/]
S --> M[meshing/] --> S
D[device/] --> H
H --> P
```
---
## Threading & Data Flow
### Haptic Thread
- Haptic thread reads device buffer
- Reads SDF from env
- Writes to proxy buffer
- Writes to physics command buffer

### Physics/World Thread
- Owns World and writes to `WorldSnapshot`
- Reads and applies physics commands from Haptic Thread
- Steps physics simulation

### Renderer/Scene Thread
- Reads `WorldSnapshot` + `HapticSnapshot`
- Renders bodies from `WorldSnapshot`
- Draws overlays from `HapticSnapshot`
- Manages Viewport controls and Ui
---
