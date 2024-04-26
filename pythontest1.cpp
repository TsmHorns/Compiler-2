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
    auto it = context.find(name);
    if (it == context.end()) {
        throw std::runtime_error("Variable " + name + " not found in context.");
    }
    context["__expr_result"] = it->second;
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


void processOperator(TokenType op, std::stack<int>& operands, std::unordered_map<std::string, int>& context) {
    if (operands.size() < 2) {
        int numOperands = operands.size();
        std::cerr << "Error: Not enough operands for operator. Needed 2, found " << numOperands << std::endl;
        throw std::runtime_error("Not enough operands for operator: " + std::to_string(static_cast<int>(op)) + ". Needed 2, found " + std::to_string(numOperands));
    }

    int right = operands.top(); operands.pop();
    int left = operands.top(); operands.pop();

    std::cout << "Processing operator " << static_cast<int>(op) << " with operands " << left << " and " << right << std::endl;

    switch (op) {
        case TokenType::PLUS: operands.push(left + right); break;
        case TokenType::MINUS: operands.push(left - right); break;
        case TokenType::MULTIPLY: operands.push(left * right); break;
        case TokenType::DIVIDE:
            if (right == 0) {
                throw std::runtime_error("Division by zero.");
            }
            operands.push(left / right);
            break;
        default:
            throw std::runtime_error("Unsupported operator encountered.");
    }
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
    auto it = context.find(id);
    if (it != context.end()) {
        return it->second;  // Directly use iterator to access the value
    } else {
        throw std::runtime_error("Variable not found in get Variable..: " + id);
    }
}





void parseEnd(const Token& token) {
    std::cout << "Parsed end token." << std::endl;
}


int evaluateExpression(const std::vector<std::string>& parts, std::unordered_map<std::string, int>& context) {
    std::stack<int> operands;
    std::stack<TokenType> operators;

    for (const auto& part : parts) {
        std::cout << "Evaluating part: " << part << std::endl;
        if (isdigit(part[0])) {  // If the part is a number
            operands.push(std::stoi(part));
        } else if (isalpha(part[0])) {  // If the part is a variable
            operands.push(context[part]);
        } else {  // If the part is an operator
            TokenType op = getTokenType(part[0]);
            std::cout << "Encountered operator: " << static_cast<int>(op) << std::endl;
            while (!operators.empty() && precedence(op) <= precedence(operators.top())) {
                if (operands.size() < 2) {
                    std::cerr << "Error: Not enough operands for operator. Needed 2, found " << operands.size() << std::endl;
                    throw std::runtime_error("Not enough operands for operator operand is here: " + std::to_string(static_cast<int>(op)));
                }
                processOperator(operators.top(), operands, context);
                operators.pop();
            }
            operators.push(op);
        }
    }

            while (!operators.empty()) {
        if (operands.size() < 2) {
            int numOperands = operands.size();
            std::cerr << "Error: Not enough operands for operator. Needed 2, found " << numOperands << std::endl;
            throw std::runtime_error("Not enough operands for operator. Needed 2, found " + std::to_string(numOperands));
        }
        processOperator(operators.top(), operands, context);
        operators.pop();
    }



    if (operands.size() != 1) {
        std::cerr << "Error: Invalid expression. More than one operand left after evaluation." << std::endl;
        throw std::runtime_error("Invalid expression: more than one operand left after evaluation.");
    }

    return operands.top();
}



bool isOperator(char c) {
    switch (c) {
        case '+': case '-': case '*': case '/': case '=':
        case '(': case ')': case '<': case '>': case '!':
            return true;
        default:
            return false;
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

    // Add spaces around the operators in the expression
    std::string spacedExpr;
    for (size_t i = 0; i < expr.size(); ++i) {
        if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/') {
            spacedExpr += ' ';
            spacedExpr += expr[i];
            spacedExpr += ' ';
        } else {
            spacedExpr += expr[i];
        }
    }
    std:: cout << "\n";
    std::cout << "Parsed assignment: in parse assigment " << id << " = " << spacedExpr << std::endl;

    // Parse the expression and store the result in the context
    std::istringstream iss(spacedExpr);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) {
        parts.push_back(part);
    }

    // Evaluate the expression
    int result = evaluateExpression(parts, context);
    context[id] = result;
}




std::vector<std::string> printVariables;

void parsePrint(const Token& token, std::unordered_map<std::string, int>& context) {
    // Extract the content inside the print statement's parentheses
    size_t startParen = token.value.find('(');
    size_t endParen = token.value.find(')');
    if (startParen == std::string::npos || endParen == std::string::npos || endParen <= startParen) {
        std::cout << "Syntax error in print statement: Missing or incorrect parentheses." << std::endl;
        return;
    }
    std::string content = token.value.substr(startParen + 1, endParen - startParen - 1);

    // Extract variable names separated by commas
    std::istringstream varStream(content);
    std::string varName;
    while (std::getline(varStream, varName, ',')) {
        // Remove leading and trailing whitespace
        varName.erase(varName.find_last_not_of(" \t\n\r\f\v") + 1);
        varName.erase(0, varName.find_first_not_of(" \t\n\r\f\v"));
        
        auto it = context.find(varName);
        if (it != context.end()) {
            std::cout << varName << " = " << it->second << std::endl;
        } else {
            //std::cout << "Variable " << varName << " not found in parsePrint." << std::endl;
        }
    }
}



void parseProgram(const std::vector<Token>& tokens, std::unordered_map<std::string, int>& context, std::vector<Token>& printStatements) {
    std::cout << "*************************" << "\n";
    for (const Token& token : tokens) {
        switch (token.type) {
            case TokenType::ASSIGN:
                parseAssignment(token, context);
                break;
            case TokenType::PRINT:
                printStatements.push_back(token); // Store print tokens for later processing
                break;
            case TokenType::END:
                // Handling end token if necessary, e.g., cleanup or summary actions
                parseEnd(token);
                break;
            default:
                throw std::runtime_error("Unexpected token type.");
        }
    }
    std::cout << "*************************" << "\n";
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
    std::vector<Token> printStatements; // Store print statements to handle after all evaluations

    // Parse and evaluate all tokens, store print statements for later
    parseProgram(tokens, context, printStatements);



    // Now handle print statements
    for (const auto& token : printStatements) {
        parsePrint(token, context);
    }

    /*
    // Optionally print all context variables
    std::cout << "Final Variable Values:\n";
    for (const auto& pair : context) {
        std::cout << pair.first << " = " << pair.second << std::endl;
    }

    */
    return 0;
}

