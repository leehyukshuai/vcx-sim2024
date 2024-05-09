#include "Assets/bundled.h"
#include "Labs/3-FEM/App.h"
#include <iostream>

int main() {
    using namespace VCX;
    return Engine::RunApp<Labs::FEM::App>(Engine::AppContextOptions {
        .Title         = "VCX-sim Labs 3: FEM",
        .WindowSize    = {1024, 768},
        .FontSize      = 16,
        .IconFileNames = Assets::DefaultIcons,
        .FontFileNames = Assets::DefaultFonts,
    });
}
