#include "FluidSimulator.h"

namespace VCX::Labs::Fluid {
    void Simulator::integrateParticles(float timeStep) {
    }

    void Simulator::pushParticlesApart(int numIters) {
    }

    void Simulator::handleParticleCollisions(glm::vec3 obstaclePos, float obstacleRadius, glm::vec3 obstacleVel) {
    }

    void Simulator::updateParticleDensity() {
    }

    void Simulator::transferVelocities(bool toGrid, float flipRatio) {
    }

    void Simulator::solveIncompressibility(int numIters, float dt, float overRelaxation, bool compensateDrift) {
    }

    void Simulator::updateParticleColors() {
    }

    inline bool Simulator::isValidVelocity(int i, int j, int k, int dir) {
        return false;
    }

    inline int Simulator::index2GridOffset(glm::ivec3 index) {
        return 0;
    }
}

