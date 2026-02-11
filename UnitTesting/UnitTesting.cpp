#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>
#include <iomanip>

struct Error {
    std::string message;
};

static std::vector<uint8_t> parseHexBytes(const std::string& text)
{
    std::vector<uint8_t> bytes;
    std::regex hexByte(R"(\$?([0-9A-Fa-f]{2}))");
    auto begin = std::sregex_iterator(text.begin(), text.end(), hexByte);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it)
        bytes.push_back(static_cast<uint8_t>(
            std::stoi((*it)[1].str(), nullptr, 16)));

    return bytes;
}

std::vector<Error> validateAssemblerOutput(const std::string& input)
{
    std::vector<Error> errors;
    std::unordered_map<std::string, uint16_t> symbols;

    std::istringstream stream(input);
    std::string line;

    bool inSymbolTable = true;

    std::regex symbolRegex(R"(^([A-Za-z_][A-Za-z0-9_]*)\s+\$([0-9A-Fa-f]+))");
    std::regex listingRegex(
        R"(\$\s*([0-9A-Fa-f]{4})\s*:\s*((?:\$\s*[0-9A-Fa-f]{2}\s*)+))");

    while (std::getline(stream, line)) {
        std::smatch match;

        // ---------- SYMBOL TABLE ----------
        if (inSymbolTable) {
            if (line.find("Processing") != std::string::npos) {
                inSymbolTable = false;
                continue;
            }

            if (std::regex_search(line, match, symbolRegex)) {
                std::string name = match[1];
                uint16_t value = static_cast<uint16_t>(
                    std::stoi(match[2], nullptr, 16));
                symbols[name] = value;
            }
            continue;
        }

        // ---------- LISTING ----------
        if (!std::regex_search(line, match, listingRegex))
            continue;

        uint16_t pc = static_cast<uint16_t>(
            std::stoi(match[1], nullptr, 16));

        auto emittedBytes = parseHexBytes(match[2]);

        // Extract expected bytes from comment
        auto semicolonPos = line.find(';');
        if (semicolonPos == std::string::npos)
            continue;

        auto expectedBytes = parseHexBytes(
            line.substr(semicolonPos + 1));

        // Compare emitted vs expected
        if (emittedBytes != expectedBytes) {
            std::ostringstream oss;
            oss << "Byte mismatch at PC $"
                << std::hex << std::uppercase << pc;
            errors.push_back({ oss.str() });
        }

        // Detect label in listing

        auto firstColon = line.find(':');
        if (firstColon != std::string::npos) {
            std::string afterPC = line.substr(firstColon + 1);


            std::regex labelRegex(R"(([A-Za-z_][A-Za-z0-9_]*)\s*:)");
            if (std::regex_search(afterPC, match, labelRegex)) {
                std::string label = match[1];

                auto it = symbols.find(label);
                if (it == symbols.end()) {
                    errors.push_back({ "Undefined symbol: " + label });
                }
                else if (it->second != pc) {
                    std::ostringstream oss;
                    oss << "Symbol mismatch for " << label
                        << " expected $" << std::hex << it->second
                        << " but found $" << pc;
                    errors.push_back({ oss.str() });
                }
            }
        }
    }

    return errors;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: verify <listing-file>\n";
        return 1;
    }

    std::string filename = argv[1];

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: cannot open file '" << filename << "'\n";
        return 1;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string input = buffer.str();

    // Now run your existing verification logic on `input`

    auto errors = validateAssemblerOutput(input);

    if (errors.empty())
        std::cout << "No errors found.\n";
    else
        for (auto& e : errors)
            std::cout << e.message << "\n";
}
