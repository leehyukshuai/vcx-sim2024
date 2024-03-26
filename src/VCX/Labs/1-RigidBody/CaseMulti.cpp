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

        _coordItem.UpdateVertexBuffer("position", Engine::make_span_bytes<glm::vec3>(c_PositionData));
        _coordItem.UpdateVertexBuffer("color", Engine::make_span_bytes<glm::vec3>(c_ColorData));

        _collisionSystem.collisionMethod = BoxCollisionSystem::METHOD_NAIVE;

        _cameraManager.AutoRotate = false;
        _cameraManager.Save(_camera);
    }

    void CaseMulti::OnSetupPropsUI() {
        if (ImGui::CollapsingHeader("Help", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextWrapped("Apply forces: Alt+MouseMove");
        }
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
        if (ImGui::CollapsingHeader("Physics"), ImGuiTreeNodeFlags_DefaultOpen) {
            ImGui::DragFloat("transl damping", &_translationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("rotate damping", &_rotationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("restitution factor", &_collisionSystem.c, 0.01f, 0.0f, 1.0f, "%.2f");
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
            for (auto & boxItem : _boxes) {
                // damping
                boxItem.box.applyRotateDamping(_rotationalDamping);
                boxItem.box.applyTranslDamping(_translationalDamping);
                // gravity
                boxItem.box.apply(-_gravity * boxItem.box.mass);
                // update
                boxItem.box.update(Engine::GetDeltaTime());
            }
            // collision detect & handle
            _collisionSystem.collisionDetect();
            _collisionSystem.collisionHandle();
            for (auto & boxItem : _boxes) {
                // update
                boxItem.box.move(Engine::GetDeltaTime());
            }
        }
        for (auto & boxItem : _boxes) {
            // update
            boxItem.updateBuffer();
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
        float movingScale = 0.3f;
        for (auto & boxItem : _boxes) {
            boxItem.box.apply(movingScale * mouseDelta);
            boxItem.box.applyTorque(movingScale * mouseDelta);
        }
    }

    void CaseMulti::OnProcessKeyControl() {
    }

    void CaseMulti::ResetScene() {
        _boxes.clear();

        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 6; ++j) {
                _boxes.resize(_boxes.size() + 1);
                auto & boxItem = _boxes.back();
                auto & box     = boxItem.box;

                boxItem.color = glm::vec3(i * 0.2, j * 0.2, 1.0);

                box.dimension = glm::vec3(1, 1, 1);
                box.setMass();
                box.setInertia();
                box.omega           = glm::vec3(0, 0, 0);
                box.angulayMomentum = glm::vec3(0, 0, 0);
                box.velocity        = glm::vec3(0, -1, 0);
                box.orientation     = glm::quat(glm::vec3(0, 0, 0));
                box.position        = glm::vec3((i - 2.5f) * 4.f, 5, (j - 2.5f) * 4.f);

                if (abs(i - 2.5f) >= 1.5f && abs(j - 2.5f) >= 1.5f || abs(i - 2.5f) <= 0.5f && abs(j - 2.5f) <= 0.5f) {
                    _boxes.erase(_boxes.end() - 1);
                    continue;
                }
            }
        }

        _boxes.resize(_boxes.size() + 1);
        auto & wall1          = _boxes.back();
        wall1.color           = glm::vec3(1.0, 0.6, 0.3);
        wall1.box.isStatic    = true;
        wall1.box.dimension   = glm::vec3(10, 0.6f, 10);
        wall1.box.orientation = glm::quat(glm::vec3(0, 0, -0.6));
        wall1.box.position    = glm::vec3(-9.5, 3, 0);

        _boxes.resize(_boxes.size() + 1);
        auto & wall2          = _boxes.back();
        wall2.color           = glm::vec3(1.0, 0.6, 0.3);
        wall2.box.isStatic    = true;
        wall2.box.dimension   = glm::vec3(10, 0.6f, 10);
        wall2.box.orientation = glm::quat(glm::vec3(0, 0, 0.6));
        wall2.box.position    = glm::vec3(9.5, 3, 0);

        _boxes.resize(_boxes.size() + 1);
        auto & wall3          = _boxes.back();
        wall3.color           = glm::vec3(1.0, 0.6, 0.3);
        wall3.box.isStatic    = true;
        wall3.box.dimension   = glm::vec3(10, 0.6f, 10);
        wall3.box.orientation = glm::quat(glm::vec3(-0.6, 0, 0));
        wall3.box.position    = glm::vec3(0, 3, 9.5);

        _boxes.resize(_boxes.size() + 1);
        auto & wall4          = _boxes.back();
        wall4.color           = glm::vec3(1.0, 0.6, 0.3);
        wall4.box.isStatic    = true;
        wall4.box.dimension   = glm::vec3(10, 0.6f, 10);
        wall4.box.orientation = glm::quat(glm::vec3(0.6, 0, 0));
        wall4.box.position    = glm::vec3(0, 3, -9.5);

        _boxes.resize(_boxes.size() + 1);
        auto & ground          = _boxes.back();
        ground.color           = glm::vec3(1.0, 0.6, 0.2);
        ground.box.isStatic    = true;
        ground.box.dimension   = glm::vec3(10, 0.6f, 10);
        ground.box.orientation = glm::quat(glm::vec3(0, 0, 0));
        ground.box.position    = glm::vec3(0, -1, 0);

        _collisionSystem.items.clear();
        for (auto & box : _boxes) {
            _collisionSystem.items.push_back(&box.box);
        }
    }

} // namespace VCX::Labs::RigidBody
