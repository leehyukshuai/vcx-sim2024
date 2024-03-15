#include "Labs/1-RigidBody/CaseCollision.h"
#include "CaseCollision.h"
#include "Engine/app.h"
#include "Labs/Common/ImGuiHelper.h"
#include <imgui.h>

namespace VCX::Labs::RigidBody {
    static constexpr auto c_PositionData = std::to_array<glm::vec3>({
        {-1000,     0,     0},
        { 1000,     0,     0},
        {    0, -1000,     0},
        {    0,  1000,     0},
        {    0,     0, -1000},
        {    0,     0,  1000},
    });

    static constexpr auto c_ColorData = std::to_array<glm::vec3>({
        {1, 0, 0},
        {1, 0, 0},
        {0, 1, 0},
        {0, 1, 0},
        {0, 0, 1},
        {0, 0, 1},
    });

    CaseCollision::CaseCollision():
        _program(
            Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/flat.vert"),
                                        Engine::GL::SharedShader("assets/shaders/flat.frag") })),
        _coordProgram(Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/coord.vert"),
                                                  Engine::GL::SharedShader("assets/shaders/coord.frag") })),
        _coordItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Static, 0).Add<glm::vec3>("color", Engine::GL::DrawFrequency::Static, 1), Engine::GL::PrimitiveType::Lines) {
        _coordItem.UpdateVertexBuffer("position", Engine::make_span_bytes<glm::vec3>(c_PositionData));
        _coordItem.UpdateVertexBuffer("color", Engine::make_span_bytes<glm::vec3>(c_ColorData));

        _cameraManager.AutoRotate = false;
        _cameraManager.Save(_camera);
    }

    void CaseCollision::OnSetupPropsUI() {
        if (ImGui::CollapsingHeader("Config", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button(!_paused ? "pause" : "start")) {
                _paused = !_paused;
            }
            ImGui::SameLine();
            if (ImGui::Button("reset")) {
                _paused = true;
                _reset = true;
            }
            int typeIndex = static_cast<int>(_type);
            ImGui::Combo("Collision Type", &typeIndex, "edge-edge\0point-face\0face-face\0");
            if (typeIndex != static_cast<int>(_type)) {
                _type  = static_cast<CollisionType>(typeIndex);
                _paused = true;
                _reset = true;
            }
        }
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("z-view")) {
                _cameraManager.Save(Engine::Camera({ .Eye = glm::vec3(0, 0, 10) }));
                _cameraManager.Reset(_camera);
            }
            ImGui::SameLine();
            if (ImGui::Button("x-view")) {
                _cameraManager.Save(Engine::Camera({ .Eye = glm::vec3(10, 0, 0) }));
                _cameraManager.Reset(_camera);
            }
            ImGui::SameLine();
            if (ImGui::Button("y-view")) {
                _cameraManager.Save(Engine::Camera({ .Eye = glm::vec3(0, 10, 0) }));
                _cameraManager.Reset(_camera);
            }
        }
        if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat("transl damping", &_translationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("rotate damping", &_rotationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
        }
    }

    Common::CaseRenderResult CaseCollision::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        // apply control first
        OnProcessMouseControl(_cameraManager.getMouseMove());
        OnProcessKeyControl();

        if (_reset) {
            _reset = false;
            ResetScene(_type);
        }

        _boxA.box.applyRotateDamping(_rotationalDamping);
        _boxA.box.applyTranslDamping(_translationalDamping);
        _boxB.box.applyRotateDamping(_rotationalDamping);
        _boxB.box.applyTranslDamping(_translationalDamping);

        // update
        _boxA.update(Engine::GetDeltaTime(), _paused);
        _boxB.update(Engine::GetDeltaTime(), _paused);

        // rendering
        _frame.Resize(desiredSize);

        _cameraManager.Update(_camera);
        _program.GetUniforms().SetByName("u_Projection", _camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _program.GetUniforms().SetByName("u_View", _camera.GetViewMatrix());
        _coordProgram.GetUniforms().SetByName("u_Projection", _camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _coordProgram.GetUniforms().SetByName("u_View", _camera.GetViewMatrix());

        gl_using(_frame);
        glEnable(GL_LINE_SMOOTH);

        _program.GetUniforms().SetByName("u_Color", _boxA.color);
        _boxA.faceItem.Draw({ _program.Use() });
        _program.GetUniforms().SetByName("u_Color", _boxB.color);
        _boxB.faceItem.Draw({ _program.Use() });

        _program.GetUniforms().SetByName("u_Color", glm::vec3(1.f, 1.f, 1.f));
        _boxA.lineItem.Draw({ _program.Use() });
        _boxB.lineItem.Draw({ _program.Use() });

        _coordItem.Draw({ _coordProgram.Use() });

        glDisable(GL_LINE_SMOOTH);

        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseCollision::OnProcessInput(ImVec2 const & pos) {
        _cameraManager.ProcessInput(_camera, pos);
    }

    void CaseCollision::OnProcessMouseControl(glm::vec3 mouseDelta) {
    }

    void CaseCollision::OnProcessKeyControl() {
    }

    void CaseCollision::ResetScene(CollisionType type) {
        auto pi = glm::pi<float>();
        auto deg2rad = [pi](float deg){return deg * pi / 180.0f;};
        switch (type) {
        case EDGE_EDGE:
            _boxA.box.orientation = glm::quat(glm::vec3(0, deg2rad(-20), deg2rad(20)));
            _boxB.box.orientation = glm::quat(glm::vec3(deg2rad(90), deg2rad(20), 0));
            break;
        case FACE_FACE:
            _boxA.box.orientation = glm::quat(glm::vec3(deg2rad(30), 0, 0));
            _boxB.box.orientation = glm::quat(glm::vec3(deg2rad(-30), 0, 0));
            break;
        case POINT_FACE:
            _boxA.box.orientation = glm::quat(glm::vec3(deg2rad(-150), deg2rad(-12), deg2rad(-100)));
            _boxB.box.orientation = glm::quat(glm::vec3(deg2rad(-30), 0, 0));
            break;
        }
        _boxA.box.omega     = glm::vec3(0, 0, 0);
        _boxB.box.omega     = glm::vec3(0, 0, 0);
        _boxA.box.position  = glm::vec3(-5, 0, 0);
        _boxB.box.position  = glm::vec3(5, 0, 0);
        _boxA.box.velocity  = glm::vec3(1, 0, 0);
        _boxB.box.velocity  = glm::vec3(-1, 0, 0);
        _boxA.box.dimension = glm::vec3(1, 2, 3);
        _boxB.box.dimension = glm::vec3(1, 2, 3);
        _boxA.box.setInertia();
        _boxB.box.setInertia();
        _boxA.box.setMass();
        _boxB.box.setMass();
    }

} // namespace VCX::Labs::RigidBody
