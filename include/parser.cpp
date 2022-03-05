
#include "parser.h"

parser::parser(std::vector<token::instance>& reference) : collects(reference), position(0) {}

#define LINE(ins) (" (#" + cast_string(ins.lineno) + ")")

#define cell collects[position]
#define peek collects[position + 1]
#define roll collects[position++]
#define next collects[++position]
#define jump(offset) collects[position + offset]

bool parser::matchToken(token::type toMatch) {
    if (peek.type != toMatch)
        return false;
    passed();
    return true;
}

Name* parser::createNameNode() {
    token::instance& ins = cell;
    if (ins.type != token::IDENTIFIER) {
        out("*** name '" + ins.source + "' is invalid in 'createNameNode'" + LINE(cell));
        return nullptr;
    }
    Name* name = new Name(ins.source);
    return name;
}

StringLiteral* parser::createStringLiteral() {
    StringLiteral* strl = new StringLiteral();
    strl->setValue((std::string)cell.source);
    return strl;
}

AstNode* parser::arrayLiteral() {
    ArrayLiteral* pn = new ArrayLiteral();
    for (;;) {
        token::type kind = peek.type;
        if (kind == token::COMMA) {
            passed();
        }
        else if (kind == token::RB) {
            passed();
            break;
        }
        else
            pn->addElement(*assignExpr());
    }
    return pn;
}

ant parser::argumentList() {
    ant result;
    if (matchToken(token::RP))
        return result;
    AstNode* en = nullptr;
    do {
        en = assignExpr();
        result.push_back(en);
    } while (matchToken(token::COMMA));

    if (!matchToken(token::RP))
        out("**** syntax: cannot match the ')' after '(' in function-args" + LINE(cell));

    return result;
}

AstNode* parser::memberExprTail(bool allowCallSyntax, AstNode* pn) {
    // if (pn == nullptr) codeBug();
    for (;;) {
        token::type kind = peek.type;
        if (kind == token::LB) {
            passed(); // jump '['
            AstNode* ele = expr(); // index
            passed(); // jump ']'
            ElementGet* g = new ElementGet();
            g->setTarget(*pn);
            g->setElement(*ele);
            pn = g;
            continue;
        }
        else if (kind == token::LP) {
            if (!allowCallSyntax) break;
            // lineno = ts.lineno;
            passed();
            // checkCallRequiresActivation(pn);
            FunctionCall* f = new FunctionCall();
            f->setTarget(*pn);
            std::list<AstNode*> args = argumentList();
            f->setArguments(args);
            pn = f;

            // bas(pn->toSource());
            // out(pn);
            continue;
        }
        else
            break;
    }
    return pn;
}

AstNode* parser::memberExpr(bool allowCallSyntax) {
    token::type kind = peek.type;
    AstNode* pn = nullptr;

    pn = primaryExpr();
    // (may) parse a new-expression here

    AstNode* tail = memberExprTail(allowCallSyntax, pn);
    return tail;
}

AstNode* parser::name() { return createNameNode(); }

AstNode* parser::parenExpr() {
    AstNode* express = expr();
    ParenthesizedExpression* pn = new ParenthesizedExpression(*express);
    if (!matchToken(token::RP)) {
        out("**** syntax: cannot match the ')' after '('" + LINE(cell));
        return nullptr;
    }
    return pn;
}

AstNode* parser::primaryExpr() {
    // out("primaryExpr: " + peek.source);
    token::type kind = peek.type;
    switch (kind) {

    case token::LB:
        passed();
        return arrayLiteral();
        /*
        case Token.LC:
            return objectLiteral();
        */
    case token::LP:
        passed(); // cell: (
        return parenExpr();

    case token::IDENTIFIER:
        passed(); // cell: IDENTIFTER
        return name();

    case token::NUMBER:
    {
        passed(); // cell: NUMBER
        std::string& str = cell.source;
        return new NumberLiteral(str, string_cast<double>(str));
    }

    case token::STRING:
        passed();
        return createStringLiteral();


    case token::NULL_T:
    case token::UNDEFINED:
        // case token::THIS:
    case token::FALSE:
    case token::TRUE:
        passed(); // cell: literal
        return new KeywordLiteral(kind);


    default:
        out("*** parse error in 'primaryExpr', err: " + peek.source + LINE(peek));
        break;
    }
    return nullptr; // never
}

AstNode* parser::unaryExpr() {

    AstNode* node = nullptr;
    token::type kind = peek.type;
    switch (kind) {
    case token::NOT:
    case token::BITNOT:
        passed();
        node = new UnaryExpression(kind, *unaryExpr());
        return node;

    case token::ADD:
        passed();
        node = new UnaryExpression(token::POS, *unaryExpr());
        return node;

    case token::SUB:
        passed();
        node = new UnaryExpression(token::NEG, *unaryExpr());
        return node;

    case token::INC:
    case token::DEC:
    {
        passed();
        UnaryExpression* expr = new UnaryExpression(kind, *primaryExpr());
        // out(expr->toSource());
        return expr;
    }

    default:
        AstNode* pn = memberExpr(true);
        kind = peek.type;
        if (!(kind == token::INC || kind == token::DEC)) {
            return pn; // value
        }
        passed(); // jump the inc / dec
        // postfix: a++ or a--
        UnaryExpression* uexpr = new UnaryExpression(kind, *pn, true);
        return uexpr;
    }
}

AstNode* parser::mulExpr() {
    AstNode* pn = unaryExpr();
    for (;;) {
        token::type kind = peek.type;
        switch (kind) {
        case token::MUL:
        case token::DIV:
        case token::MOD:
        case token::POW:
            passed();
            pn = new InfixExpression(kind, *pn, *unaryExpr());
            continue;
        }
        break;
    }
    return pn;
}

AstNode* parser::addExpr() {
    AstNode* pn = mulExpr();
    for (;;) {
        token::type kind = peek.type;
        if (kind == token::ADD || kind == token::SUB) {
            passed(); // cell: ADD/SUB
            pn = new InfixExpression(kind, *pn, *mulExpr());
            continue;
        }
        break;
    }
    return pn;
}

AstNode* parser::shiftExpr() {
    AstNode* pn = addExpr();
    for (;;) {
        token::type kind = peek.type;
        switch (kind) {
        case token::LSH:
        case token::RSH:
            passed();
            pn = new InfixExpression(kind, *pn, *addExpr());
            continue;
        }
        break;
    }
    return pn;
}

AstNode* parser::relExpr() {
    AstNode* pn = shiftExpr();
    for (;;) {
        token::type kind = peek.type;
        switch (kind) {
        case token::LE:
        case token::LT:
        case token::GE:
        case token::GT:
            passed();
            pn = new InfixExpression(kind, *pn, *shiftExpr());
            continue;
        }
        break;
    }
    return pn;
}

AstNode* parser::eqExpr() {
    AstNode* pn = relExpr();
    for (;;) {
        token::type kind = peek.type;
        switch (kind) {
        case token::EQ:
        case token::NE:
            passed();
            pn = new InfixExpression(kind, *pn, *relExpr());
            continue;
        }
        break;
    }
    return pn;
}

AstNode* parser::bitAndExpr() {
    AstNode* pn = eqExpr();
    while (matchToken(token::BITAND)) {
        pn = new InfixExpression(token::BITAND, *pn, *eqExpr());
    }
    return pn;
}

AstNode* parser::bitXorExpr() {
    AstNode* pn = bitAndExpr();
    while (matchToken(token::BITXOR)) {
        pn = new InfixExpression(token::BITXOR, *pn, *bitAndExpr());
    }
    return pn;
}

AstNode* parser::bitOrExpr() {
    AstNode* pn = bitXorExpr();
    while (matchToken(token::BITOR)) {
        pn = new InfixExpression(token::BITOR, *pn, *bitXorExpr());
    }
    return pn;
}

AstNode* parser::andExpr() {
    AstNode* pn = bitOrExpr();
    if (matchToken(token::AND)) {
        pn = new InfixExpression(token::AND, *pn, *andExpr());
    }
    return pn;
}

AstNode* parser::orExpr() {
    AstNode* pn = andExpr();
    if (matchToken(token::OR)) {
        pn = new InfixExpression(token::OR, *pn, *orExpr());
    }
    return pn;
}

AstNode* parser::condExpr() {
    AstNode* pn = orExpr();
    if (matchToken(token::HOOK)) {
        AstNode* ifTrue = assignExpr();
        if (!matchToken(token::COLON)) {
            out("**** syntax: cannot match the colon after hook" + LINE(cell));
            return nullptr;
        }
        AstNode* ifFalse = assignExpr();
        ConditionalExpression* ce = new ConditionalExpression();
        ce->setTestExpression(*pn);
        ce->setTrueExpression(*ifTrue);
        ce->setFalseExpression(*ifFalse);
        pn = ce;
    }
    return pn;
}

AstNode* parser::assignExpr() {
    AstNode* pn = condExpr();
    token::type kind = peek.type;
    // out(token::kind(peek.type));

    // if (Token.FIRST_ASSIGN <= tt && tt <= Token.LAST_ASSIGN) {
    if (token::ASSIGN <= kind && kind <= token::ASSIGN_POW) {
        passed();
        pn = new Assignment(kind, *pn, *assignExpr());
    }

    return pn;
}

AstNode* parser::expr() {
    AstNode* pn = assignExpr();
    while (matchToken(token::COMMA)) {
        pn = new InfixExpression(token::COMMA, *pn, *assignExpr());
    }
    return pn;
}

VariableDeclaration* parser::variables(bool isStatement) {
    VariableDeclaration* pn = new VariableDeclaration();
    for (;;) {
        Name* name = nullptr;
        name = createNameNode();

        AstNode* init = nullptr;
        if (!matchToken(token::ASSIGN))
            out("*** cannot match the assign after identifier: " + name->getIdentifier() + LINE(cell));

        // out("name is: " + name->getIdentifier());
        init = assignExpr();

        VariableInitializer* vi = new VariableInitializer();
        vi->setTarget(*name);
        vi->setInitializer(*init);
        pn->addVariable(*vi);
        if (matchToken(token::COMMA)) ++position; // skip the comma
        else break;
    }

    /*
    ++position; // cell: should be ';'
    if (token::SEMI != cell.type)
        out("*** cannot match the ';' after variable declared");
    */
    if (isStatement && peek.type == token::SEMI)
        passed();
    pn->setIsStatement(isStatement);

    // bas(pn->toSource());
    // out(pn);

    return pn;
}

// *assign
AstNode* parser::nameVisa() {
    // out("ado: " + cast_string(position));
    AstNode* exp = expr();
    // out("passing");
    AstNode* node = new ExpressionStatement(*exp, false /* !insideFunction() */);
    // node->setLineno(exp->getLineno());

    // [ExpressionStatement] must be ended by a [SEMI]

    /*
    ++position; // cell: should be ';'
    // if (token::SEMI != cell.type)
        // out("cannot match the ';' after assignment");

    // bas(node->toSource());
    // out(node);
    */

    return node;
}



AstNode* parser::statements(AstNode* parent) {
    AstNode* block = parent != nullptr ? parent : new Block();
    token::type kind;
    while ((kind = cell.type) != token::RC) {
        block->addChild(*statement());
        passed();
    }
    // out(cell.source);
    return block;
}

AstNode* parser::returnStatement() {
    AstNode* e = nullptr;
    switch (peek.type) {
    case token::SEMI:
    case token::RP:
    case token::RB:
    case token::RC:
        break;
    default:
        e = expr();
    }
    AstNode* ret = nullptr;
    if (e != nullptr)
        ret = new ReturnStatement(*e);
    // out(ret->toSource());
    if (peek.type != token::SEMI) {
        out("<return-statement> cannot match ';' at the end of the statement");
    }
    else passed();

    return ret;
}


AstNode* parser::block() {
    passed();
    Scope* block = new Scope();
    statements(block);
    // out("block: " + block->toSource());
    /*
    pushScope(block);
    try {
        statements(block);
        mustMatchToken(Token.RC, "msg.no.brace.block");
        block.setLength(ts.tokenEnd - pos);
        return block;
    } finally {
        popScope();
    }
    */
    return block;
}


AstNode* parser::statement() {
    AstNode* pn = statementHelper();
    return pn;
}

AstNode* parser::statementHelper() {

    AstNode* pn = nullptr;
    token::type kind = cell.type;

    switch (kind) {

    case token::LC:
        return block();

    case token::IF:
        return ifStatement();

    case token::WHILE:
        return whileLoop();

    case token::FOR:
        return forLoop();

        /*

        case Token.SWITCH:
            return switchStatement();

        case Token.DO:
            return doLoop();

        case Token.TRY:
            return tryStatement();

        case Token.THROW:
            pn = throwStatement();
            break;

        case Token.BREAK:
            pn = breakStatement();
            break;

        case Token.CONTINUE:
            pn = continueStatement();
            break;

        case Token.WITH:
            return withStatement();
            case Token.LET:
            pn = letStatement();
            if (pn instanceof VariableDeclaration
                && peekToken() == Token.SEMI)
                break;
            return pn;
            */


            // case token::CONST:
    case token::VAR:
        passed(); // cell: identifiter
        pn = variables(true);

        /*
        out(">>> the counter is: ");
        out(Node::counted());
        delete pn, pn = nullptr;
        out(">>> after delete, the counter is: ");
        out(Node::counted());
        */

        break;

    case token::RETURN:
        pn = returnStatement();
        break;


    case token::SEMI:
        pn = new EmptyStatement();
        return pn;

    case token::IDENTIFIER:
        --position; // prepare infix (peek: left)
        // out("visa: " + peek.source);
        pn = nameVisa();

        break;

    case token::FUNC:
        return function();

    default:
        --position; // prepare prefix
        // out(peek.source);
        pn = new ExpressionStatement(*expr(), false/* !insideFunction() */);
        // out(pn->toSource());
        break;
    }

    return pn;
}




IfStatement* parser::ifStatement() {
    if (!matchToken(token::LP))
        out("*** missing '(' before if-condition" + LINE(cell));
    AstNode* condition = expr();
    if (!matchToken(token::RP))
        out("*** missing ')' after if-condition" + LINE(cell));
    passed(); // jump ')'
    IfStatement* pn = new IfStatement();
    AstNode* ifTrue = statement();
    pn->setCondition(*condition);
    pn->setThenPart(*ifTrue);
    AstNode* ifFalse = nullptr;
    if (matchToken(token::ELSE)) {
        passed();
        ifFalse = statement();
        pn->setElsePart(*ifFalse);
    }
    // out(pn->toSource());
    return pn;
}





void parser::parseFunctionParams(FunctionNode* fnNode) {
    if (!matchToken(token::LP))
        out("*** syntax: cannot match the '(' before function-params" + LINE(cell));
    if (matchToken(token::RP)) return;
    do {
        if (matchToken(token::IDENTIFIER))
            fnNode->addParam(*createNameNode());
    } while (matchToken(token::COMMA));
    if (!matchToken(token::RP))
        out("*** syntax: cannot match the ')' after function-params" + LINE(cell));
}

AstNode* parser::parseFunctionBody() {
    if (!matchToken(token::LC))
        out("*** syntax: cannot match the '{' before function-body" + LINE(cell));

    Block* pn = new Block();
    for (;;) {
        AstNode* n = nullptr;
        token::type kind = peek.type;
        if (kind == token::RC) break;
        passed();
        n = statement();
        pn->addStatement(*n);
    }
    if (!matchToken(token::RC))
        out("*** syntax: cannot match the '}' before function-body" + LINE(cell));

    // out("<parse-function-body> " + pn->toSource());
    return pn;
}

FunctionNode* parser::function() {
    Name* name = nullptr;
    if (!matchToken(token::IDENTIFIER))
        out("*** name '" + peek.source + "' is invalid in 'function'" + LINE(cell));
    name = createNameNode();

    FunctionNode* fnNode = new FunctionNode(*name);
    // fnNode->setFunctionType(type);
    parseFunctionParams(fnNode);
    fnNode->setBody(*parseFunctionBody());
    // out("<function> " + fnNode->toSource());
    return fnNode;
}





WhileLoop* parser::whileLoop() {
    if (!matchToken(token::LP))
        out("*** missing '(' before while-condition" + LINE(cell));
    AstNode* condition = expr();
    if (!matchToken(token::RP))
        out("*** missing ')' after while-condition" + LINE(cell));

    WhileLoop* pn = new WhileLoop();
    passed(); // jump ')'
    AstNode* body = statement();
    pn->setCondition(*condition);
    pn->setBody(*body);

    return pn;
}



AstNode* parser::forLoopInit(token::type kind) {
    AstNode* init = nullptr;
    if (kind == token::SEMI) {
        init = new EmptyExpression();
    }
    else if (kind == token::VAR) {
        position += 2; // cell: identifiter
        init = variables(false);
    }
    else {
        init = expr();
    }
    return init;
}

Loop* parser::forLoop() {
    Loop* pn = nullptr;
    if (!matchToken(token::LP))
        out("*** missing '(' before for-init" + LINE(cell));
    AstNode* init = forLoopInit(peek.type);
    if (!matchToken(token::SEMI))
        out("*** missing ';' after for-initializer" + LINE(cell));
    AstNode* condition = peek.type == token::SEMI ? new EmptyExpression() : expr();
    if (!matchToken(token::SEMI))
        out("*** missing ';' after for-condition" + LINE(cell));
    AstNode* incr = peek.type == token::SEMI ? new EmptyExpression() : expr();
    if (!matchToken(token::RP))
        out("*** missing ')' after for-increment" + LINE(cell));
    ForLoop* fl = new ForLoop();
    fl->setInitializer(*init);
    fl->setCondition(*condition);
    fl->setIncrement(*incr);
    pn = fl;
    passed(); // jump ')'
    AstNode* body = statement();
    pn->setBody(*body);

    return pn;
}




AstRoot* parser::parse() {

    AstRoot* root = new AstRoot();
    currentScope = currentScriptOrFn = root;
    token::type kind;
    AstNode* node = nullptr;
    position = 0;
    const int length = collects.size();
    for (; position != length; ++position) {
        // out(cell.source);
        node = statement();
        // out(cast_string(Node::counted()));
        // out("<parse> " + node->toSource() + cast_string(node));

        // bas(token::kind(node->getType()));
        // bas(" ");
        // out(node);
        // out("");

        root->addChildToBack(*node);
        node->setParent(*root);
    }
    return root;
}


#undef cell
#undef peek
#undef roll
#undef next
#undef jump







