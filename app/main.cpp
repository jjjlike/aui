#include "aether/AetherApplication.h"
#include <windows.h>
#include <iostream>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    
    jaether::JAetherApplication app;
    
    if (!app.initialize(hInstance, nCmdShow)) {
        std::cerr << "Failed to initialize application" << std::endl;
        return 1;
    }
    
    app.run();
    
    CoUninitialize();
    
    return 0;
}
