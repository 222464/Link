#include <duktape/duktape.h>

#include <string>

namespace action {
	// writes a string to the terminal (no end line ending appended)
	duk_ret_t write(duk_context* ctx);

	// reads a line from the terminal
	duk_ret_t readLine(duk_context* ctx);

	// registers the given function in the given context with the given name
	void registerFunction(duk_context* ctx, duk_ret_t(*function)(duk_context*), const std::string& name);

	// registers all functions in the given context
	void registerAllFunctions(duk_context* ctx);
}