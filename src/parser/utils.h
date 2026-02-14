#pragma once
#include <string>
#include <vector>

/*
 utils.h
 -------
 Utility helpers used by the parser/assembler for string sanitization,
 numeric segment normalization, AST data extraction, case conversion,
 output-size calculation and simple padding helpers.

 All functions are declared with the same signatures used throughout the
 codebase; implementations live in the corresponding .cpp file.
*/

 /*
 Convert an input literal or raw string into a sanitized string suitable
 for printing or further parsing. This will typically interpret escape
 sequences, remove surrounding quotes where appropriate and translate any
 assembler-specific escapes into their textual representation.
 Parameters:
   - input: source text to sanitize.
 Returns:
   - sanitized textual representation.
*/
extern std::string sanitizeString(const std::string& input);

/*
 Variant of sanitizeString that converts the input into a sequence of raw
 byte values and appends them to 'output'. Useful when a string literal
 represents binary data that should be emitted as bytes.
 Parameters:
   - input: source text to sanitize/parse.
   - output: byte vector that will receive the parsed bytes.
*/
extern void sanitizeString(const std::string& input, std::vector<uint8_t>& output);

/*
 Normalize or join numeric segments inside a textual numeric expression.
 Many assemblers allow numbers to be written in segmented forms (for
 example with separators). This helper returns a canonical / joined
 representation suitable for numeric parsing.
 Parameters:
   - num: numeric string possibly containing separators or segments.
 Returns:
   - normalized/joined numeric string.
*/
extern std::string join_segments(std::string num);

/*
 Extract byte-oriented data values from an AST node into a numeric vector.
 Intended for .byte / .db style directives where each expression yields
 an 8-bit value. Values are stored in the lower 8 bits of each uint16_t.
 Parameters:
   - node: AST node containing expressions or data directives.
   - data: vector that will receive numeric results.
*/
extern void extractdata(std::shared_ptr<ASTNode>& node, std::vector<uint16_t>& data);

/*
 Extract word-oriented (16-bit) data values from an AST node into a numeric vector.
 Intended for .word / .dw style directives where each expression yields a 16-bit value.
 Parameters:
   - node: AST node containing expressions or data directives.
   - data: vector that will receive 16-bit numeric results.
*/
extern void extractworddata(std::shared_ptr<ASTNode>& node, std::vector<uint16_t>& data);

/*
 Return an uppercase copy of the provided string.
 Note: the parameter is non-const to match existing call sites; the function
 returns a new string and does not mutate the caller's object.
 Parameters:
   - input: string to convert (passed by reference).
 Returns:
   - uppercase copy of input.
*/
extern std::string toupper(std::string& input);

/*
 Return a lowercase copy of the provided string.
 Parameters:
   - input: string to convert (passed by reference).
 Returns:
   - lowercase copy of input.
*/
extern std::string tolower(std::string& input);

/*
 Calculate or accumulate the output size produced by an AST node.
 The helper updates 'size' to account for the number of bytes (or
 other units) that the node will emit when assembled. Used during
 pass counting and layout calculation.
 Parameters:
   - size: reference to an integer accumulator (modified in place).
   - node: AST node whose output size should be accounted for.
*/
extern void calcoutputsize(int& size, std::shared_ptr<ASTNode> node);

/*
 Return a new string that is the input padded on the left with spaces
 until it reaches 'totalwidth'. If the input is already equal or longer
 than totalwidth the original string is returned.
 Parameters:
   - str: source text.
   - totalwidth: desired minimum width in characters.
 Returns:
   - left-padded string.
*/
extern std::string paddLeft(const std::string& str, size_t totalwidth);

/*
 Return a new string that is the input padded on the right with spaces
 until it reaches 'totalwidth'. If the input is already equal or longer
 than totalwidth the original string is returned.
 Parameters:
   - str: source text.
   - totalwidth: desired minimum width in characters.
 Returns:
   - right-padded string.
*/
extern std::string paddRight(const std::string& str, size_t totalwidth);
