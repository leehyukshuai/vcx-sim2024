#include "FluidSimulator.h"

namespace VCX::Labs::Fluid {
    void Simulator::integrateParticles(float timeStep) {
        for (int i = 0; i < m_iNumSpheres; ++i) {
            m_particleVel[i] += timeStep * gravity;
            m_particlePos[i] += timeStep * m_particleVel[i];
        }
    }

    void Simulator::pushParticlesApart(int numIters) {
        for (auto & ps : m_cid2pids) {
            ps.clear();
        }
        for (int pid = 0; pid < m_iNumSpheres; ++pid) {
            int cid = position2GridOffset(m_particlePos[pid]);
            m_cid2pids[cid].push_back(pid);
        }
        float minDist  = m_particleRadius * 2;
        float minDist2 = minDist * minDist;
        while (numIters--) {
            for (int pid = 0; pid < m_iNumSpheres; ++pid) {
                auto &     pos   = m_particlePos[pid];
                glm::ivec3 index = position2CellIndex(pos);
                int        x0    = glm::max(index.x - 1, 0);
                int        y0    = glm::max(index.y - 1, 0);
                int        z0    = glm::max(index.z - 1, 0);
                int        x1    = glm::min(index.x + 1, m_iCellX - 1);
                int        y1    = glm::min(index.y + 1, m_iCellY - 1);
                int        z1    = glm::min(index.z + 1, m_iCellZ - 1);
                for (int x = x0; x <= x1; ++x) {
                    for (int y = y0; y <= y1; ++y) {
                        for (int z = z0; z <= z1; ++z) {
                            glm::ivec3   _index     = glm::ivec3(x, y, z);
                            int          _cid       = index2GridOffset(_index);
                            const auto & _particles = m_cid2pids[_cid];
                            for (int _pid : _particles) {
                                if (pid == _pid) continue;
                                auto & _pos  = m_particlePos[_pid];
                                auto   delta = _pos - pos;
                                float  dist2 = glm::dot(delta, delta);
                                if (dist2 == 0.0f || dist2 >= minDist2) continue;
                                float dist = glm::sqrt(dist2);
                                float dt   = (minDist - dist) * 0.5f;
                                delta      = delta / dist * dt;
                                _pos += delta;
                                pos -= delta;
                            }
                        }
                    }
                }
            }
        }
    }

    void Simulator::handleParticleCollisions(glm::vec3 obstaclePos, float obstacleRadius, glm::vec3 obstacleVel) {
        float minDist  = m_particleRadius + obstacleRadius;
        float minDist2 = minDist * minDist;
        float minX     = m_h + m_particleRadius;
        float minY     = m_h + m_particleRadius;
        float minZ     = m_h + m_particleRadius;
        float maxX     = m_h * (m_iCellX - 1) - m_particleRadius;
        float maxY     = m_h * (m_iCellY - 1) - m_particleRadius;
        float maxZ     = m_h * (m_iCellZ - 1) - m_particleRadius;
        for (int pid = 0; pid < m_iNumSpheres; ++pid) {
            auto & pos = m_particlePos[pid];
            auto & vel = m_particleVel[pid];

            // // obstacle
            auto  delta = pos - obstaclePos;
            float dist2 = glm::dot(delta, delta);
            if (dist2 < minDist2) {
                float dist = glm::sqrt(dist2);
                auto  norm = delta / dist;
                float dt   = minDist - dist;
                pos += norm * dt;
                vel = obstacleVel;
            }

            // wall
            if (pos.x < minX) {
                pos.x = minX;
                vel.x = 0;
            }
            if (pos.y < minY) {
                pos.y = minY;
                vel.y = 0;
            }
            if (pos.z < minZ) {
                pos.z = minZ;
                vel.z = 0;
            }
            if (pos.x > maxX) {
                pos.x = maxX;
                vel.x = 0;
            }
            if (pos.y > maxY) {
                pos.y = maxY;
                vel.y = 0;
            }
            if (pos.z > maxZ) {
                pos.z = maxZ;
                vel.z = 0;
            }
        }
    }

    void Simulator::updateParticleDensity() {
        m_particleDensity.assign(m_iNumCells, 0);
        float hh = m_h * 0.5f;
        for (int pid = 0; pid < m_iNumSpheres; ++pid) {
            const auto & pos    = m_particlePos[pid];
            auto         index0 = glm::ivec3(glm::floor((pos - glm::vec3(hh)) * m_fInvSpacing));
            auto         index1 = index0 + glm::ivec3(1);
            auto         t      = (pos - glm::vec3(hh) - glm::vec3(index0) * m_h) * m_fInvSpacing;
            auto         s      = glm::vec3(1.0f) - t;
            index0              = glm::max(glm::ivec3(1), index0);
            index1              = glm::min(index1, glm::ivec3(m_iCellX - 2, m_iCellY - 2, m_iCellZ - 2));
            m_particleDensity[index2GridOffset(index0.x, index0.y, index0.z)] += s.x * s.y * s.z;
            m_particleDensity[index2GridOffset(index1.x, index0.y, index0.z)] += t.x * s.y * s.z;
            m_particleDensity[index2GridOffset(index0.x, index1.y, index0.z)] += s.x * t.y * s.z;
            m_particleDensity[index2GridOffset(index0.x, index0.y, index1.z)] += s.x * s.y * t.z;
            m_particleDensity[index2GridOffset(index1.x, index1.y, index0.z)] += t.x * t.y * s.z;
            m_particleDensity[index2GridOffset(index0.x, index1.y, index1.z)] += s.x * t.y * t.z;
            m_particleDensity[index2GridOffset(index1.x, index0.y, index1.z)] += t.x * s.y * t.z;
            m_particleDensity[index2GridOffset(index1.x, index1.y, index1.z)] += t.x * t.y * t.z;
        }
        if (m_particleRestDensity == 0.0f) {
            int   fluidCellNum = 0;
            float densitySum   = 0;
            for (int cid = 0; cid < m_iNumCells; ++cid) {
                if (m_type[cid] == FLUID_CELL) fluidCellNum++;
                densitySum += m_particleDensity[cid];
            }
            m_particleRestDensity = densitySum / fluidCellNum;
        }
    }

    void Simulator::updateCellType() {
        for (int cid = 0; cid < m_iNumCells; ++cid) {
            m_type[cid] = (m_s[cid] == 0.0f) ? SOLID_CELL : EMPTY_CELL;
        }
        for (int pid = 0; pid < m_iNumSpheres; ++pid) {
            int cid     = position2GridOffset(m_particlePos[pid]);
            m_type[cid] = FLUID_CELL;
        }
    }

    void Simulator::transferVelocities(bool toGrid, float flipRatio) {
        if (toGrid) {
            updateCellType();
            for (int i = 0; i < 3; ++i) {
                m_vel[i].assign(m_iNumCells, 0.0f);
                m_vel_divisor[i].assign(m_iNumCells, 0.0f);
            }
        } else {
            m_particlePreVel.assign(m_iNumSpheres, glm::vec3());
            m_particleCurVel.assign(m_iNumSpheres, glm::vec3());
        }
        for (int axis = 0; axis < 3; ++axis) {
            for (int pid = 0; pid < m_iNumSpheres; ++pid) {
                auto &    pos   = m_particlePos[pid];
                auto &    vel   = m_particleVel[pid];
                glm::vec3 delta = 0.5f * m_h * glm::vec3((axis == 0 ? 0 : 1), (axis == 1 ? 0 : 1), (axis == 2 ? 0 : 1));
                auto      index = glm::ivec3(glm::floor((pos - delta) * m_fInvSpacing));
                glm::vec3 t     = pos - delta - glm::vec3(index) * m_h;
                glm::vec3 s     = glm::vec3(1.0f) - t;

                static std::vector<glm::ivec3> dd {
                    glm::ivec3(0, 0, 0),
                    glm::ivec3(1, 0, 0),
                    glm::ivec3(0, 1, 0),
                    glm::ivec3(0, 0, 1),
                    glm::ivec3(0, 1, 1),
                    glm::ivec3(1, 0, 1),
                    glm::ivec3(1, 1, 0),
                    glm::ivec3(1, 1, 1),
                };
                for (int i = 0; i < 8; ++i) {
                    auto cur_index = index + dd[i];
                    cur_index      = glm::max(glm::ivec3((axis == 0 ? 2 : 1), (axis == 1 ? 2 : 1), (axis == 2 ? 2 : 1)), cur_index);
                    cur_index      = glm::min(cur_index, glm::ivec3(m_iCellX - 2, m_iCellY - 2, m_iCellZ - 2));
                    int   cid      = index2GridOffset(cur_index);
                    float weight   = (dd[i].x ? t.x : s.x) * (dd[i].y ? t.y : s.y) * (dd[i].z ? t.z : s.z);
                    if (toGrid) {
                        m_vel[axis][cid] += vel[axis] * weight;
                        m_vel_divisor[axis][cid] += weight;
                    } else {
                        m_particlePreVel[pid][axis] += m_pre_vel[axis][cid] * weight;
                        m_particleCurVel[pid][axis] += m_vel[axis][cid] * weight;
                    }
                }
            }
            if (toGrid) {
                for (int cid = 0; cid < m_iNumCells; ++cid) {
                    float weight = m_vel_divisor[axis][cid];
                    if (weight > 0.0f) m_vel[axis][cid] /= weight;
                }
            }
        }
        if (! toGrid) {
            float picRatio = 1 - flipRatio;
            for (int pid = 0; pid < m_iNumSpheres; ++pid) {
                auto & vel     = m_particleVel[pid];
                auto & cur_vel = m_particleCurVel[pid];
                auto & pre_vel = m_particlePreVel[pid];
                vel            = ((cur_vel - pre_vel) + vel) * flipRatio + (cur_vel) *picRatio;
            }
        }
    }

    void Simulator::solveIncompressibility(int numIters, float dt, float overRelaxation, bool compensateDrift) {
        static const float k = 1.0f;
        for (int i = 0; i < 3; ++i) {
            m_pre_vel[i] = m_vel[i];
        }
        while (numIters--) {
            for (int x = 1; x < m_iCellX - 1; ++x) {
                for (int y = 1; y < m_iCellY - 1; ++y) {
                    for (int z = 1; z < m_iCellZ - 1; ++z) {
                        int cid = index2GridOffset(x, y, z);
                        if (m_type[cid] == FLUID_CELL) {
                            float d = 0, s = 0;

                            float & v0 = m_vel[0][index2GridOffset(x, y, z)];
                            float & v1 = m_vel[1][index2GridOffset(x, y, z)];
                            float & v2 = m_vel[2][index2GridOffset(x, y, z)];
                            float & v3 = m_vel[0][index2GridOffset(x + 1, y, z)];
                            float & v4 = m_vel[1][index2GridOffset(x, y + 1, z)];
                            float & v5 = m_vel[2][index2GridOffset(x, y, z + 1)];

                            d = (-v0 - v1 - v2 + v3 + v4 + v5);
                            if (compensateDrift) {
                                float compression = m_particleDensity[cid] - m_particleRestDensity;
                                if (compression > 0.0f) {
                                    d -= k * compression;
                                }
                            }
                            float s0 = m_s[index2GridOffset(x - 1, y, z)];
                            float s1 = m_s[index2GridOffset(x, y - 1, z)];
                            float s2 = m_s[index2GridOffset(x, y, z - 1)];
                            float s3 = m_s[index2GridOffset(x + 1, y, z)];
                            float s4 = m_s[index2GridOffset(x, y + 1, z)];
                            float s5 = m_s[index2GridOffset(x, y, z + 1)];

                            s       = s0 + s1 + s2 + s3 + s4 + s5;
                            float p = d * overRelaxation / s;
                            v0 += s0 * p;
                            v1 += s1 * p;
                            v2 += s2 * p;
                            v3 -= s3 * p;
                            v4 -= s4 * p;
                            v5 -= s5 * p;
                        }
                    }
                }
            }
        }
    }

    void Simulator::updateParticleColors() {
        for (int pid = 0; pid < m_iNumSpheres; ++pid) {
            auto & color = m_particleColor[pid];
            int   cid = position2GridOffset(m_particlePos[pid]);
            float d   = m_particleDensity[cid] / m_particleRestDensity * 0.5;
            color = glm::clamp(glm::vec3(0.2f + d, 0.8f - d, 1.0f), glm::vec3(0), glm::vec3(1));
        }
    }

    inline int Simulator::index2GridOffset(glm::ivec3 index) {
        return index.x * (m_iCellY * m_iCellZ) + index.y * m_iCellZ + index.z;
    }
    inline int Simulator::index2GridOffset(int x, int y, int z) {
        return x * (m_iCellY * m_iCellZ) + y * m_iCellZ + z;
    }
    inline glm::ivec3 Simulator::position2CellIndex(glm::vec3 pos) {
        return glm::ivec3(glm::clamp(glm::floor(pos * m_fInvSpacing), glm::vec3(0), glm::vec3(m_iCellX - 1, m_iCellY - 1, m_iCellZ - 1)));
    }
    inline int Simulator::position2GridOffset(glm::vec3 pos) {
        return index2GridOffset(position2CellIndex(pos));
    }

    void Simulator::initialize() {
        m_busyFlag = false;
        m_iterCounter = 0;
        m_busyCounter = 0;
    }
} // namespace VCX::Labs::Fluid
