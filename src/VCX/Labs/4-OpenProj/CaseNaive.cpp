#include "Labs/4-OpenProj/CaseNaive.h"
#include "CaseNaive.h"
#include "Engine/app.h"
#include "Labs/Common/ImGuiHelper.h"
#include <imgui.h>

namespace VCX::Labs::OpenProj {

    CaseNaive::CaseNaive() {
        _cameraManager.AutoRotate = false;
        _cameraManager.Save(_camera);

        _collisionSystem.miu_N = 0.6f;
        _collisionSystem.miu_T = 5.0f;

        resetScene();
    }

    void CaseNaive::OnSetupPropsUI() {
        if (ImGui::CollapsingHeader("Help", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextWrapped("Rotation: Alt+MouseMove");
        }

        if (ImGui::CollapsingHeader("Config", ImGuiTreeNodeFlags_DefaultOpen)) {
            const char * items[]     = { "Frictionless Impulse", "Frictional Impulse" };
            int          currentItem = static_cast<int>(_collisionSystem.collisionMethod);
            if (ImGui::Combo("Collision Method", &currentItem, items, IM_ARRAYSIZE(items))) {
                _collisionSystem.collisionMethod = static_cast<CollisionSystem::CollisionHandleMethod>(currentItem);
            }

            ImGui::DragFloat("transl damping", &_translationalDamping, 0.001f, 0.0f, 1.0f, "%.3f");
            ImGui::DragFloat("rotate damping", &_rotationalDamping, 0.001f, 0.0f, 1.0f, "%.3f");
            ImGui::DragFloat("gravity", &_gravity, 0.01f, 0.0f, 10.0f, "%.2f");
            ImGui::DragFloat("restitution factor", &_collisionSystem.c, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("miu_N", &_collisionSystem.miu_N, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat("miu_T", &_collisionSystem.miu_T, 0.01f, 0.01f, 5.0f);
            ImGui::Checkbox("solve contact", &_collisionSystem.solveContact);
            ImGui::Checkbox("xray", &_renderSystem.xrayed);
            if (ImGui::Button("Reset")) {
                resetScene();
                _pause = true;
            }
            ImGui::SameLine();
            if (ImGui::Button(! _pause ? "pause" : "start")) {
                _pause = ! _pause;
            }
        }
    }

    Common::CaseRenderResult CaseNaive::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        // apply mouse control first
        OnProcessMouseControl(_cameraManager.getMouseMove());

        const int substeps = 8;
        float     dt       = Engine::GetDeltaTime();
        float     st       = dt / substeps;
        if (! _pause) {
            for (int t = 0; t < substeps; ++t) {
                _sphere->rigidBody->applyGravity(_gravity);
                _sphere->rigidBody->applyTranslDamping(_translationalDamping);
                _sphere->rigidBody->applyRotateDamping(_rotationalDamping);
                _sphere->rigidBody->update(st);
                _collisionSystem.collisionDetect();
                _collisionSystem.collisionHandle();
                _sphere->rigidBody->move(st);
            }
        }

        // camera
        _cameraManager.Update(_camera);
        float aspect          = float(desiredSize.first) / desiredSize.second;
        auto  cameraTransform = _camera.GetTransformationMatrix(aspect);

        // rendering
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

    void CaseNaive::resetScene() {
        _sphere                         = new Sphere(0.5f);
        _sphere->rigidBody->position.y  = 3;
        _sphere->rigidBody->angularMomentum = glm::vec3(1, 0, 3);
        _sphere->renderItem.color       = glm::vec3(1.0f, 0.8f, 0.4f);
        _sphere->initialize();
        _floor                      = new Box(glm::vec3(10, 1, 10));
        _floor->rigidBody->isStatic = true;
        _floor->initialize();

        _collisionSystem.items.clear();
        _collisionSystem.items.push_back(_sphere);
        _collisionSystem.items.push_back(_floor);

        _renderSystem.items.clear();
        _renderSystem.items.push_back(_sphere);
        _renderSystem.items.push_back(_floor);
    }

    void CaseNaive::OnProcessInput(ImVec2 const & pos) {
        _cameraManager.ProcessInput(_camera, pos);
    }

    void CaseNaive::OnProcessMouseControl(glm::vec3 mouseDelta) {
        float movingScale = 8.0f;
        _sphere->rigidBody->applyTorque(movingScale * mouseDelta);
    }

} // namespace VCX::Labs::OpenProj
