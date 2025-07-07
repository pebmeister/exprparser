#pragma once
#include <string>
#include <vector>

extern std::string sanitizeString(const std::string& input);
extern void sanitizeString(const std::string& input, std::vector<uint8_t>& output);
extern std::string join_segments(std::string num);
extern void extract(std::shared_ptr<ASTNode>& node, std::vector<uint16_t>& data);
extern void extractwords(std::shared_ptr<ASTNode>& node, std::vector<uint16_t>& data);
extern void handle_symbol(std::shared_ptr<ASTNode>& node, Parser& p, const Token& tok, 
    std::map<std::string, Sym>& symTable, bool isGlobal);
extern void handle_local_sym(std::shared_ptr<ASTNode>& node, Parser& p, const Token& tok);
extern void handle_global_sym(std::shared_ptr<ASTNode>& node, Parser& p, const Token& tok);
extern std::shared_ptr<ASTNode> processRule(RULE_TYPE ruleType,
    const std::vector<RuleArg>& args, Parser& p, int count);
extern std::shared_ptr<ASTNode> processRule(std::vector<RULE_TYPE> rule,
    RuleArg l, RuleArg r, Parser& p, int count);
