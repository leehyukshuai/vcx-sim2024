#include <spdlog/spdlog.h>
#include "Engine/app.h"
#include "Labs/2-FluidSimulation/CaseFluid.h"
#include "Labs/Common/ImGuiHelper.h"
#include <iostream>

namespace VCX::Labs::Fluid {
    const std::vector<glm::vec3> vertex_pos = {
            glm::vec3(-0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f, -0.5f, -0.5f),  
            glm::vec3(0.5f, 0.5f, -0.5f),  
            glm::vec3(-0.5f, 0.5f, -0.5f), 
            glm::vec3(-0.5f, -0.5f, 0.5f),  
            glm::vec3(0.5f, -0.5f, 0.5f),   
            glm::vec3(0.5f, 0.5f, 0.5f),   
            glm::vec3(-0.5f, 0.5f, 0.5f)
    };
    const std::vector<std::uint32_t> line_index = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 }; // line index

    CaseFluid::CaseFluid() :
        _program(
            Engine::GL::UniqueProgram({
                Engine::GL::SharedShader("assets/shaders/fluid.vert"),
                Engine::GL::SharedShader("assets/shaders/fluid.frag") })),
        _lineprogram(
            Engine::GL::UniqueProgram({
                Engine::GL::SharedShader("assets/shaders/flat.vert"),
                Engine::GL::SharedShader("assets/shaders/flat.frag") })),
        _sceneObject(1),
        _BoundaryItem(Engine::GL::VertexLayout()
            .Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream , 0), Engine::GL::PrimitiveType::Lines){ 
        _cameraManager.AutoRotate = false;
        _program.BindUniformBlock("PassConstants", 1);
        _lineprogram.GetUniforms().SetByName("u_Color",  glm::vec3(1.0f));
        _BoundaryItem.UpdateElementBuffer(line_index);

        _sceneObject.ReplaceScene(VCX::Labs::Rendering::Content::Scenes[std::size_t(Assets::ExampleScene::Fluid)]);
        ResetSystem();
    }

    void CaseFluid::OnSetupPropsUI() {
        if (ImGui::CollapsingHeader("Config", ImGuiTreeNodeFlags_DefaultOpen)) {
            if(ImGui::Button("Reset System")) 
                ResetSystem();
            ImGui::SameLine();
            if(ImGui::Button(_stopped ? "Start Simulation":"Stop Simulation"))
                _stopped = ! _stopped;
            if (ImGui::SliderInt("Resolution", &_res, 4, 64))
                _recompute = true;
        }
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Appearance")) {
            _uniformDirty |= ImGui::RadioButton("Phong Model", &_useBlinn, 0);
            ImGui::SameLine();
            _uniformDirty |= ImGui::RadioButton("Blinn-Phong Model", &_useBlinn, 1);
            _uniformDirty |= ImGui::SliderFloat("Shininess", &_shininess, 1, 128, "%.1f", ImGuiSliderFlags_Logarithmic);
            _uniformDirty |= ImGui::SliderFloat("Ambient", &_ambientScale, 0.f, 2.f, "%.2fx");
            _uniformDirty |= ImGui::Checkbox("Gamma Correction", &_useGammaCorrection);
            _uniformDirty |= ImGui::SliderInt("Attenuation", &_attenuationOrder, 0, 2);

            ImGui::SliderFloat("Bnd Width", &_BndWidth, 0.f, 6.f, "%.1f");
            ImGui::Combo("Anti-Aliasing", &_msaa, "None\0002x MSAA\0004x MSAA\0008x MSAA\0");
            Common::ImGuiHelper::SaveImage(_frame.GetColorAttachment(), _frame.GetSize(), true);
        }
        ImGui::Spacing();

		if (ImGui::CollapsingHeader("Control")) {
            ImGui::Checkbox("Ease Touch", &_cameraManager.EnableDamping);
        }
        ImGui::Spacing();
    }


    Common::CaseRenderResult CaseFluid::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        if (_recompute) {
            _recompute = false;
            ResetSystem();
        }
        if (! _stopped) _simulation.SimulateTimestep(Engine::GetDeltaTime());
        
        _BoundaryItem.UpdateVertexBuffer("position", Engine::make_span_bytes<glm::vec3>(vertex_pos));
        _frame.Resize(desiredSize, 1 << _msaa);

        _cameraManager.Update(_sceneObject.Camera);
        _sceneObject.PassConstantsBlock.Update(&VCX::Labs::Rendering::SceneObject::PassConstants::Projection, _sceneObject.Camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _sceneObject.PassConstantsBlock.Update(&VCX::Labs::Rendering::SceneObject::PassConstants::View, _sceneObject.Camera.GetViewMatrix());
        _sceneObject.PassConstantsBlock.Update(&VCX::Labs::Rendering::SceneObject::PassConstants::ViewPosition, _sceneObject.Camera.Eye);
        _lineprogram.GetUniforms().SetByName("u_Projection", _sceneObject.Camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _lineprogram.GetUniforms().SetByName("u_View"      , _sceneObject.Camera.GetViewMatrix());
        
        if (_uniformDirty) {
            _uniformDirty = false;
            _program.GetUniforms().SetByName("u_AmbientScale"      , _ambientScale);
            _program.GetUniforms().SetByName("u_UseBlinn"          , _useBlinn);
            _program.GetUniforms().SetByName("u_Shininess"         , _shininess);
            _program.GetUniforms().SetByName("u_UseGammaCorrection", int(_useGammaCorrection));
            _program.GetUniforms().SetByName("u_AttenuationOrder"  , _attenuationOrder);            
        }
        
        gl_using(_frame);

        glEnable(GL_DEPTH_TEST);
        glLineWidth(_BndWidth);
        _BoundaryItem.Draw({ _lineprogram.Use() });
        glLineWidth(1.f);

        _r = _simulation.m_particleRadius;
        _numofSpheres = _simulation.m_iNumSpheres;
        _sphere = Engine::Model{.Mesh = Engine::Sphere(6,_r), .MaterialIndex = 0};
        Rendering::ModelObject m = Rendering::ModelObject(_sphere,_simulation.m_particlePos,_simulation.m_particleColor);
        auto const & material    = _sceneObject.Materials[0];
        m.Mesh.Draw({ material.Albedo.Use(),  material.MetaSpec.Use(), material.Height.Use(),_program.Use() },
            _sphere.Mesh.Indices.size(), 0, _numofSpheres);
        
        glDepthFunc(GL_LEQUAL);
        glDepthFunc(GL_LESS);
        glDisable(GL_DEPTH_TEST);

        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseFluid::OnProcessInput(ImVec2 const& pos) {
        _cameraManager.ProcessInput(_sceneObject.Camera, pos);
    }

    void CaseFluid::ResetSystem(){
        _simulation.setupScene(_res);
    }
}