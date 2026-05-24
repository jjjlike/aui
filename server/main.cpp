#include "aether/aether.h"
#include "aether/RPCServer.h"
#include <iostream>
#include <csignal>
#include <thread>
#include <atomic>

using namespace aether;

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

    LogicLayer logicLayer;

    std::cout << "Creating test components..." << std::endl;
    auto root = logicLayer.createComponent(ComponentType::Container);
    logicLayer.setProperty(root, PropertyId::Width, PropertyValue(800.0f));
    logicLayer.setProperty(root, PropertyId::Height, PropertyValue(600.0f));

    auto button1 = logicLayer.createComponent(ComponentType::Button, root);
    logicLayer.setProperty(button1, PropertyId::Width, PropertyValue(200.0f));
    logicLayer.setProperty(button1, PropertyId::Height, PropertyValue(50.0f));
    logicLayer.setProperty(button1, PropertyId::Text, PropertyValue(std::string("Click Me")));

    auto button2 = logicLayer.createComponent(ComponentType::Button, root);
    logicLayer.setProperty(button2, PropertyId::Width, PropertyValue(200.0f));
    logicLayer.setProperty(button2, PropertyId::Height, PropertyValue(50.0f));
    logicLayer.setProperty(button2, PropertyId::Text, PropertyValue(std::string("Submit")));

    logicLayer.runFrame();

    std::cout << "Initial components created successfully." << std::endl;
    std::cout << "Component tree:" << std::endl;
    std::cout << logicLayer.getTestController().getComponentTreeJSON() << std::endl;

    RPCServer server;

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
