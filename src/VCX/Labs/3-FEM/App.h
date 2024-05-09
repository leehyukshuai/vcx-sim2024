#pragma once

#include "Engine/app.h"
#include "Labs/3-FEM/CaseTetra.h"
#include "Labs/Common/UI.h"
#include <vector>

namespace VCX::Labs::FEM {
    class App : public Engine::IApp {
    private:
        Common::UI _ui;

        CaseTetra _caseTetra;

        std::size_t _caseId = 0;

        std::vector<std::reference_wrapper<Common::ICase>> _cases = { _caseTetra };

    public:
        App();

        void OnFrame() override;
    };
}
