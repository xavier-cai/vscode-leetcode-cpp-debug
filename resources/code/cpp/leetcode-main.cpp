#include "leetcode-entry.h"

int main() {
    try {
        lc::SIMO io(INPUT, OUTPUT);
        try {
            lc::Entry::Run(io);
            std::cout << "Program compelete." << std::endl;
        }
        catch (lc::StringException e) { // append input info
            int line;
            std::streampos pos;
            std::tie(line, pos) = io.Position();
            std::string ei(e.what());
            if (line > 0) ei += " @File line: " + std::to_string(line)
                              + ", position: " + std::to_string(pos);
            throw lc::StringException(ei);
        }
    }
    catch (lc::StringException e) {
        std::cerr << '\n' << e.what() << std::endl;
        // pause here in terminal
        std::cout << "Press Any to Continue..." << std::endl;
        std::cin.sync(); // clear
        std::cin.get(); // pause
    }
    return 0;
}