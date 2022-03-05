
#include <sstream>

namespace commons
{
	std::string& replace_all_distinct(std::string& str, const std::string& old_value, const std::string& new_value);
    
	template <typename type> 
	static inline type string_cast(const std::string& str) {
		std::istringstream iss(str);
		type result;
		iss >> result;
		return result;
	}

	template <typename type>
	static inline std::string cast_string(type any) {
		std::ostringstream oss;
		oss << any;
		return oss.str();
	}

	static inline bool is_ostter(char ch) {
		return ch == '\v' || ch == '\n' || 
			ch == '\r' || ch == '\f' || 
			ch == '\0' || ch == '\u000b';
	}

	static inline bool is_filter(char ch) {
		return ch == ' ' || ch == '\t' || is_ostter(ch);
	}

	static inline bool is_upper(char ch) {
		return ch >= 'A' && ch <= 'Z';
	}

	static inline bool is_lower(char ch) {
		return ch >= 'a' && ch <= 'z';
	}

	static inline bool is_letter(char ch) {
		return is_lower(ch) || is_upper(ch) || 
			ch == '_' || ch == '$';
	}
	
	static inline bool is_digit(char ch) {
		return ch >= '0' && ch <= '9';
	}
	
	static inline bool is_digit16(char ch) {
		return is_digit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
	}

	static inline bool is_splitor(char ch) {
		return ch == ',' || ch == ';' || 
			ch == '(' || ch == ')' || 
			ch == '[' || ch == ']' || 
			ch == '{' || ch == '}';
	}

	static inline bool is_appendable(char ch) {
		return ch == '|' || ch == '!' ||
			ch == '+' || ch == '-' ||
			ch == '*' || ch == '/' ||
			ch == '%' || ch == '^' ||
			ch == '&' || ch == '=' ||
			ch == '<' || ch == '>';
	}

	static inline bool is_symbol(char ch) {
		return is_splitor(ch) || is_appendable(ch) || 
			ch == '~' || ch == '?' || ch == ':';
	}
}

