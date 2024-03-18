#include "Labs/1-RigidBody/CaseBox.h"
#include "CaseBox.h"
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

    CaseBox::CaseBox():
        _program(
            Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/flat.vert"),
                                        Engine::GL::SharedShader("assets/shaders/flat.frag") })),
        _coordProgram(Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/coord.vert"),
                                                  Engine::GL::SharedShader("assets/shaders/coord.frag") })),
        _coordItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Static, 0).Add<glm::vec3>("color", Engine::GL::DrawFrequency::Static, 1), Engine::GL::PrimitiveType::Lines) {
        ResetScene();

        _coordItem.UpdateVertexBuffer("position", Engine::make_span_bytes<glm::vec3>(c_PositionData));
        _coordItem.UpdateVertexBuffer("color", Engine::make_span_bytes<glm::vec3>(c_ColorData));

        _cameraManager.AutoRotate = false;
        _cameraManager.Save(_camera);
    }

    void CaseBox::OnSetupPropsUI() {
        if (ImGui::CollapsingHeader("Help", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextWrapped("Movement: I:-z K:+z J:-x L:+x U:+y O:-y");
            ImGui::TextWrapped("Rotation: Alt+MouseMove");
        }
        if (ImGui::CollapsingHeader("Config")) {
            if (ImGui::Button(! _paused ? "pause" : "start")) {
                _paused = ! _paused;
            }
            ImGui::SameLine();
            if (ImGui::Button("reset")) {
                _reset = true;
            }
        }
        if (ImGui::CollapsingHeader("Camera")) {
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
        if (ImGui::CollapsingHeader("Physics")) {
            ImGui::DragFloat("transl damping", &_translationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("rotate damping", &_rotationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::InputFloat3("position", glm::value_ptr(_box.box.position), "%.1f");
            ImGui::InputFloat3("velocity", glm::value_ptr(_box.box.velocity), "%.1f");
            auto eulerAngles = glm::eulerAngles(_box.box.orientation) * 180.0f / glm::pi<float>();
            ImGui::InputFloat3("orientation", glm::value_ptr(eulerAngles), "%.1f");
            eulerAngles *= glm::pi<float>() / 180.0f;
            _box.box.orientation = glm::quat(eulerAngles);
            ImGui::InputFloat3("omega", glm::value_ptr(_box.box.omega), "%.1f");
        }
        if (ImGui::CollapsingHeader("Appearance")) {
            ImGui::ColorEdit3("Box Color", glm::value_ptr(_box.color));
            bool flag = false;
            flag |= ImGui::SliderFloat("dim-x", &_box.box.dimension.x, 0.5, 4);
            flag |= ImGui::SliderFloat("dim-y", &_box.box.dimension.y, 0.5, 4);
            flag |= ImGui::SliderFloat("dim-z", &_box.box.dimension.z, 0.5, 4);
            if (flag) {
                _box.box.setMass();
                _box.box.setInertia();
            }
        }
    }

    Common::CaseRenderResult CaseBox::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        // apply control first
        OnProcessMouseControl(_cameraManager.getMouseMove());
        OnProcessKeyControl();

        if (_reset) {
            _reset = false;
            ResetScene();
        }

        // damping
        if (! _paused) {
            _box.box.applyTranslDamping(_translationalDamping);
            _box.box.applyRotateDamping(_rotationalDamping);
            _box.box.update(Engine::GetDeltaTime());
            _box.box.move(Engine::GetDeltaTime());
        }
        _box.updateBuffer();

        // rendering
        _frame.Resize(desiredSize);

        _cameraManager.Update(_camera);
        _program.GetUniforms().SetByName("u_Projection", _camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _program.GetUniforms().SetByName("u_View", _camera.GetViewMatrix());
        _coordProgram.GetUniforms().SetByName("u_Projection", _camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _coordProgram.GetUniforms().SetByName("u_View", _camera.GetViewMatrix());

        gl_using(_frame);

        _program.GetUniforms().SetByName("u_Color", _box.color);
        _box.faceItem.Draw({ _program.Use() });
        _program.GetUniforms().SetByName("u_Color", glm::vec3(1.f, 1.f, 1.f));
        _box.lineItem.Draw({ _program.Use() });

        _coordItem.Draw({ _coordProgram.Use() });

        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseBox::OnProcessInput(ImVec2 const & pos) {
        _cameraManager.ProcessInput(_camera, pos);
    }

    void CaseBox::OnProcessMouseControl(glm::vec3 mouseDelta) {
        float movingScale = 8.0f;
        _box.box.applyTorque(movingScale * mouseDelta);
    }

    void CaseBox::OnProcessKeyControl() {
        if (ImGui::IsKeyDown(ImGuiKey_J)) {
            _box.box.apply({ -1, 0, 0 });
        }
        if (ImGui::IsKeyDown(ImGuiKey_L)) {
            _box.box.apply({ 1, 0, 0 });
        }
        if (ImGui::IsKeyDown(ImGuiKey_I)) {
            _box.box.apply({ 0, 0, -1 });
        }
        if (ImGui::IsKeyDown(ImGuiKey_K)) {
            _box.box.apply({ 0, 0, 1 });
        }
        if (ImGui::IsKeyDown(ImGuiKey_U)) {
            _box.box.apply({ 0, 1, 0 });
        }
        if (ImGui::IsKeyDown(ImGuiKey_O)) {
            _box.box.apply({ 0, -1, 0 });
        }
    }

    void CaseBox::ResetScene() {
        _box.box = Box();
    }

} // namespace VCX::Labs::RigidBody
