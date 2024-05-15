#include "Renderer.h"
#include <unordered_set>

VCX::Labs::FEM::Renderer::Renderer():
    _faceItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Triangles),
    _lineItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Lines),
    _program(Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/flat1u.vert"),
                                         Engine::GL::SharedShader("assets/shaders/flat.frag") })) {
}

void VCX::Labs::FEM::Renderer::setXRay(bool xRay) {
    _xRay = xRay;
}

void VCX::Labs::FEM::Renderer::bind(const SoftBody & softBody) {
    const auto & faces = softBody._faces;
    const auto & tetras = softBody._tetras;

    std::vector<uint32_t> faceIndices;
    faceIndices.reserve(faces.size() * 3);
    std::vector<uint32_t> lineIndices;
    lineIndices.reserve(faces.size() * 6);
    std::unordered_set<uint64_t> addedEdges;

    for (auto & f : faces) {
        faceIndices.push_back(f.x);
        faceIndices.push_back(f.y);
        faceIndices.push_back(f.z);
    }
    for (auto & t : tetras) {
        uint64_t v01 = static_cast<uint64_t>(t[0]) << 32 | t[1];
        uint64_t v02 = static_cast<uint64_t>(t[0]) << 32 | t[2];
        uint64_t v03 = static_cast<uint64_t>(t[0]) << 32 | t[3];
        uint64_t v12 = static_cast<uint64_t>(t[1]) << 32 | t[2];
        uint64_t v13 = static_cast<uint64_t>(t[1]) << 32 | t[3];
        uint64_t v23 = static_cast<uint64_t>(t[2]) << 32 | t[3];
        if (addedEdges.find(v01) == addedEdges.end()) {
            addedEdges.insert(v01);
            lineIndices.push_back(t[0]);
            lineIndices.push_back(t[1]);
        }
        if (addedEdges.find(v02) == addedEdges.end()) {
            addedEdges.insert(v02);
            lineIndices.push_back(t[0]);
            lineIndices.push_back(t[2]);
        }
        if (addedEdges.find(v03) == addedEdges.end()) {
            addedEdges.insert(v03);
            lineIndices.push_back(t[0]);
            lineIndices.push_back(t[3]);
        }
        if (addedEdges.find(v12) == addedEdges.end()) {
            addedEdges.insert(v12);
            lineIndices.push_back(t[1]);
            lineIndices.push_back(t[2]);
        }
        if (addedEdges.find(v13) == addedEdges.end()) {
            addedEdges.insert(v13);
            lineIndices.push_back(t[1]);
            lineIndices.push_back(t[3]);
        }
        if (addedEdges.find(v23) == addedEdges.end()) {
            addedEdges.insert(v23);
            lineIndices.push_back(t[2]);
            lineIndices.push_back(t[3]);
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
    if (_xRay) {
        glDisable(GL_DEPTH_TEST);
    } else {
        glDepthFunc(GL_LEQUAL);
    }
    _program.GetUniforms().SetByName("u_Color", lineColor);
    _lineItem.Draw({ _program.Use() });
    glEnable(GL_DEPTH_TEST);
}
