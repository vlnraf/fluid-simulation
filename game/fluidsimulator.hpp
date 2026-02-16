#ifndef FLUID_SIMULATOR
#define FLUID_SIMULATOR

#include "core.hpp"

struct FluidSimulator{
    Arena* a;
    float* p;
    float* dens;
    float* densTemp;
    float* v; //vertical velocities
    float* u; //horizontal velocities
    float* vTemp; //vertical velocities
    float* uTemp; //horizontal velocities
    bool* solid;

    float density;
    int gridSizeX;
    int gridSizeY;
    int cellSize;
};

void drawVelocities(FluidSimulator* fs);
void drawGrid(FluidSimulator* fs);
void drawDivergence(FluidSimulator* fs);
void drawDensity(FluidSimulator* fs);
void drawPressure(FluidSimulator* fs);

FluidSimulator createFluidSimulator(Arena* a, int countX, int countY, int cellSize);
int ij(FluidSimulator* fs, int i, int j);
int uij(FluidSimulator* fs, int i, int j);
int vij(FluidSimulator* fs, int i, int j);

void step(FluidSimulator* fs, float dt);

#endif