#include "Labs/4-OpenProj/CaseCollide.h"
#include "Labs/Common/ImGuiHelper.h"
#include "Engine/app.h"
#include <imgui.h>
#include "CaseCollide.h"

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
            ImGui::DragFloat("Restitution", &_collisionSystem.c, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("miu_N", &_collisionSystem.miu_N, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("miu_T", &_collisionSystem.miu_T, 0.01f, 0.0f, 1.0f);
            if (ImGui::Button("Reset")) {
                resetScene();
            }
        }
    }

    Common::CaseRenderResult CaseCollide::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        // apply mouse control first
        OnProcessMouseControl(_cameraManager.getMouseMove());

        const int substeps = 5;
        float dt = Engine::GetDeltaTime();
        float st = dt / substeps;

        for (int t = 0; t < substeps; ++t) {
            for (auto & c : _cylinders) {
                c.rigidBody->applyTranslDamping(0.1f);
                c.rigidBody->applyRotateDamping(0.1f);
                c.rigidBody->apply(-_gravity * c.rigidBody->mass);
                c.rigidBody->update(st);
            }
            _collisionSystem.collisionDetect();
            _collisionSystem.collisionHandle();
            for (auto & c : _cylinders) {
                c.rigidBody->move(st);
            }
        }
        for (auto & c : _cylinders) {
            c.updateBuffer();
        }

        // rendering
        _frame.Resize(desiredSize);

        _cameraManager.Update(_camera);
        _program.GetUniforms().SetByName("u_Projection", _camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _program.GetUniforms().SetByName("u_View", _camera.GetViewMatrix());

        gl_using(_frame);
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(.5f);

        glEnable(GL_DEPTH_TEST);
        for (auto & c : _cylinders) {
            _program.GetUniforms().SetByName("u_Color", c.renderItem.color);
            c.renderItem.faceItem.Draw({ _program.Use() });
        }
        _program.GetUniforms().SetByName("u_Color", _floor.renderItem.color);
        _floor.renderItem.faceItem.Draw({ _program.Use() });
        glDisable(GL_DEPTH_TEST);
        _program.GetUniforms().SetByName("u_Color", glm::vec3(1, 1, 1));
        for (auto & c : _cylinders) {
            c.renderItem.lineItem.Draw({ _program.Use() });
        }
        _floor.renderItem.lineItem.Draw({ _program.Use() });

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
        _cylinders[0].rigidBody->applyTorque(movingScale * mouseDelta);
    }

    void CaseCollide::resetScene() {
        _floor.boxBody.reset();
        _floor.boxBody.dimension = glm::vec3(10, 1, 10);
        _floor.rigidBody->isStatic = true;
        _floor.renderItem.color = glm::vec3(0, 0.8, 0.9);
        _floor.initialize();

        _cylinders.clear();
        _cylinders.resize(3);
        _cylinders[0].rigidBody->position = glm::vec3(0, 2, -0.8);
        _cylinders[0].renderItem.color = glm::vec3(1, 0, 0);
        _cylinders[0].initialize();
        _cylinders[1].rigidBody->position = glm::vec3(0, 3.2, 0);
        _cylinders[1].renderItem.color = glm::vec3(0, 1, 0);
        _cylinders[1].initialize();
        _cylinders[2].rigidBody->position = glm::vec3(0, 5, 0.2);
        _cylinders[2].renderItem.color = glm::vec3(0, 0, 1);
        _cylinders[2].initialize(); 

        _collisionSystem.items = std::vector<Object *>({
            &_cylinders[0], &_cylinders[1], &_cylinders[2], &_floor
        });
    }

} // namespace VCX::Labs::OpenProj
