#include <lcl.h>
#include <lcl_complex.h>
#include <math.h>
#include <lualib.h>
#include <lauxlib.h>

static int complex__call(lua_State *L){
	lua_Number z[2];
	if(lua_istable(L, 2) && 2 == lcl_len(L, 2)){
		int i;
		for(i = 0; i < 2; ++i){
			lcl_geti(L, 2, i+1);
			if(!lua_isnumber(L, -1)){
				return luaL_error(L, "expected a table of 2 numbers");
			}
			z[i] = lua_tonumber(L, -1);
			lua_pop(L, 1);
		}
		return lcl_complex_push(L, z[0], z[1]);
	}
	if(lua_isnumber(L, 2)){
		if(lua_isnumber(L, 3)){
			return lcl_complex_push(L, lua_tonumber(L, 2), lua_tonumber(L, 3));
		}else{
			return lcl_complex_push(L, lua_tonumber(L, 2), 0);
		}
	}
	if(lua_isstring(L, 2)){
		// no parser yet:
		return 0;
	}
	return 0;
}

static int complex_expi2pi(lua_State *L){
	if(lua_isnumber(L, 1)){
		lua_Number cs[2];
		lua_Number x = lua_tonumber(L, 1);
		lcl_cossin2pi(x, &cs[0], &cs[1]);
		return lcl_complex_push(L, cs[0], cs[1]);
	}else{
		const lcl_complex *z = lcl_complex_check(L, 1);
		lua_Number cs[2];
		lcl_cossin2pi(z->r, &cs[0], &cs[1]);
		const lua_Number a = exp(-2*M_PI*z->i);
		return lcl_complex_push(L, a*cs[0], a*cs[1]);
	}
	return luaL_error(L, "expected numeric argument");
}

// Metatable functions
static int lcomplex__gc(lua_State *L){
	return 0;
}
static int lcomplex__tostring(lua_State *L){
	const lcl_complex *z = lcl_complex_check(L, 1);
	lua_pushfstring(L, "(%f,%f)", z->r, z->i);
	return 1;
}

static int lcomplex__add(lua_State *L){
	lcl_complex z[2];
	if(!lcl_complex_get(L, 1, &z[0])){
		z[0].r = luaL_checknumber(L, 1);
		z[0].i = 0;
	}
	if(!lcl_complex_get(L, 2, &z[1])){
		z[1].r = luaL_checknumber(L, 2);
		z[1].i = 0;
	}
	z[0].r += z[1].r;
	z[0].i += z[1].i;
	if(0 == z[0].i){
		lua_pushnumber(L, z[0].r);
		return 1;
	}else{
		return lcl_complex_push(L, z[0].r, z[0].i);
	}
}
static int lcomplex__sub(lua_State *L){
	lcl_complex z[2];
	if(!lcl_complex_get(L, 1, &z[0])){
		z[0].r = luaL_checknumber(L, 1);
		z[0].i = 0;
	}
	if(!lcl_complex_get(L, 2, &z[1])){
		z[1].r = luaL_checknumber(L, 2);
		z[1].i = 0;
	}
	z[0].r -= z[1].r;
	z[0].i -= z[1].i;
	if(0 == z[0].i){
		lua_pushnumber(L, z[0].r);
		return 1;
	}else{
		return lcl_complex_push(L, z[0].r, z[0].i);
	}
}
static int lcomplex__unm(lua_State *L){
	lcl_complex z;
	lcl_complex_get(L, 1, &z);
	if(0 == z.i){
		lua_pushnumber(L, -z.r);
		return 1;
	}else{
		return lcl_complex_push(L, -z.r, -z.i);
	}
}
static int lcomplex__mul(lua_State *L){
	lcl_complex z[2];
	lcl_complex ret;
	if(!lcl_complex_get(L, 1, &z[0])){
		z[0].r = luaL_checknumber(L, 1);
		z[0].i = 0;
	}
	if(!lcl_complex_get(L, 2, &z[1])){
		z[1].r = luaL_checknumber(L, 2);
		z[1].i = 0;
	}
	ret.r = z[0].r*z[1].r-z[0].i*z[1].i;
	ret.i = z[0].r*z[1].i+z[0].i*z[1].r;
	if(0 == ret.i){
		lua_pushnumber(L, ret.r);
		return 1;
	}else{
		return lcl_complex_push(L, ret.r, ret.i);
	}
}
static int lcomplex__div(lua_State *L){
	lcl_complex x, y;
	lua_Number yy, p, q;
	if(!lcl_complex_get(L, 1, &x)){
		x.r = luaL_checknumber(L, 1);
		x.i = 0;
	}
	if(!lcl_complex_get(L, 2, &y)){
		y.r = luaL_checknumber(L, 2);
		y.i = 0;
	}

	yy = y.r*y.r + y.i*y.i;
	p = y.r*x.r + y.i*x.i;
	q = x.i*y.r - x.r*y.i;
	if(0 == q){
		lua_pushnumber(L, p/yy);
		return 1;
	}else{
		return lcl_complex_push(L, p/yy, q/yy);
	}
}
static int lcomplex__pow(lua_State *L){
	lcl_complex a, b;
	lua_Number x, y;
	if(!lcl_complex_get(L, 1, &a)){
		a.r = luaL_checknumber(L, 1);
		a.i = 0;
	}
	if(!lcl_complex_get(L, 2, &b)){
		b.r = luaL_checknumber(L, 2);
		b.i = 0;
	}
	x = b.r;
	y = b.i;
	lua_Number absa = l_mathop(hypot)(a.r, a.i);
	if(0 == absa){
		lua_pushnumber(L, 0);
		return 1;
	}
	lua_Number arga = l_mathop(atan2)(a.i, a.r);
	lua_Number r = l_mathop(pow)(absa, x);
	lua_Number theta = x*arga;
	if(0 != y){
		r *= exp(-y * arga);
		theta += y*log(absa);
	}
	return lcl_complex_push(L, r*cos(theta), r*sin(theta));
}
static int lcomplex__len(lua_State *L){
	lcl_complex z;
	lcl_complex_get(L, 1, &z);
	lua_pushnumber(L, l_mathop(hypot)(z.r, z.i));
	return 1;
}
static int lcomplex__eq(lua_State *L){
	lcl_complex a, b;
	if(!lcl_complex_get(L, 1, &a)){
		a.r = luaL_checknumber(L, 1);
		a.i = 0;
	}
	if(!lcl_complex_get(L, 2, &b)){
		b.r = luaL_checknumber(L, 2);
		b.i = 0;
	}
	lua_pushboolean(L, a.r == b.r && a.i == b.i);
	return 1;
}

static int lcomplex_get_arg(lua_State *L){
	const lcl_complex *z = lcl_complex_check(L, 1);
	lua_pushnumber(L, l_mathop(atan2)(z->i, z->r));
	return 1;
}
static int lcomplex_get_conj(lua_State *L){
	const lcl_complex *z = lcl_complex_check(L, 1);
	lcl_complex_push(L, z->r, -z->i);
	return 1;
}
static int lcomplex_get_imag(lua_State *L){
	const lcl_complex *z = lcl_complex_check(L, 1);
	lua_pushnumber(L, z->i);
	return 1;
}
static int lcomplex_get_norm(lua_State *L){
	const lcl_complex *z = lcl_complex_check(L, 1);
	lua_pushnumber(L, l_mathop(hypot)(z->r, z->i));
	return 1;
}
static int lcomplex_get_norm1(lua_State *L){
	const lcl_complex *z = lcl_complex_check(L, 1);
	lua_pushnumber(L, l_mathop(fabs)(z->r)+l_mathop(fabs)(z->i));
	return 1;
}
static int lcomplex_get_normsq(lua_State *L){
	const lcl_complex *z = lcl_complex_check(L, 1);
	lua_pushnumber(L, z->r*z->r + z->i*z->i);
	return 1;
}
static int lcomplex_get_real(lua_State *L){
	const lcl_complex *z = lcl_complex_check(L, 1);
	lua_pushnumber(L, z->r);
	return 1;
}

static int lcomplex__index(lua_State *L){
	static const luaL_Reg getters[] = {
		{ "arg", &lcomplex_get_arg },
		{ "conj", &lcomplex_get_conj },
		{ "imag", &lcomplex_get_imag },
		{ "norm", &lcomplex_get_norm },
		{ "norm1", &lcomplex_get_norm1 },
		{ "normsq", &lcomplex_get_normsq },
		{ "real", &lcomplex_get_real },
		{ NULL, NULL }
	};
	return lcl_type_prop_parse(L, getters);
}

static int lcomplex_set_imag(lua_State *L){
	lcl_complex *z = lcl_complex_check(L, 1);
	lua_Number val = luaL_checknumber(L, 3);
	z->i = val;
	return 0;
}
static int lcomplex_set_norm(lua_State *L){
	lcl_complex *z = lcl_complex_check(L, 1);
	lua_Number val = luaL_checknumber(L, 3);
	val /= l_mathop(hypot)(z->r, z->i);
	z->r *= val; z->i *= val;
	return 0;
}
static int lcomplex_set_real(lua_State *L){
	lcl_complex *z = lcl_complex_check(L, 1);
	lua_Number val = luaL_checknumber(L, 3);
	z->r = val;
	return 0;
}
static int lcomplex__newindex(lua_State *L){
	static const luaL_Reg setters[] = {
		{ "imag", &lcomplex_set_imag },
		{ "norm", &lcomplex_set_norm },
		{ "real", &lcomplex_set_real },
		{ NULL, NULL }
	};
	return lcl_type_prop_parse(L, setters);
}

static const luaL_Reg lcl_complex_metamethods[] = {
	{"__gc", lcomplex__gc},
	{"__index", lcomplex__index},
	{"__newindex", lcomplex__newindex},
	{"__tostring", lcomplex__tostring},
	{"__add", lcomplex__add},
	{"__sub", lcomplex__sub},
	{"__unm", lcomplex__unm},
	{"__mul", lcomplex__mul},
	{"__div", lcomplex__div},
	{"__pow", lcomplex__pow},
	{"__len", lcomplex__len},
	{"__eq", lcomplex__eq},
	{NULL, NULL}
};
static const lcl_usertype lcl_complex_type = {
	"complex",
	lcl_complex_metamethods,
	NULL
};

int lcl_complex_loaded(lua_State *L){
	int ret;
	luaL_getmetatable(L, lcl_complex_type.metatable_name);
	ret = !lua_isnil(L, -1);
	lua_pop(L, 1);
	return ret;
}

int lcl_complex_push(lua_State *L, lua_Number r, lua_Number i){
	lcl_complex *pz = (lcl_complex*)lcl_type_push(L, &lcl_complex_type, sizeof(lcl_complex));
	pz->r = r; pz->i = i;
	return 1;
}

int lcl_complex_get(lua_State *L, int i, lcl_complex *z){
	const lcl_complex *pz = (const lcl_complex*)lcl_type_test(L, &lcl_complex_type, i);
	if(NULL != pz){
		z->r = pz->r;
		z->i = pz->i;
		return 1;
	}
	if(lua_istable(L, i) && 2 == lcl_len(L, i)){
		lcl_geti(L, i, 1);
		z->r = luaL_checknumber(L, i);
		lua_pop(L, 1);

		lcl_geti(L, i, 2);
		z->i = luaL_checknumber(L, i);
		lua_pop(L, 1);
		return 1;
	}
	return 0;
}

lcl_complex* lcl_complex_test(lua_State *L, int i){
	return (lcl_complex*)lcl_type_test(L, &lcl_complex_type, i);
}
lcl_complex* lcl_complex_check(lua_State *L, int i){
	return (lcl_complex*)lcl_type_check(L, &lcl_complex_type, i);
}

int lcl_complex_load(lua_State *L){
	lua_pop(L, lcl_newtype(L, &lcl_complex_type));

	static const luaL_Reg complex_lib[] = {
		{"expi2pi", complex_expi2pi},
		{NULL, NULL}
	};
	lcl_newlibrary(L, complex_lib);

	lua_createtable(L, 0, 1);
	lua_pushcfunction(L, complex__call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);

	// Create the top level constants
	lcl_complex_push(L, 0, 0);
	lua_setfield(L, -2, "zero");
	lcl_complex_push(L, 1, 0);
	lua_setfield(L, -2, "one");
	lcl_complex_push(L, 0, 1);
	lua_setfield(L, -2, "i");

	return 1;
}

