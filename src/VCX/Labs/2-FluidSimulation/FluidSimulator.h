#pragma once

#include <algorithm>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <glm/glm.hpp>
#include <iostream>
#include <utility>
#include <vector>

namespace VCX::Labs::Fluid {
    struct Simulator {
        bool      m_busyFlag { false };
        long long m_iterCounter { 0 };
        long long m_busyCounter { 0 };

        std::vector<glm::vec3> m_particlePos;    // Particle m_particlePos
        std::vector<glm::vec3> m_particleVel;    // Particle Velocity
        std::vector<glm::vec3> m_particlePreVel; // Particle Velocity from cell
        std::vector<glm::vec3> m_particleCurVel; // Particle Velocity from cell
        std::vector<glm::vec3> m_particleColor;

        float m_fRatio { 0.95f };
        int   m_iCellX;
        int   m_iCellY;
        int   m_iCellZ;
        float m_h;
        float m_fInvSpacing;
        int   m_iNumCells;

        int   m_iNumSpheres;
        float m_particleRadius;

        std::vector<float> m_vel[3];
        std::vector<float> m_pre_vel[3];
        std::vector<float> m_vel_divisor[3];

        std::vector<std::vector<int>> m_cid2pids;

        std::vector<float> m_p; // Pressure array
        std::vector<float> m_s; // 0.0 for solid cells, 1.0 for fluid cells, used to update m_type
        enum Type { EMPTY_CELL,
                    FLUID_CELL,
                    SOLID_CELL };
        std::vector<Type>  m_type;
        std::vector<float> m_particleDensity; // Particle Density per cell, saved in the grid cell
        float              m_particleRestDensity;

        glm::vec3 m_obstaclePos { 0.5f }; // obstacle can be moved with mouse, as a user interaction
        glm::vec3 m_obstacleVel { 0.0f };
        float     m_obstacleRadius { 0.15f };
        glm::vec3 m_obstacleColor { 0.5f, 1.0f, 0.3f };

        glm::vec3 gravity { 0, -9.81f, 0 };

        void integrateParticles(float timeStep);
        void pushParticlesApart(int numIters);
        void handleParticleCollisions(glm::vec3 obstaclePos, float obstacleRadius, glm::vec3 obstacleVel);
        void updateParticleDensity();

        void              transferVelocities(bool toGrid, float flipRatio);
        void              solveIncompressibility(int numIters, float dt, float overRelaxation, bool compensateDrift);
        void              updateParticleColors();
        inline int        index2GridOffset(glm::ivec3 index);
        inline int        index2GridOffset(int x, int y, int z);
        inline glm::ivec3 position2CellIndex(glm::vec3 pos);
        inline int        position2GridOffset(glm::vec3 pos);

        void SimulateTimestep(float dt);
        void setupScene(int res);
        void initialize();
    };
} // namespace VCX::Labs::Fluid