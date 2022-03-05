
/**
  * Please note. This is an incomplete implementation, it is specifically designed to implement 
  * simple programs and comes with a basic math library. In particular, the flaw of this inter-
  * preter that does not contain a stack is that it does not support recursion at all (because
  * there is no structure like a function stack, exactly, inline), its `func` keyword is imple-
  * mented as a quasi variable (ie, one scope with the label `func`), and some other issues.
**/

namespace hykilpikonna
{
	static AstNode* exec_node(AstNode* node);
	static AstNode* exec_node_hybrid(AstNode* node);

	static AstNode* void_literal = nullptr;
	static AstNode* undefined_literal = nullptr;

	static std::stack<FunctionCall*> calleds;
    
	static AstNode* copy_literal(AstNode* value) {
		auto kind = value->getType();
		AstNode* result_node = nullptr;
		switch (kind) {
		case token::NUMBER:
			result_node = new NumberLiteral(((NumberLiteral*)value)->getNumber());
			break;
		case token::STRING:
			result_node = new StringLiteral((std::string)((StringLiteral*)value)->getValue());
			break;
		case token::ARRAYLIT:
		{
			result_node = new ArrayLiteral();
			auto& arr = ((ArrayLiteral*)value)->getElements();
			for (int index = 0; index != arr.size(); ++index) {
				AstNode* element = arr[index];
				((ArrayLiteral*)result_node)->addElement(*copy_literal(element));
			}
			break;
		}
		default:
			out(std::string("[error]: cannot copy the [") + token::kind(kind) + "] value '" + value->toSource() + "'");
			result_node = value;
			break;
		}
		return result_node;
	}

	template<typename T>
	static T to_number(AstNode* node) {
		auto kind = node->getType();
		switch (kind) {
		case token::STRING: 
		    return string_cast<T>(((StringLiteral*)node)->getValue());
		case token::NUMBER: 
		    return (T)((NumberLiteral*)node)->getNumber();
		default:
			out("[error]: cannot cast the '" + node->toSource() + "' to a number");
			return 0;
		}
	}

	static std::string to_string(AstNode* node) {
		auto kind = node->getType();
		switch (kind)
		{
		case token::STRING: return ((StringLiteral*)node)->getValue();
		default:
			return node->toSource();
		}
	}

	static AstNode* pop_arg(ant& args) {
		auto node = exec_node(args.front());
		args.pop_front();
		return node;
	}

	static ScriptNode* get_script_node(AstNode* node) {
		auto type = node->getType();
		if (type == token::SCRIPT)
			return (ScriptNode*)node;
		AstNode* pn = &(node->getParent());
		token::type kind;
		while ((kind = pn->getType()) != token::FUNC && kind != token::SCRIPT) {
			pn = &(pn->getParent());
		}
		// out(pn->toSource());
		return (ScriptNode*)pn;
	}

	static AstNode* identifter_find(ScriptNode* script, std::string& name) {
		auto& variables = script->getVariables();
		if (variables.find(name) == variables.end()) {
			/*
			if (script->getType() == token::FUNC) {
				script = get_script_node(&(script->getParent()));
				return identifter_find(script, name);
			}
			out("*** id-find: '" + name + "' was not declared in this scope");
			*/
			if (script->hasParent()) {
				script = get_script_node(&(script->getParent()));
			    return identifter_find(script, name);
		    }
			out("*** id-find: '" + name + "' was not declared in this scope");
			return undefined_literal;
		} 
		return variables[name];
	}
    
	static AstNode* func_call(ScriptNode* script, std::string& name, ant& args, FunctionCall* func) {
		auto& functions = script->getFunctions();
		if (functions.find(name) == functions.end()) {
            //  is a quasi func (undefined in system)
			auto& variables = script->getVariables();
			if (variables.find(name) == variables.end()) {
				if (script->getType() == token::FUNC) {
					script = get_script_node(&(script->getParent()));
					return func_call(script, name, args, func);
				}
				out("call: '" + name + "' was not declared in this scope");
				return undefined_literal;
			}
			auto fn = (FunctionNode*)variables[name];
			auto& fv = fn->getVariables();
			anr& params = fn->getParams();

			std::list<AstNode*> table = args; // copy args
			for (auto iter = table.begin(); iter != table.end(); ++iter) {
				*iter = pop_arg(args);
				// out("arg: " + (*iter)->toSource());
			}

			fv.clear();
			for (AstNode* p : params) {
				std::string& id = ((Name*)p)->getIdentifier();
				AstNode* value = table.front();// exec_node(args.front());
				table.pop_front();
				fv[id] = value;
			}

			// out(fn->getBody().toSource());
			auto body = &(fn->getBody());

			// auto& variables = ((FunctionNode*)script)->getVariables();
			// for (auto iter = body->begin(); iter != body->end(); ++iter) {
			    // out("source: " + ((AstNode*)*iter) -> toSource());
			    // exec_node((AstNode*)*iter);
			// }
			// out(fn->getBody().toSource());


			auto proc = exec_node(body);
			// return proc;
			// return &undefined_literal;
			if (func->returnValue == nullptr)
			    return undefined_literal;

			out("|pass: " + func->toSource() + " -> " + func->returnValue->toSource());
			out(variables["val"]);
			return func->returnValue;
		}
        // out(".func_call " + name + "'s size: " + cast_string(args.size()));
        
        auto result = functions[name](args);
        
        return result;
	}

	static AstNode* func_call_proc(FunctionCall* func, ScriptNode* script) {
		// out("<CALL> " + func->toSource());
		AstNode& target = func->getTarget();
		std::string& name = ((Name*)& target)->getIdentifier();
		// args pass by value, don't use 'ant&'
		ant args = func->getArguments();
        // out(".func_call_proc " + name + "'s size: " + cast_string(args.size()));
		auto result = func_call(script, name, args, func);
		// 
		// out(name + "'s result: " + result->toSource());
		return result;
	}






    static AstNode* identifier_assign(ScriptNode* script, std::string& name, AstNode* result_node) {
    	auto& variables = script->getVariables();
    	if (variables.find(name) == variables.end()) {
    		if (script->hasParent()) {
    			script = get_script_node(&(script->getParent()));
    			return identifier_assign(script, name, result_node);
    		}
    		out("*** id-assign: '" + name + "' was not declared in this scope");
    		return undefined_literal;
    	}
    	return variables[name] = result_node;
    }

	static AstNode* assign_hybrid(AstNode* node, AstNode* target, AstNode* result_node /* exec_ed */) {
		AstNode* orgin = nullptr;
		auto kind = target->getType();
		switch (kind)
		{
		case token::IDENTIFIER:
		{
			auto script = get_script_node(node);
			auto& variables = script->getVariables();
			std::string& name = ((Name*)target)->getIdentifier();
			if (name == "pi" || name == "e") {
                out("constant '" + name + "' cannot be re-assigned a value.");
                return variables[name];
            }
            return identifier_assign(script, name, result_node);
		}
		case token::GETELEM:
		{
			auto g = (ElementGet*)target;
			auto arrl = (ArrayLiteral*)exec_node(&(g->getTarget()));
			anr& arr = arrl->getElements();
			auto numl = (NumberLiteral*)exec_node(&(g->getElement()));
			int index = (int)numl->getNumber();

			if (index >= arr.size()) {
				out("index out of array bounds in '" + g->toSource() + "'");
				return result_node;
			}

			AstNode* element = arr[index];
			result_node = copy_literal(result_node);
			node->addChildToBack(*element);
			return arr[index] = result_node;
		}

		case token::ARRAYLIT:
		{
			if (result_node->getType() != token::ARRAYLIT) {
				out("<assign> destructing exception - type incorrect");
				return result_node;
			}
			auto script = get_script_node(node);
			auto& variables = script->getVariables();
			anr& ord = ((ArrayLiteral*)result_node)->getElements();
			anr& arr = ((ArrayLiteral*)target)->getElements();
			for (int index = 0; index != arr.size(); ++index) {
				Name* identifier = (Name*)arr[index];
				variables[identifier->getIdentifier()] = ord[index];
			}
			return result_node;
		}
		default:
			return result_node;
			break;
		}
	}



	static AstNode* exec_node_hybrid(AstNode* node) {
		auto script = get_script_node(node);
		token::type type = node->getType();
		switch (type) {
		case token::VAR:
		{
			auto& variables = script->getVariables();
			auto pn = (VariableDeclaration*)node;
			for (auto vi : pn->getVariables()) {
				AstNode& target = vi->getTarget();
				std::string& name = ((Name*)& target)->getIdentifier();
				AstNode* value = exec_node(&(vi->getInitializer()));
				// node->addChildToBack(*variables[name]);
				variables[name] = value;
				// out("var " + name + " = " + value->toSource());
			}
			return void_literal;
		}
		case token::ASSIGN:
		{
			auto infix = (Assignment*)node;
			AstNode* value = exec_node(&(infix->getRight()));
			return assign_hybrid(node, &(infix->getLeft()), value);
		}
		case token::IDENTIFIER:
		{
			std::string& name = ((Name*)node)->getIdentifier();
			return identifter_find(script, name);
		}
		case token::RETURN:
		{
			auto rets = (ReturnStatement*)node;
			if (script->getType() != token::FUNC)
				out("*** error: return-statement must be used in function-inside");
			auto fn = (FunctionNode*)script;
			auto& variables = fn->getVariables();
			auto value = exec_node(&(rets->getReturnValue()));

			// out(rets->toSource());
			
			FunctionCall* called = calleds.top();
			// calleds.pop();
			called->returnValue = value;
			out("|ret: record " + called->toSource() + " = " + value->toSource());
			// variables["return-value"] = value;
			// return void_literal;
			return value;
		}
		case token::BLOCK:
		case token::SCRIPT:
		{
			AstNode* result = void_literal;
			if (script->getType() == token::FUNC) {
				// in function-scope, body exec
				auto& variables = ((FunctionNode*)script)->getVariables();
				for (auto iter = node->begin(); iter != node->end(); ++iter) {
					exec_node((AstNode*)* iter);
					FunctionCall* called = calleds.top();
					// out("scope called: " + called->toSource());
					if (called->returnValue != nullptr) {
						result = called->returnValue;
						break;
					}
					// out(">>> " + result->toSource());
				}
			}
			else {
				for (auto iter = node->begin(); iter != node->end(); ++iter) {
					exec_node((AstNode*)* iter);
				}
			}
			// out("[scope] result: " + result->toSource());
			return result; // &void_literal;
		}
		case token::CALL: 
		{
		    auto called = (FunctionCall*)node;
		    calleds.push(called);
		    out("|= size: " + cast_string(calleds.size()));
		    out("|top: " + calleds.top() -> toSource());
		    return func_call_proc(called, script);
		}
		case token::FUNC:
		{
			auto fn = (FunctionNode*)node;
			std::string name = fn->getName();
			script->getVariables()[name] = fn;
			return void_literal;
		}

		default:
			return node;
		}
	}

	static AstNode* extra_node_add(AstNode* left, AstNode* right) {
		token::type left_type = left->getType();
		token::type right_type = right->getType();
		if (left_type == token::NUMBER && right_type == token::NUMBER)
			return new NumberLiteral(((NumberLiteral*)left)->getNumber() + ((NumberLiteral*)right)->getNumber());
		else if (left_type == token::STRING && right_type == token::STRING)
			return new StringLiteral(((StringLiteral*)left)->getValue() + ((StringLiteral*)right)->getValue());
		else if (left_type == token::STRING && right_type == token::NUMBER)
			return new StringLiteral(((StringLiteral*)left)->getValue() + ((NumberLiteral*)right)->getValue());
		else if (left_type == token::NUMBER && right_type == token::STRING)
			return new StringLiteral(((NumberLiteral*)left)->getValue() + ((StringLiteral*)right)->getValue());
		out(std::string("unsupport operation: ") + token::kind(left_type) + " + " + token::kind(right_type));
		return nullptr;
	}

	static AstNode* extra_node_mul(AstNode* left, AstNode* right) {
		token::type left_type = left->getType();
		token::type right_type = right->getType();
		if (left_type == token::NUMBER && right_type == token::NUMBER)
			return new NumberLiteral(((NumberLiteral*)left)->getNumber() * ((NumberLiteral*)right)->getNumber());
		else if (left_type == token::NUMBER && right_type == token::STRING)
			return new NumberLiteral(((NumberLiteral*)left)->getNumber() * to_number<double>(right));
		else if (left_type == token::STRING && right_type == token::NUMBER) {
			auto str = ((StringLiteral*)left)->getValue();
			auto ustr = std::string();
			int count = (int)(((NumberLiteral*)right)->getNumber());
			if (count < 0) {
				std::reverse(str.begin(), str.end());
				count = -count;
			}
			while (count-- > 0) ustr.append(str);
			return new StringLiteral((std::string)ustr);
		}
		out(std::string("unsupport operation: ") + token::kind(left_type) + " * " + token::kind(right_type));
		return nullptr;
	}

	static AstNode* exec_node(AstNode* node) {

		token::type type = node->getType();
		switch (type) {

		case token::LP:
			return exec_node(&(((ParenthesizedExpression*)node)->getExpression()));
		case token::EXPR_VOID:
			return exec_node(&(((ExpressionStatement*)node)->getExpression()));
		case token::COMMA:
		{
			auto infix = (InfixExpression*)node;
			exec_node(&(infix->getLeft()));
			exec_node(&(infix->getRight()));
			return void_literal;
		}
		case token::BITNOT:
		{
			auto unary = (UnaryExpression*)node;
			auto operand = (NumberLiteral*)exec_node(&(unary->getOperand()));
			auto result_node = new NumberLiteral(~((int)operand->getNumber()));
			node->addChildToBack(*result_node);
			return result_node;
		}
		case token::POS:
		{
			auto unary = (UnaryExpression*)node;
			auto operand = exec_node(&(unary->getOperand()));
			auto result_node = new NumberLiteral(to_number<double>(operand));
			node->addChildToBack(*result_node);
			return result_node;
		}
		case token::NEG:
		{
			auto unary = (UnaryExpression*)node;
			auto operand = exec_node(&(unary->getOperand()));
			auto type = operand->getType();
			auto result_node = operand;
			if (type == token::NUMBER)
				result_node = new NumberLiteral(-((NumberLiteral*)operand)->getNumber());
			else if (type == token::STRING) {
				auto str = ((StringLiteral*)operand)->getValue();
				std::reverse(str.begin(), str.end());
				result_node = new StringLiteral((std::string)str);
			}
			else if (type == token::ARRAYLIT) {
				result_node = (ArrayLiteral*)copy_literal(operand);
				anr& arr = ((ArrayLiteral*)result_node)->getElements();
				std::reverse(arr.begin(), arr.end());
			}
			node->addChildToBack(*result_node);
			return result_node;
		}
		case token::GETELEM:
		{
			auto g = (ElementGet*)node;
			auto arrl = (ArrayLiteral*)exec_node(&(g->getTarget()));
			anr& arr = arrl->getElements();
			int index = (int)((NumberLiteral*)exec_node(&(g->getElement())))->getNumber();
			if (index >= arr.size()) {
				out("index out of array bounds in '" + g->toSource() + "'");
				return undefined_literal;
			}
			return arr.at(index);
		}
		case token::ARRAYLIT:
		{
			auto& arr = ((ArrayLiteral*)node)->getElements();
			auto length = arr.size();
			auto arrl = new ArrayLiteral();
			for (int index = 0; index != length; ++index) {
				AstNode* element = arr[index];
				arrl->addElement(*copy_literal(exec_node(element)));
			}
			node->addChildToBack(*arrl);
			return arrl;
		}

#define __some_repeated__ \
    auto infix = (InfixExpression*)node; \
    auto left = (KeywordLiteral*)exec_node(&(infix->getLeft())); \
    auto right = (KeywordLiteral*)exec_node(&(infix->getRight()));

#define __some_cased__(type, other) \
    case token::type: \
    { \
        __some_repeated__ \
        bool condit = left->getType() == token::TRUE other right->getType() == token::TRUE; \
        auto result_node = new KeywordLiteral(condit ? token::TRUE : token::FALSE); \
        node->addChildToBack(*result_node); \
        return result_node; \
    }

		__some_cased__(AND, &&)
			__some_cased__(OR, || )
		case token::NOT:
		{
			auto unary = (UnaryExpression*)node;
			auto keyl = (KeywordLiteral*)exec_node(&(unary->getOperand()));
			if (keyl->isBooleanLiteral()) {
				auto result_node = new KeywordLiteral(keyl->getType() == token::TRUE ? token::FALSE : token::TRUE);
				// the value(result_node) will be released by the parent(node)
				node->addChildToBack(*result_node);
				return result_node;
			}
			out(unary->getOperand().toSource() + ": " + keyl->toSource() + " is not a boolean");
			return undefined_literal;
		}

#undef __some_cased__
#define __some_cased__(type, other) \
    case token::type: \
	{ \
        auto unary = (UnaryExpression*)node; \
		auto numl = (NumberLiteral*)exec_node(&(unary->getOperand())); \
		auto result_node = new NumberLiteral(other); \
		node->addChildToBack(*result_node); \
		return assign_hybrid(node, &(unary->getOperand()), result_node); \
	}

		__some_cased__(INC, numl->getNumber() + 1)
		__some_cased__(DEC, numl->getNumber() - 1)

#define __some_type__ AstNode
#undef __some_repeated__
#define __some_repeated__ \
    auto infix = (InfixExpression*)node; \
    auto left = (__some_type__*)exec_node(&(infix->getLeft())); \
    auto right = (__some_type__*)exec_node(&(infix->getRight()));
#define __some_result__ result_node
#define __some_extra__(x) x
#undef __some_cased__
#define __some_cased__(type, other) \
    case token::type: \
    { \
        __some_repeated__ \
        auto result_node = __some_extra__(other); \
        node->addChildToBack(*result_node); \
        return __some_result__; \
    }
			__some_cased__(ADD, extra_node_add(left, right))
			__some_cased__(MUL, extra_node_mul(left, right))

#undef __some_type__
#define __some_type__ NumberLiteral
#undef __some_extra__
#define __some_extra__(x) new NumberLiteral(x)
			__some_cased__(SUB, left->getNumber() - right->getNumber())
			__some_cased__(DIV, left->getNumber() / right->getNumber())
			__some_cased__(MOD, fmod(left->getNumber(), right->getNumber()))
			__some_cased__(POW, pow(left->getNumber(), right->getNumber()))

#undef __some_result__
#define __some_result__ assign_hybrid(node, &(infix->getLeft()), result_node)
#undef __some_type__
#define __some_type__ AstNode
#undef __some_extra__
#define __some_extra__(x) x
			__some_cased__(ASSIGN_ADD, extra_node_add(left, right))
			__some_cased__(ASSIGN_MUL, extra_node_mul(left, right))
#undef __some_type__
#define __some_type__ NumberLiteral
#undef __some_extra__
#define __some_extra__(x) new NumberLiteral(x)
			__some_cased__(ASSIGN_SUB, left->getNumber() - right->getNumber())
			__some_cased__(ASSIGN_DIV, left->getNumber() / right->getNumber())
			__some_cased__(ASSIGN_MOD, fmod(left->getNumber(), right->getNumber()))
			__some_cased__(ASSIGN_POW, pow(left->getNumber(), right->getNumber()))
#undef __some_result__
#define __some_result__ result_node
#undef __some_cased__
#define __some_cased__(type, other) \
    case token::type: \
    { \
        __some_repeated__ \
        auto result_node = new NumberLiteral((int)left->getNumber() other (int)right->getNumber()); \
        node->addChildToBack(*result_node); \
        return __some_result__; \
    }
			__some_cased__(BITAND, &)
			__some_cased__(BITOR, | )
			__some_cased__(BITXOR, ^)
			__some_cased__(LSH, << )
			__some_cased__(RSH, >> )
#undef __some_result__
#define __some_result__ assign_hybrid(node, &(infix->getLeft()), result_node)
			__some_cased__(ASSIGN_BITAND, &)
			__some_cased__(ASSIGN_BITOR, | )
			__some_cased__(ASSIGN_BITXOR, ^)
			__some_cased__(ASSIGN_LSH, << )
			__some_cased__(ASSIGN_RSH, >> )

#undef __some_cased__
#define __some_cased__(type, other) \
    case token::type: \
    { \
        __some_repeated__ \
        AstNode* result_node = nullptr; \
        bool handed_number = left->getType() == token::NUMBER && right->getType() == token::NUMBER; \
        result_node = handed_number ? new KeywordLiteral(((NumberLiteral*)left)->getNumber() other ((NumberLiteral*)right)->getNumber() ? token::TRUE : token::FALSE) : \
            new KeywordLiteral(left->toSource() other right->toSource() ? token::TRUE : token::FALSE); \
        node->addChildToBack(*result_node); \
        return result_node; \
    }

			__some_cased__(EQ, == )
			__some_cased__(NE, != )
#undef __some_cased__
#define __some_cased__(type, other) \
    case token::type: \
    { \
        __some_repeated__ \
        auto result_node = new KeywordLiteral(left->getNumber() other right->getNumber() ? token::TRUE : token::FALSE); \
        node->addChildToBack(*result_node); \
        return result_node; \
    }

			__some_cased__(LT, < )
			__some_cased__(LE, <= )
			__some_cased__(GT, > )
			__some_cased__(GE, >= )


		case token::HOOK:
		{
			auto cond = (ConditionalExpression*)node;
			auto test = (KeywordLiteral*)exec_node(&(cond->getTestExpression()));
			if (!test->isBooleanLiteral()) {
				out(cond->getTestExpression().toSource() + ": " + test->toSource() + " is not a boolean");
				return undefined_literal;
			}
			return test->getType() == token::TRUE ? exec_node(&(cond->getTrueExpression())) :
				exec_node(&(cond->getFalseExpression()));
		}
		case token::IF:
		{
			auto ifStmt = (IfStatement*)node;
			auto cond = (KeywordLiteral*)exec_node(&(ifStmt->getCondition()));
			if (!cond->isBooleanLiteral()) {
				out(ifStmt->getCondition().toSource() + ": " + cond->toSource() + " is not a boolean");
				return void_literal;
			}
			// need to check else-part is non-null
			if (cond->getType() == token::TRUE)
				return exec_node(&(ifStmt->getThenPart()));
			else if (ifStmt->hasElsePart())
				return exec_node(&(ifStmt->getElsePart()));
			return void_literal;
		}
		case token::WHILE:
		{
			auto whileStmt = (WhileLoop*)node;
			auto condNode = &(whileStmt->getCondition());
			auto cond = (KeywordLiteral*)exec_node(condNode);
			if (!cond->isBooleanLiteral()) {
				out(whileStmt->getCondition().toSource() + ": " + cond->toSource() + " is not a boolean");
				return void_literal;
			}
			AstNode* bodyNode = &(whileStmt->getBody());
			while (((KeywordLiteral*)exec_node(condNode))->getType() == token::TRUE) {
				exec_node(bodyNode);
			}
			return void_literal;
		}
		case token::FOR:
		{
			auto fl = (ForLoop*)node;
			auto init = &(fl->getInitializer());
			exec_node(init);
			auto condNode = &(fl->getCondition());
			auto cond = (KeywordLiteral*)exec_node(condNode);
			if (!cond->isBooleanLiteral()) {
				out(fl->getCondition().toSource() + ": " + cond->toSource() + " is not a boolean");
				return void_literal;
			}
			AstNode* increment = &(fl->getIncrement());
			AstNode* bodyNode = &(fl->getBody());
			while (((KeywordLiteral*)exec_node(condNode))->getType() == token::TRUE) {
				exec_node(bodyNode);
				exec_node(increment);
			}

			return void_literal;
		}

		default:
			return exec_node_hybrid(node);
		}
	}

	static void inside_symbol_init(ScriptNode* script)
	{
	    void_literal = new KeywordLiteral(token::VOID);
		undefined_literal = new KeywordLiteral(token::UNDEFINED);

		auto& variables = script->getVariables();
		AstNode* node = variables["pi"] = new NumberLiteral(3.141592653589793);
        script->addChildToBack(*node);
        node->setParent(*script);
        node = variables["e"] = new NumberLiteral(2.7182818284590452);
		script->addChildToBack(*node);
        node->setParent(*script);
        node = nullptr;
        auto& functions = script->getFunctions();

		#include "hykilp-stand.inl"
		
	}
}
