
#define __min_argument_count__(info, count) \
if (args.size() < count) { \
	out(info ": wrong of the argument size, at least " + cast_string(count)); \
	return undefined_literal; \
}
#define __1P_MATH_FUNC__(name, impl) \
functions[name] = [](ant& args) -> AstNode * { \
	__min_argument_count__(name, 1); \
	auto num = (NumberLiteral*)pop_arg(args); \
	auto result_node = new NumberLiteral(impl(num->getNumber())); \
	num->addChildToBack(*result_node); \
	return result_node; \
}

// trigonometric
__1P_MATH_FUNC__("sin", sin);
__1P_MATH_FUNC__("cos", cos);
__1P_MATH_FUNC__("tan", tan);
__1P_MATH_FUNC__("arcsin", asin);
__1P_MATH_FUNC__("arccos", acos);
__1P_MATH_FUNC__("arctan", atan);

functions["arctan2"] = [](ant& args) -> AstNode * {
	__min_argument_count__("log", 1);
	auto y = (NumberLiteral*)pop_arg(args);
	auto x = (NumberLiteral*)pop_arg(args);
	auto result_node = new NumberLiteral(atan2(y->getNumber(), x->getNumber()));
	y->addChildToBack(*result_node);
	return result_node;
};
// hyperbolic
__1P_MATH_FUNC__("sinh", sinh);
__1P_MATH_FUNC__("cosh", cosh);
__1P_MATH_FUNC__("tanh", tanh);
__1P_MATH_FUNC__("arcsinh", asinh);
__1P_MATH_FUNC__("arccosh", acosh);
__1P_MATH_FUNC__("arctanh", atanh);
// exponential and logarithmic
__1P_MATH_FUNC__("exp", exp);
__1P_MATH_FUNC__("exp2", exp2);
__1P_MATH_FUNC__("ln", log);
__1P_MATH_FUNC__("lg", log10);
// __1P_MATH_FUNC__("log2", log2);
functions["log"] = [](ant& args) -> AstNode * {
	__min_argument_count__("log", 1);
	auto a = (NumberLiteral*)pop_arg(args);
	auto n = (NumberLiteral*)pop_arg(args);
	auto result_node = new NumberLiteral(log(n->getNumber()) / log(a->getNumber()));
	a->addChildToBack(*result_node);
	return result_node;
};
// power
__1P_MATH_FUNC__("sqrt", sqrt);
__1P_MATH_FUNC__("cbrt", cbrt);
functions["hypot"] = [](ant& args) -> AstNode * {
	__min_argument_count__("hypot", 1);
	auto x = (NumberLiteral*)pop_arg(args);
	auto y = (NumberLiteral*)pop_arg(args);
	auto result_node = new NumberLiteral(hypot(y->getNumber(), x->getNumber()));
	y->addChildToBack(*result_node);
	return result_node;
};
// error and gamma
__1P_MATH_FUNC__("erf", erf);
__1P_MATH_FUNC__("erfc", erfc);
__1P_MATH_FUNC__("tgamma", tgamma);
__1P_MATH_FUNC__("lgamma", lgamma);
// rounding and remainder
__1P_MATH_FUNC__("round", round);
__1P_MATH_FUNC__("floor", floor);
__1P_MATH_FUNC__("ceil", ceil);
// other
__1P_MATH_FUNC__("abs", fabs);

functions["out"] = [](ant& args) -> AstNode * {
	AstNode* current = nullptr;
	while (args.size() != 0) {
		current = pop_arg(args);
		if (current -> getType() == token::NUMBER)
			printf("%.16g\n", ((NumberLiteral*)current)->getNumber());
		else
			printf("%s\n", to_string(current).c_str());
	}
	return void_literal;
};

functions["length"] = [](ant& args) -> AstNode * {
	__min_argument_count__("length", 1);
	auto arrl = (ArrayLiteral*)pop_arg(args);
	auto result_node = new NumberLiteral(arrl->getElements().size());
	arrl->addChildToBack(*result_node);
	return result_node;
};

functions["array"] = [](ant& args) -> AstNode * {
	__min_argument_count__("array", 2);
	auto arrl = new ArrayLiteral();
	size_t length = (int)((NumberLiteral*)pop_arg(args))->getNumber();
	auto init_value = pop_arg(args);
	if (init_value == undefined_literal) {
		out("array: <init-value> is undefined");
		return arrl;
	}
	for (int index = 0; index != length; ++index) {
		arrl->addElement(*copy_literal(init_value));
	}
	init_value->addChildToBack(*arrl);
	return arrl;
};

functions["split"] = [](ant& args) -> AstNode * {
	__min_argument_count__("split", 1);
	size_t size = args.size();
	auto node = (StringLiteral*)pop_arg(args);
	auto source = node->getValue();
	std::string separator;
	if (size > 1) separator = ((StringLiteral*)pop_arg(args))->getValue();
	size_t start = 0;
	ArrayLiteral* arrl = new ArrayLiteral();
	if (separator.length() != 0) {
		size_t end, sepalen = separator.length();
		while ((end = source.find(separator, start)) != std::string::npos) {
			arrl->addElement(*new StringLiteral(source.substr(start, end - start)));
			start = end + sepalen;
		}
		arrl->addElement(*new StringLiteral(source.substr(start)));
	}
	else
		while (start < source.size()) arrl->addElement(*new StringLiteral(source.substr(start++, 1)));
	node->addChildToBack(*arrl);
	return arrl;
};

functions["implode"] = [](ant& args) -> AstNode * {
	__min_argument_count__("implode", 1);
	size_t size = args.size();
	auto node = pop_arg(args);
	auto arr = ((ArrayLiteral*)node)->getElements();
	std::string result;
	if (size > 1) {
		auto separator = ((StringLiteral*)pop_arg(args))->getValue();
		for (anr::iterator iter = arr.begin(); iter != arr.end(); ++iter) {
			result += to_string(*iter);
			if (iter + 1 != arr.end())
				result += separator;
		}
	}
	else {
		for (auto element : arr)
			result += to_string(element);
	}
	auto result_node = new StringLiteral((std::string)result);
	node->addChildToBack(*result_node);
	return result_node;
};

functions["range"] = [](ant& args) -> AstNode * {
	__min_argument_count__("range", 1);
	size_t size = args.size();
	int start = 0, stop = 0, step = 1;
	auto first = pop_arg(args);
	if (size == 1) stop = (int)((NumberLiteral*)first)->getNumber();
	else if (size > 1) {
		start = (int)((NumberLiteral*)first)->getNumber();
		stop = (int)((NumberLiteral*)pop_arg(args))->getNumber();
		if (size > 2) step = (int)((NumberLiteral*)pop_arg(args))->getNumber();
	}
	const bool increment = step > 0;
	auto arrl = new ArrayLiteral();
	first->addChildToBack(*arrl);
	if ((increment && start > stop) || (!increment && start < stop) || step == 0) {
		out("range: illegal endless loop");
		return arrl;
	}
	for (int index = start; increment ? index < stop : index > stop; index += step) {
		arrl->addElement(*new NumberLiteral(index));
	}
	return arrl;
};

functions["shuffle"] = [](ant& args) -> AstNode * {
	__min_argument_count__("shuffle", 1);
	size_t size = args.size();
	auto node = pop_arg(args);
	auto type = node->getType();
	auto result_node = node;
	unsigned int seed = (size == 1) ? unsigned(std::time(0)) :
	(unsigned int)(((NumberLiteral*)pop_arg(args))->getNumber());
	std::srand(seed);
	if (type == token::STRING) {
		auto& str = ((StringLiteral*)node)->getValue();
		std::random_shuffle(str.begin(), str.end());
		result_node = new StringLiteral((std::string)str);
	}
	else if (type == token::ARRAYLIT) {
		result_node = (ArrayLiteral*)copy_literal(node);
		anr& arr = ((ArrayLiteral*)result_node)->getElements();
		std::random_shuffle(arr.begin(), arr.end());
	}
	node->addChildToBack(*result_node);
	return result_node;
};
