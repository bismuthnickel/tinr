#include <algorithm>
#include <string>
#include "stringtoint.hpp"

int
stringToInt(const std::string& str) {
    if (str.empty()) {
        return 0;
    }

    for (char c : str) {
        if (!std::isdigit(c)) {
            return 0;
        }
    }

    try {
        return std::stoi(str);
    } catch (const std::exception& e) {
        return 0;
    }
}
