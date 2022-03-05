
#include "logger.h"

#include "commons.h"
using namespace commons;

#include "ast.h"

int Node::counter;
std::map<Node*, bool> Node::balances;

Node::Node() { balances[this] = true; ++Node::counter; }
Node::~Node() {
    balances[this] = false;
    --Node::counter;
    for (auto iter = begin(); iter != end(); ++iter) 
        delete* iter;
}

using namespace ast;

// AstNode
void AstNode::setParent(AstNode& parent) {
    if (&parent == this->parent)
        return;
    this->parent = &parent;
}
int AstNode::getLineno() {
    if (lineno != -1)
        return lineno;
    if (parent != nullptr)
        return parent->getLineno();
    return -1;
}
void AstNode::addChild(AstNode& kid) {
    addChildToBack(kid);
    kid.setParent(*this);
}
AstNode::AstNode() : parent(nullptr) {}

static inline void release_safe(AstNode* node, std::string const& info) {
    if (!Node::balances[node])
        out("[" + info + "] node <" + cast_string((int64_t)(void*)node) + "> has been released.");
    delete node;
}

// Block
std::string Block::toSource() {
    std::string source = std::string();
    source.append("{\n");
    for (auto iter = begin(); iter != end(); ++iter) {
        source.append(((AstNode*)* iter)->toSource());
    }
    source.append("}\n");
    return source;
}
Block::Block() { type = token::BLOCK; }

// Jump
Jump::Jump() : target(nullptr), target2(nullptr), jumpNode(nullptr) { type = token::ERROR; }
Jump::Jump(token::type nodeType) : target(nullptr), target2(nullptr), jumpNode(nullptr) { type = nodeType; }
Jump::Jump(token::type nodeType, int lineno) : target(nullptr), target2(nullptr), jumpNode(nullptr) {
    type = nodeType;
    setLineno(lineno);
}
Jump::Jump(token::type nodeType, Node& child) : target(nullptr), target2(nullptr), jumpNode(nullptr) {
    type = nodeType;
    addChildToBack(child);
}
Jump::Jump(token::type nodeType, Node& child, int lineno) : target(nullptr), target2(nullptr), jumpNode(nullptr) {
    type = nodeType;
    addChildToBack(child);
    setLineno(lineno);
}
Jump::~Jump() {
    delete target, target = nullptr;
    delete target2, target2 = nullptr;
    delete jumpNode, jumpNode = nullptr;
}

// Scope
std::string Scope::toSource() {
    std::string source = std::string();
    source.append("{\n");
    for (auto iter = begin(); iter != end(); ++iter) {
        source.append(((AstNode*)* iter)->toSource());
    }
    source.append("}\n");
    return source;
}
void Scope::setParentScope(Scope& parentScope) {
    this->parentScope = &parentScope;
    this->top = this->parentScope == nullptr ? (ScriptNode*)this : parentScope.top;
}
void Scope::addChildScope(Scope& child) {
    childScopes.push_back(&child);
    child.setParentScope(*this);
}
Scope::Scope() : top(nullptr), parentScope(nullptr) { type = token::BLOCK; }
Scope::~Scope() { delete parentScope, parentScope = nullptr; }

// ScriptNode
ScriptNode::ScriptNode() { this->type = token::SCOPE; }

// AstRoot
std::string AstRoot::toSource() {
    std::string source = std::string();
    for (auto iter = begin(); iter != end(); ++iter) {
        source.append(((AstNode*)* iter)->toSource());
    }
    return source;
}
AstRoot::AstRoot() { this->type = token::SCRIPT; }

// Name
std::string Name::toSource() { return identifier; }
Name::Name(const std::string name) : identifier(name), scope(nullptr) { type = token::IDENTIFIER; }
Name::Name(Name& base) : identifier(base.getIdentifier()), scope(nullptr) { type = token::IDENTIFIER; }

// NumberLiteral
std::string NumberLiteral::toSource() { return value; }
NumberLiteral::NumberLiteral(const std::string val, double number) : value(val) {
    type = token::NUMBER;
    setNumber(number);
}
NumberLiteral::NumberLiteral(double number) {
    type = token::NUMBER;
    setNumber(number);
    setValue(cast_string<double>(number));
}
NumberLiteral::NumberLiteral(const std::string val) : value(val), number(string_cast<double>(val)) { type = token::NUMBER; }

// KeywordLiteral
KeywordLiteral& KeywordLiteral::setType(token::type nodeType) {
    if (!(nodeType == token::NULL_T || 
        nodeType == token::UNDEFINED ||
        nodeType == token::VOID ||
        nodeType == token::TRUE || 
        nodeType == token::FALSE))
        out(std::string("Invalid node type: ") + token::kind(nodeType));
    type = nodeType;
    return *this;
}
std::string KeywordLiteral::toSource() {
    switch (type)
    {
    case token::NULL_T: return "null";
    case token::UNDEFINED: return "undefined";
    case token::VOID: return "void";
    case token::TRUE: return "true";
    case token::FALSE: return "false";
    default:
        return std::string(token::kind(type));
    }
}
KeywordLiteral::KeywordLiteral(token::type nodeType) { setType(nodeType); }

// StringLiteral
std::string StringLiteral::toSource() {
    std::string source = value;
    return "\"" + commons::replace_all_distinct(source, "\n", "\\n") + "\""; 
}
StringLiteral::StringLiteral() { type = token::STRING; }
StringLiteral::StringLiteral(std::string str) : value(str) { type = token::STRING; }
StringLiteral::StringLiteral(std::string& str) : value(str) { type = token::STRING; }

// ArrayLiteral
std::string ArrayLiteral::toSource() {
    std::string source = std::string();
    source.append("[");
    printList(elements, source);
    source.append("]");
    return source;
}
void ArrayLiteral::setElements(anr elements) {
    this->elements.clear();
    for (AstNode* e : elements) {
        addElement(*e);
    }
}
void ArrayLiteral::addElement(AstNode& element) {
    elements.push_back(&element);
    element.setParent(*this);
}
ArrayLiteral::ArrayLiteral() { type = token::ARRAYLIT; }
ArrayLiteral::~ArrayLiteral() {
    for (AstNode* e : elements)
        release_safe(e, "ArrayLiteral");
}

// VariableInitializer
std::string VariableInitializer::toSource() {
    std::string source = std::string();
    source.append(((Name*)target)->toSource());
    source.append(" = ");
    source.append(initializer->toSource());
    return source;
}
void VariableInitializer::setTarget(AstNode& target) {
    this->target = &target;
    target.setParent(*this);
}
void VariableInitializer::setInitializer(AstNode& initializer) {
    this->initializer = &initializer;
    if (&initializer != nullptr)
        initializer.setParent(*this);
}
VariableInitializer::VariableInitializer() : target(nullptr), initializer(nullptr) { type = token::VAR; }
VariableInitializer::~VariableInitializer() {
    delete target, target = nullptr;
    release_safe(initializer, "VariableInitializer");
    initializer = nullptr;
}

// VariableDeclaration
std::string VariableDeclaration::toSource() {
    std::string source = std::string();
    source.append(declTypeName());
    source.append(" ");
    printList(variables, source);
    if (isStatement())
        source.append(";\n");
    return source;
}
void VariableDeclaration::setVariables(std::list<VariableInitializer*> variables) {
    this->variables.clear();
    for (VariableInitializer* vi : variables) {
        addVariable(*vi);
    }
}
void VariableDeclaration::addVariable(VariableInitializer& v) {
    variables.push_back(&v);
    v.setParent(*this);
}
Node& VariableDeclaration::setType(token::type type) {
    if (type != token::VAR) {
        out(std::string("invalid decl type: ") + token::kind(type));
        return *this;
    }
    return ((Node*)this)->setType(type);
}
VariableDeclaration::VariableDeclaration() : is_statement(true) { type = token::VAR; }
VariableDeclaration::~VariableDeclaration() {
    for (VariableInitializer* vi : variables) delete vi;
    variables.clear();
}

// EmptyExpression
std::string EmptyExpression::toSource() { return ""; }
EmptyExpression::EmptyExpression() { type = token::EMPTY; }

// UnaryExpression
std::string UnaryExpression::toSource() {
    std::string source = std::string();
    // bool prefix_adable = type != token::POS && type != token::NEG;
    if (!is_postfix) // is prefix
        source += operatorNames[type];
    source += operand->toSource();
    if (is_postfix) // is postfix
        source += operatorNames[type];
    return source;
}
void UnaryExpression::setOperand(AstNode& operand) {
    this->operand = &operand;
    operand.setParent(*this);
}
UnaryExpression::UnaryExpression(token::type operater, AstNode& operand) : is_postfix(false) {
    type = operater;
    setOperand(operand);
}
UnaryExpression::UnaryExpression(token::type operater, AstNode& operand, bool postFix) : is_postfix(postFix) {
    type = operater;
    setOperand(operand);
}

// InfixExpression
std::string InfixExpression::toSource() {
    std::string source = std::string();
    source += left->toSource();
    source += ' ';
    source += operatorNames[type];
    source += ' ';
    source += right->toSource();
    return source;
}
void InfixExpression::setLeftAndRight(AstNode& left, AstNode& right) {
    setLeft(left);
    setRight(right);
}
void InfixExpression::setLeft(AstNode& left) {
    this->left = &left;
    setLineno(left.getLineno());
    left.setParent(*this);
}
void InfixExpression::setRight(AstNode& right) {
    this->right = &right;
    right.setParent(*this);
}
InfixExpression::InfixExpression(AstNode& left, AstNode& right) {
    setLeftAndRight(left, right);
}
InfixExpression::InfixExpression(token::type operater, AstNode& left, AstNode& right) {
    setType(operater);
    setLeftAndRight(left, right);
}
InfixExpression::~InfixExpression() {
    delete left, left = nullptr;
    delete right, right = nullptr;
}

// ConditionalExpression
std::string ConditionalExpression::toSource() {
    std::string source = std::string();
    source += testExpression->toSource();
    source += " ? ";
    source += trueExpression->toSource();
    source += " : ";
    source += falseExpression->toSource();
    return source;
}
void ConditionalExpression::setTestExpression(AstNode& testExpression) {
    this->testExpression = &testExpression;
    testExpression.setParent(*this);
}
void ConditionalExpression::setTrueExpression(AstNode& trueExpression) {
    this->trueExpression = &trueExpression;
    trueExpression.setParent(*this);
}
void ConditionalExpression::setFalseExpression(AstNode& falseExpression) {
    this->falseExpression = &falseExpression;
    falseExpression.setParent(*this);
}
ConditionalExpression::~ConditionalExpression() {
    delete testExpression, testExpression = nullptr;
    delete trueExpression, trueExpression = nullptr;
    delete falseExpression, falseExpression = nullptr;
}

// Assignment
Assignment::Assignment(AstNode& left, AstNode& right) : InfixExpression(left, right) {}
Assignment::Assignment(token::type operater, AstNode& left, AstNode& right) : InfixExpression(operater, left, right) {}

// ParenthesizedExpression
void ParenthesizedExpression::setExpression(AstNode& expression) {
    this->expression = &expression;
    expression.setParent(*this);
}
std::string ParenthesizedExpression::toSource() { return "(" + expression->toSource() + ")"; }
ParenthesizedExpression::ParenthesizedExpression() : expression(nullptr) { type = token::LP; }
ParenthesizedExpression::ParenthesizedExpression(AstNode& expr) : expression(nullptr) {
    type = token::LP;
    setExpression(expr);
}

// FunctionCall
std::string FunctionCall::toSource() {
    std::string source = std::string();
    source.append(target->toSource());
    source.append("(");
    printList(arguments, source);
    source.append(")");
    return source;
}
void FunctionCall::setTarget(AstNode& target) {
    this->target = &target;
    target.setParent(*this);
}
void FunctionCall::setArguments(ant arguments) {
    this->arguments.clear();
    for (AstNode* arg : arguments) {
        addArgument(*arg);
    }
}
void FunctionCall::addArgument(AstNode& arg) {
    arguments.push_back(&arg);
    arg.setParent(*this);
}
FunctionCall::FunctionCall() : target(nullptr) { type = token::CALL; }
FunctionCall::~FunctionCall() {
    if (target != nullptr) delete target, target = nullptr;
    for (AstNode* arg : arguments) 
        release_safe(arg, "FunctionCall");
}

// ElementGet
std::string ElementGet::toSource() {
    std::string source = std::string();
    source.append(target->toSource());
    source.append("[");
    source.append(element->toSource());
    source.append("]");
    return source;
}
void ElementGet::setTarget(AstNode& target) {
    this->target = &target;
    target.setParent(*this);
}
void ElementGet::setElement(AstNode& element) {
    this->element = &element;
    element.setParent(*this);
}
ElementGet::ElementGet() : target(nullptr), element(nullptr) { type = token::GETELEM; }
ElementGet::ElementGet(AstNode& target, AstNode& element) {
    type = token::GETELEM;
    setTarget(target);
    setElement(element);
}
ElementGet::~ElementGet() {
    delete target, target = nullptr;
    delete element, element = nullptr;
}

// FunctionNode
std::string FunctionNode::toSource() {
    std::string source = std::string();
    source.append("func ");
    source.append(functionName->toSource());
    source.append("(");
    printList(params, source);
    source.append(") ");
    source.append(body->toSource());
    return source;
}
void FunctionNode::setFunctionName(Name& name) {
    this->functionName = &name;
    name.setParent(*this);
}
void FunctionNode::setParams(anr params) {
    this->params.clear();
    for (AstNode* param : params)
        addParam(*param);
}
void FunctionNode::addParam(AstNode& param) {
    params.push_back(&param);
    param.setParent(*this);
}
void FunctionNode::setBody(AstNode& body) {
    this->body = &body;
    body.setParent(*this);
}
FunctionNode::FunctionNode() : functionName(nullptr), body(nullptr) { type = token::FUNC; }
FunctionNode::FunctionNode(Name& name) {
    type = token::FUNC;
    setFunctionName(name);
}
FunctionNode::~FunctionNode() {
    delete functionName, functionName = nullptr;
    for (AstNode* par : params) 
        release_safe(par, "FunctionNode");
    delete body, body = nullptr;
}


// EmptyStatement
std::string EmptyStatement::toSource() { return ";\n"; }
EmptyStatement::EmptyStatement() { type = token::EMPTY; }

// ExpressionStatement
std::string ExpressionStatement::toSource() {
    std::string source = std::string();
    source.append(expr->toSource());
    // source.append(";\n");
    return source;
}
void ExpressionStatement::setExpression(AstNode& expression) {
    expr = &expression;
    expression.setParent(*this);
    setLineno(expression.getLineno());
}
ExpressionStatement::ExpressionStatement() : expr(nullptr) { type = token::EXPR_VOID; }
ExpressionStatement::ExpressionStatement(AstNode& expr) {
    type = token::EXPR_VOID;
    setExpression(expr);
}
ExpressionStatement::ExpressionStatement(AstNode& expr, bool hasResult) {
    type = token::EXPR_VOID;
    setExpression(expr);
    if (hasResult) setHasResult();
}
ExpressionStatement::~ExpressionStatement() {
    release_safe(expr, "ExpressionStatement");
    expr = nullptr;
}

// ReturnStatement
std::string ReturnStatement::toSource() {
    std::string source = std::string();
    source.append("return");
    if (returnValue != nullptr) {
        source.append(" ");
        source.append(returnValue->toSource());
    }
    source.append(";\n");
    return source;
}
void ReturnStatement::setReturnValue(AstNode& returnValue) {
    this->returnValue = &returnValue;
    returnValue.setParent(*this);
}
ReturnStatement::ReturnStatement() : returnValue(nullptr) { type = token::RETURN; }
ReturnStatement::ReturnStatement(AstNode& returnValue) {
    type = token::RETURN;
    setReturnValue(returnValue);
}
ReturnStatement::~ReturnStatement() { delete returnValue, returnValue = nullptr; }

// IfStatement
std::string IfStatement::toSource() {
    std::string source = std::string();
    source += "if (";
    source += condition->toSource();
    source += ") ";
    source += thenPart->toSource();
    if (elsePart != nullptr) {
        source += "else ";
        source += elsePart->toSource();
    }
    return source;
}
void IfStatement::setCondition(AstNode& condition) {
    this->condition = &condition;
    condition.setParent(*this);
}
void IfStatement::setThenPart(AstNode& thenPart) {
    this->thenPart = &thenPart;
    thenPart.setParent(*this);
}
void IfStatement::setElsePart(AstNode& elsePart) {
    this->elsePart = &elsePart;
    elsePart.setParent(*this);
}
IfStatement::IfStatement() : condition(nullptr), thenPart(nullptr), elsePart(nullptr) { type = token::IF; }
IfStatement::~IfStatement() {
    delete condition, condition = nullptr;
    delete thenPart, thenPart = nullptr;
    if (hasElsePart()) delete elsePart, elsePart = nullptr;
}

// Loop
void Loop::setBody(AstNode& body) {
    this->body = &body;
    body.setParent(*this);
}
Loop::Loop() : body(nullptr) {};
Loop::~Loop() { delete body, body = nullptr; }

// WhileLoop
std::string WhileLoop::toSource() {
    std::string source = std::string();
    source.append("while (");
    source.append(condition->toSource());
    source.append(") ");
    if (body->getType() == token::BLOCK) {
        source.append(body->toSource());
        source.append("\n");
    }
    else {
        source.append("\n");
        source.append(body->toSource());
    }
    return source;
}
void WhileLoop::setCondition(AstNode& condition) {
    this->condition = &condition;
    condition.setParent(*this);
}
WhileLoop::WhileLoop() : condition(nullptr) { type = token::WHILE; }
WhileLoop::~WhileLoop() { delete condition, condition = nullptr; }

// ForLoop
std::string ForLoop::toSource() {
    std::string source = std::string();
    source.append("for (");
    source.append(initializer->toSource());
    source.append("; ");
    source.append(condition->toSource());
    source.append("; ");
    source.append(increment->toSource());
    source.append(") ");
    if (body->getType() == token::BLOCK) {
        source.append(body->toSource());
        source.append("\n");
    }
    else {
        source.append("\n");
        source.append(body->toSource());
    }
    return source;
}
void ForLoop::setInitializer(AstNode& initializer) {
    this->initializer = &initializer;
    initializer.setParent(*this);
}
void ForLoop::setCondition(AstNode& condition) {
    this->condition = &condition;
    condition.setParent(*this);
}
void ForLoop::setIncrement(AstNode& increment) {
    this->increment = &increment;
    increment.setParent(*this);
}
ForLoop::ForLoop() : initializer(nullptr), condition(nullptr), increment(nullptr) { type = token::FOR; }
ForLoop::~ForLoop() {
    delete initializer, initializer = nullptr;
    delete condition, condition = nullptr;
    delete increment, increment = nullptr;
}
