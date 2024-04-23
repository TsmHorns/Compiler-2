#include <iostream>
#include <unordered_map>
#include <memory>
#include <variant>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <variant>

using ValueType = std::variant<int, std::string>;



enum class TokenType {
    ID, NUM, ASSIGN, PRINT, STRING, SEMICOLON, END, COMMENT, PLUS, MINUS, MULTIPLY, DIVIDE,
    GREATER_THAN, LESS_THAN, GREATER_THAN_EQUAL, LESS_THAN_EQUAL, EQUALS, NOT_EQUALS,
    LEFT_PAREN, RIGHT_PAREN, NUMBER, COMMA, NEWLINE, IF, ELSE, LEFT_BRACE, RIGHT_BRACE
};

struct Token {
    TokenType type;
    std::string value;

    Token(TokenType type, std::string value) : type(type), value(value) {}
};

template<typename T>
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void evaluate(std::unordered_map<std::string, int>& context) = 0;
};

template<typename T>
class IntegerNode : public ASTNode<T> {
    T value;
public:
    IntegerNode(T val) : value(val) {}
    void evaluate(std::unordered_map<std::string, int>& context) override {
        // This node evaluates to an integer
    }
};

template<typename T>
class PrintNode : public ASTNode<T> {
        std::tuple<std::unique_ptr<ASTNode<T>>, std::string> message;
public:
    template<typename... Args>
     PrintNode(std::unique_ptr<ASTNode<T>> node, const std::string& str) 
        : message(std::make_tuple(std::move(node), str)) {}

     void evaluate(std::unordered_map<std::string, int>& context) override {
        // implementation to evaluate message and print
    }

private:
    template<typename Arg, typename... Args>
    void printArgs(std::unordered_map<std::string, int>& context, Arg&& arg, Args&&... args) {
        std::cout << evaluateArg(context, std::forward<Arg>(arg)) << " ";
        printArgs(context, std::forward<Args>(args)...);
    }

    void printArgs(std::unordered_map<std::string, int>& context) {
        std::cout << std::endl;
    }

    template<typename Arg>
    auto evaluateArg(std::unordered_map<std::string, int>& context, Arg&& arg) {
        if constexpr (std::is_same_v<std::decay_t<Arg>, std::string>) {
            return arg;
        } else {
            return evaluateNode(arg, context);
        }
    }

};

template<typename T>
class IfNode : public ASTNode<T> {
    std::unique_ptr<ASTNode<T>> condition;
    std::unique_ptr<ASTNode<T>> ifBlock;
    std::unique_ptr<ASTNode<T>> elseBlock;  // Optional

public:
    IfNode(std::unique_ptr<ASTNode<T>> cond, std::unique_ptr<ASTNode<T>> ifBlk, std::unique_ptr<ASTNode<T>> elseBlk = nullptr)
        : condition(std::move(cond)), ifBlock(std::move(ifBlk)), elseBlock(std::move(elseBlk)) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
        // Implementation depends on your runtime execution logic
    }
};


template<typename T>
class VariableNode : public ASTNode<T> {
    std::string name;
public:
    VariableNode(const std::string& n) : name(n) {}
    void evaluate(std::unordered_map<std::string, int>& context) override {
        // Just access the variable from context
    }
};

template<typename T>
class BinaryOperationNode : public ASTNode<T> {
    std::unique_ptr<ASTNode<T>> left;
    std::unique_ptr<ASTNode<T>> right;
    TokenType op;
public:
    BinaryOperationNode(std::unique_ptr<ASTNode<T>> left, TokenType op, std::unique_ptr<ASTNode<T>> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
        left->evaluate(context);
        int leftVal = context["__expr_result"];
        right->evaluate(context);
        int rightVal = context["__expr_result"];

        switch (op) {
            case TokenType::PLUS:
                context["__expr_result"] = leftVal + rightVal;
                break;
            case TokenType::MINUS:
                context["__expr_result"] = leftVal - rightVal;
                break;
            case TokenType::MULTIPLY:
                context["__expr_result"] = leftVal * rightVal;
                break;
            case TokenType::DIVIDE:
                context["__expr_result"] = leftVal / rightVal;
                break;
            case TokenType::GREATER_THAN:
                context["__expr_result"] = (leftVal > rightVal);
                break;
            case TokenType::LESS_THAN:
                context["__expr_result"] = (leftVal < rightVal);
                break;
            case TokenType::GREATER_THAN_EQUAL:
                context["__expr_result"] = (leftVal >= rightVal);
                break;
            case TokenType::LESS_THAN_EQUAL:
                context["__expr_result"] = (leftVal <= rightVal);
                break;
            case TokenType::EQUALS:
                context["__expr_result"] = (leftVal == rightVal);
                break;
            case TokenType::NOT_EQUALS:
                context["__expr_result"] = (leftVal != rightVal);
                break;
            default:
                throw std::runtime_error("Unsupported binary operation");
        }
    }
};

template<typename T>
class BlockNode : public ASTNode<T> {
    std::vector<std::unique_ptr<ASTNode<T>>> statements;
public:
    BlockNode(std::vector<std::unique_ptr<ASTNode<T>>> stmts) : statements(std::move(stmts)) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
        for (auto& stmt : statements) {
            stmt->evaluate(context);
        }
    }
};

template<typename T>
class AssignmentNode : public ASTNode<T> {
    std::string variable;
    std::unique_ptr<ASTNode<T>> expression;
public:
    AssignmentNode(const std::string& var, std::unique_ptr<ASTNode<T>> expr)
        : variable(var), expression(std::move(expr)) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
        if (expression) {
            expression->evaluate(context);
            context[variable] = context["__expr_result"];
        } else {
            throw std::runtime_error("AssignmentNode: Expression is empty");
        }
    }
};



template<typename T>
std::vector<Token> tokenize(const std::string& input)  {
    std::vector<Token> tokens;
    std::istringstream iss(input);
    std::string str;
    std::string comment;
    char ch;

    while (iss >> std::noskipws >> ch) {
        if (isspace(ch)) {
            if (ch == '\n') {
                continue; // Optionally handle new line specific logic
            }
            continue; // Skip whitespace
        }

        switch (ch) {
                case '>':
            if (iss.peek() == '=') {
                iss.get();
                tokens.emplace_back(TokenType::GREATER_THAN_EQUAL, ">=");
            } else {
                tokens.emplace_back(TokenType::GREATER_THAN, ">");
            }
            break;
        case '<':
            if (iss.peek() == '=') {
                iss.get();
                tokens.emplace_back(TokenType::LESS_THAN_EQUAL, "<=");
            } else {
                tokens.emplace_back(TokenType::LESS_THAN, "<");
            }
            break;
        case '=':
            if (iss.peek() == '=') {
                iss.get();
                tokens.emplace_back(TokenType::EQUALS, "==");
            } else {
                tokens.emplace_back(TokenType::ASSIGN, "=");
            }
            break;
            case '+':
                tokens.emplace_back(TokenType::PLUS, "+");
                std::cout << "Added token: PLUS, +" << std::endl;
                break;
            case '(':
                tokens.emplace_back(TokenType::LEFT_PAREN, "(");
                std::cout << "Added token: LEFT_PAREN, (" << std::endl;
                break;
            case ')':
                tokens.emplace_back(TokenType::RIGHT_PAREN, ")");
                std::cout << "Added token: RIGHT_PAREN, )" << std::endl;
                break;
            case ',':
                tokens.emplace_back(TokenType::COMMA, ",");
                std::cout << "Added token: COMMA, ," << std::endl;
                break;
            case ';':
                tokens.emplace_back(TokenType::SEMICOLON, ";");
                std::cout << "Added token: SEMICOLON, ;" << std::endl;
                break;
            case '\"':
                getline(iss, str, '\"');
                tokens.emplace_back(TokenType::STRING, str);
                std::cout << "Added token: STRING, \"" << str << "\"" << std::endl;
                break;
            case '#':
                getline(iss, comment);
                tokens.emplace_back(TokenType::COMMENT, "#" + comment);
                std::cout << "Added token: COMMENT, #" << comment << std::endl;
                break;
            default:
                if (isdigit(ch)) {
                    iss.putback(ch);
                    int num;
                    iss >> num;
                    tokens.emplace_back(TokenType::NUM, std::to_string(num));
                    std::cout << "Added token: NUM, " << num << std::endl;
                } else if (isalpha(ch)) {
                    std::string identifier(1, ch);
                    while (iss.peek() != EOF && isalnum(iss.peek())) {
                        iss >> ch;
                        identifier += ch;
                    }
                    TokenType type = identifier == "print" ? TokenType::PRINT : TokenType::ID;
                    tokens.emplace_back(type, identifier);
                    std::cout << "Added token: " << (type == TokenType::PRINT ? "PRINT" : "ID") << ", " << identifier << std::endl;
                }
                break;
        }
    }
    tokens.emplace_back(TokenType::END, "");
    std::cout << "Added token: END, " << std::endl;
    return tokens;
}

template<typename T>
std::unique_ptr<ASTNode<T>> parseExpression(std::vector<Token>& tokens, typename std::vector<Token>::iterator& it);

template<typename T>
std::unique_ptr<ASTNode<T>> parsePrimary(std::vector<Token>& tokens, typename std::vector<Token>::iterator& it) {
    if (it->type == TokenType::ID) {
        std::string identifier = it->value;
        ++it; // Consume the ID token
        return std::make_unique<VariableNode<T>>(identifier);
    } else if (it->type == TokenType::NUM) {
        int value = std::stoi(it->value);
        ++it; // Consume the NUM token
        return std::make_unique<IntegerNode<T>>(value);
    } else if (it->type == TokenType::LEFT_PAREN) {
        ++it; // Consume the LEFT_PAREN token
        auto expression = parseExpression<T>(tokens, it);
        if (it->type != TokenType::RIGHT_PAREN) {
            throw std::runtime_error("Expected ')' after expression");
        }
        ++it; // Consume the RIGHT_PAREN token
        return expression;
    } else {
        throw std::runtime_error("Unexpected token in expression");
    }
}

template <typename T>
std::unique_ptr<IfNode<T>> parseIf(std::vector<Token>& tokens, typename std::vector<Token>::iterator& it);

template <typename T>
std::unique_ptr<PrintNode<T>> parsePrint(std::vector<Token>& tokens, typename std::vector<Token>::iterator& it);

template <typename T>
std::unique_ptr<AssignmentNode<T>> parseAssignment(std::vector<Token>& tokens, typename std::vector<Token>::iterator& it);

template<typename T>
std::unique_ptr<BlockNode<T>> parseBlock(std::vector<Token>& tokens, typename std::vector<Token>::iterator& it) {
    std::vector<std::unique_ptr<ASTNode<T>>> statements;

    // Parse statements until you reach 'ELSE', 'END', or unmatched 'NEWLINE'
    while (it != tokens.end() && it->type != TokenType::ELSE && it->type != TokenType::END) {
        if (it->type == TokenType::NEWLINE) {
            ++it; // skip newlines within the block
            continue;
        } else if (it->type == TokenType::IF) {
            statements.push_back(parseIf<T>(tokens, it));
        } else if (it->type == TokenType::PRINT) {
            statements.push_back(parsePrint<T>(tokens, it));
        } else if (it->type == TokenType::ID) {
            statements.push_back(parseAssignment<T>(tokens, it));
        }
        ++it; // Move to the next token
    }

    return std::make_unique<BlockNode<T>>(std::move(statements));
}


template<typename T>
std::unique_ptr<IfNode<T>> parseIf(std::vector<Token>& tokens, typename std::vector<Token>::iterator& it) {
    ++it;  // Skip 'if'
    auto condition = parseExpression<T>(tokens, it);
    auto ifBlock = parseBlock<T>(tokens, it);

    std::unique_ptr<BlockNode<T>> elseBlock = nullptr;
    if (it != tokens.end() && it->type == TokenType::ELSE) {
        ++it;  // Skip 'else'
        elseBlock = parseBlock<T>(tokens, it);
    }

    return std::make_unique<IfNode<T>>(std::move(condition), std::move(ifBlock), std::move(elseBlock));
}




template<typename T>
std::unique_ptr<ASTNode<T>> parseStatement(std::vector<Token>& tokens, typename std::vector<Token>::iterator& it) {
    if (it->type == TokenType::ID) {
        std::string varName = it->value;
        ++it; // Skip the ID token
        if (it->type == TokenType::ASSIGN) {
            ++it; // Skip ASSIGN
            auto expr = parseExpression<T>(tokens, it);
            return std::make_unique<AssignmentNode<T>>(varName, std::move(expr));
        }
    }
    throw std::runtime_error("Parsing error: Expected a statement");
}




template<typename T>
std::unique_ptr<ASTNode<T>> parseTerm(std::vector<Token>& tokens, typename std::vector<Token>::iterator& it) {
    auto left = parsePrimary<T>(tokens, it);

    while (it != tokens.end() && (it->type == TokenType::MULTIPLY || it->type == TokenType::DIVIDE)) {
        TokenType op = it->type;
        ++it; // Consume the operator token
        auto right = parsePrimary<T>(tokens, it);
        left = std::make_unique<BinaryOperationNode<T>>(std::move(left), op, std::move(right));
    }

    return left;
}

template<typename T>
std::unique_ptr<ASTNode<T>> parseExpression(std::vector<Token>& tokens, typename std::vector<Token>::iterator& it) {
    auto left = parseTerm<T>(tokens, it);

    while (it != tokens.end() && (it->type == TokenType::PLUS || it->type == TokenType::MINUS)) {
        TokenType op = it->type;
        ++it; // Consume the operator token
        auto right = parseTerm<T>(tokens, it);
        left = std::make_unique<BinaryOperationNode<T>>(std::move(left), op, std::move(right));
    }

    return left;
}

template<typename T>
std::unique_ptr<PrintNode<T>> parsePrint(std::vector<Token>& tokens, typename std::vector<Token>::iterator& it) {
    ++it; // Skip the 'print' keyword
    if (it->type != TokenType::LEFT_PAREN) {
        throw std::runtime_error("Expected '(' after 'print'");
    }
    ++it; // Skip '('
    auto expr = parseExpression<T>(tokens, it);
    if (it->type != TokenType::RIGHT_PAREN) {
        throw std::runtime_error("Expected ')' after expression");
    }
    ++it; // Skip ')'
    return std::make_unique<PrintNode<T>>(std::move(expr), "print");
}

template<typename T>
std::unique_ptr<AssignmentNode<T>> parseAssignment(std::vector<Token>& tokens, typename std::vector<Token>::iterator& it) {
    std::string varName = it->value;
    

    ++it; // Skip the variable name
    if (it->type != TokenType::ASSIGN) {
        throw std::runtime_error("Expected '=' after variable name");
    }
    
    ++it; // Skip '='
    auto expr = parseExpression<T>(tokens, it);
    if (it->type != TokenType::NEWLINE && it->type != TokenType::END) {
        ++it; // Skip to the end of the statement
    }
    return std::make_unique<AssignmentNode<T>>(varName, std::move(expr));
}


template<typename T>
std::vector<std::unique_ptr<ASTNode<T>>> parse(std::vector<Token>& tokens) {
    std::vector<std::unique_ptr<ASTNode<T>>> program;
    auto it = tokens.begin();

    while (it != tokens.end() && it->type != TokenType::END) {
        std::cout << "Processing token: " << it->value << " of type: " << static_cast<int>(it->type) << "\n";

        switch (it->type) {
            case TokenType::COMMENT:
                // Skip comments
                ++it;
                while (it != tokens.end() && it->type == TokenType::NEWLINE) {
                    ++it; // Skip newlines that may follow comments directly
                }
                break;
            case TokenType::NEWLINE:
                // Skip newlines
                ++it;
                break;
            case TokenType::PRINT:
                // Handle print statements
                program.push_back(parsePrint<T>(tokens, it));
                break;
            case TokenType::IF:
                // Handle if statements
                program.push_back(parseIf<T>(tokens, it));
                break;
            case TokenType::ID:
                // Handle assignments and other ID-based statements
                if (std::next(it) != tokens.end() && std::next(it)->type == TokenType::ASSIGN) {
                    program.push_back(parseAssignment<T>(tokens, it));
                } else {
                    ++it; // Skip the ID if it's not part of an assignment
                }
                break;
            default:
                // Advance past unrecognized tokens
                ++it;
                break;
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
    auto tokens = tokenize<ValueType>(input);
    auto program = parse<ValueType>(tokens);

    std::unordered_map<std::string, int> context;
    for (auto& node : program) {
        node->evaluate(context);
    }

    std::cout << "Final context values:\n";
    for (const auto& pair : context) {
        std::cout << pair.first << " = " << pair.second << "\n";
    }

    return 0;
}
