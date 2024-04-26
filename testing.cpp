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


struct Token { // Debugging: Defined a struct for tokens
    TokenType type; // Debugging: Added a member for token type
    std::string value; // Debugging: Added a member for token value
    int indent_level; // Added to track indentation level

        Token(TokenType type, std::string value, int indent_level)
        : type(type), value(value), indent_level(indent_level) {}
};

class FunctionDefNode;
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, std::unique_ptr<FunctionDefNode>>& functions) = 0;
    virtual std::string toString() const = 0;  // Pure virtual function
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

            void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, std::unique_ptr<FunctionDefNode>>& functions) override {

        // Implementation of evaluate for FunctionCallNode
        // This will depend on how you've implemented your language
    }
};

class FunctionDefNode : public ASTNode {
public:
    std::string name;
    std::vector<Token> bodyTokens;
FunctionDefNode(const std::string& name, const std::vector<Token>& bodyTokens) : name(name), bodyTokens(bodyTokens) {}
    std::string toString() const override {
        return "FunctionDefNode: " + name + "(" + /* replace this with the string representation of bodyTokens */ + ")";
    }


            void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, std::unique_ptr<FunctionDefNode>>& functions) override {

        // Implementation of evaluate for FunctionCallNode
        // This will depend on how you've implemented your language
    }
};



class Interpreter {
    std::string current_token;
    std::stack<std::unordered_map<std::string, int>> scopes;
    std::unordered_map<std::string, std::unique_ptr<FunctionDefNode>> functions;
    int returnValue;
    std::string returnVariable;

public:
    
    std::stack<FunctionCallNode*> functionCallStack;

   std::unique_ptr<FunctionDefNode> createFunctionDefNode(const std::string& name, const std::vector<Token>& tokens) {
    return std::make_unique<FunctionDefNode>(name, tokens);
}

    void enterScope() {
        scopes.push(std::unordered_map<std::string, int>());
    }

    void leaveScope() {
        if (!scopes.empty()) {
            scopes.pop();
        }
    }
    
    int getVariable(const std::string& name) {
        if (!scopes.empty() && scopes.top().count(name) > 0) {
            return scopes.top().at(name);
        }
        throw std::runtime_error("Variable not found inside getVariable in Interpreter: " + name);
    }

    void addFunction(const std::string& name, std::unique_ptr<FunctionDefNode> functionDefNode) {
    functions[name] = std::move(functionDefNode);
}
    void setCurrentToken(const std::string& token) { current_token = token; }

    std::string getCurrentToken() { return current_token; }

    int evaluate(const std::string& expression) {
        // TODO: Implement expression evaluation
        return 0;
    }

    void setReturnValue(int value) {
        returnValue = value;
    }

    void setReturnVariable(const std::string& variable) {
        returnVariable = variable;
    }

    int getReturnVariableValue() {
        if (scopes.top().count(returnVariable) == 0) {
            throw std::runtime_error("Return variable not found: " + returnVariable);
        }
        return scopes.top().at(returnVariable);
    }

    int callFunction(const std::string& functionName) {
        if (functions.count(functionName) == 0) {
            throw std::runtime_error("Function not found: " + functionName);
        }

        // Call the function and return the result
        functions[functionName]->evaluate(scopes, functions);
        return getReturnVariableValue();
    }



void parseFunctionDef(const std::vector<Token>& tokens) {
    // Extract the function name from tokens
    std::string functionName = tokens[0].value; // replace `value` with the correct method or property

    // Create a vector of Tokens for the body
    std::vector<Token> bodyTokens;
    for (auto it = tokens.begin() + 1; it != tokens.end(); ++it) {
        bodyTokens.push_back(*it);
    }

    functions[functionName] = std::make_unique<FunctionDefNode>(functionName, bodyTokens);
}

void parseFunctionCall(const Token& token) {
    // Extract the function name and arguments from token
    std::string functionName = token.value; // replace `value` with the correct method or property
    std::string arguments = ""; // You need to implement this part based on your Token structure
    FunctionCallNode* node = new FunctionCallNode(functionName, arguments);
}

};


void parseFunctionDef(const std::vector<Token>& tokens, Interpreter& interpreter) {
    // Assuming the first token is the function name
    std::string functionName = tokens[0].value;

    // Create a vector of Tokens for the body
    std::vector<Token> bodyTokens;
    for (auto it = tokens.begin() + 1; it != tokens.end(); ++it) {
        bodyTokens.push_back(*it);
    }

    std::unique_ptr<FunctionDefNode> node = std::make_unique<FunctionDefNode>(functionName, bodyTokens);
    interpreter.addFunction(functionName, std::move(node));
}


class VariableNode : public ASTNode {
    std::string name;
public:
    VariableNode(const std::string& n) : name(n) {}
    void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, std::unique_ptr<FunctionDefNode>>& functions) override {
        if (contexts.empty()) {
            throw std::runtime_error("No context available in variable node .");
        }
        auto& context = contexts.top();  // Get the current context
        auto it = context.find(name);
        if (it == context.end()) {
            throw std::runtime_error("Variable " + name + " not found in context in variablenode.");
        }
        context["__expr_result"] = it->second;
    }
};

class PrintNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> parts;

public:
    PrintNode(std::vector<std::unique_ptr<ASTNode>> parts) : parts(std::move(parts)) {}

    void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, std::unique_ptr<FunctionDefNode>>& functions) override {
    for (auto& part : parts) {
        part->evaluate(contexts, functions);
        int result = contexts.top()["__expr_result"];
        std::cout << result << " ";
    }
    std::cout << std::endl;
}

    


};

class IfNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> ifBlock;
    std::vector<std::unique_ptr<ASTNode>> elseBlock;

public:
    IfNode(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> ifBlk, std::vector<std::unique_ptr<ASTNode>> elseBlk)
        : condition(std::move(cond)), ifBlock(std::move(ifBlk)), elseBlock(std::move(elseBlk)) {}

    void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, std::unique_ptr<FunctionDefNode>>& functions) override {
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


class AssignmentNode : public ASTNode {
    std::string name;
    std::unique_ptr<ASTNode> value;
public:
    AssignmentNode(const std::string& n, std::unique_ptr<ASTNode> v) : name(n), value(std::move(v)) {}
    void evaluate(std::stack<std::unordered_map<std::string, int>>& contexts, std::unordered_map<std::string, std::unique_ptr<FunctionDefNode>>& functions) override {
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
    int currentIndentLevel = 0;  // Keep track of the current indentation level
    int indentSize = -1;  // The number of spaces that represent one indent level
    std::string currentFunctionName;  // Keep track of the current function name

    while (std::getline(iss, line)) {
        while (!line.empty() && std::iscntrl(line.back())) {
            line.pop_back();  // Remove trailing control character
        }

        if (line.empty() || line[0] == '#') {
            continue; // Skip empty lines and comments
        }

        int indentLevel = 0;
        while (indentLevel < line.size() && line[indentLevel] == ' ') {
            ++indentLevel;
        }

        if (indentSize == -1 && indentLevel > 0) {
            indentSize = indentLevel;  // Set the indent size on the first indented line
        }

        if (indentSize != -1 && indentSize != 0) {  // Check if indentSize is not zero before division
            currentIndentLevel = indentLevel / indentSize;  // Calculate the current indent level
        }
        if (line.substr(indentLevel, 3) == "def") {
    std::string functionName = line.substr(indentLevel + 4);  // Extract the function name
    tokens.emplace_back(TokenType::FUNCTION_DEF, functionName, currentIndentLevel);  // Emplace the def token with the function name as its value
    std::cout << "\nEmplacing def with name: " << functionName << "\nIndent level: " << currentIndentLevel << "\n";
    currentFunctionName = functionName;  // Assign the current function name

    // Create a FunctionDefNode from tokens
    std::vector<Token> functionTokens(tokens.begin() + 1, tokens.end()); // Assuming the function name is the first token
    std::unique_ptr<FunctionDefNode> functionDefNode = interpreter.createFunctionDefNode(functionName, functionTokens);

    // Add function to the interpreter
    interpreter.addFunction(functionName, std::move(functionDefNode));
}
        else if (line.substr(indentLevel, 5) == "print") {
            tokens.emplace_back(TokenType::PRINT, line, currentIndentLevel);
            std::cout << "Emplacing PRINT token with value: " << line << std::endl;
        } 
        else if (line.find('(') != std::string::npos && line.find(')') != std::string::npos && line.find('=') != std::string::npos) {
            tokens.emplace_back(TokenType::ASSIGNMENT_FUNCTION_CALL, line, currentIndentLevel);  // Emplace the assignment function call token with the entire line as its value
            std::cout << "\nEmplacing ASSIGNMENT_FUNCTION_CALL token with value: " << line << "\nIndent level: " << currentIndentLevel << "\n";
        } else if (line.find('=') != std::string::npos) {
            tokens.emplace_back(TokenType::ASSIGN, line, currentIndentLevel);
            std::cout << "\nEmplacing ASSIGNMENT token with value: " << line << "\nIndent level: " << currentIndentLevel << "\n";
        }
        else if (line.find("return") != std::string::npos) {
            tokens.emplace_back(TokenType::RETURN, line, currentIndentLevel);  // Emplace the return token with the entire line as its value
            std::cout << "\nEmplacing RETURN token with value: " << line << "\nIndent level: " << currentIndentLevel << "\n";
        } else if (line.find(currentFunctionName) != std::string::npos) {
            tokens.emplace_back(TokenType::FUNCTION_CALL, line, currentIndentLevel);  // Emplace the function call token with the entire line as its value
            std::cout << "\nEmplacing FUNCTION_CALL token with value: " << line << "\nIndent level: " << currentIndentLevel << "\n";
        } 
        if (line.substr(indentLevel, 2) == "if") {
            tokens.emplace_back(TokenType::IF, line, currentIndentLevel);
            std::cout << "\nEmplacing IF token with value: " << line << "\nIndent level: " << currentIndentLevel << "\n";
        } else if (line.substr(indentLevel, 4) == "else") {
            tokens.emplace_back(TokenType::ELSE, "", currentIndentLevel);
            std::cout << "\nEmplacing ELSE token\nIndent level: " << currentIndentLevel << "\n";
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

    // If the second part ends with a closing parenthesis ")", remove it
    if (!secondPart.empty() && secondPart.back() == ')') {
        secondPart.pop_back();
    }

    // Find the position of the opening parenthesis "(" in the first part
    std::size_t openParenPos = firstPart.find('(');
    if (openParenPos != std::string::npos) {
        // Remove everything before and including the opening parenthesis
        firstPart = firstPart.substr(openParenPos + 1);
    }

    // Remove quotation marks from the first part
    firstPart.erase(remove(firstPart.begin(), firstPart.end(), '\"'), firstPart.end());

    // Check if the variable exists in the context
    if (context.find(secondPart) == context.end()) {
        throw std::runtime_error("Variable " + secondPart + " not found in parsePrint.");
    }

    // Print the first part and the value of the variable
    std::cout << firstPart << " " << context[secondPart] << std::endl;
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

void parseReturn(const Token& token, Interpreter& interpreter) {
    // Extract the return variable from the token value
    std::string returnVariable = token.value.substr(7); // Skip the "return " part

    // Set the return variable in the interpreter
    interpreter.setReturnVariable(returnVariable);
}

void parseAssignmentFunctionCall(const Token& token, std::unordered_map<std::string, int>& context, Interpreter& interpreter) {
    // Extract the variable name, function name, and arguments from the token value
    size_t equalsPos = token.value.find('=');
    std::string variableName = token.value.substr(0, equalsPos);
    std::string functionCall = token.value.substr(equalsPos + 1);

    // Call the function and get the return value
    int returnValue = interpreter.callFunction(functionCall);

    // Assign the return value to the variable in the context
    context[variableName] = returnValue;
}

void parseProgram(const std::vector<Token>& tokens, std::unordered_map<std::string, int>& context, std::vector<Token>& printStatements, Interpreter& interpreter) {
    std::cout << "*************************" << "\n";
    size_t i = 0;
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
             case TokenType::FUNCTION_DEF: {
                std::vector<Token> functionTokens;
                // Increment `i` to skip the function definition token
                i++;
                // Collect all tokens that belong to the function definition
                while (i < tokens.size() && tokens[i].indent_level != 0) {
                functionTokens.push_back(tokens[i]);
                i++;
            }
                parseFunctionDef(functionTokens, interpreter);
                break;
            }
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
            case TokenType::RETURN:
                parseReturn(token, interpreter);
                break;
            case TokenType::ASSIGNMENT_FUNCTION_CALL:
                parseAssignmentFunctionCall(token, context, interpreter);
                break;
             case TokenType::IF:
                std::cout << "Encountered IF token with value: " << token.value << "\n";
                break;
            case TokenType::ELSE:
                std::cout << "Encountered ELSE token\n";
                break;
            default:
                std::stringstream ss;
                ss << "Unexpected token type in parseProgram: " << static_cast<int>(token.type);
                throw std::runtime_error(ss.str());
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
