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
        std::vector<glm::vec3> m_particlePos; // Particle m_particlePos
        std::vector<glm::vec3> m_particleVel; // Particle Velocity
        std::vector<glm::vec3> m_particleColor;

        float m_fRatio;
        int   m_iCellX;
        int   m_iCellY;
        int   m_iCellZ;
        float m_h;
        float m_fInvSpacing;
        int   m_iNumCells;

        int   m_iNumSpheres;
        float m_particleRadius;

        std::vector<glm::vec3> m_vel;
        std::vector<glm::vec3> m_pre_vel;
        std::vector<float>     m_near_num[3];

        std::vector<int> m_hashtable;
        std::vector<int> m_hashtableindex;

        std::vector<float> m_p;               // Pressure array
        std::vector<float> m_s;               // 0.0 for solid cells, 1.0 for fluid cells, used to update m_type
        std::vector<int>   m_type;            // Flags array (const int EMPTY_CELL = 0; const int FLUID_CELL = 1; const int SOLID_CELL = 2;)
                                              // m_type = SOLID_CELL if m_s == 0.0;
                                              // m_type = FLUID_CELL if has particle and m_s == 1;
                                              // m_type = EMPTY_CELL if has No particle and m_s == 1;
        std::vector<float> m_particleDensity; // Particle Density per cell, saved in the grid cell
        float              m_particleRestDensity;

        glm::vec3 gravity { 0, -9.81f, 0 };

        void integrateParticles(float timeStep);
        void pushParticlesApart(int numIters);
        void handleParticleCollisions(glm::vec3 obstaclePos, float obstacleRadius, glm::vec3 obstacleVel);
        void updateParticleDensity();

        void        transferVelocities(bool toGrid, float flipRatio);
        void        solveIncompressibility(int numIters, float dt, float overRelaxation, bool compensateDrift);
        void        updateParticleColors();
        inline bool isValidVelocity(int i, int j, int k, int dir);
        inline int  index2GridOffset(glm::ivec3 index);

        void SimulateTimestep(float const dt) {
            int   numSubSteps       = 1;
            int   numParticleIters  = 5;
            int   numPressureIters  = 30;
            bool  separateParticles = true;
            float overRelaxation    = 0.5;
            bool  compensateDrift   = true;

            float     flipRatio = m_fRatio;
            glm::vec3 obstaclePos(0.0f); // obstacle can be moved with mouse, as a user interaction
            glm::vec3 obstacleVel(0.0f);

            float sdt = dt / numSubSteps;

            for (int step = 0; step < numSubSteps; step++) {
                integrateParticles(sdt);
                handleParticleCollisions(obstaclePos, 0.0, obstacleVel);
                if (separateParticles)
                    pushParticlesApart(numParticleIters);
                handleParticleCollisions(obstaclePos, 0.0, obstacleVel);
                transferVelocities(true, flipRatio);
                updateParticleDensity();
                solveIncompressibility(numPressureIters, sdt, overRelaxation, compensateDrift);
                transferVelocities(false, flipRatio);
            }
            updateParticleColors();
        }

        void setupScene(int res) {
            glm::vec3 tank(1.0f);
            glm::vec3 relWater = { 0.6f, 0.8f, 0.6f };

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
            m_iCellX         = res + 1;
            m_iCellY         = res + 1;
            m_iCellZ         = res + 1;
            m_h              = 1.0 / float(res);
            m_fInvSpacing    = float(res);
            m_iNumCells      = m_iCellX * m_iCellY * m_iCellZ;
            m_particleRadius = point_r; // modified

            // update particle array
            m_particlePos.clear();
            m_particlePos.resize(m_iNumSpheres, glm::vec3(0.0f));
            m_particleVel.clear();
            m_particleVel.resize(m_iNumSpheres, glm::vec3(0.0f));
            m_particleColor.clear();
            m_particleColor.resize(m_iNumSpheres, glm::vec3(1.0f));
            m_hashtable.clear();
            m_hashtable.resize(m_iNumSpheres, 0);
            m_hashtableindex.clear();
            m_hashtableindex.resize(m_iNumCells + 1, 0);

            // update grid array
            m_vel.clear();
            m_vel.resize(m_iNumCells, glm::vec3(0.0f));
            m_pre_vel.clear();
            m_pre_vel.resize(m_iNumCells, glm::vec3(0.0f));
            for (int i = 0; i < 3; ++i) {
                m_near_num[i].clear();
                m_near_num[i].resize(m_iNumCells, 0.0f);
            }

            m_p.clear();
            m_p.resize(m_iNumCells, 0.0);
            m_s.clear();
            m_s.resize(m_iNumCells, 0.0);
            m_type.clear();
            m_type.resize(m_iNumCells, 0);
            m_particleDensity.clear();
            m_particleDensity.resize(m_iNumCells, 0.0f);

            // the rest density can be assigned after scene initialization
            m_particleRestDensity = 0.0;

            // create particles
            int p = 0;
            for (int i = 0; i < numX; i++) {
                for (int j = 0; j < numY; j++) {
                    for (int k = 0; k < numZ; k++) {
                        m_particlePos[p++] = glm::vec3(m_h + point_r + dx * i + (j % 2 == 0 ? 0.0 : point_r), m_h + point_r + dy * j, m_h + point_r + dz * k + (j % 2 == 0 ? 0.0 : point_r)) + glm::vec3(-0.5f);
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
                        if (i == 0 || i >= m_iCellX - 2 || j == 0 || j >= m_iCellY - 2 || k == 0 || k >= m_iCellZ - 2)
                            s = 0.0f; // solid
                        m_s[i * n + j * m + k] = s;
                    }
                }
            }
        }
    };
} // namespace VCX::Labs::Fluid