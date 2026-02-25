# Fluid Simulation

Real-time 2D fluid simulation based on the incompressible Navier-Stokes equations, built on top of [EXIS](https://github.com/vlnraf/Engine). Runs on desktop via OpenGL and in the browser via WebGL.

https://github.com/user-attachments/assets/4ecbab66-e353-4d59-ad81-a72b497a177f


## Features

- MAC (Marker-and-Cell) grid for velocity and pressure fields
- Incompressible fluid solver: advection, pressure projection, divergence-free enforcement
- GPU-accelerated rendering using ping-pong framebuffers and GLSL fragment shaders
- Cross-platform: desktop (OpenGL) and browser (WebGL)
- Interactive mouse input

## How it works

The simulation runs entirely on the GPU. Each step:
1. **Advect** — velocity and density fields are advected using semi-Lagrangian backtracing
2. **Pressure solve** — a Gauss-Seidel iterative solver enforces the divergence-free constraint
3. **Project** — velocities are corrected using the pressure gradient

Rendering uses ping-pong framebuffers: the simulation reads from one texture and writes to another each frame, avoiding read/write conflicts on the GPU.

A full write-up is available on my blog: [Fluid Simulation from Scratch: Navier-Stokes in 2D](https://vlnraf.github.io/en)

## Build

**Desktop (OpenGL):**
```bash
make
```

**Web (WebGL):**
```bash
build_web.bat
```
Then open `game.html` in a browser.
Or just go to
[https://vlnraf.itch.io/fluid-simulator](Demo)

## Dependencies

- [EXIS](https://github.com/vlnraf/Engine) — included as a submodule
- OpenGL / WebGL
- Emscripten (for web build)

```bash
git clone --recurse-submodules https://github.com/vlnraf/fluid-simulation
```
