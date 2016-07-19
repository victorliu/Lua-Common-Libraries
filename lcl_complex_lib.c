#include <lcl_complex.h>

int luaopen_complex(lua_State *L){
	return lcl_complex_load(L);
}
