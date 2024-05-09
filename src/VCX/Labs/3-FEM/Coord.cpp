#include "Coord.h"

VCX::Labs::FEM::Coord::Coord():
    _program(
        Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/coord1u.vert"),
                                    Engine::GL::SharedShader("assets/shaders/coord.frag") })),
    _renderItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Static, 0).Add<glm::vec3>("color", Engine::GL::DrawFrequency::Static, 1), Engine::GL::PrimitiveType::Lines) {
    _renderItem.UpdateVertexBuffer("position", Engine::make_span_bytes<glm::vec3>(std::to_array<glm::vec3>({
        { -1000,     0,     0 },
        {  1000,     0,     0 },
        {     0, -1000,     0 },
        {     0,  1000,     0 },
        {     0,     0, -1000 },
        {     0,     0,  1000 },
    })));
    _renderItem.UpdateVertexBuffer("color", Engine::make_span_bytes<glm::vec3>(std::to_array<glm::vec3>({
        { 1, 0, 0 },
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 0, 1, 0 },
        { 0, 0, 1 },
        { 0, 0, 1 },
    })));
}

void VCX::Labs::FEM::Coord::draw(glm::mat4 cameraTransform) {
    glDisable(GL_DEPTH_TEST);
    _program.GetUniforms().SetByName("u_Transform", cameraTransform);
    _renderItem.Draw({ _program.Use() });
    glEnable(GL_DEPTH_TEST);
}
