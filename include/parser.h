
#include <vector>

#include "logger.h"

#include "commons.h"
using namespace commons;

#include "ast.h"
using namespace ast;

class parser
{
private:
    int position;
    std::vector<token::instance>& collects;
    // std::stack<ast::AstNode> nodes;
    ScriptNode* currentScriptOrFn = nullptr;
    Scope* currentScope = nullptr;
public:
    inline void passed() {
        if (position < 0 && position + 1 >= (int) collects.size())
            return;
        ++position;
    }
private:
    bool matchToken(token::type toMatch);
    Name* createNameNode();
    StringLiteral* createStringLiteral();
    AstNode* arrayLiteral();
    ant argumentList();
    AstNode* memberExprTail(bool allowCallSyntax, AstNode* pn);
    AstNode* memberExpr(bool allowCallSyntax);
    AstNode* name();
    AstNode* parenExpr();
    AstNode* primaryExpr();
    AstNode* unaryExpr();
    AstNode* mulExpr();
    AstNode* addExpr();
    AstNode* shiftExpr();
    AstNode* relExpr();
    AstNode* eqExpr();
    AstNode* bitAndExpr();
    AstNode* bitXorExpr();
    AstNode* bitOrExpr();
    AstNode* andExpr();
    AstNode* orExpr();
    AstNode* condExpr();
    AstNode* assignExpr();
    AstNode* expr();
    VariableDeclaration* variables(bool isStatement);
    AstNode* nameVisa();
    AstNode* statements(AstNode* parent);
    AstNode* returnStatement();
    AstNode* block();
    AstNode* statement();
    AstNode* statementHelper();
    IfStatement* ifStatement();
    void parseFunctionParams(FunctionNode* fnNode);
    AstNode* parseFunctionBody();
    FunctionNode* function();
    WhileLoop* whileLoop();
    AstNode* forLoopInit(token::type kind);
    Loop* forLoop();
public:
    AstRoot* parse();
public:
    parser(std::vector<token::instance>& reference);
};


