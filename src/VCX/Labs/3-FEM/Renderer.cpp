#include "Renderer.h"
#include <unordered_set>

VCX::Labs::FEM::Renderer::Renderer():
    _faceItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Triangles),
    _lineItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Lines),
    _program(Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/flat1u.vert"),
                                         Engine::GL::SharedShader("assets/shaders/flat.frag") })) {
}

void VCX::Labs::FEM::Renderer::bind(const SoftBody & softBody) {
    const auto & faces = softBody._faces;

    std::vector<uint32_t> faceIndices;
    faceIndices.reserve(faces.size() * 3);
    std::vector<uint32_t> lineIndices;
    lineIndices.reserve(faces.size() * 3);
    std::unordered_set<uint64_t> addedEdges;

    for (auto & f : faces) {
        faceIndices.push_back(f.x);
        faceIndices.push_back(f.y);
        faceIndices.push_back(f.z);
        uint64_t xy = static_cast<uint64_t>(f.x) << 32 | f.y;
        uint64_t xz = static_cast<uint64_t>(f.x) << 32 | f.z;
        uint64_t yz = static_cast<uint64_t>(f.y) << 32 | f.z;
        if (addedEdges.find(xy) == addedEdges.end()) {
            addedEdges.insert(xy);
            lineIndices.push_back(f.x);
            lineIndices.push_back(f.y);
        }
        if (addedEdges.find(xz) == addedEdges.end()) {
            addedEdges.insert(xz);
            lineIndices.push_back(f.x);
            lineIndices.push_back(f.z);
        }
        if (addedEdges.find(yz) == addedEdges.end()) {
            addedEdges.insert(yz);
            lineIndices.push_back(f.y);
            lineIndices.push_back(f.z);
        }
    }
    _faceItem.UpdateElementBuffer(faceIndices);
    _lineItem.UpdateElementBuffer(lineIndices);
    update(softBody);
}

void VCX::Labs::FEM::Renderer::update(const SoftBody & softBody) {
    auto span_bytes = Engine::make_span_bytes<glm::vec3>(softBody._position);
    _faceItem.UpdateVertexBuffer("position", span_bytes);
    _lineItem.UpdateVertexBuffer("position", span_bytes);
}

void VCX::Labs::FEM::Renderer::draw(glm::mat4 cameraTransform, glm::vec3 faceColor, glm::vec3 lineColor) {
    _program.GetUniforms().SetByName("u_Transform", cameraTransform);
    glEnable(GL_DEPTH_TEST);
    _program.GetUniforms().SetByName("u_Color", faceColor);
    _faceItem.Draw({ _program.Use() });
    glDisable(GL_DEPTH_TEST);
    _program.GetUniforms().SetByName("u_Color", lineColor);
    _lineItem.Draw({ _program.Use() });
    glEnable(GL_DEPTH_TEST);
}
