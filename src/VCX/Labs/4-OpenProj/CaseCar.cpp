#include "CaseCar.h"
#include "Engine/app.h"
#include "Labs/Common/ImGuiHelper.h"
#include <imgui.h>

namespace VCX::Labs::OpenProj {

    CaseCar::CaseCar() {
        _cameraManager.AutoRotate = false;
        _cameraManager.Save(_camera);
        _collisionSystem.miu_N = 0.8f;
        _collisionSystem.miu_T = 5.0f;
        resetScene();
    }

    void CaseCar::OnSetupPropsUI() {
        if (ImGui::CollapsingHeader("Help", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextWrapped("IJKL: move around, semicolon: brake");
        }
        if (ImGui::CollapsingHeader("Config", ImGuiTreeNodeFlags_DefaultOpen)) {
            const char * items[]     = { "Frictionless Impulse", "Frictional Impulse" };
            int          currentItem = static_cast<int>(_collisionSystem.collisionMethod);
            if (ImGui::Combo("Collision Method", &currentItem, items, IM_ARRAYSIZE(items))) {
                _collisionSystem.collisionMethod = static_cast<CollisionSystem::CollisionHandleMethod>(currentItem);
            }
            ImGui::DragFloat("restitution factor", &_collisionSystem.c, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("miu_N", &_collisionSystem.miu_N, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("miu_T", &_collisionSystem.miu_T, 0.01f, 0.01f, 40.0f);
            ImGui::Spacing();
            ImGui::DragFloat("transl damping", &_translationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("rotate damping", &_rotationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("gravity", &_gravity, 0.01f, 0.0f, 10.0f, "%.2f");
            ImGui::Spacing();
            ImGui::DragFloat("torque", &_car.torque, 0.01f, 0.0f, 100.0f);
            ImGui::DragFloat("brake", &_car.brake, 0.01f, 0.0f, 100.0f);
            ImGui::DragFloat("stiffness", &_car.stiffness, 0.1f, 10.0f, 1000.0f);
            ImGui::Spacing();
            ImGui::Checkbox("xray", &_renderSystem.xrayed);
            if (ImGui::Button("Reset")) {
                resetScene();
            }
            ImGui::SameLine();
            if (ImGui::Button(! _pause ? "pause" : "start")) {
                _pause = ! _pause;
            }
        }
    }

    void CaseCar::OnProcessMouseControl(glm::vec3 mouseDelta) {
    }

    void CaseCar::OnProcessKeyControl() {
        keyMove[0] = ImGui::IsKeyDown(ImGuiKey_I);
        keyMove[1] = ImGui::IsKeyDown(ImGuiKey_J);
        keyMove[2] = ImGui::IsKeyDown(ImGuiKey_K);
        keyMove[3] = ImGui::IsKeyDown(ImGuiKey_L);
        keyMove[4] = ImGui::IsKeyDown(ImGuiKey_Semicolon);
    }

    Common::CaseRenderResult CaseCar::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        // apply control first
        OnProcessMouseControl(_cameraManager.getMouseMove());
        OnProcessKeyControl();

        auto objs = _car.objects();

        const int substeps = 5;
        float     dt       = 0.03f;
        // float     dt       = Engine::GetDeltaTime();
        float st = dt / substeps;
        if (! _pause) {
            for (int t = 0; t < substeps; ++t) {
                _car.move(keyMove);
                _car.rigidify();
                for (auto & obj : objs) {
                    obj->rigidBody->applyTranslDamping(_translationalDamping);
                    obj->rigidBody->applyRotateDamping(_rotationalDamping);
                    obj->rigidBody->applyGravity(_gravity);
                    obj->rigidBody->update(st);
                }
                _collisionSystem.collisionDetect();
                _collisionSystem.collisionHandle();
                for (auto & obj : objs) {
                    obj->rigidBody->move(st);
                }
            }
        }

        // rendering
        _cameraManager.Update(_camera);
        _camera.Target = _car.body.boxBody.position;
        float aspect          = float(desiredSize.first) / desiredSize.second;
        auto  cameraTransform = _camera.GetTransformationMatrix(aspect);

        _frame.Resize(desiredSize);
        gl_using(_frame);
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(.5f);
        _renderSystem.draw(cameraTransform);
        _coord.draw(cameraTransform);
        glLineWidth(1.f);
        glPointSize(1.f);
        glDisable(GL_LINE_SMOOTH);

        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseCar::OnProcessInput(ImVec2 const & pos) {
        _cameraManager.ProcessInput(_camera, pos);
    }

    void CaseCar::resetScene() {
        _car.reset();
        _road.reset();
        _collisionSystem.items.clear();
        for (auto cb : _car.objects()) _collisionSystem.items.push_back(cb);
        for (auto rb : _road.objects()) _collisionSystem.items.push_back(rb);
        _renderSystem.items.clear();
        for (auto cb : _car.objects()) _renderSystem.items.push_back(cb);
        for (auto rb : _road.objects()) _renderSystem.items.push_back(rb);
    }
} // namespace VCX::Labs::OpenProj
