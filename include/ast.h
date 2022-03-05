

#include <map>
#include <list>
#include <vector>

#include "lexer.h"

class Node : public std::list<Node*>
{
private:
    static int counter;
public:
    static std::map<Node*, bool> balances;
public:
    static inline int counted() { return counter; }
protected:
    token::type type = token::ERROR;
    int lineno = -1;
public:
    inline token::type getType() { return type; }
    inline Node& setType(token::type type) { this->type = type; return *this; }
    inline int getLineno() { return lineno; }
    inline void setLineno(int lineno) { this->lineno = lineno; }
    inline void addChildToBack(Node& child) { push_back(&child); }
public:
    Node();
    Node(token::type nodeType) : type(nodeType) { ++Node::counter; }
    virtual ~Node();
};

namespace ast
{
    static std::map<token::type, const char*> operatorNames = {
        { token::COMMA, "," }, { token::COLON, ":" },
        { token::OR, "||" }, { token::AND, "&&" },
        { token::INC, "++" }, { token::DEC, "--" },
        { token::BITOR, "|" }, { token::BITAND, "&" }, { token::BITXOR, "^" },
        { token::EQ, "==" }, { token::NE, "!=" },
        { token::LT, "<" }, { token::GT, ">" },
        { token::LE, "<=" }, { token::GE, ">=" },
        { token::LSH, "<<" }, { token::RSH, ">>" },
        { token::POS, "+" }, { token::NEG, "-" },
        { token::ADD, "+" }, { token::SUB, "-" },
        { token::MUL, "*" }, { token::DIV, "/" },
        { token::MOD, "%" }, { token::POW, "**" },
        { token::NOT, "!" }, { token::BITNOT, "~" },
        { token::ASSIGN, "=" },
        { token::ASSIGN_BITOR, "|=" }, { token::ASSIGN_BITAND, "&=" }, { token::ASSIGN_BITXOR, "^=" },
        { token::ASSIGN_LSH, "<<=" }, { token::ASSIGN_RSH, ">>=" },
        { token::ASSIGN_ADD, "+=" }, { token::ASSIGN_SUB, "-=" },
        { token::ASSIGN_MUL, "*=" }, { token::ASSIGN_DIV, "/=" },
        { token::ASSIGN_MOD, "%=" }, { token::ASSIGN_POW, "**=" },
    };

    // extendable
    class AstNode : public Node
    {
    protected:
        AstNode* parent;
    public:
        virtual std::string toSource() = 0;
    public:
        template<typename T>
        static void printList(T& items, std::string& source) {
            int max = items.size();
            int count = 0;
            for (AstNode* item : items) {
                source.append(item->toSource());
                if (count++ < max - 1) {
                    source.append(", ");
                }
            }
        }
        inline bool hasParent() { return parent != nullptr; }
        inline AstNode& getParent() { return *parent; }
        void setParent(AstNode& parent);
        int getLineno();
        void addChild(AstNode& kid);
    public:
        AstNode();
    };

    // extendable
    class Block : public AstNode {
    public:
        Block();
    public:
        inline void addStatement(AstNode& statement) { addChild(statement); }
    public:
        virtual std::string toSource();
    };

    // extendable
    class Jump : public AstNode {
    public:
        Node* target;
    private:
        Node* target2;
        Jump* jumpNode;
    public:
        inline Jump& getJumpStatement() { return *jumpNode; }
        inline void setJumpStatement(Jump& jumpStatement) { this->jumpNode = &jumpStatement; }
        inline Node& getDefault() { return *target2; }
        inline void setDefault(Node& defaultTarget) { target2 = &defaultTarget; }
        inline Node& getFinally() { return *target2; }
        inline void setFinally(Node& finallyTarget) { target2 = &finallyTarget; }
        inline Jump& getLoop() { return *jumpNode; }
        inline void setLoop(Jump& loop) { jumpNode = &loop; }
        inline Node& getContinue() { return *target2; }
        inline  void setContinue(Node& continueTarget) { target2 = &continueTarget; }
    public:
        Jump();
        Jump(token::type nodeType);
        Jump(token::type nodeType, int lineno);
        Jump(token::type nodeType, Node& child);
        Jump(token::type nodeType, Node& child, int lineno);
        virtual ~Jump();
    };

    typedef std::list<AstNode*> ant;
    class ScriptNode;
    // extendable
    class Scope : public Jump {
    protected:
        Scope* parentScope;
        ScriptNode* top;
    private:
        std::list<Scope*> childScopes;
        std::map<std::string, AstNode*> variables;
        std::map<std::string, AstNode* (*)(ant&)> functions;
    public:
        virtual std::string toSource();
    public:
        inline std::map<std::string, AstNode*>& getVariables() { return variables; }
        inline std::map<std::string, AstNode* (*)(ant&)>& getFunctions() { return functions; }
        inline Scope& getParentScope() { return *parentScope; }
        void setParentScope(Scope& parentScope);
        inline void clearParentScope() { this->parentScope = nullptr; }
        inline std::list<Scope*> getChildScopes() { return childScopes; }
        void addChildScope(Scope& child);
    public:
        Scope();
        virtual ~Scope();
    };

    // extendable
    class ScriptNode : public Scope {
    public:
        ScriptNode();
    };

    class AstRoot : public ScriptNode {
    public:
        std::string toSource();
    public:
        AstRoot();
    };

    class Name : public AstNode
    {
    private:
        std::string identifier;
        Scope* scope;
    public:
        virtual std::string toSource();
    public:
        inline std::string& getIdentifier() { return identifier; }
        inline void setIdentifier(std::string& identifier) { this->identifier = identifier; }
    public:
        Name(const std::string name);
        Name(Name& base);
        // ~Name() {}
    };

    class NumberLiteral : public AstNode {
    private:
        std::string value;
        double number;
    public:
        virtual std::string toSource();
    public:
        inline std::string& getValue() { return value; }
        inline void setValue(std::string value) { this->value = value; }
        inline double getNumber() { return number; }
        inline void setNumber(double value) { this->number = value; }
    public:
        NumberLiteral(const std::string val);
        NumberLiteral(const std::string val, double number);
        NumberLiteral(double number);
    };

    class KeywordLiteral : public AstNode {
    public:
        KeywordLiteral& setType(token::type nodeType);
        inline bool isBooleanLiteral() { return type == token::TRUE || type == token::FALSE; }
    public:
        virtual std::string toSource();
    public:
        KeywordLiteral(token::type nodeType);
    };

    class StringLiteral : public AstNode {
    private:
        std::string value;
    public:
        virtual std::string toSource();
    public:
        inline std::string& getValue() { return value; }
        inline void setValue(std::string value) { this->value = value; }
        inline void setValue(std::string& value) { this->value = value; }
    public:
        StringLiteral();
        StringLiteral(std::string str);
        StringLiteral(std::string& str);
    };

    typedef std::vector<AstNode*> anr;
    class ArrayLiteral : public AstNode
    {
    private:
        anr elements;
    public:
        virtual std::string toSource();
    public:
        inline anr& getElements() { return elements; }
        void setElements(anr elements);
        void addElement(AstNode& element);
    public:
        ArrayLiteral();
        ~ArrayLiteral();
    };

    class VariableInitializer : public AstNode
    {
    private:
        AstNode* target;
        AstNode* initializer;
    public:
        virtual std::string toSource();
    public:
        inline AstNode& getTarget() { return *target; }
        void setTarget(AstNode& target);
        inline AstNode& getInitializer() { return *initializer; }
        void setInitializer(AstNode& initializer);
    public:
        VariableInitializer();
        ~VariableInitializer();
    };

    class VariableDeclaration : public AstNode
    {
    private:
        std::list<VariableInitializer*> variables;
        bool is_statement;
        const char* declTypeName() { return token::kind(type); }
    public:
        virtual std::string toSource();
    public:
        inline std::list<VariableInitializer*>& getVariables() { return variables; }
        void setVariables(std::list<VariableInitializer*> variables);
        void addVariable(VariableInitializer& v);
        Node& setType(token::type type);
        inline bool isVar() { return type == token::VAR; }
        inline bool isConst() { return type == token::CONST; }
        inline bool isStatement() { return is_statement; }
        inline void setIsStatement(bool isStatement) { this->is_statement = isStatement; }
    public:
        VariableDeclaration();
        ~VariableDeclaration();
    };

    class EmptyExpression : public AstNode {
    public:
        virtual std::string toSource();
    public:
        EmptyExpression();
    };

    class UnaryExpression : public AstNode {
    private:
        AstNode* operand;
        bool is_postfix;
    public:
        virtual std::string toSource();
    public:
        inline bool isPostfix() { return is_postfix; }
        inline bool isPrefix() { return !is_postfix; }
        inline void setIsPostfix(bool isPostfix) { this->is_postfix = isPostfix; }
        inline AstNode& getOperand() { return *operand; }
        void setOperand(AstNode& operand);
    public:
        UnaryExpression(token::type operater, AstNode& operand);
        UnaryExpression(token::type operater, AstNode& operand, bool postFix);
        inline ~UnaryExpression() { delete operand, operand = nullptr; }
    };

    // extendable
    class InfixExpression : public AstNode {
    protected:
        AstNode* left;
        AstNode* right;
    public:
        virtual std::string toSource();
    public:
        void setLeftAndRight(AstNode& left, AstNode& right);
        inline token::type getOperator() { return getType(); }
        inline void setOperator(token::type operater) { setType(operater); }
        inline AstNode& getLeft() { return *left; }
        void setLeft(AstNode& left);
        inline AstNode& getRight() { return *right; }
        void setRight(AstNode& right);
    public:
        InfixExpression(AstNode& left, AstNode& right);
        InfixExpression(token::type operater, AstNode& left, AstNode& right);
        virtual ~InfixExpression();
    };

    class ConditionalExpression : public AstNode {
    private:
        AstNode* testExpression;
        AstNode* trueExpression;
        AstNode* falseExpression;
    public:
        virtual std::string toSource();
    public:
        inline AstNode& getTestExpression() { return *testExpression; }
        void setTestExpression(AstNode& testExpression);
        inline AstNode& getTrueExpression() { return *trueExpression; }
        void setTrueExpression(AstNode& trueExpression);
        inline AstNode& getFalseExpression() { return *falseExpression; }
        void setFalseExpression(AstNode& falseExpression);
    public:
        ConditionalExpression() : testExpression(nullptr), trueExpression(nullptr), falseExpression(nullptr) { type = token::HOOK; }
        ~ConditionalExpression();
    };

    class Assignment : public InfixExpression {
    public:
        Assignment(AstNode& left, AstNode& right);
        Assignment(token::type operater, AstNode& left, AstNode& right);
    };

    class ParenthesizedExpression : public AstNode {
    private:
        AstNode* expression;
    public:
        virtual std::string toSource();
    public:
        inline AstNode& getExpression() { return *expression; }
        void setExpression(AstNode& expression);
    public:
        ParenthesizedExpression();
        ParenthesizedExpression(AstNode& expr);
        ~ParenthesizedExpression() { delete expression, expression = nullptr; }
    };

    class FunctionCall : public AstNode {
    protected:
        AstNode* target;
        ant arguments;
    public:
        AstNode* returnValue = nullptr;
    public:
        virtual std::string toSource();
    public:
        inline AstNode& getTarget() { return *target; }
        void setTarget(AstNode& target);
        inline ant& getArguments() { return arguments; }
        void setArguments(ant arguments);
        void addArgument(AstNode& arg);
    public:
        FunctionCall();
        ~FunctionCall();
    };

    class ElementGet : public AstNode {
    private:
        AstNode* target;
        AstNode* element;
    public:
        virtual std::string toSource();
    public:
        inline AstNode& getTarget() { return *target; }
        void setTarget(AstNode& target);
        inline AstNode& getElement() { return *element; }
        void setElement(AstNode& element);
    public:
        ElementGet();
        ElementGet(AstNode& target, AstNode& element);
        ~ElementGet();
    };

    class FunctionNode : public ScriptNode {
    private:
        Name* functionName;
        anr params;
        AstNode* body;
    public:
        virtual std::string toSource();
    public:
        inline std::string& getName() { return functionName->getIdentifier(); }
        inline Name& getFunctionName() { return *functionName; }
        void setFunctionName(Name& name);
        inline anr& getParams() { return params; }
        void setParams(anr params);
        void addParam(AstNode& param);
        inline AstNode& getBody() { return *body; }
        void setBody(AstNode& body);
    public:
        FunctionNode();
        FunctionNode(Name& name);
        ~FunctionNode();
    };

    class EmptyStatement : public AstNode {
    public:
        virtual std::string toSource();
    public:
        EmptyStatement();
    };

    class ExpressionStatement : public AstNode {
    private:
        AstNode* expr;
    public:
        virtual std::string toSource();
    public:
        inline void setHasResult() { type = token::EXPR_RESULT; }
        inline AstNode& getExpression() { return *expr; }
        void setExpression(AstNode& expression);
    public:
        ExpressionStatement();
        ExpressionStatement(AstNode& expr);
        ExpressionStatement(AstNode& expr, bool hasResult);
        ~ExpressionStatement();
    };

    class ReturnStatement : public AstNode {
    private:
        AstNode* returnValue;
    public:
        std::string toSource();
    public:
        inline AstNode& getReturnValue() { return *returnValue; }
        void setReturnValue(AstNode& returnValue);
    public:
        ReturnStatement();
        ReturnStatement(AstNode& returnValue);
        ~ReturnStatement();
    };

    class IfStatement : public AstNode {
    private:
        AstNode* condition;
        AstNode* thenPart;
        AstNode* elsePart;
    public:
        virtual std::string toSource();
    public:
        inline bool hasElsePart() { return nullptr != elsePart; }
        inline AstNode& getCondition() { return *condition; }
        void setCondition(AstNode& condition);
        inline AstNode& getThenPart() { return *thenPart; }
        void setThenPart(AstNode& thenPart);
        inline AstNode& getElsePart() { return *elsePart; }
        void setElsePart(AstNode& elsePart);
    public:
        IfStatement();
        ~IfStatement();
    };

    class Loop : public Scope {
    protected:
        AstNode* body;
    public:
        virtual std::string toSource() = 0;
    public:
        inline AstNode& getBody() { return *body; }
        void setBody(AstNode& body);
    public:
        Loop();
        ~Loop();
    };

    class WhileLoop : public Loop {
    private:
        AstNode* condition;
    public:
        virtual std::string toSource();
    public:
        inline AstNode& getCondition() { return *condition; }
        void setCondition(AstNode& condition);
    public:
        WhileLoop();
        ~WhileLoop();
    };

    class ForLoop : public Loop {
    private:
        AstNode* initializer;
        AstNode* condition;
        AstNode* increment;
    public:
        virtual std::string toSource();
    public:
        inline AstNode& getInitializer() { return *initializer; }
        void setInitializer(AstNode& initializer);
        inline AstNode& getCondition() { return *condition; }
        void setCondition(AstNode& condition);
        inline AstNode& getIncrement() { return *increment; }
        void setIncrement(AstNode& increment);
    public:
        ForLoop();
        ~ForLoop();
    };
}

