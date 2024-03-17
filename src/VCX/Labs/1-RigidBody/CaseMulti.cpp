#include "Labs/1-RigidBody/CaseMulti.h"
#include "CaseMulti.h"
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

    CaseMulti::CaseMulti():
        _program(
            Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/flat.vert"),
                                        Engine::GL::SharedShader("assets/shaders/flat.frag") })),
        _coordProgram(Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/coord.vert"),
                                                  Engine::GL::SharedShader("assets/shaders/coord.frag") })),
        _coordItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Static, 0).Add<glm::vec3>("color", Engine::GL::DrawFrequency::Static, 1), Engine::GL::PrimitiveType::Lines) {
        ResetScene();
        for (auto & box : _boxes) {
            _collisionSystem.items.push_back(&box.box);
        }

        _coordItem.UpdateVertexBuffer("position", Engine::make_span_bytes<glm::vec3>(c_PositionData));
        _coordItem.UpdateVertexBuffer("color", Engine::make_span_bytes<glm::vec3>(c_ColorData));

        _cameraManager.AutoRotate = false;
        _cameraManager.Save(_camera);
    }

    void CaseMulti::OnSetupPropsUI() {
        if (ImGui::CollapsingHeader("Config", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button(! _paused ? "pause" : "start")) {
                _paused = ! _paused;
            }
            ImGui::SameLine();
            if (ImGui::Button("reset")) {
                _paused = true;
                _reset  = true;
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
            ImGui::DragFloat("gravity", &_gravity.y, 0.1f, 0.0f, 2.0f, "%.1f");
        }
    }

    Common::CaseRenderResult CaseMulti::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        // apply control first
        OnProcessMouseControl(_cameraManager.getMouseMove());
        OnProcessKeyControl();

        if (_reset) {
            _reset = false;
            ResetScene();
        }

        if (! _paused) {
            for (auto & box : _boxes) {
                // damping
                box.box.applyRotateDamping(_rotationalDamping);
                box.box.applyTranslDamping(_translationalDamping);
                // gravity
                // box.box.apply(_gravity * box.box.mass);
                // update
                box.update(0.01f);
            }
            // collision detect & handle
            _collisionSystem.collisionDetect();
            _collisionSystem.collisionHandle();
        } else {
            for (auto & box : _boxes) {
                box.update(0.f);
            }
        }

        // rendering
        _frame.Resize(desiredSize);

        _cameraManager.Update(_camera);
        _program.GetUniforms().SetByName("u_Projection", _camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _program.GetUniforms().SetByName("u_View", _camera.GetViewMatrix());
        _coordProgram.GetUniforms().SetByName("u_Projection", _camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _coordProgram.GetUniforms().SetByName("u_View", _camera.GetViewMatrix());

        gl_using(_frame);
        glEnable(GL_LINE_SMOOTH);

        glEnable(GL_DEPTH_TEST);
        for (auto & box : _boxes) {
            _program.GetUniforms().SetByName("u_Color", box.color);
            box.faceItem.Draw({ _program.Use() });
        }
        glDisable(GL_DEPTH_TEST);
        for (auto & box : _boxes) {
            _program.GetUniforms().SetByName("u_Color", glm::vec3(1.f, 1.f, 1.f));
            box.lineItem.Draw({ _program.Use() });
        }

        _coordItem.Draw({ _coordProgram.Use() });

        glDisable(GL_LINE_SMOOTH);

        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseMulti::OnProcessInput(ImVec2 const & pos) {
        _cameraManager.ProcessInput(_camera, pos);
    }

    void CaseMulti::OnProcessMouseControl(glm::vec3 mouseDelta) {
    }

    void CaseMulti::OnProcessKeyControl() {
    }

    void CaseMulti::ResetScene() {
        _boxes.resize(17);
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                auto & boxItem = _boxes[i * 4 + j];
                auto & box     = boxItem.box;

                boxItem.color = glm::vec3(i * 0.2, j * 0.2, 1.0);

                box.dimension = glm::vec3(1, 1, 1);
                box.setMass();
                box.setInertia();
                box.omega       = glm::vec3(0, 0, 0);
                box.velocity    = glm::vec3(0, -1, 0);
                box.orientation = glm::quat(glm::vec3(0, 0, 0));

                box.position = glm::vec3((i - 1.5f) * 1.1f, (i + j) * 1.1f, (j - 1.5f) * 1.1f);
            }
        }
        auto &boxItem = _boxes.back();
        boxItem.color = glm::vec3(1.0, 0.6, 0.2);
        auto &box = boxItem.box;
        box.isStatic = true;
        box.dimension = glm::vec3(10, 0.6f, 10);
        box.setMass();
        box.setInertia();
        box.omega       = glm::vec3(0, 0, 0);
        box.velocity    = glm::vec3(0, 0, 0);
        box.orientation = glm::quat(glm::vec3(0, 0, 0));

        box.position = glm::vec3(0,-1,0);
    }

} // namespace VCX::Labs::RigidBody
