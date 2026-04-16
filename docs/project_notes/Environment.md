
## Primitive Shapes

The plan is to for now model the environment as a series of primitive shapes.
We model contact using a **signed distance function** (SDF) for each shape.
And we use this to project the proxy to the surface of these shapes if the device where to go into it.

This is based on the [[A_constraint-based_god-object_method_for_haptic_display.pdf]] that discusses projecting the point of a proxy to the closest point from the device reference to the surface of the virtual object.
### General SDF idea

- **Signed distance**:  
  $$
  \phi(\mathbf{x}) =
  \begin{cases}
  +\,\mathrm{dist}(\mathbf{x}, \partial\Omega) & \text{outside} \\
  0 & \text{on the surface} \\
  -\,\mathrm{dist}(\mathbf{x}, \partial\Omega) & \text{inside}
  \end{cases}
  $$

- **Gradient (future use)**:  
  $\nabla\phi(\mathbf{x})$ points along the **outward normal**.  
  If $\phi$ is a true SDF, then $\|\nabla\phi(\mathbf{x})\| \approx 1$ almost everywhere.

**Projection (closest point) using $\phi$**  
If the proxy is inside ($\phi(\mathbf{x}_p)<0$), snap it to the surface:

$$
\mathbf{x}_{\text{proj}} = \mathbf{x}_p - \phi(\mathbf{x}_p)\,\hat{\mathbf{n}}, 
\quad \hat{\mathbf{n}} = \frac{\nabla\phi(\mathbf{x}_p)}{\|\nabla\phi(\mathbf{x}_p)\|}
\ \ (\text{or recover } \hat{\mathbf{n}} \parallel \mathbf{x}_p - \mathbf{x}_{\text{proj}})
$$

### Plane
Using [[[Christer_Ericson-Real-Time_Collision_Detection-EN.pdf]] I found equations and also example code to define a plane and then project to the surface of a plane.

#### Defining a Plane
- A plane is described in the book like this:
	- "A plane in 3D space can be thought of as a flat surface extending indefinitely in all directions. It can be described in several different ways. For example by: 
		● Three points not on a straight line (forming a triangle on the plane)
		● A normal and a point on the plane
		● A normal and a distance from the origin"
		- [[Christer_Ericson-Real-Time_Collision_Detection-EN.pdf#page=93&selection=137,0,154,39|p.93]]
- The book gives a number of equations for planes
	- where $X$ is any point on the plane, and $P$ is a specific point lying on the plane.
	- point-normal form:$$\mathbf{n} \cdot (X - P) = 0$$

	- Constant–normal form: $$\mathbf{n} \cdot {X} = d, \quad d = \mathbf{n} \cdot P$$
		- If $\mathbf{n}$ is unit length, then $|d|$ is the signed distance from the origin to the plane.

Using these equations it can be seen that in order to store a plane all I need is the unit length normal vector $\mathbf{n}$ and the signed distance from origin $|d|$.

This can be represented in code as a simple struct:
``` cpp
struct Plane{
	Vector n;
	float d;
};
```

#### Project to point on Plane
![[Christer_Ericson-Real-Time_Collision_Detection-EN.pdf#page=166&rect=64,413,472,599|p.166]]
The book describes having a point $Q$ and a plane $\pi$ it projects $Q$ projected onto the plane is given by $R$ this is found by moving Q along the vector $\mathbf{n}$ towards the plane.
$R$ can be given as: $R = Q - t\mathbf{n}$
![[Christer_Ericson-Real-Time_Collision_Detection-EN.pdf#page=165&rect=64,249,474,398|p.165]]
But we are storing the normal $\mathbf{n}$ as a unit vector so we can simplify our expression for $t$ and therefore $R$
$$t=\mathbf{n} \cdot (Q-P), \quad R=Q-t\mathbf{n} $$
We can see from the formula for $t$ that if $Q$ is in front of the plane then $t$ is positive and if negative $Q$ is behind the plane.
This will be a useful relationship for us to determine if we need to project the proxy to the surface of the plane.

We can now see that this fits our general SDF form discussed above where $t = \phi$ 

We can now create a code function to check if a point lies behind plane and if so project to the surface.

```cpp
bool is_behind_plane(Point q, Plane p){
    float phi = Dot(p.n, q) - p.d;
    return (t < 0.0f);
}

Point project_to_plane(Point q, plane p){
    float phi = Dot(p.n, q) - p.d;
    return q - t * p.n
}
```

### Sphere 

#### Defining a Sphere
In [[Christer_Ericson-Real-Time_Collision_Detection-EN.pdf]] a sphere is defined by a centre coordinate and a radius.
This can be represented in code as a simple struct:
``` cpp
struct Sphere{
	Point C; //Sphere Center
	float r; //Sphere radius
}
```

A Sphere can also be represented as a mathematical function:
$${Sphere}(c, r) = \{\, x \in \mathbb{R}^3 \mid \|x - c\| = r \,\}.$$
this shows that a sphere is the set of all points $x$ at a fixed distance $r$ from the centre $c$:
#### Projecting a point onto Sphere surface

For any arbitrary point $p$  the distance to the centre $c$ is: $||p-c||$
The distance from the surface therefore is: $||p-c||-r$
Therefore the SDF function for a sphere is: $$\phi(p)=||p-c||-r$$
The gradient of $\phi$ gives the outward normal direction:
$$
\nabla\phi(p) = \frac{p - c}{\|p - c\|}, \quad \text{for } p \neq c
$$

This is simply the normalized vector from the centre to $p$.
To get from $p$ to the nearest point on the surface, move $p$ by exactly $-\phi(p)$ along the normal direction:

$$
p_{\text{proj}} = p - \phi(p)\,\nabla\phi(p).
$$

Substitute $\phi(p) = \|p - c\| - r$ and $\nabla\phi(p) = \dfrac{p - c}{\|p - c\|}$:

$$
p_{\text{proj}} = p - \big(\|p - c\| - r\big)\,\frac{p - c}{\|p - c\|},
$$

$$
= p - \frac{\|p - c\| - r}{\|p - c\|}\,(p - c),
$$

$$
= c + r\,\frac{p - c}{\|p - c\|}.
$$
Using these equations we can now create code functions to see if a point lies in a sphere and to project it to the surface if needed.

``` cpp
bool is_in_sphere(Point q, Sphere s) {
    float phi = norm(q - s.c) - s.r;  // signed distance
    return (phi < 0.0f);              // inside if negative
}

Point project_to_sphere(Point q, Sphere s) {
    Vec3 v = q - s.c;
    float d = norm(v);
    if (d < 1e-9f) {
        // arbitrary projection direction if at the exact center
        return {s.c.x + s.r, s.c.y, s.c.z};
    }
    return s.c + v * (s.r / d);
}
```
Note if statement to avoid error when diving by zero, if point is in centre of sphere.