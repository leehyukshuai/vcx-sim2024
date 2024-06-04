#include "Render.h"

namespace VCX::Labs::OpenProj {
    RenderItem::RenderItem():
        faceItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Triangles),
        lineItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Lines) {}

    RenderItem::RenderItem(const Mesh & mesh):
        faceItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Triangles),
        lineItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Lines) {
        initialize(mesh);
    }

    void RenderItem::initialize(const Mesh & mesh) {
        positions = mesh.positions;
        faceItem.UpdateElementBuffer(mesh.triIndices);
        lineItem.UpdateElementBuffer(mesh.lineIndices);
        updateBuffer(glm::vec3(0), glm::quat_identity<float, glm::defaultp>());
    }

    void RenderItem::updateBuffer(const glm::vec3 & translation, const glm::quat & rotation) {
        std::vector<glm::vec3> updated(positions);
        for (auto & vert : updated) {
            vert = glm::rotate(rotation, vert);
            vert = translation + vert;
        }
        auto span_bytes = Engine::make_span_bytes<glm::vec3>(updated);
        lineItem.UpdateVertexBuffer("position", span_bytes);
        faceItem.UpdateVertexBuffer("position", span_bytes);
    }
    Mesh Mesh::generateBoxMesh(glm::vec3 dim) {
        Mesh  ret;
        float x         = dim[0] / 2.0;
        float y         = dim[1] / 2.0;
        float z         = dim[2] / 2.0;
        ret.positions   = std::vector<glm::vec3>({
            { -x, -y,  z },
            {  x, -y,  z },
            {  x, -y, -z },
            { -x, -y, -z },
            { -x,  y,  z },
            {  x,  y,  z },
            {  x,  y, -z },
            { -x,  y, -z },
        });
        ret.triIndices  = std::vector<std::uint32_t>({ 0, 1, 2, 0, 2, 3, 1, 4, 0, 1, 4, 5, 1, 6, 5, 1, 2, 6, 2, 3, 7, 2, 6, 7, 0, 3, 7, 0, 4, 7, 4, 5, 6, 4, 6, 7 });
        ret.lineIndices = std::vector<std::uint32_t>({ 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 });
        return ret;
    }

    Mesh Mesh::generateCylinderMesh(float radius, float height, int precision) {
        Mesh  ret;
        float halfHeight = height / 2.0f;
        float angleStep  = 2.0f * glm::pi<float>() / precision;

        // Generate vertices for the bottom and top circles
        for (int i = 0; i < precision; ++i) {
            float angle = i * angleStep;
            float x     = radius * cos(angle);
            float z     = radius * sin(angle);

            // Bottom circle vertices
            ret.positions.push_back(glm::vec3(x, -halfHeight, z));

            // Top circle vertices
            ret.positions.push_back(glm::vec3(x, halfHeight, z));
        }

        // Add center vertices for the top and bottom circles
        ret.positions.push_back(glm::vec3(0.0f, -halfHeight, 0.0f)); // Bottom center
        ret.positions.push_back(glm::vec3(0.0f, halfHeight, 0.0f));  // Top center

        int bottomCenterIndex = precision * 2;
        int topCenterIndex    = bottomCenterIndex + 1;

        // Generate triangle indices
        for (int i = 0; i < precision; ++i) {
            int next = (i + 1) % precision;

            // Bottom circle triangles
            ret.triIndices.push_back(bottomCenterIndex);
            ret.triIndices.push_back(i * 2);
            ret.triIndices.push_back(next * 2);

            // Top circle triangles
            ret.triIndices.push_back(topCenterIndex);
            ret.triIndices.push_back(next * 2 + 1);
            ret.triIndices.push_back(i * 2 + 1);

            // Side triangles
            ret.triIndices.push_back(i * 2);
            ret.triIndices.push_back(i * 2 + 1);
            ret.triIndices.push_back(next * 2);

            ret.triIndices.push_back(next * 2);
            ret.triIndices.push_back(i * 2 + 1);
            ret.triIndices.push_back(next * 2 + 1);
        }

        // Generate line indices for the edges
        for (int i = 0; i < precision; ++i) {
            int next = (i + 1) % precision;

            // Bottom circle edges
            ret.lineIndices.push_back(i * 2);
            ret.lineIndices.push_back(next * 2);

            // Top circle edges
            ret.lineIndices.push_back(i * 2 + 1);
            ret.lineIndices.push_back(next * 2 + 1);

            // Side edges
            ret.lineIndices.push_back(i * 2);
            ret.lineIndices.push_back(i * 2 + 1);
        }

        return ret;
    }

} // namespace VCX::Labs::OpenProj