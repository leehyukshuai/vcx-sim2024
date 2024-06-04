#include "Labs/4-OpenProj/CaseNaive.h"
#include "Labs/Common/ImGuiHelper.h"

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
        }
    }

    Common::CaseRenderResult CaseNaive::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        // apply mouse control first
        OnProcessMouseControl(_cameraManager.getMouseMove());

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
        float movingScale = 0.1f;
    }

} // namespace VCX::Labs::OpenProj
