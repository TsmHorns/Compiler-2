#include <iostream> // Debugging: Included iostream for standard input/output
#include <unordered_map> // Debugging: Included unordered_map for storing variables
#include <memory> // Debugging: Included memory for smart pointers
#include <fstream> // Debugging: Included fstream for file handling
#include <sstream> // Debugging: Included sstream for string streaming
#include <vector> // Debugging: Included vector for storing tokens
#include <string> // Debugging: Included string for string manipulation

enum class TokenType { // Debugging: Defined an enum class for token types
    ID, NUM, ASSIGN, PRINT, STRING, SEMICOLON, END, COMMENT, PLUS, MINUS, MULTIPLY, DIVIDE,
    GREATER_THAN, LESS_THAN, GREATER_THAN_EQUAL, LESS_THAN_EQUAL, EQUALS, NOT_EQUALS,
    LEFT_PAREN, RIGHT_PAREN, NUMBER, COMMA, NEWLINE, IF, ELSE
};



struct Token { // Debugging: Defined a struct for tokens
    TokenType type; // Debugging: Added a member for token type
    std::string value; // Debugging: Added a member for token value

    Token(TokenType type, std::string value) : type(type), value(value) {} // Debugging: Initialized token with type and value
};

class ASTNode { // Debugging: Defined a base class for abstract syntax tree nodes
public:
    virtual ~ASTNode() = default; // Debugging: Added a virtual destructor for base class
    virtual void evaluate(std::unordered_map<std::string, int>& context) = 0; // Debugging: Added a virtual evaluate function
};



class IntegerNode : public ASTNode { // Debugging: Defined a class for integer nodes
    int value; // Debugging: Added a member for integer value
public:
    IntegerNode(int val) : value(val) {} // Debugging: Initialized integer node with value
    void evaluate(std::unordered_map<std::string, int>& context) override { // Debugging: Implemented evaluate function for integer node
        context["__expr_result"] = value; // Debugging: Assigned value directly to context
    }
};

class VariableNode : public ASTNode { // Debugging: Defined a class for variable nodes
    std::string name; // Debugging: Added a member for variable name
public:
    VariableNode(const std::string& n) : name(n) {} // Debugging: Initialized variable node with name
    void evaluate(std::unordered_map<std::string, int>& context) override { // Debugging: Implemented evaluate function for variable node
        if (context.find(name) != context.end()) { // Debugging: Checked if variable exists in context
            context["__expr_result"] = context[name]; // Debugging: Assigned variable value to context
        } else {
            std::cout << "Variable '" << name << "' not found." << std::endl; // Debugging: Output message for variable not found
        }
    }
};


class PrintNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> parts;

public:
    PrintNode(std::vector<std::unique_ptr<ASTNode>> parts) : parts(std::move(parts)) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
        for (auto& part : parts) {
            part->evaluate(context);
            std::cout << context["__expr_result"];
        }
        std::cout << std::endl;
    }
};

class IfNode : public ASTNode { // Debugging: Defined a class for if nodes
    std::unique_ptr<ASTNode> condition; // Debugging: Added a member for condition
    std::vector<std::unique_ptr<ASTNode>> ifBlock; // Debugging: Added a member for if block
    std::vector<std::unique_ptr<ASTNode>> elseBlock; // Debugging: Added a member for else block
public:
    IfNode(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> ifBlk, std::vector<std::unique_ptr<ASTNode>> elseBlk)
        : condition(std::move(cond)), ifBlock(std::move(ifBlk)), elseBlock(std::move(elseBlk)) {} // Debugging: Initialized if node with condition and blocks

    void evaluate(std::unordered_map<std::string, int>& context) override { // Debugging: Implemented evaluate function for if node
        condition->evaluate(context); // Debugging: Evaluated condition
        int result = context["__expr_result"]; // Debugging: Stored result in a variable
        if (result != 0) { // Debugging: Checked if result is not zero
            for (auto& stmt : ifBlock) { // Debugging: Iterated over statements in if block
                stmt->evaluate(context); // Debugging: Evaluated statement
            }
        } else {
            for (auto& stmt : elseBlock) { // Debugging: Iterated over statements in else block
                stmt->evaluate(context); // Debugging: Evaluated statement
            }
        }
    }
};

class AssignmentNode : public ASTNode { // Debugging: Defined a class for assignment nodes
    std::string id; // Debugging: Added a member for variable id
    std::unique_ptr<ASTNode> expression; // Debugging: Added a member for expression
public:
    AssignmentNode(std::string varId, std::unique_ptr<ASTNode> expr)
        : id(varId), expression(std::move(expr)) {} // Debugging: Initialized assignment node with id and expression

    void evaluate(std::unordered_map<std::string, int>& context) override { // Debugging: Implemented evaluate function for assignment node
        expression->evaluate(context); // Debugging: Evaluated expression
        context[id] = context["__expr_result"]; // Debugging: Assigned result to variable in context
    }
};

class BinaryOperationNode : public ASTNode { // Debugging: Defined a class for binary operation nodes
    std::unique_ptr<ASTNode> left; // Debugging: Added a member for left operand
    std::unique_ptr<ASTNode> right; // Debugging: Added a member for right operand
    TokenType op; // Debugging: Added a member for operator
public:
    BinaryOperationNode(std::unique_ptr<ASTNode> l, TokenType o, std::unique_ptr<ASTNode> r)
        : left(std::move(l)), op(o), right(std::move(r)) {} // Debugging: Initialized binary operation node with operands and operator

    void evaluate(std::unordered_map<std::string, int>& context) override { // Debugging: Implemented evaluate function for binary operation node
        left->evaluate(context); // Debugging: Evaluated left operand
        int leftVal = context["__expr_result"]; // Debugging: Stored result in a variable
        right->evaluate(context); // Debugging: Evaluated right operand
        int rightVal = context["__expr_result"]; // Debugging: Stored result in a variable
        switch (op) { // Debugging: Checked operator type
            case TokenType::PLUS:
                context["__expr_result"] = leftVal + rightVal; // Debugging: Added left and right operands
                break;
            case TokenType::MINUS:
                context["__expr_result"] = leftVal - rightVal; // Debugging: Subtracted right operand from left operand
                break;
            case TokenType::MULTIPLY:
                context["__expr_result"] = leftVal * rightVal; // Debugging: Multiplied left and right operands
                break;
            case TokenType::DIVIDE:
                if (rightVal == 0) { // Debugging: Checked for division by zero
                    throw std::runtime_error("Division by zero");
                }
                context["__expr_result"] = leftVal / rightVal; // Debugging: Divided left operand by right operand
                break;
            default:
                std::cout << "Unsupported operation." << std::endl; // Debugging: Output message for unsupported operation
                break;
        }
    }
};

// Forward declarations for parsing functions
std::unique_ptr<ASTNode> parseExpression(std::vector<Token>& tokens, std::vector<Token>::iterator& it); // Debugging: Declared parseExpression function

std::unique_ptr<ASTNode> parseFactor(std::vector<Token>& tokens, std::vector<Token>::iterator& it); // Debugging: Declared parseFactor function

std::unique_ptr<ASTNode> parseTerm(std::vector<Token>& tokens, std::vector<Token>::iterator& it) { // Debugging: Defined parseTerm function
    std::cout << "Entering parseTerm" << std::endl;
    std::unique_ptr<ASTNode> left = parseFactor(tokens, it); // Debugging: Parsed left operand
    while (it != tokens.end() && (it->type == TokenType::MULTIPLY || it->type == TokenType::DIVIDE)) { // Debugging: Checked for additional terms
        TokenType op = it->type; // Debugging: Stored operator type in a variable
        ++it;  // Debugging: Moved past the operator
        auto right = parseFactor(tokens, it); // Debugging: Parsed right operand
        left = std::make_unique<BinaryOperationNode>(std::move(left), op, std::move(right)); // Debugging: Created binary operation node
    }
    std::cout << "Exiting parseTerm" << std::endl;
    return left; // Debugging: Returned result
}

std::unique_ptr<ASTNode> parseExpression(std::vector<Token>& tokens, std::vector<Token>::iterator& it) { // Debugging: Defined parseExpression function
    std::unique_ptr<ASTNode> left = parseTerm(tokens, it); // Debugging: Parsed left operand
    while (it != tokens.end() && (it->type == TokenType::PLUS || it->type == TokenType::MINUS)) { // Debugging: Checked for additional expressions
        TokenType op = it->type; // Debugging: Stored operator type in a variable
        ++it;  // Debugging: Moved past the operator
        auto right = parseTerm(tokens, it); // Debugging: Parsed right operand
        left = std::make_unique<BinaryOperationNode>(std::move(left), op, std::move(right)); // Debugging: Created binary operation node
    }

    std::cout << "Exiting parseExpression" << std::endl;
    return left; // Debugging: Returned result
}

std::unique_ptr<ASTNode> parseFactor(std::vector<Token>& tokens, std::vector<Token>::iterator& it) {
    switch (it->type) {
        case TokenType::NUM:
            {
                int value = std::stoi(it->value);
                ++it;  // Consume the NUM token
                return std::make_unique<IntegerNode>(value);
            }
        case TokenType::ID:
            {
                std::string varName = it->value;
                ++it;  // Consume the ID token
                while (it != tokens.end() && it->type == TokenType::COMMA) {
                    ++it;  // Consume ','
                }
                return std::make_unique<VariableNode>(varName);
            }
        case TokenType::LEFT_PAREN:
            {
                ++it;  // Consume '('
                auto expr = parseExpression(tokens, it);
                if (it->type != TokenType::RIGHT_PAREN) {
                    throw std::runtime_error("Expected ')' after expression");
                }
                ++it;  // Consume ')'
                return expr;
            }
        default:
            {
                std::cout << "Unexpected token in parseFactor: " << it->value << " of type " << static_cast<int>(it->type) << std::endl;
                throw std::runtime_error("Syntax error: unexpected token in expression in parse factor ");
            }
    }
}



// Define StringNode correctly
class StringNode : public ASTNode {
    std::string value;
public:
    StringNode(const std::string& val) : value(val) {}
    void evaluate(std::unordered_map<std::string, int>& context) override {
        context["__expr_result"] = std::stoi(value); // Simple conversion for demonstration
    }
};

// Update the parsePrintStatement to correctly handle ASTNode types
std::unique_ptr<ASTNode> parsePrintStatement(std::vector<Token>& tokens, std::vector<Token>::iterator& it) {
    std::vector<std::unique_ptr<ASTNode>> parts;
    ++it; // Skip '('

    while (it != tokens.end() && it->type != TokenType::RIGHT_PAREN) {
        if (it->type == TokenType::STRING) {
            parts.push_back(std::make_unique<StringNode>(it->value));
        } else if (it->type == TokenType::ID) {
            parts.push_back(std::make_unique<VariableNode>(it->value));
        }
        ++it;
        if (it->type == TokenType::COMMA) ++it; // Skip commas
    }

    ++it; // Skip ')'
    return std::make_unique<PrintNode>(std::move(parts));
}


TokenType getTokenType(char ch) { // Debugging: Defined getTokenType function
    switch (ch) { // Debugging: Checked for different characters
        case '>': return TokenType::GREATER_THAN;
        case '<': return TokenType::LESS_THAN;
        case '=': return TokenType::ASSIGN;
        case '+': return TokenType::PLUS;
        case '-': return TokenType::MINUS;
        case '*': return TokenType::MULTIPLY;
        case '/': return TokenType::DIVIDE;
        case '(': return TokenType::LEFT_PAREN;
        case ')': return TokenType::RIGHT_PAREN;
        case ',': return TokenType::COMMA;
        case ';': return TokenType::SEMICOLON;
        case '\"': return TokenType::STRING;
        case '#': return TokenType::COMMENT;
        default:
            if (isdigit(ch)) return TokenType::NUM;
            else if (isalpha(ch)) return TokenType::ID;
            else return TokenType::END;
    }
}


std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    std::istringstream iss(input);
    char ch;

    while (iss >> std::noskipws >> ch) {
        std::cout << "Processing character: " << ch << std::endl;

        if (isspace(ch)) continue;

        if (ch == '"') { // Handle strings
            std::string str = "\"";
            while (iss >> std::noskipws >> ch && ch != '"') {
                str += ch;
            }
            str += '"';
            tokens.emplace_back(TokenType::STRING, str);
            std::cout << "Added STRING token: " << str << std::endl;
        } else if (isdigit(ch)) { // Handle numbers
            std::string num;
            iss.putback(ch);
            iss >> num;
            tokens.emplace_back(TokenType::NUM, num);
            std::cout << "Added NUM token: " << num << std::endl;
        } else if (isalpha(ch)) { // Handle identifiers and keywords
            std::string ident(1, ch);
            while (iss.peek() != EOF && isalnum(iss.peek())) {
                iss >> ch;
                ident += ch;
            }
            if (ident == "print") {
                tokens.emplace_back(TokenType::PRINT, ident);
                std::cout << "Added PRINT token: " << ident << std::endl;
            } else {
                tokens.emplace_back(TokenType::ID, ident);
                std::cout << "Added ID token: " << ident << std::endl;
            }
        } else {
            switch (ch) {
                case '=':
                    if (iss.peek() == '=') {
                        iss.get();
                        tokens.emplace_back(TokenType::EQUALS, "==");
                        std::cout << "Added EQUALS token: ==" << std::endl;
                    } else {
                        tokens.emplace_back(TokenType::ASSIGN, "=");
                        std::cout << "Added ASSIGN token: =" << std::endl;
                    }
                    break;
                case '+':
                    tokens.emplace_back(TokenType::PLUS, "+");
                    std::cout << "Added PLUS token: +" << std::endl;
                    break;
                case '-':
                    tokens.emplace_back(TokenType::MINUS, "-");
                    std::cout << "Added MINUS token: -" << std::endl;
                    break;
                case '*':
                    tokens.emplace_back(TokenType::MULTIPLY, "*");
                    std::cout << "Added MULTIPLY token: *" << std::endl;
                    break;
                case '/':
                    tokens.emplace_back(TokenType::DIVIDE, "/");
                    std::cout << "Added DIVIDE token: /" << std::endl;
                    break;
                case '(':
                    tokens.emplace_back(TokenType::LEFT_PAREN, "(");
                    std::cout << "Added LEFT_PAREN token: (" << std::endl;
                    break;
                case ')':
                    tokens.emplace_back(TokenType::RIGHT_PAREN, ")");
                    std::cout << "Added RIGHT_PAREN token: )" << std::endl;
                    break;
                case ',':
                    tokens.emplace_back(TokenType::COMMA, ",");
                    std::cout << "Added COMMA token: ," << std::endl;
                    break;
                case ';':
                    tokens.emplace_back(TokenType::SEMICOLON, ";");
                    std::cout << "Added SEMICOLON token: ;" << std::endl;
                    break;
                default:
                    // Handle unrecognized characters
                    std::cout << "Unrecognized character: " << ch << std::endl;
                    break;
            }
        }
    }
    tokens.emplace_back(TokenType::END, ""); // Mark the end of the input
    std::cout << "Finished tokenizing" << std::endl;
    return tokens;
}




std::unique_ptr<ASTNode> parseStatement(std::vector<Token>& tokens, std::vector<Token>::iterator& it) { // Debugging: Defined parseStatement function
    // Parse variable name
    if (it->type != TokenType::ID) { // Debugging: Checked for ID token
        throw std::runtime_error("Expected variable name in assignment statement"); // Debugging: Threw error if ID token not found
    }
    std::string id = it->value; // Debugging: Get variable name
    ++it; // Debugging: Consume variable name token

    // Ensure next token is assignment operator
    if (it->type != TokenType::ASSIGN) { // Debugging: Checked for assignment operator
        throw std::runtime_error("Expected '=' in assignment statement"); // Debugging: Threw error if assignment operator not found
    }
    ++it; // Debugging: Consume assignment operator token

    // Parse expression
    auto expression = parseExpression(tokens, it); // Debugging: Parsed expression

    // Ensure next token is semicolon
    if (it->type != TokenType::SEMICOLON) { // Debugging: Checked for semicolon
        throw std::runtime_error("Expected ';' at the end of statement"); // Debugging: Threw error if semicolon not found
    }
    ++it; // Debugging: Consume semicolon token

    return std::make_unique<AssignmentNode>(id, std::move(expression)); // Debugging: Created assignment node
}

std::unique_ptr<ASTNode> parseIf(std::vector<Token>& tokens, std::vector<Token>::iterator& it) { // Debugging: Defined parseIf function
    ++it; // Debugging: Skip the 'if' token
    auto condition = parseExpression(tokens, it); // Debugging: Parsed condition
    std::vector<std::unique_ptr<ASTNode>> ifBlock; // Debugging: Created vector for 'if' block
    std::vector<std::unique_ptr<ASTNode>> elseBlock; // Debugging: Created vector for 'else' block

    // Parse the 'if' block
    while (it != tokens.end() && it->type != TokenType::ELSE && it->type != TokenType::END) { // Debugging: Checked for end of 'if' block
        ifBlock.push_back(parseStatement(tokens, it)); // Debugging: Parsed statement for 'if' block
    }

    // Parse the 'else' block
    if (it != tokens.end() && it->type == TokenType::ELSE) { // Debugging: Checked for 'else'
        ++it; // Debugging: Skip the 'else' token
        while (it != tokens.end() && it->type != TokenType::END) { // Debugging: Checked for end of 'else' block
            elseBlock.push_back(parseStatement(tokens, it)); // Debugging: Parsed statement for 'else' block
        }
    }

    return std::make_unique<IfNode>(std::move(condition), std::move(ifBlock), std::move(elseBlock)); // Debugging: Created if node
}





void evaluateAssignment(const std::string& id, const std::string& expression, std::unordered_map<std::string, std::string>& context) {
    std::cout << "Evaluating assignment: " << id << " = " << expression << std::endl;

    // Check if the variable already exists in the symbol table
    if (context.find(id) != context.end()) {
        // Variable already exists, pop the old value
        std::cout << "Popping old value of " << id << std::endl;
        context.erase(id);
    }

    // Store the expression in the symbol table
    context[id] = expression;
}


std::unique_ptr<ASTNode> parseAssignment(std::vector<Token>& tokens, std::vector<Token>::iterator& it) { // Debugging: Defined parseAssignment function
    std::cout << "gets into parse assignment" << "\n";
    std::string id = it->value; // Debugging: Get variable name
    ++it; // Debugging: Skip the ID token
    ++it; // Debugging: Skip the '=' token
    auto expression = parseExpression(tokens, it); // Debugging: Parsed expression
    std::cout << "gets into end of parse assignment" << "\n";
    return std::make_unique<AssignmentNode>(id, std::move(expression)); // Debugging: Created assignment node
}

int getVariableValue(const std::string& id, const std::unordered_map<std::string, int>& context) { // Debugging: Defined getVariableValue function
    if (context.find(id) != context.end()) { // Debugging: Checked if variable exists in context
        return context.at(id); // Debugging: Returned variable value
    } else {
        throw std::runtime_error("Variable not found: " + id); // Debugging: Threw error if variable not found
    }
}



std::vector<std::unique_ptr<ASTNode>> parse(std::vector<Token>& tokens) {
    std::cout << "************************************" << std::endl;
    std::cout << "Entering parse" << std::endl;
    std::cout << "\n";
    std::vector<std::unique_ptr<ASTNode>> program;
    std::unordered_map<std::string, int> context;
    auto it = tokens.begin();

    while (it != tokens.end() && it->type != TokenType::END) {
        std::cout << "Current token: " << it->value << " Character: " << it->value<< std::endl; // Print the current token and character

        // Skip comments
        if (it->type == TokenType::COMMENT) {
            ++it; // Move to the next token
            continue; // Skip to the next iteration of the loop
        }

        switch (it->type) {
            case TokenType::PRINT: {
                std::cout << "Parsing PRINT statement" << std::endl;
                auto printNode = parsePrintStatement(tokens, it); // Correct call

                program.push_back(std::move(printNode));
                break;
            }
            case TokenType::ID: {
                std::cout << "Parsing assignment statement" << std::endl;
                auto assignmentNode = parseAssignment(tokens, it);
                program.push_back(std::move(assignmentNode));
                break;
            }
            case TokenType::IF: {
                std::cout << "Parsing IF statement" << std::endl;
                auto ifNode = parseIf(tokens, it);
                program.push_back(std::move(ifNode));
                break;
            }
             case TokenType::ASSIGN: {
                std::cout << "Parsing Assign statement" << std::endl;
                auto AssignmentNode = parseAssignment(tokens, it);
                program.push_back(std::move(AssignmentNode));
                break;
            }
            default:
                throw std::runtime_error("Unexpected token in parse");
        }
        std::cout << "\n";
    }
    std::cout << "\n";
    std::cout << "Exiting parse" << std::endl;
    std::cout << "\n";
    return program;
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Could not open file " << argv[1] << "\n";
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string input = buffer.str();
    auto tokens = tokenize(input);
    

    auto program = parse(tokens);
    
    std::unordered_map<std::string, int> context;
    for (auto& node : program) {
        node->evaluate(context);
    }

    // Printing values for demonstration
    for (const auto& pair : context) {
        std::cout << pair.first << ": " << pair.second << "\n";
    }
    
    return 0;
    
}
