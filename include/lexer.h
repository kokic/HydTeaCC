
namespace token
{
    enum type_t
    {
        ERROR = -1,

        // , . : ; 
        COMMA, DOT, COLON, SEMI,

        LP, RP, // left and right parentheses ()
        LB, RB, // left and right brackets []
        LC, RC, // left and right curlies (braces) {}

        // increment/decrement (++ --)
        INC, DEC,
        POS, NEG, // + -

        // INFIX SIMPLE:
        ADD, SUB, MUL, DIV, MOD, POW,

        // BIT
        BITAND,  // &
        BITOR,  // |
        BITXOR,  // ^
        BITNOT, // ~
        LSH,  // <<
        RSH,  // >>

        // LOGIC
        AND, // &&
        OR, // ||
        NOT, // !

        // ASSIGN
        ASSIGN,  // =
        ASSIGN_BITAND,  // &=
        ASSIGN_BITOR,  // |=
        ASSIGN_BITXOR,  // ^=
        ASSIGN_LSH,  // <<=
        ASSIGN_RSH,  // >>=
        ASSIGN_ADD,  // +=
        ASSIGN_SUB,  // -=
        ASSIGN_MUL,  // *=
        ASSIGN_DIV,  // /=
        ASSIGN_MOD,  // %=
        ASSIGN_POW,  // **=

        // RELATION
        EQ, // ==
        NE, // !=
        LT, // > 
        LE, // >=
        GT, // <
        GE, // <=

        HOOK, // ?

        // COMMENT
        COMMENT_LINE,
        COMMENT_BLOCK,

        /* literal */
        UNDEFINED, // undefined
        NULL_T, // null
        TRUE, // true
        FALSE, // false

        NUMBER, // type ( double )
        STRING,

        /* letter */
        IDENTIFIER, // name

        ARRAYLIT,
        OBJECTLIT,

        // KEYWORD
        VAR,
        FUNC,
        RETURN,
        IF,
        ELSE,
        WHILE,
        FOR,
        BREAK,
        CONTINUE,
        VOID,

        // ACTION
        CALL,
        GETELEM,

        EMPTY,
        GET_REF, SET_REF, DEL_REF,

        RETURN_RESULT,
        EXPR_VOID, EXPR_RESULT,

        BLOCK, SCOPE, SCRIPT,

        // FLAGS
        BINDNAME, SETNAME,


        // SAVED
        LET,
        CONST,

        // ::OOP
        THIS,
        NEW,
        PRIVATE,
        PROTECTED,
        PUBLIC,
        STATIC,

        RESERVED
    };
    using type = type_t;

    static const char* kind(type type)
    {
        switch (type)
        {
        case token::LP: return "LP";
        case RP: return "RP";
        case LB: return "LB";
        case RB: return "RB";
        case LC: return "LC";
        case RC: return "RC";
        case COMMA: return "COMMA";
        case COLON: return "COLON";
        case SEMI: return "SEMI";

        case POS: return "POS";
        case NEG: return "NEG";
        case INC: return "INC";
        case DEC: return "DEC";
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case MOD: return "MOD";
        case POW: return "POW";
        case BITAND: return "BITAND";
        case BITOR: return "BITOR";
        case BITXOR: return "BITXOR";
        case LSH: return "LSH";
        case RSH: return "RSH";
        case AND: return "AND";
        case OR: return "OR";
        case NOT: return "NOT";

        case ASSIGN: return "ASSIGN";
        case ASSIGN_BITOR: return "ASSIGN_BITOR";
        case ASSIGN_BITXOR: return "ASSIGN_BITXOR";
        case ASSIGN_BITAND: return "ASSIGN_BITAND";
        case ASSIGN_LSH: return "ASSIGN_LSH";
        case ASSIGN_RSH: return "ASSIGN_RSH";
        case ASSIGN_ADD: return "ASSIGN_ADD";
        case ASSIGN_SUB: return "ASSIGN_SUB";
        case ASSIGN_MUL: return "ASSIGN_MUL";
        case ASSIGN_DIV: return "ASSIGN_DIV";
        case ASSIGN_MOD: return "ASSIGN_MOD";
        case ASSIGN_POW: return "ASSIGN_POW";

        case EQ: return "EQ";
        case NE: return "NE";
        case LT: return "LT";
        case LE: return "LE";
        case GT: return "GT";
        case GE: return "GE";

        case COMMENT_LINE: return "COMMENT_LINE";
        case COMMENT_BLOCK: return "COMMENT_BLOCK";

        case UNDEFINED: return "UNDEFINED";
        case NULL_T: return "NULL";
        case TRUE: return "TRUE";
        case FALSE: return "FALSE";

        case NUMBER: return "NUMBER";
        case STRING: return "STRING";
        case IDENTIFIER: return "IDENTIFIER";

        case ARRAYLIT: return "ARRAYLIT";
        case OBJECTLIT: return "OBJECTLIT";

        case VAR: return "VAR";
        case FUNC: return "FUNC";
        case RETURN: return "RETURN";
        case IF: return "IF";
        case ELSE: return "ELSE";
        case WHILE: return "WHILE";
        case FOR: return "FOR";
        case BREAK: return "BREAK";
        case CONTINUE: return "CONTINUE";
        case VOID: return "VOID";

        case CALL: return "CALL";
        case NEW: return "NEW";
        case GETELEM: return "GETELEM";

        case RETURN_RESULT: return "RETURN_RESULT";
        case EXPR_VOID: return "EXPR_VOID";
        case EXPR_RESULT: return "EXPR_RESULT";
        case EMPTY: return "EMPTY";

        case GET_REF: return "GET_REF";
        case SET_REF: return "SET_REF";
        case DEL_REF: return "DEL_REF";

        case BLOCK: return "BLOCK";
        case SCOPE: return "SCOPE";
        case SCRIPT: return "SCRIPT";

        default: return "BAD::KIND";
        }
    }
    
    struct instance
    {
        type_t type;
        int lineno; // 1 ~ n
        int offset; // 0 ~ n
        int position; // 0 ~ n, offset of first character
        std::string source;
    };
}


class lexer
{
private:
    int lineno;
    int offset;
    int position;
    char current;
    bool demeaning;
    std::string collector;
    int length;
    std::vector<token::instance> buffer;
private:
    inline token::instance creator(token::type type);
    inline void destroy();
public:
    std::vector<token::instance>& tokens();
public:
    lexer(std::string& source);
};


