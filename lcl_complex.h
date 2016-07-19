#ifndef LCL_COMPLEX_H_INCLUDED
#define LCL_COMPLEX_H_INCLUDED

#include <lua.h>

typedef struct{
	lua_Number r, i;
} lcl_complex;

int lcl_complex_loaded(lua_State *L); /*
 Determine if the userdata type exists within the Lua VM
 */

int lcl_complex_load(lua_State *L);

int lcl_complex_push(lua_State *L, lua_Number r, lua_Number i); /*
 Push a complex number on the stack. If the userdata type does not exist,
 pushes a table of length 2.
 */

int lcl_complex_get(lua_State *L, int i, lcl_complex *z); /*
 Check to see if the value at index i is a complex number.
 If it is either the userdata type or a table of length 2, returns
 nonzero, and optionally fills in parameter z.
 */

lcl_complex* lcl_complex_test(lua_State *L, int i); /*
 Check to see if the value at index i is a complex number.
 */
lcl_complex* lcl_complex_check(lua_State *L, int i); /*
 Check to see if the value at index i is a complex number.
 */



#endif /* LCL_COMPLEX_H_INCLUDED */
