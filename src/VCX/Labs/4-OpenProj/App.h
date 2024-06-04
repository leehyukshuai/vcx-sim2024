#pragma once

#include "Engine/app.h"
#include "Labs/Common/UI.h"
#include "CaseNaive.h"
#include <vector>

namespace VCX::Labs::OpenProj {
    class App : public Engine::IApp {
    private:
        Common::UI _ui;

        CaseNaive        _caseNaive;

        std::size_t _caseId = 0;

        std::vector<std::reference_wrapper<Common::ICase>> _cases = { _caseNaive };

    public:
        App();

        void OnFrame() override;
    };
}
