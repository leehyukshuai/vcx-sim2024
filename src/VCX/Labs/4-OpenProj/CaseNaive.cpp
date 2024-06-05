#include "Labs/4-OpenProj/CaseNaive.h"
#include "Labs/Common/ImGuiHelper.h"
#include "Engine/app.h"
#include <imgui.h>

namespace VCX::Labs::OpenProj {

    CaseNaive::CaseNaive():
        _program(
            Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/flat.vert"),
                                        Engine::GL::SharedShader("assets/shaders/flat.frag") })) {
        _cameraManager.AutoRotate = false;
        _cameraManager.Save(_camera);
    }

    void CaseNaive::OnSetupPropsUI() {
        if (ImGui::CollapsingHeader("Config", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool recompute = false;
            float *radius = &_cylinder.cylinderBody.radius;
            float *height = &_cylinder.cylinderBody.height;
            recompute |= ImGui::DragFloat("Radius", radius, 0.01f, 0.5f, 10.f);
            recompute |= ImGui::DragFloat("Height", height, 0.01f, 0.5f, 10.f);
            if (recompute) {
                _cylinder = Cylinder(*radius, *height);
                _cylinder.initialize();
            }
        }
    }

    Common::CaseRenderResult CaseNaive::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        // apply mouse control first
        OnProcessMouseControl(_cameraManager.getMouseMove());

        // damping
        _cylinder.cylinderBody.applyTranslDamping(0.1f);
        _cylinder.cylinderBody.applyRotateDamping(0.1f);
        _cylinder.cylinderBody.update(Engine::GetDeltaTime());
        _cylinder.cylinderBody.move(Engine::GetDeltaTime());

        _cylinder.renderItem.updateBuffer(_cylinder.cylinderBody.position, _cylinder.cylinderBody.orientation);

        // rendering
        _frame.Resize(desiredSize);

        _cameraManager.Update(_camera);
        _program.GetUniforms().SetByName("u_Projection", _camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _program.GetUniforms().SetByName("u_View", _camera.GetViewMatrix());

        gl_using(_frame);
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(.5f);

        _program.GetUniforms().SetByName("u_Color", _color);
        _cylinder.renderItem.faceItem.Draw({ _program.Use() });
        _program.GetUniforms().SetByName("u_Color", glm::vec3(1, 1, 1));
        _cylinder.renderItem.lineItem.Draw({ _program.Use() });

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

    void CaseNaive::OnProcessInput(ImVec2 const & pos) {
        _cameraManager.ProcessInput(_camera, pos);
    }

    void CaseNaive::OnProcessMouseControl(glm::vec3 mouseDelta) {
        float movingScale = 8.0f;
        _cylinder.cylinderBody.applyTorque(movingScale * mouseDelta);
    }

} // namespace VCX::Labs::OpenProj
