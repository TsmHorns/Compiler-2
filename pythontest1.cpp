#include <iostream> // Debugging: Included iostream for standard input/output
#include <unordered_map> // Debugging: Included unordered_map for storing variables
#include <memory> // Debugging: Included memory for smart pointers
#include <fstream> // Debugging: Included fstream for file handling
#include <sstream> // Debugging: Included sstream for string streaming
#include <vector> // Debugging: Included vector for storing tokens
#include <string> // Debugging: Included string for string manipulation
#include <cctype>  // for std::iscntrl
#include <stack>
#include <algorithm> 


enum class TokenType { // Debugging: Defined an enum class for token types
    ID, NUM, ASSIGN, PRINT, STRING, SEMICOLON, END, COMMENT, PLUS, MINUS, MULTIPLY, DIVIDE,
    GREATER_THAN, LESS_THAN, GREATER_THAN_EQUAL, LESS_THAN_EQUAL, EQUALS, NOT_EQUALS,
    LEFT_PAREN, RIGHT_PAREN, NUMBER, COMMA, NEWLINE, IF, ELSE, EXPRESSION
};



struct Token { // Debugging: Defined a struct for tokens
    TokenType type; // Debugging: Added a member for token type
    std::string value; // Debugging: Added a member for token value

    Token(TokenType type, std::string value) : type(type), value(value) {} // Debugging: Initialized token with type and value
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void evaluate(std::unordered_map<std::string, int>& context) = 0;
    virtual std::string toString() const = 0;  // Pure virtual function
};




class VariableNode : public ASTNode {
    std::string name;
public:
    VariableNode(const std::string& n) : name(n) {}
    void evaluate(std::unordered_map<std::string, int>& context) override {
    if (context.find(name) == context.end()) {
        throw std::runtime_error("Variable " + name + " not found in context.");
    }
    context["__expr_result"] = context[name];
}

    std::string toString() const override {
        return "VariableNode: " + name;
    }
};


class PrintNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> parts;

public:
    PrintNode(std::vector<std::unique_ptr<ASTNode>> parts) : parts(std::move(parts)) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
    for (auto& part : parts) {
        part->evaluate(context);
        int result = context["__expr_result"];
        std::cout << result << " ";
    }
    std::cout << std::endl;
}

    std::string toString() const override {
        return "PrintNode with " + std::to_string(parts.size()) + " parts";
    }
};

class IfNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> ifBlock;
    std::vector<std::unique_ptr<ASTNode>> elseBlock;

public:
    IfNode(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> ifBlk, std::vector<std::unique_ptr<ASTNode>> elseBlk)
        : condition(std::move(cond)), ifBlock(std::move(ifBlk)), elseBlock(std::move(elseBlk)) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
        condition->evaluate(context);
        int result = context["__expr_result"];
        if (result != 0) {
            for (auto& stmt : ifBlock) {
                stmt->evaluate(context);
            }
        } else {
            for (auto& stmt : elseBlock) {
                stmt->evaluate(context);
            }
        }
    }

    std::string toString() const override {
        return "IfNode with " + std::to_string(ifBlock.size()) + " ifBlock parts and " + std::to_string(elseBlock.size()) + " elseBlock parts";
    }
};

class AssignmentNode : public ASTNode {
    std::string id;
    std::unique_ptr<ASTNode> expression;

public:
    AssignmentNode(std::string varId, std::unique_ptr<ASTNode> expr)
        : id(varId), expression(std::move(expr)) {}

    void evaluate(std::unordered_map<std::string, int>& context) override {
    std::cout << "Evaluating expression for assignment to " << id << "...\n";
    expression->evaluate(context);  // Evaluate the expression on the right-hand side
    if (context.count("__expr_result") > 0) {
        context[id] = context["__expr_result"];  // Assign the result to the variable in the context
        std::cout << id << " is now " << context[id] << "\n";  // Output the new value of the variable
    } else {
        throw std::runtime_error("Expression did not produce a result for assignment.");
    }
}


    std::string toString() const override {
        return "AssignmentNode with id: " + id;
    }
};




class StringNode : public ASTNode {
    std::string value;
public:
    StringNode(const std::string& val) : value(val) {}
    void evaluate(std::unordered_map<std::string, int>& context) override {
        context["__expr_result"] = std::stoi(value); // Simple conversion for demonstration
    }

    std::string toString() const override {
        return "StringNode: " + value;
    }
};




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
    std::string line;

    while (std::getline(iss, line)) {
        while (!line.empty() && std::iscntrl(line.back())) {
            line.pop_back();  // Remove trailing control character
        }

        if (line.empty() || line[0] == '#') {
            continue; // Skip empty lines and comments
        }

        if (line.substr(0, 5) == "print") {
            tokens.emplace_back(TokenType::PRINT, line);
            std::cout << "Emplacing PRINT token with value: " << line << std::endl;
        } else {
            size_t equalsPos = line.find('=');
            if (equalsPos != std::string::npos) {
                tokens.emplace_back(TokenType::ASSIGN, line);
                std::cout << "Emplacing ASSIGNMENT token with value: " << line << std::endl;
            }
        }
    }

    std::cout << "Emplacing END token" << std::endl;
    tokens.emplace_back(TokenType::END, "");
    std::cout << "Finished tokenizing" << std::endl;
    return tokens;
}

int precedence(TokenType op) {
    switch (op) {
        case TokenType::PLUS:
        case TokenType::MINUS:
            std::cout << "Operator precedence for PLUS or MINUS\n";
            return 1;
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE:
            std::cout << "Operator precedence for MULTIPLY or DIVIDE\n";
            return 2;
        default:
            std::cout << "Operator precedence default\n";
            return 0;
    }
}

void processOperator(TokenType op, std::stack<int>& operands) {
    if (operands.size() < 2) {
        throw std::runtime_error("Invalid expression.");
    }

    int right = operands.top();
    operands.pop();
    int left = operands.top();
    operands.pop();

    int result;
    switch (op) {
        case TokenType::PLUS:
            result = left + right;
            std::cout << "Processed PLUS operator: " << left << " + " << right << " = " << result << "\n";
            break;
        case TokenType::MINUS:
            result = left - right;
            std::cout << "Processed MINUS operator: " << left << " - " << right << " = " << result << "\n";
            break;
        case TokenType::MULTIPLY:
            result = left * right;
            std::cout << "Processed MULTIPLY operator: " << left << " * " << right << " = " << result << "\n";
            break;
        case TokenType::DIVIDE:
            if (right == 0) {
                throw std::runtime_error("Division by zero.");
            }
            result = left / right;
            std::cout << "Processed DIVIDE operator: " << left << " / " << right << " = " << result << "\n";
            break;
        default:
            throw std::runtime_error("Invalid operator.");
    }

    operands.push(result);
}


   

void evaluateAssignment(const std::string& id, const std::string& expression, std::unordered_map<std::string, int>& context) {
    std::cout << "Evaluating assignment: " << id << " = " << expression << std::endl;

    // Evaluate the expression and store the result in the context
    std::istringstream iss(expression);
    int result = 0;
    iss >> result;
    context[id] = result;
}



int getVariableValue(const std::string& id, const std::unordered_map<std::string, int>& context) {
    if (context.find(id) != context.end()) { // Debugging: Checked if variable exists in context
        return context.at(id); // Debugging: Returned variable value
    } else {
        throw std::runtime_error("Variable not found: " + id); // Debugging: Threw error if variable snot found
    }
}





void parseEnd(const Token& token) {
    std::cout << "Parsed end token." << std::endl;
}


int evaluateExpression(const std::vector<std::string>& expression, std::unordered_map<std::string, int>& context) {
    std::stack<int> operands;
    std::stack<TokenType> operators;

    for (const auto& token : expression) {
        if (isdigit(token[0])) {
            operands.push(std::stoi(token));
        } else if (isalpha(token[0])) {
            if (context.find(token) != context.end()) {
                operands.push(context[token]);
            } else {
                throw std::runtime_error("Unknown variable: " + token);
            }
        } else {
            TokenType op = getTokenType(token[0]);
            while (!operators.empty() && precedence(op) <= precedence(operators.top())) {
                processOperator(operators.top(), operands);
                operators.pop();
            }
            operators.push(op);
        }
    }

    while (!operators.empty()) {
        processOperator(operators.top(), operands);
        operators.pop();
    }

    if (operands.size() == 1) {
        return operands.top();
    } else {
        throw std::runtime_error("Invalid expression.");
    }
}



void parseAssignment(const Token& token, std::unordered_map<std::string, int>& context) {
    std::string::size_type equalsPos = token.value.find('=');
    if (equalsPos == std::string::npos) {
        throw std::runtime_error("Invalid assignment format.");
    }
    std::string id = token.value.substr(0, equalsPos);
    std::string expr = token.value.substr(equalsPos + 1);
    // Remove any spaces from the variable name
    id.erase(std::remove_if(id.begin(), id.end(), ::isspace), id.end());
    std::cout << "Parsed assignment: " << id << " = " << expr << std::endl;

    // Parse the expression and store the result in the context
    std::istringstream iss(expr);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) {
        parts.push_back(part);
    }

    // Evaluate the expression
    int result = evaluateExpression(parts, context);
    context[id] = result;
}


void parseProgram(const std::vector<Token>& tokens, std::unordered_map<std::string, int>& context) {
    std::cout << "*************************" << "\n";
    for (const Token& token : tokens) {
        switch (token.type) {
            case TokenType::ASSIGN:
                parseAssignment(token, context);
                break;
            case TokenType::PRINT:
                //parsePrint(token); // Assuming this was meant to be parseEnd
                parseEnd(token);
                break;
            case TokenType::END:
                parseEnd(token);
                break;
            default:
                throw std::runtime_error("Unexpected token type.");
        }
    }
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
    
    std::unordered_map<std::string, int> context;  // This will hold variable values

    parseProgram(tokens, context);

    // Print out the context to see all variable values
    for (const auto& pair : context) {
        std::cout << pair.first << " = " << pair.second << std::endl;
    }
    
    return 0;
}
