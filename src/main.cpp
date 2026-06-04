#include <iostream>
#include <sstream>
#include <string>

#include "InstallationEngine.hpp"

int main()
{
    InstallationEngine engine;
    std::string line;

    while (std::getline(std::cin, line))
    {
        if (line.empty() || line == "_")
        {
            continue;
        }

        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "ADD")
        {
            std::string type, id;
            iss >> type >> id;

            if (type != "MODULE" && type != "PACKAGE")
            {
                std::cout << "ERROR: Invalid command\n";
                continue;
            }

            std::string title;
            std::getline(iss >> std::ws, title);

            if (type == "MODULE")
            {
                engine.addModule(id, title);
            }
            else
            {
                engine.addPackage(id, title);
            }
        }
        else if (command == "ATTACH")
        {
            std::string parentId, childId;
            iss >> parentId >> childId;
            engine.attach(parentId, childId);
        }
        else if (command == "INSTALL")
        {
            std::string target;
            iss >> target;

            if (target == "-A")
            {
                engine.installAll();
            }
            else
            {
                engine.install(target);
            }
        }
        else if (command == "UNINSTALL")
        {
            std::string target;
            iss >> target;

            if (target == "-A")
            {
                engine.uninstallAll();
            }
            else
            {
                engine.uninstall(target);
            }
        }
        else if (command == "MOCK_FAIL")
        {
            std::string id;
            iss >> id;
            engine.mockFail(id);
        }
        else if (command == "RESOLVE_FAIL")
        {
            std::string id;
            iss >> id;
            engine.resolve(id);
        }
        else if (command == "CMD")
        {
            std::string id;
            iss >> id;
            engine.printComponent(id);
        }
        else if (command == "END" || command == "EN")
        {
            break;
        }
        else
        {
            std::cout << "ERROR: Invalid command\n";
        }
    }

    return 0;
}
