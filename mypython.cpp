#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <stack>
#include <sstream>
#include <variant>
#include <unordered_map>
#include <memory>
#include <filesystem> 

enum class TokenType {
    ID, NUM, ASSIGN, PRINT, STRING, SEMICOLON, END, COMMENT, PLUS, LEFT_PAREN, RIGHT_PAREN, EQUALS, NUMBER, COMMA, NEWLINE, MINUS
};

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
        // This node evaluates to an integer
    }
};

class VariableNode : public ASTNode {
    std::string name;
public:
    VariableNode(const std::string& n) : name(n) {}
    void evaluate(std::unordered_map<std::string, int>& context) override {
        // Just access the variable from context
    }
};
class AssignmentNode : public ASTNode {
    std::string variable;
    std::unique_ptr<ASTNode> expression;

public:
    AssignmentNode(const std::string& var, std::unique_ptr<ASTNode> expr) 
        : variable(var), expression(std::move(expr)) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
        // Evaluate the expression and assign the result to the variable in the context
        if (expression) {
            expression->evaluate(context);
            context[variable] = context["__expr_result"];  // Assuming expression evaluation sets this
        }
    }
};

class PrintNode : public ASTNode {
    std::string message;
    std::string variable;
public:
    PrintNode(const std::string& msg, const std::string& var) : message(msg), variable(var) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
    std::cout << "PrintNode::evaluate is being called" << std::endl;
    std::string filename = "out01.txt"; // Static filename for simplicity

    // Open the file in append mode
    std::ofstream outfile(filename, std::ios_base::app);

    // Check if the file is open
    if (!outfile.is_open()) {
        std::cerr << "Failed to open or create file: " << filename << std::endl;
        std::cerr << "Current path: " << std::filesystem::current_path() << std::endl;
        return;
    }

    // Check if the variable exists in the context
    if (context.find(variable) != context.end()) {
        outfile << variable << " = " << context[variable] << std::endl;
        std::cout << "Writing to file: " << variable << " = " << context[variable] << std::endl;
    } else {
        std::cerr << "Variable not found in context: " << variable << std::endl;
    }

    outfile.close(); // Close the file

}
};

std::vector<Token> tokenize(const std::string& input) {
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
            case '=':
                tokens.emplace_back(TokenType::ASSIGN, "=");
                std::cout << "Added token: ASSIGN, =" << std::endl;
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
            case '-':
            tokens.emplace_back(TokenType::MINUS, "-");
            std::cout << "Added token: MINUS, -" << std::endl;
            break;
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

class ExpressionNode : public ASTNode {
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    char op; // Operator: '+', '-'

public:
    ExpressionNode(std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r, char oper)
        : left(std::move(l)), right(std::move(r)), op(oper) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
        int leftVal = 0, rightVal = 0;
        // Evaluate left side
        if (left) {
            left->evaluate(context);
            leftVal = context["__expr_result"];
        }
        // Evaluate right side
        if (right) {
            right->evaluate(context);
            rightVal = context["__expr_result"];
        }
        // Apply operation
        if (op == '+') {
            context["__expr_result"] = leftVal + rightVal;
        } else if (op == '-') {
            context["__expr_result"] = leftVal - rightVal;
        }
    }
};



std::vector<std::unique_ptr<ASTNode>> parse(std::vector<Token>& tokens) {
    std::cout << "Entering parse function\n";
    std::vector<std::unique_ptr<ASTNode>> program;
    auto it = tokens.begin();

    while (it != tokens.end() && it->type != TokenType::END) {
    if (it->type == TokenType::ID) {
        std::string varName = it->value;
        ++it;  // Move to the next token, which should be ASSIGN
        if (it != tokens.end() && it->type == TokenType::ASSIGN) {
            ++it;  // Move to the expression (right side of the assignment)
            std::unique_ptr<ASTNode> exprNode;
            if (it != tokens.end() && it->type == TokenType::NUM) {
                exprNode = std::make_unique<IntegerNode>(std::stoi(it->value));
                ++it;  // Move past the number
                if (it != tokens.end() && (it->type == TokenType::PLUS || it->type == TokenType::MINUS)) {
                    char op = it->value[0];
                    ++it;  // Move to the next part of the expression
                    if (it != tokens.end() && it->type == TokenType::NUM) {
                        auto right = std::make_unique<IntegerNode>(std::stoi(it->value));
                        exprNode = std::make_unique<ExpressionNode>(std::move(exprNode), std::move(right), op);
                        ++it;  // Move past the second number
                    }
                }
            }
            program.push_back(std::make_unique<AssignmentNode>(varName, std::move(exprNode)));
        }
    }

        if (it->type == TokenType::PRINT) {
            std::cout << "Found PRINT token\n";
            ++it; // Skip the PRINT token
            if (it == tokens.end() || it->type != TokenType::LEFT_PAREN) throw std::runtime_error("Expected '(' after 'print'");
            ++it; // Skip the '(' token
            if (it == tokens.end() || it->type != TokenType::STRING) throw std::runtime_error("Expected string after 'print('");
            std::string message = it->value;
            ++it; // Skip the string token
            if (it == tokens.end() || it->type != TokenType::COMMA) throw std::runtime_error("Expected ',' after 'print(\"" + message + "\"'");
            ++it; // Skip the ',' token
            if (it == tokens.end() || it->type != TokenType::ID) throw std::runtime_error("Expected identifier after 'print(\"" + message + "\", '");
            std::string variable = it->value;
            ++it; // Skip the ID token
            if (it == tokens.end() || it->type != TokenType::RIGHT_PAREN) throw std::runtime_error("Expected ')' after 'print(\"" + message + "\", " + variable + "')");
            ++it; // Skip the ')' token
            program.push_back(std::make_unique<PrintNode>(message, variable));
            std::cout << "Added PrintNode to program\n";
        } else if (it->type == TokenType::ID && std::next(it) != tokens.end() && std::next(it)->type == TokenType::ASSIGN) {
            std::string varName = it->value;
            ++it; // Skip the ID token
            ++it; // Skip the ASSIGN token
            if (it == tokens.end() || it->type != TokenType::NUM) throw std::runtime_error("Expected number after '=" + varName + "'");
            int value = std::stoi(it->value);
            program.push_back(std::make_unique<AssignmentNode>(varName, value));
            std::cout << "Added AssignmentNode to program\n";
            ++it; // Skip the NUM token
        } else {
            std::cout << "Skipping token: " << it->value << "\n";
            ++it;
        }
    }

    std::cout << "Exiting parse function\n";
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
    std::cout << "Gets here 1 after tokenize :\n";
    auto program = parse(tokens);
    std::cout << "Gets here 2 after parse:\n";
    std::unordered_map<std::string, int> context;
    for (auto& node : program) {
        std::cout << "Evaluating node\n";
        node->evaluate(context);
    }

    std::cout << "Final context values:\n";
    for (const auto& pair : context) {
        std::cout << pair.first << " = " << pair.second << "\n";
    }

    return 0;
}