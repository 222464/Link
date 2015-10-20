#include <action/ActionFunctions.h>

#include <iostream>

duk_ret_t action::write(duk_context* ctx) {
	// number of args
	int n = duk_get_top(ctx);

	// loop through args and cout each
	for (int i = 0; i < n; i++)
		std::cout << duk_to_string(ctx, i);

	// return 0 values (void)
	return 0;
}

duk_ret_t action::readLine(duk_context* ctx) {
	std::string input;
	std::cin >> input;

	// push the input string
	duk_push_string(ctx, input.c_str());

	// return 1 value (string)
	return 1;
}

void action::registerFunction(duk_context* ctx, duk_ret_t(*function)(duk_context*), const std::string& name) {
	duk_push_global_object(ctx);
	duk_push_c_function(ctx, function, DUK_VARARGS);
	duk_put_prop_string(ctx, -2, name.c_str());
	duk_pop(ctx);
}

void action::registerAllFunctions(duk_context* ctx) {
	registerFunction(ctx, write, "write");
	registerFunction(ctx, readLine, "readLine");
}