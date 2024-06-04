#include "Assets/bundled.h"
#include "Labs/4-OpenProj/App.h"

int main() {
    using namespace VCX;
    return Engine::RunApp<Labs::OpenProj::App>(Engine::AppContextOptions {
        .Title         = "VCX-sim Labs 4: Open Project",
        .WindowSize    = {1024, 768},
        .FontSize      = 16,
        .IconFileNames = Assets::DefaultIcons,
        .FontFileNames = Assets::DefaultFonts,
    });
}
