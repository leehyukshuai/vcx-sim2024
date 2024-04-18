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
        glm::vec3 m_obstacleColor { 0.0f, 0.0f, 0.0f };

        glm::vec3 gravity { 0, -9.81f, 0 };

        void integrateParticles(float timeStep);
        void pushParticlesApart(int numIters);
        void handleParticleCollisions(glm::vec3 obstaclePos, float obstacleRadius, glm::vec3 obstacleVel);
        void updateParticleDensity();
        void updateCellType();

        void              transferVelocities(bool toGrid, float flipRatio);
        void              solveIncompressibility(int numIters, float dt, float overRelaxation, bool compensateDrift);
        void              updateParticleColors();
        inline int        index2GridOffset(glm::ivec3 index);
        inline int        index2GridOffset(int x, int y, int z);
        inline glm::ivec3 position2CellIndex(glm::vec3 pos);
        inline int        position2GridOffset(glm::vec3 pos);

        void initialize();

        void SimulateTimestep(float dt) {
            int   numParticleIters  = 5;
            int   numPressureIters  = 30;
            bool  separateParticles = true;
            float overRelaxation    = 0.5;
            bool  compensateDrift   = true;

            m_iterCounter++;
            if (dt > 0.04f) {
                m_busyCounter++;
                dt = 0.02f;
            }
            m_busyFlag = (1.0 * m_busyCounter / m_iterCounter) > 0.5;

            integrateParticles(dt);
            handleParticleCollisions(m_obstaclePos, 0.0, m_obstacleVel);
            if (separateParticles)
                pushParticlesApart(numParticleIters);
            handleParticleCollisions(m_obstaclePos, m_obstacleRadius, m_obstacleVel);
            transferVelocities(true, m_fRatio);
            updateParticleDensity();
            solveIncompressibility(numPressureIters, dt, overRelaxation, compensateDrift);
            transferVelocities(false, m_fRatio);
            updateParticleColors();
        }

        void setupScene(int res) {
            glm::vec3 tank(1.0f);
            glm::vec3 relWater = { 0.4f, 1.0f, 1.0f };

            float _h      = tank.y / res;
            float point_r = 0.3 * _h;
            float dx      = 2.0 * point_r;
            float dy      = sqrt(3.0) / 2.0 * dx;
            float dz      = dx;

            int numX = floor((relWater.x * tank.x - 2.0 * _h - 2.0 * point_r) / dx);
            int numY = floor((relWater.y * tank.y - 2.0 * _h - 2.0 * point_r) / dy);
            int numZ = floor((relWater.z * tank.z - 2.0 * _h - 2.0 * point_r) / dz);

            // update object member attributes
            m_iNumSpheres    = numX * numY * numZ;
            m_iCellX         = res;
            m_iCellY         = res;
            m_iCellZ         = res;
            m_h              = 1.0 / float(res);
            m_fInvSpacing    = float(res);
            m_iNumCells      = m_iCellX * m_iCellY * m_iCellZ;
            m_particleRadius = point_r; // modified

            // update particle array
            m_particlePos.clear();
            m_particlePos.resize(m_iNumSpheres, glm::vec3(0.0f));
            m_particleVel.clear();
            m_particleVel.resize(m_iNumSpheres, glm::vec3(0.0f));
            m_particlePreVel.clear();
            m_particlePreVel.resize(m_iNumSpheres, glm::vec3(0.0f));
            m_particleCurVel.clear();
            m_particleCurVel.resize(m_iNumSpheres, glm::vec3(0.0f));
            m_particleColor.clear();
            m_particleColor.resize(m_iNumSpheres, glm::vec3(1.0f));
            m_cid2pids.resize(m_iNumCells);

            // update grid array
            for (int i = 0; i < 3; ++i) {
                m_vel[i].clear();
                m_vel[i].resize(m_iNumCells, 0.0f);
                m_pre_vel[i].clear();
                m_pre_vel[i].resize(m_iNumCells, 0.0f);
                m_vel_divisor[i].clear();
                m_vel_divisor[i].resize(m_iNumCells, 0.0f);
            }

            m_p.clear();
            m_p.resize(m_iNumCells, 0.0);
            m_s.clear();
            m_s.resize(m_iNumCells, 0.0);
            m_type.clear();
            m_type.resize(m_iNumCells, EMPTY_CELL);
            m_particleDensity.clear();
            m_particleDensity.resize(m_iNumCells, 0.0f);

            // the rest density can be assigned after scene initialization
            m_particleRestDensity = 0.0;

            // create particles
            int p = 0;
            for (int i = 0; i < numX; i++) {
                for (int j = 0; j < numY; j++) {
                    for (int k = 0; k < numZ; k++) {
                        m_particlePos[p++] = glm::vec3(m_h + point_r + dx * i + (j % 2 == 0 ? 0.0 : point_r), m_h + point_r + dy * j, m_h + point_r + dz * k + (j % 2 == 0 ? 0.0 : point_r));
                    }
                }
            }
            // setup grid cells for tank
            int n = m_iCellY * m_iCellZ;
            int m = m_iCellZ;

            for (int i = 0; i < m_iCellX; i++) {
                for (int j = 0; j < m_iCellY; j++) {
                    for (int k = 0; k < m_iCellZ; k++) {
                        float s = 1.0; // fluid
                        if (i == 0 || i == m_iCellX - 1 || j == 0 || j == m_iCellY - 1 || k == 0 || k == m_iCellZ - 1)
                            s = 0.0f; // solid
                        m_s[i * n + j * m + k] = s;
                    }
                }
            }

            updateCellType();
            updateParticleDensity();
            updateParticleColors();
        }
    };
} // namespace VCX::Labs::Fluid