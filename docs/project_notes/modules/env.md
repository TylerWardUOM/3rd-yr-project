## EnvInterface
- **Path:** `env/EnvInterface.h`
- **Role:** Define a **world-space implicit surface**: signed distance `phi`, gradient `grad`, projection `project`, and transform `update`.
- **Public API:**
	- ~EnvInterface() = default;`
	- `phi`(`dvec3& x`) - Signed distance in **world** space (m) `<0` inside =`0` surface.
	- `grad`(`dvec3& x`) - $\nabla \phi$ in **world** space.
	- `project` (`dvec3& x`) - Returns coordinates of point projected to surface.
## PlaneEnv

## SphereEnv
