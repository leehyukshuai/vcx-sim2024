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

        // std::vector<glm::vec3> VertsPosition;
        // glm::vec3              new_x = _dim[0] / 2 * glm::vec3(1.f, 0.f, 0.f);
        // glm::vec3              new_y = _dim[1] / 2 * glm::vec3(0.f, 1.f, 0.f);
        // glm::vec3              new_z = _dim[2] / 2 * glm::vec3(0.f, 0.f, 1.f);
        // VertsPosition.resize(8);
        // VertsPosition[0] = _center - new_x + new_y + new_z;
        // VertsPosition[1] = _center + new_x + new_y + new_z;
        // VertsPosition[2] = _center + new_x + new_y - new_z;
        // VertsPosition[3] = _center - new_x + new_y - new_z;
        // VertsPosition[4] = _center - new_x - new_y + new_z;
        // VertsPosition[5] = _center + new_x - new_y + new_z;
        // VertsPosition[6] = _center + new_x - new_y - new_z;
        // VertsPosition[7] = _center - new_x - new_y - new_z;

        // auto span_bytes = Engine::make_span_bytes<glm::vec3>(VertsPosition);

        // _program.GetUniforms().SetByName("u_Color", _boxColor);
        // _boxItem.UpdateVertexBuffer("position", span_bytes);
        // _boxItem.Draw({ _program.Use() });

        // _program.GetUniforms().SetByName("u_Color", glm::vec3(1.f, 1.f, 1.f));
        // _lineItem.UpdateVertexBuffer("position", span_bytes);
        // _lineItem.Draw({ _program.Use() });

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
