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


enum class TokenType {
    ID, NUM, ASSIGN, PRINT, STRING, SEMICOLON, END, COMMENT, 
    PLUS, MINUS, MULTIPLY, DIVIDE, GREATER_THAN, LESS_THAN, 
    GREATER_THAN_EQUAL, LESS_THAN_EQUAL, EQUALS, NOT_EQUALS,
    LEFT_PAREN, RIGHT_PAREN, NUMBER, COMMA, NEWLINE, 
    IF, ELSE, FUNCTION_DEF, FUNCTION_CALL, RETURN, SCOPE, ASSIGNMENT_FUNCTION_CALL
};


class FunctionDefNode; // Forward declaration


struct Token { // Debugging: Defined a struct for tokens
    TokenType type; // Debugging: Added a member for token type
    std::string value; // Debugging: Added a member for token value
    int indent_level; // Added to track indentation level

        Token(TokenType type, std::string value, int indent_level)
        : type(type), value(value), indent_level(indent_level) {}
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, FunctionDefNode*>& functions) = 0;
    virtual std::string toString() const = 0;  // Pure virtual function
};


class FunctionDefNode : public ASTNode {
private:
    std::vector<std::unique_ptr<ASTNode>> body;
    std::string name;
public:
    FunctionDefNode(const std::string& n, std::vector<std::unique_ptr<ASTNode>> b) : name(n), body(std::move(b)) {}
    void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, FunctionDefNode*>& functions) override {
        auto functionDef = dynamic_cast<FunctionDefNode*>(functions[name]);
        if (!functionDef) {
            throw std::runtime_error("Function not found: " + name);
        }
        contexts.push(std::unordered_map<std::string, int>());  // Push a new context onto the stack for the function's local variables
        for (auto& statement : functionDef->getBody()) {
            statement->evaluate(contexts, functions);
        }
        contexts.pop();  // Pop the function's local context off the stack
    }
    const std::vector<std::unique_ptr<ASTNode>>& getBody() const {
        return body;
    }
    const std::string& getName() const {
        return name;
    }
};

class FunctionCallNode : public ASTNode {
public:
    std::string functionName;
    std::string arguments;

    FunctionCallNode(const std::string& functionName, const std::string& arguments)
        : functionName(functionName), arguments(arguments) {}

    std::string toString() const override {
        return "FunctionCallNode: " + functionName + "(" + arguments + ")";
    }

    void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, FunctionDefNode*>& functions) override {
        // Implementation of evaluate for FunctionCallNode
        // This will depend on how you've implemented your language
    }
};


class Interpreter {
    std::string current_token;  // Add this line
    std::stack<std::unordered_map<std::string, int>> scopes;
    std::unordered_map<std::string, std::unique_ptr<FunctionDefNode>> functions;  // Add this line

public:
    std::stack<FunctionCallNode*> functionCallStack;  // Stack of function calls

    void enterScope() {
        scopes.push(std::unordered_map<std::string, int>());  // Push a new scope
    }

    void leaveScope() {
        if (!scopes.empty()) {
            scopes.pop();  // Pop the current scope
        }
    }

    int getVariable(const std::string& name) {
        if (!scopes.empty() && scopes.top().count(name) > 0) {
            return scopes.top().at(name);
        }
        throw std::runtime_error("Variable not found: " + name);
    }

    void addFunction(const std::string& name, std::unique_ptr<FunctionDefNode> functionDefNode) {
        functions[name] = std::move(functionDefNode);
    }
    void setCurrentToken(const std::string& token) { current_token = token; }  // Setter for current_token

    std::string getCurrentToken() { return current_token; }  // Add this line

};

/*
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

*/

class VariableNode : public ASTNode {
    std::string name;
public:
    VariableNode(const std::string& n) : name(n) {}
    void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, FunctionDefNode*>& functions) override {
        if (contexts.empty()) {
            throw std::runtime_error("No context available.");
        }
        auto& context = contexts.top();  // Get the current context
        auto it = context.find(name);
        if (it == context.end()) {
            throw std::runtime_error("Variable " + name + " not found in context.");
        }
        context["__expr_result"] = it->second;
    }
};

class PrintNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> parts;

public:
    PrintNode(std::vector<std::unique_ptr<ASTNode>> parts) : parts(std::move(parts)) {}

    void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, FunctionDefNode*>& functions) override {
    for (auto& part : parts) {
        part->evaluate(contexts, functions);
        int result = contexts.top()["__expr_result"];
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

    void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, FunctionDefNode*>& functions) override {
    if (contexts.empty()) {
        throw std::runtime_error("No context available.");
    }
    auto& context = contexts.top();  // Get the current context
    condition->evaluate(contexts, functions);
    // rest of the code
}

    std::string toString() const override {
        return "IfNode with " + std::to_string(ifBlock.size()) + " ifBlock parts and " + std::to_string(elseBlock.size()) + " elseBlock parts";
    }
};

/*
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


*/

class AssignmentNode : public ASTNode {
    std::string name;
    std::unique_ptr<ASTNode> value;
public:
    AssignmentNode(const std::string& n, std::unique_ptr<ASTNode> v) : name(n), value(std::move(v)) {}
    void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, FunctionDefNode*>& functions) override {
    if (contexts.empty()) {
        throw std::runtime_error("No context available.");
    }
    auto& context = contexts.top();  // Get the current context
    value->evaluate(contexts, functions);
    // rest of the code
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



std::vector<Token> tokenize(const std::string& input, Interpreter& interpreter) {
    std::vector<Token> tokens;
    std::istringstream iss(input);
    std::string line;
    int spacesPerIndent = 2;  // Adjust this to match your language's indentation rules
    int currentIndentLevel = 0;  // Keep track of the current indentation level
    std::string currentFunctionName;  // Keep track of the current function name

    int indentLevel = 0;
    while (std::getline(iss, line)) {
    while (!line.empty() && std::iscntrl(line.back())) {
        line.pop_back();  // Remove trailing control character
    }

    if (line.empty() || line[0] == '#') {
        continue; // Skip empty lines and comments
    }

    indentLevel = 0;
    while (indentLevel < line.size() && line[indentLevel] == ' ') {
        ++indentLevel;
    }
    indentLevel /= spacesPerIndent;  // Calculate the indentation level

    if (indentLevel < currentIndentLevel) {
        currentFunctionName = "";
        currentIndentLevel = indentLevel;
    }
if (line.substr(indentLevel, 3) == "def") {
    std::string functionName = line.substr(indentLevel + 4);  // Extract the function name
    tokens.emplace_back(TokenType::FUNCTION_DEF, functionName, indentLevel);  // Emplace the def token with the function name as its value
    std::cout << "\nEmplacing def with name: " << functionName << "\nIndent level: " << indentLevel << "\n";
    currentFunctionName = functionName;  // Assign the current function name
    currentIndentLevel++;  // Increment the current indentation level
}  
else if (line.substr(0, 5) == "print") {
        tokens.emplace_back(TokenType::PRINT, line, indentLevel);
        std::cout << "Emplacing PRINT token with value: " << line << std::endl;
} 
else if (line.find('(') != std::string::npos && line.find(')') != std::string::npos && line.find('=') != std::string::npos) {
    tokens.emplace_back(TokenType::ASSIGNMENT_FUNCTION_CALL, line, indentLevel);  // Emplace the assignment function call token with the entire line as its value
    std::cout << "\nEmplacing ASSIGNMENT_FUNCTION_CALL token with value: " << line << "\nIndent level: " << indentLevel << "\n";
} else if (line.find('=') != std::string::npos) {
    tokens.emplace_back(TokenType::ASSIGN, line, indentLevel);
    std::cout << "\nEmplacing ASSIGNMENT token with value: " << line << "\nIndent level: " << indentLevel << "\n";
}
   else if (line.find("return") != std::string::npos) {
    tokens.emplace_back(TokenType::RETURN, line, indentLevel);  // Emplace the return token with the entire line as its value
    std::cout << "\nEmplacing RETURN token with value: " << line << "\nIndent level: " << indentLevel << "\n";
} else if (line.find(currentFunctionName) != std::string::npos) {
    tokens.emplace_back(TokenType::FUNCTION_CALL, line, indentLevel);  // Emplace the function call token with the entire line as its value
    std::cout << "\nEmplacing FUNCTION_CALL token with value: " << line << "\nIndent level: " << indentLevel << "\n";
} else {
    size_t equalsPos = line.find('=');
    if (equalsPos != std::string::npos) {
        tokens.emplace_back(TokenType::ASSIGN, line, indentLevel);
        std::cout << "\nEmplacing ASSIGNMENT token with value: " << line << "\nIndent level: " << indentLevel << "\n";
    }
}
        }

    std::cout << "\nEmplacing END token\n";
    tokens.emplace_back(TokenType::END, "", 0);
    std::cout << "\nFinished tokenizing\n";
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


void processOperator(TokenType op, std::stack<int>& operands, std::unordered_map<std::string, int>& context, Interpreter& interpret) {
    if (operands.size() < 2) {
        int numOperands = operands.size();
        std::cout << "Debug: Current token is " << interpret.getCurrentToken() << std::endl;
        std::cout << "Debug: Value of val is " << context["val"] << std::endl;
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




void parseEnd(const Token& token, Interpreter& interpreter) {
    std::cout << "Parsed end token." << std::endl;
}


int evaluateExpression(const std::vector<std::string>& parts, std::unordered_map<std::string, int>& context, Interpreter& interpret) {
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
                processOperator(operators.top(), operands, context, interpret);
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
        processOperator(operators.top(), operands, context, interpret);
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

/*
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
*/

void parseAssignment(const Token& token, std::unordered_map<std::string, int>& context, Interpreter& interpreter) {
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

    // Parse the expression and store the result in the context
    std::istringstream iss(spacedExpr);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) {
        parts.push_back(part);
    }

    // Evaluate the expression
    int result = evaluateExpression(parts, context, interpreter);
    context[id] = result;
}





std::vector<std::string> printVariables;

void parsePrint(const Token& token, std::unordered_map<std::string, int>& context, Interpreter& interpreter) {
    // Split the token value into parts at the comma
    std::size_t commaPos = token.value.find(',');
    if (commaPos == std::string::npos) {
        throw std::runtime_error("Invalid print format.");
    }

    std::string firstPart = token.value.substr(0, commaPos);
    std::string secondPart = token.value.substr(commaPos + 1);

    // Remove whitespace from the second part
    secondPart.erase(remove_if(secondPart.begin(), secondPart.end(), isspace), secondPart.end());

    // Remove quotation marks from the first part
    firstPart.erase(remove(firstPart.begin(), firstPart.end(), '\"'), firstPart.end());

    // Check if the variable exists in the context
    if (context.find(secondPart) == context.end()) {
        throw std::runtime_error("Variable " + secondPart + " not found.");
    }

    // Print the first part and the value of the variable
    std::cout << firstPart << " " << context[secondPart] << std::endl;
}

void parseFunctionDef(const Token& token, Interpreter& interpreter) {
    // Tokenize the function definition
    // The first token should be the function name
    // The remaining tokens represent the function body
    // Create a FunctionDefNode with the function name and body
    // Add the FunctionDefNode to the interpreter
}

void parseFunctionCall(const Token& token, Interpreter& interpreter) {
    // Extract the function name and arguments from the token value
    size_t openParenthesisPos = token.value.find('(');
    size_t closeParenthesisPos = token.value.find(')');
    std::string functionName = token.value.substr(0, openParenthesisPos);
    std::string arguments = token.value.substr(openParenthesisPos + 1, closeParenthesisPos - openParenthesisPos - 1);

    // Create a FunctionCallNode with the function name and arguments
    FunctionCallNode* node = new FunctionCallNode(functionName, arguments);

    // Push the FunctionCallNode onto the function call stack
    interpreter.functionCallStack.push(node);
}


void parseProgram(const std::vector<Token>& tokens, std::unordered_map<std::string, int>& context, std::vector<Token>& printStatements, Interpreter& interpreter) {
    std::cout << "*************************" << "\n";
    for (const Token& token : tokens) {
        switch (token.type) {
            case TokenType::ASSIGN:
                parseAssignment(token, context, interpreter);
                break;
            case TokenType::PRINT:
                printStatements.push_back(token); // Store print tokens for later processing
                break;
            case TokenType::END:
                // Handling end token if necessary, e.g., cleanup or summary actions
                parseEnd(token, interpreter);
                break;
            case TokenType::FUNCTION_DEF:
                parseFunctionDef(token, interpreter);
                break;
            case TokenType::SCOPE:
                if (token.value == "{") {
                    interpreter.enterScope();
                } else if (token.value == "}") {
                    interpreter.leaveScope();
                }
                break;
            case TokenType::FUNCTION_CALL:
            parseFunctionCall(token, interpreter);
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

    Interpreter interpreter;  // Create an Interpreter object
    auto tokens = tokenize(input, interpreter);  // Pass the Interpreter object to the tokenize function
    
    std::unordered_map<std::string, int> context;  // This will hold variable values
    std::vector<Token> printStatements; // Store print statements to handle after all evaluations

    // Parse and evaluate all tokens, store print statements for later
    parseProgram(tokens, context, printStatements, interpreter);  // Pass the Interpreter object to the parseProgram function

       // Optionally print all context variables
    std::cout << "Final Variable Values:\n";
    for (const auto& pair : context) {
        std::cout << pair.first << " = " << pair.second << std::endl;
    }
    std:: cout << "********************" << "\n";
    // Now handle print statements
    for (const auto& token : printStatements) {
        parsePrint(token, context, interpreter);  // Pass the Interpreter object to the parsePrint function
    }

    
 

    return 0;
}
