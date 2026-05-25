#include "aether/aether.h"
#include "aether/RPCServer.h"
#include <iostream>
#include <csignal>
#include <thread>
#include <atomic>

using namespace jaether;

static std::atomic<bool> gRunning(true);

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        gRunning.store(false);
    }
}

int main(int argc, char* argv[]) {
    std::cout << "===== Aether Logic Server =====" << std::endl;
    std::cout << "Version: " << AETHER_VERSION << std::endl;
    std::cout << "==============================" << std::endl;

    std::string pipeName = "aether_logic";
    if (argc > 1) {
        pipeName = argv[1];
    }

    JLogicLayer logicLayer;

    std::cout << "Creating test components..." << std::endl;
    auto root = logicLayer.createComponent(JComponentType::Container);
    logicLayer.setProperty(root, JPropertyId::Width, JPropertyValue(800.0f));
    logicLayer.setProperty(root, JPropertyId::Height, JPropertyValue(600.0f));

    auto button1 = logicLayer.createComponent(JComponentType::Button, root);
    logicLayer.setProperty(button1, JPropertyId::Width, JPropertyValue(200.0f));
    logicLayer.setProperty(button1, JPropertyId::Height, JPropertyValue(50.0f));
    logicLayer.setProperty(button1, JPropertyId::Text, JPropertyValue(std::string("Click Me")));

    auto button2 = logicLayer.createComponent(JComponentType::Button, root);
    logicLayer.setProperty(button2, JPropertyId::Width, JPropertyValue(200.0f));
    logicLayer.setProperty(button2, JPropertyId::Height, JPropertyValue(50.0f));
    logicLayer.setProperty(button2, JPropertyId::Text, JPropertyValue(std::string("Submit")));

    logicLayer.runFrame();

    std::cout << "Initial components created successfully." << std::endl;
    std::cout << "Component tree:" << std::endl;
    std::cout << logicLayer.getTestController().getComponentTreeJSON() << std::endl;

    JRPCServer server;

    std::cout << "Starting RPC server on pipe " << pipeName << "..." << std::endl;
    if (!server.start(pipeName)) {
        std::cerr << "Failed to start RPC server" << std::endl;
        return 1;
    }

    std::cout << "Server is running. Press Ctrl+C to stop." << std::endl;

    while (gRunning.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Stopping server..." << std::endl;
    server.shutdown();

    std::cout << "Server stopped." << std::endl;
    return 0;
}
