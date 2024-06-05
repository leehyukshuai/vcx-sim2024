#include "Labs/4-OpenProj/CaseCollide.h"
#include "CaseCollide.h"
#include "Engine/app.h"
#include "Labs/Common/ImGuiHelper.h"
#include <imgui.h>

namespace VCX::Labs::OpenProj {

    CaseCollide::CaseCollide():
        _program(
            Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/flat.vert"),
                                        Engine::GL::SharedShader("assets/shaders/flat.frag") })) {
        _cameraManager.AutoRotate = false;
        _cameraManager.Save(_camera);

        resetScene();
    }

    void CaseCollide::OnSetupPropsUI() {
        if (ImGui::CollapsingHeader("Config", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat("transl damping", &_translationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("rotate damping", &_rotationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("gravity", &_gravity, 0.01f, 0.0f, 10.0f, "%.2f");
            ImGui::DragFloat("miu_N", &_collisionSystem.miu_N, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat("miu_T", &_collisionSystem.miu_T, 0.01f, 0.01f, 5.0f);
            if (ImGui::Button("Reset")) {
                resetScene();
            }
            ImGui::SameLine();
            if (ImGui::Button(! _pause ? "pause" : "start")) {
                _pause = ! _pause;
            }

            ImGui::DragFloat3("Pos", glm::value_ptr(_player->rigidBody->position));
        }
    }

    Common::CaseRenderResult CaseCollide::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        // apply mouse control first
        OnProcessMouseControl(_cameraManager.getMouseMove());
        OnProcessKeyControl();

        const int substeps = 5;
        // float     dt       = Engine::GetDeltaTime();
        float     dt       = 0.03f;
        float     st       = dt / substeps;
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
        for (auto rb : _rigidBodys) {
            rb->updateBuffer();
        }

        // rendering
        _frame.Resize(desiredSize);

        _cameraManager.Update(_camera);
        _program.GetUniforms().SetByName("u_Projection", _camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _program.GetUniforms().SetByName("u_View", _camera.GetViewMatrix());

        gl_using(_frame);
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(.5f);
        for (auto rb : _rigidBodys) {
            rb->renderItem.drawFace(_program);
        }
        for (auto sb : _staticBodys) {
            sb->renderItem.drawFace(_program);
        }
        for (auto rb : _rigidBodys) {
            rb->renderItem.drawLine(_program);
        }
        for (auto sb : _staticBodys) {
            sb->renderItem.drawLine(_program);
        }
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

        Sphere * player = new Sphere(0.5f);
        Sphere *   obs3 = new Sphere(0.8f);
        Box *      obs4 = new Box(glm::vec3(1.0f));
        obs3->rigidBody->position   = glm::vec3(-1, 2, -2);
        obs4->rigidBody->position   = glm::vec3(-2, 2, 2);
        player->rigidBody->position = glm::vec3(0, 2, 0);

        std::vector<Sphere *> spheres(5);
        for (int i = 0; i < spheres.size(); ++i) {
            spheres[i] = new Sphere();

            spheres[i]->rigidBody->position = glm::vec3(randomFloat() - 0.5, 5 + i * 1.2f, randomFloat() - 0.5);
        }

        _player = player;
        for (auto rb : _rigidBodys) {
            delete rb;
        }
        _rigidBodys = std::vector<Object *>({ player, obs3, obs4 });
        for (int i = 0; i < spheres.size(); ++i) {
            _rigidBodys.push_back(spheres[i]);
        }
        for (auto rb : _rigidBodys) {
            rb->renderItem.color = randomColor();
            rb->initialize();
        }

        _collisionSystem.items.clear();
        for (auto sb : _staticBodys) _collisionSystem.items.push_back(sb);
        for (auto rb : _rigidBodys) _collisionSystem.items.push_back(rb);
    }

} // namespace VCX::Labs::OpenProj
