#include "Labs/4-OpenProj/CaseCollide.h"
#include "CaseCollide.h"
#include "Engine/app.h"
#include "Labs/Common/ImGuiHelper.h"
#include <imgui.h>

namespace VCX::Labs::OpenProj {

    CaseCollide::CaseCollide() {
        _cameraManager.AutoRotate = false;
        _cameraManager.Save(_camera);

        resetScene();
    }

    void CaseCollide::OnSetupPropsUI() {
        if (ImGui::CollapsingHeader("Config", ImGuiTreeNodeFlags_DefaultOpen)) {
            const char * items[]     = { "Frictionless Impulse", "Frictional Impulse" };
            int          currentItem = static_cast<int>(_collisionSystem.collisionMethod);
            if (ImGui::Combo("Collision Method", &currentItem, items, IM_ARRAYSIZE(items))) {
                _collisionSystem.collisionMethod = static_cast<CollisionSystem::CollisionHandleMethod>(currentItem);
            }

            ImGui::DragFloat("transl damping", &_translationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("rotate damping", &_rotationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("gravity", &_gravity, 0.01f, 0.0f, 10.0f, "%.2f");
            ImGui::DragFloat("restitution factor", &_collisionSystem.c, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("miu_N", &_collisionSystem.miu_N, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat("miu_T", &_collisionSystem.miu_T, 0.01f, 0.01f, 5.0f);

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

    Common::CaseRenderResult CaseCollide::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        // apply control first
        OnProcessMouseControl(_cameraManager.getMouseMove());
        OnProcessKeyControl();

        const int substeps = 5;
        // float     dt       = Engine::GetDeltaTime();
        float dt = 0.03f;
        float st = dt / substeps;
        if (! _pause) {
            for (int t = 0; t < substeps; ++t) {
                for (auto rb : _rigidBodys) {
                    rb->rigidBody->applyTranslDamping(_translationalDamping);
                    rb->rigidBody->applyRotateDamping(_rotationalDamping);
                    rb->rigidBody->applyGravity(_gravity);
                    rb->rigidBody->update(st);
                }
                _collisionSystem.collisionDetect();
                _collisionSystem.collisionHandle();
                for (auto rb : _rigidBodys) {
                    rb->rigidBody->move(st);
                }
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

    void CaseCollide::OnProcessInput(ImVec2 const & pos) {
        _cameraManager.ProcessInput(_camera, pos);
    }

    void CaseCollide::OnProcessMouseControl(glm::vec3 mouseDelta) {
        float movingScale = 8.0f;
        _player->rigidBody->applyTorque(movingScale * mouseDelta);
    }

    void CaseCollide::OnProcessKeyControl() {
        float rs = 8.0f;
        float ts = 10.0f;
        if (ImGui::IsKeyDown(ImGuiKey_J)) {
            _player->rigidBody->applyTorque({ -rs, 0, 0 });
        }
        if (ImGui::IsKeyDown(ImGuiKey_L)) {
            _player->rigidBody->applyTorque({ rs, 0, 0 });
        }
        if (ImGui::IsKeyDown(ImGuiKey_I)) {
            _player->rigidBody->applyTorque({ 0, 0, -rs });
        }
        if (ImGui::IsKeyDown(ImGuiKey_K)) {
            _player->rigidBody->applyTorque({ 0, 0, rs });
        }
        if (ImGui::IsKeyDown(ImGuiKey_U)) {
            _player->rigidBody->apply({ 0, ts, 0 });
        }
        if (ImGui::IsKeyDown(ImGuiKey_O)) {
            _player->rigidBody->apply({ 0, -ts, 0 });
        }
    }

    void CaseCollide::resetScene() {
        std::srand(2);
        auto randomFloat = []() {
            return std::rand() * 1.0 / RAND_MAX;
        };
        auto randomColor = []() {
            return glm::vec3(
                std::rand() * 1.0 / RAND_MAX,
                std::rand() * 1.0 / RAND_MAX,
                std::rand() * 1.0 / RAND_MAX);
        };

        Box * floor                   = new Box(glm::vec3(10, 0.6f, 10));
        Box * wall1                   = new Box(glm::vec3(10, 0.6f, 10));
        Box * wall2                   = new Box(glm::vec3(10, 0.6f, 10));
        Box * wall3                   = new Box(glm::vec3(10, 0.6f, 10));
        Box * wall4                   = new Box(glm::vec3(10, 0.6f, 10));
        floor->rigidBody->position    = glm::vec3(0, -1, 0);
        wall1->rigidBody->position    = glm::vec3(-6, 2, 0);
        wall1->rigidBody->orientation = glm::quat(glm::vec3(0, 0, -1.5));
        wall2->rigidBody->position    = glm::vec3(6, 2, 0);
        wall2->rigidBody->orientation = glm::quat(glm::vec3(0, 0, 1.5));
        wall3->rigidBody->position    = glm::vec3(0, 2, 6);
        wall3->rigidBody->orientation = glm::quat(glm::vec3(-1.5, 0, 0));
        wall4->rigidBody->position    = glm::vec3(0, 2, -6);
        wall4->rigidBody->orientation = glm::quat(glm::vec3(1.5, 0, 0));

        for (auto sb : _staticBodys) {
            delete sb;
        }
        _staticBodys = std::vector<Object *>({ floor, wall1, wall2, wall3, wall4 });
        for (auto sb : _staticBodys) {
            sb->rigidBody->isStatic = true;
            sb->renderItem.color    = randomColor();
            sb->initialize();
        }

        for (auto rb : _rigidBodys) {
            delete rb;
        }
        _rigidBodys.clear();
        for (int i = 0; i < 20; ++i) {
            if (i % 2) {
                float r = 1.0f + randomFloat() * 1.5f;
                float x = randomFloat() - 0.5;
                float y = 10 + i * 8;
                float z = randomFloat() - 0.5;

                Sphere * sphere             = new Sphere(r);
                sphere->rigidBody->position = glm::vec3(x, y, z);
                _rigidBodys.push_back(sphere);
            } else {
                float dx = 1.0f + randomFloat() * 4.0f;
                float dy = 1.0f + randomFloat() * 4.0f;
                float dz = 1.0f + randomFloat() * 4.0f;
                float x  = randomFloat() - 0.5;
                float y  = 10 + i * 8;
                float z  = randomFloat() - 0.5;

                Box * box                = new Box(glm::vec3(dx, dy, dz));
                box->rigidBody->position = glm::vec3(x, y, z);
                _rigidBodys.push_back(box);
            }
            if (i == 0) {
                _player = _rigidBodys.back();
            }
        }
        for (auto rb : _rigidBodys) {
            rb->renderItem.color = randomColor();
            rb->initialize();
        }

        _collisionSystem.items.clear();
        for (auto sb : _staticBodys) _collisionSystem.items.push_back(sb);
        for (auto rb : _rigidBodys) _collisionSystem.items.push_back(rb);
        _renderSystem.items.clear();
        for (auto sb : _staticBodys) _renderSystem.items.push_back(sb);
        for (auto rb : _rigidBodys) _renderSystem.items.push_back(rb);
    }

} // namespace VCX::Labs::OpenProj
