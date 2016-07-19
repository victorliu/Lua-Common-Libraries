#ifndef LCL_H_INCLUDED
#define LCL_H_INCLUDED

#include <lua.h>
#include <lauxlib.h>
#include <stdio.h>

#if LUA_VERSION_NUM <= 502
#define lua_isinteger(L, I) (lua_isnumber((L), (I)) && lua_tonumber((L), (I)) == (lua_Number)lua_tointeger((L), (I)))
#endif

int lcl_checkversion(lua_State *L);
int lcl_absindex(lua_State *L, int idx);
int lcl_len(lua_State *L, int idx);
int lcl_geti(lua_State *L, int idx, lua_Integer i);
int lcl_seti(lua_State *L, int idx, lua_Integer i);
int lcl_getfield(lua_State *L, int idx, const char *key);
int lcl_setfield(lua_State *L, int idx, const char *key);

void lcl_dumpvalue(lua_State *L, int idx, FILE *fp);

typedef struct{
	int (*handler)(lua_State *L, int argbase, int *parsed_list, void *data);
	const char *argnames;
	int allow_unnamed;
/*
	argnames  ::= <reqlist> <optlist>
	<reqlist> ::= <arglist>
	<optlist> ::= ';' <arglist>
	<arglist> ::= <argalt> | <argalt> ',' <arglist>
	<argalt>  ::= <name> | <name> '|' <argalt>
	<name>    ::= [a-zA-Z_][a-zA-Z0-9_]*
*/
} lcl_named_argset;

// named argument parsing and checking
int lcl_named_argparse(
	lua_State *L, int i, const lcl_named_argset *args,
	void *data
); /*
For a given table of named arguments at index i, parses and checks for
the arguments given in a set of alternatives specified in args.
The handler function for the matching set is called, and the
first argument parsed is located at stack index argbase. The list
parsed_list specifies which alternative was parsed when present, or
0 if an optional argument was omitted (nil is pushed onto the stack
for omitted arguments).
When argnames is the empty string, then the argument list is treated
as an array (integer keys) and parsed_list is a single integer containing
the number of integer-key entries.
Returns the value returned by the handler function called.
*/

/* Named argument parsing example:
int lib_new_transform(lua_State *L){
	static const lcl_named_argset args[] = {
		{ &lib_new_transform_translate, "translate", 0 },
		{ &lib_new_transform_rotate3, "axis,degrees|radians", 0 },
		{ &lib_new_transform_rotate2, "degrees|radians", 0 },
		{ &lib_new_transform_scale, "scale", 0 },
		{ &lib_new_transform_explicit, "", 1 },
		{ NULL, NULL, 0 }
	};
	struct mydata data;
	int ret = lcl_named_argparse(L, 1, args, &data);
}
*/

typedef struct{
	const char *metatable_name;
	const luaL_Reg *metamethods;
	const luaL_Reg *methods;
} lcl_usertype;

/* Creates a new metatable and method table based on the information in typedata.
 * Returns the metatable and the method table on the stack and returns 2.
 */
int lcl_newtype(lua_State *L, const lcl_usertype *typedata);

void* lcl_type_push(lua_State *L, const lcl_usertype *typedata, int nbytes);
void* lcl_type_test(lua_State *L, const lcl_usertype *typedata, int idx);
void* lcl_type_check(lua_State *L, const lcl_usertype *typedata, int idx);

int lcl_type_prop_parse(lua_State *L, const luaL_Reg *props); /*
props must be in lexicographical order */

int lcl_newlibrary(lua_State *L, const luaL_Reg *funcs);

/* Math functions */

/* computes c = cos(2*pi*x), s = sin(2*pi*x) */
void lcl_cossin2pi(lua_Number x, lua_Number *c, lua_Number *s);

#endif /* LCL_H_INCLUDED */

