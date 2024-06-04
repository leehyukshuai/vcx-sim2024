#include "Assets/bundled.h"
#include "Labs/4-OpenProj/App.h"

namespace VCX::Labs::OpenProj {

    App::App():
        _ui(Labs::Common::UIOptions {}) {
    }

    void App::OnFrame() {
        _ui.Setup(_cases, _caseId);
    }
}
