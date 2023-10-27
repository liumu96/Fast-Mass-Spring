# Fast-Mass-Spring

Fast Simulation of Mass-Spring Systems

## Dependencies

- OpenGL, GLFW, GLEW, GLM for rendering.
- OpenMesh for computing normals.
- Eigen for sparse matrix algebra

## Algorithms

---

1. Build Mesh : vertices → faces
2. Build Spring System: build grid
3. Solver:
   1. precompute system matrix
   2. iterations:
   - fix x → solve the optimal d (local)
   - fix d → solve the optima x (global)

---

## The Notes

![](./notes/mass-spring-01.png)
![](./notes/mass-spring-02.png)
![](./notes/mass-spring-03.png)
