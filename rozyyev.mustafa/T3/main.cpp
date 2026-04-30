#include "Commands.h"
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return EXIT_FAILURE;
    }

    std::ifstream input(argv[1]);
    if (!input) {
        std::cerr << "Could not open input file\n";
        return EXIT_FAILURE;
    }

    std::vector<Polygon> polygons;
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        Polygon p;
        if (iss >> p) {
            iss >> std::ws;
            if (iss.eof()) {
                polygons.push_back(p);
            }
        }
    }

    input.close();

    processCommands(polygons, std::cin, std::cout);

    return EXIT_SUCCESS;
}
