#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <sstream>

enum class TokenType {
    ID, NUM, ASSIGN, PRINT, STRING, COLON, END, COMMENT, PLUS, 
    LEFT_PAREN, RIGHT_PAREN, EQUALS, NUMBER, COMMA, NEWLINE, MINUS, 
    GREATER_THAN, LESS_THAN, IF, LEFT_BRACE,RIGHT_BRACE,ELSE
};


void printTokenType(TokenType type) {
    switch (type) {
        case TokenType::ID:
            std::cout << "ID";
            break;
        case TokenType::NUM:
            std::cout << "NUM";
            break;
        case TokenType::ASSIGN:
            std::cout << "ASSIGN";
            break;
        case TokenType::PRINT:
            std::cout << "PRINT";
            break;
        case TokenType::STRING:
            std::cout << "STRING";
            break;
        case TokenType::COLON:
            std::cout << "COLON";
            break;
        case TokenType::END:
            std::cout << "END";
            break;
        case TokenType::COMMENT:
            std::cout << "COMMENT";
            break;
        case TokenType::PLUS:
            std::cout << "PLUS";
            break;
        case TokenType::LEFT_PAREN:
            std::cout << "LEFT_PAREN";
            break;
        case TokenType::RIGHT_PAREN:
            std::cout << "RIGHT_PAREN";
            break;
        case TokenType::EQUALS:
            std::cout << "EQUALS";
            break;
        case TokenType::NUMBER:
            std::cout << "NUMBER";
            break;
        case TokenType::COMMA:
            std::cout << "COMMA";
            break;
        case TokenType::NEWLINE:
            std::cout << "NEWLINE";
            break;
        case TokenType::MINUS:
            std::cout << "MINUS";
            break;
        case TokenType::GREATER_THAN:
            std::cout << "GREATER_THAN";
            break;
        case TokenType::LESS_THAN:
            std::cout << "LESS_THAN";
            break;
        case TokenType::IF:
            std::cout << "IF";
            break;
        case TokenType::LEFT_BRACE:
            std::cout << "LEFT_BRACE";
            break;
        case TokenType::RIGHT_BRACE:
            std::cout << "RIGHT_BRACE";
            break;
        case TokenType::ELSE:
            std::cout << "ELSE";
            break;
        default:
            std::cout << "UNKNOWN";
    }
}

struct Token {
    TokenType type;
    std::string value;
    Token(TokenType type, std::string value) : type(type), value(value) {}
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void evaluate(std::unordered_map<std::string, int>& context) = 0;
};

class IntegerNode : public ASTNode {
    int value;
public:
    IntegerNode(int val) : value(val) {}
    void evaluate(std::unordered_map<std::string, int>& context) override {
        context["__expr_result"] = value;  // Set the result of expression evaluation
    }
};

class VariableNode : public ASTNode {
    std::string name;
public:
    VariableNode(const std::string& n) : name(n) {}
    void evaluate(std::unordered_map<std::string, int>& context) override {
        if (context.find(name) != context.end()) {
            context["__expr_result"] = context[name];
        }
    }
};

class ComparisonNode : public ASTNode {
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    char op;
public:
    ComparisonNode(std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r, char oper)
        : left(std::move(l)), right(std::move(r)), op(oper) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
        left->evaluate(context);
        int leftVal = context["__expr_result"];
        right->evaluate(context);
        int rightVal = context["__expr_result"];
        context["__expr_result"] = (op == '>' ? (leftVal > rightVal) : (leftVal < rightVal));
    }
};
class IfElseNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> trueBranch, falseBranch;

public:
    IfElseNode(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> trueBr, std::vector<std::unique_ptr<ASTNode>> falseBr)
        : condition(std::move(cond)), trueBranch(std::move(trueBr)), falseBranch(std::move(falseBr)) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
        condition->evaluate(context);
        if (context["__expr_result"]) {
            for (auto& node : trueBranch) node->evaluate(context);
        } else {
            for (auto& node : falseBranch) node->evaluate(context);
        }
    }
};

class AssignmentNode : public ASTNode {
    std::string variable;
    std::unique_ptr<ASTNode> expression;
public:
    AssignmentNode(const std::string& var, std::unique_ptr<ASTNode> expr)
        : variable(var), expression(std::move(expr)) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
        expression->evaluate(context);  // Evaluate the expression
        context[variable] = context["__expr_result"];  // Assign the result to the variable
    }
};

class ExpressionNode : public ASTNode {
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    char op;
public:
    ExpressionNode(std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r, char oper)
        : left(std::move(l)), right(std::move(r)), op(oper) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
        left->evaluate(context);
        int leftVal = context["__expr_result"];
        right->evaluate(context);
        int rightVal = context["__expr_result"];

        if (op == '+') {
            context["__expr_result"] = leftVal + rightVal;
        } else if (op == '-') {
            context["__expr_result"] = leftVal - rightVal;
        }
    }
};

class PrintNode : public ASTNode {
    std::string message;
    std::string variable;
public:
    PrintNode(const std::string& msg, const std::string& var) : message(msg), variable(var) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
        std::cout << message << " " << (context.find(variable) != context.end() ? std::to_string(context[variable]) : "undefined") << std::endl;
    }
};

std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    std::istringstream iss(input);
    std::string str;
    std::string comment;
    char ch;
    while (iss >> std::noskipws >> ch) {
        // Handle whitespace characters
        if (isspace(ch)) {
            if (ch == '\n') {
                tokens.emplace_back(TokenType::NEWLINE, "\\n");
            }
            continue;
        }

        // Handle operators and special characters
        switch (ch) {
            case '=':
                tokens.emplace_back(TokenType::ASSIGN, "=");
                break;
            case '+':
                tokens.emplace_back(TokenType::PLUS, "+");
                break;
            case '-':
                tokens.emplace_back(TokenType::MINUS, "-");
                break;
            case '(':
                tokens.emplace_back(TokenType::LEFT_PAREN, "(");
                break;
            case ')':
                tokens.emplace_back(TokenType::RIGHT_PAREN, ")");
                break;
            case ',':
                tokens.emplace_back(TokenType::COMMA, ",");
                break;
            case ';':
                tokens.emplace_back(TokenType::COLON, ";");
                break;
            case '\"':
                getline(iss, str, '\"');
                tokens.emplace_back(TokenType::STRING, str);
                break;
            case '#':
                getline(iss, comment); // This reads the comment and moves the stream past the end of line
                // No token is created for the comment
                break;
            case '>':
                tokens.emplace_back(TokenType::GREATER_THAN, ">");
                break;
            case '<':
                tokens.emplace_back(TokenType::LESS_THAN, "<");
                break;
            case 'i':  // Check for 'if' keyword
                iss >> ch;  // Consume 'f'
                tokens.emplace_back(TokenType::IF, "if");
                break;
            case 'e':  // Check for 'else' keyword
                iss >> ch;  // Consume 'l'
                iss >> ch;  // Consume 's'
                iss >> ch;  // Consume 'e'
                tokens.emplace_back(TokenType::ELSE, "else");
                break;
            case ':':
                tokens.emplace_back(TokenType::COLON, ":");
                tokens.emplace_back(TokenType::END, "");
                break;
            default:
                if (isdigit(ch)) {
                    iss.putback(ch);
                    int num;
                    iss >> num;
                    tokens.emplace_back(TokenType::NUM, std::to_string(num));
                } else if (isalpha(ch)) {
                    std::string identifier(1, ch);
                    while (iss.peek() != EOF && isalnum(iss.peek())) {
                        iss >> ch;
                        identifier += ch;
                    }
                    tokens.emplace_back(TokenType::ID, identifier);
                }
                break;
        }
    }

    tokens.emplace_back(TokenType::END, "");
    return tokens;
}


std::unique_ptr<ASTNode> parseExpression(std::vector<Token>::iterator& it, std::vector<Token>& tokens) {
    std::unique_ptr<ASTNode> left;

    // Handle the first part of the expression (number or variable)
    if (it->type == TokenType::NUM) {
        left = std::make_unique<IntegerNode>(std::stoi(it->value));
        ++it;
    } else if (it->type == TokenType::ID) {
        left = std::make_unique<VariableNode>(it->value);
        ++it;
    }

    // Handle binary operations
    while (it != tokens.end() && (it->type == TokenType::PLUS || it->type == TokenType::MINUS || it->type == TokenType::GREATER_THAN || it->type == TokenType::LESS_THAN)) {
        char op = it->value[0];
        ++it; // move past the operator

        std::unique_ptr<ASTNode> right;
        if (it->type == TokenType::NUM) {
            right = std::make_unique<IntegerNode>(std::stoi(it->value));
        } else if (it->type == TokenType::ID) {
            right = std::make_unique<VariableNode>(it->value);
        }
        ++it; // move past the number or variable

        // Create the appropriate node based on the operation
        if (op == '+' || op == '-') {
            left = std::make_unique<ExpressionNode>(std::move(left), std::move(right), op);
        } else if (op == '>' || op == '<') {
            left = std::make_unique<ComparisonNode>(std::move(left), std::move(right), op);
        }
    }

    return left;
}

std::unique_ptr<ASTNode> parseStatement(std::vector<Token>::iterator& it, std::vector<Token>& tokens) {
    if (it->type == TokenType::ID) {
        std::string varName = it->value;
        ++it;  // Expect ASSIGN
        if (it != tokens.end() && it->type == TokenType::ASSIGN) {
            ++it;  // Move to expression
            auto exprNode = parseExpression(it, tokens);
            return std::make_unique<AssignmentNode>(varName, std::move(exprNode));
        }
    } else if (it->type == TokenType::PRINT) {
        // Assume proper print handling as above
        // Just advance iterator appropriately
    }
    return nullptr; // or handle other statement types
}

std::vector<std::unique_ptr<ASTNode>> parse(std::vector<Token>& tokens) {
    std::vector<std::unique_ptr<ASTNode>> program;
    auto it = tokens.begin();

    while (it != tokens.end() && it->type != TokenType::END) {
        // Skip newline tokens
        if (it->type == TokenType::NEWLINE) {
            ++it;
            continue;
        }

        std::cout << "Token type: ";
        printTokenType(it->type);
        std::cout << std::endl;

        if (it->type == TokenType::ID) {
            std::string varName = it->value;
            ++it;  // Check for ASSIGN
            if (it != tokens.end() && it->type == TokenType::ASSIGN) {
                ++it;  // Move to the expression
                std::unique_ptr<ASTNode> exprNode = parseExpression(it, tokens);
                program.push_back(std::make_unique<AssignmentNode>(varName, std::move(exprNode)));
            }
        } else if (it->type == TokenType::IF) {  // Handle if-else
            ++it; // Assuming 'if' is already consumed
            std::unique_ptr<ASTNode> cond = parseExpression(it, tokens);
            std::vector<std::unique_ptr<ASTNode>> trueBranch, falseBranch;

            if (it->type == TokenType::LEFT_BRACE) {
                ++it;  // Skip '{'
                while (it != tokens.end() && it->type != TokenType::RIGHT_BRACE) {
                    trueBranch.push_back(parseStatement(it, tokens));
                }
                ++it;  // Skip '}'
            }

            if (it != tokens.end() && it->type == TokenType::ELSE) {
                ++it;  // Skip 'else'
                if (it->type == TokenType::LEFT_BRACE) {
                    ++it;  // Skip '{'
                    while (it != tokens.end() && it->type != TokenType::RIGHT_BRACE) {
                        falseBranch.push_back(parseStatement(it, tokens));
                    }
                    ++it;  // Skip '}'
                }
            }

            program.push_back(std::make_unique<IfElseNode>(std::move(cond), std::move(trueBranch), std::move(falseBranch)));
        } else if (it->type == TokenType::PRINT) {  // Handle print
            ++it; // Skip 'print'
            if (it->type == TokenType::LEFT_PAREN) {
                ++it; // Skip '('
                if (it->type == TokenType::STRING) {
                    std::string msg = it->value;
                    ++it; // Skip the string token
                    if (it->type == TokenType::COMMA) {
                        ++it; // Skip ','
                        if (it->type == TokenType::ID) {
                            std::string var = it->value;
                            ++it; // Skip variable name
                            if (it->type == TokenType::RIGHT_PAREN) {
                                ++it; // Skip ')'
                                program.push_back(std::make_unique<PrintNode>(msg, var));
                            }
                        }
                    }
                }
            }
        } else {
            std::cerr << "Syntax Error: Unexpected token '" << it->value << "' of type " << static_cast<int>(it->type) << " at position: " << std::distance(tokens.begin(), it) << std::endl;
            return {};
        }
    }

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

    std::cout << "Program executed "<< std::endl;

    return 0;
}
