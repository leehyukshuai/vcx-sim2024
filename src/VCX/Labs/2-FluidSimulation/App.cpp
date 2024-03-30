#include "Assets/bundled.h"
#include "Labs/2-FluidSimulation/App.h"

namespace VCX::Labs::Fluid {

    App::App():
        _ui(Labs::Common::UIOptions {}){
    }

    void App::OnFrame() {
        _ui.Setup(_cases, _caseId);
    }
}
