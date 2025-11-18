#pragma once
#include <string>
#include <vector>

extern std::string sanitizeString(const std::string& input);
extern void sanitizeString(const std::string& input, std::vector<uint8_t>& output);
extern std::string join_segments(std::string num);
extern void extractdata(std::shared_ptr<ASTNode>& node, std::vector<uint16_t>& data);
extern void extractworddata(std::shared_ptr<ASTNode>& node, std::vector<uint16_t>& data);

extern std::string toupper(std::string& input);
extern std::string tolower(std::string& input);
extern void calcoutputsize(int& size, std::shared_ptr<ASTNode> node);
extern std::string paddLeft(const std::string& str, size_t totalwidth);
extern std::string paddRight(const std::string& str, size_t totalwidth);
