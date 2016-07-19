#include <lcl.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int lcl_absindex(lua_State *L, int idx){
	if(idx > 0 || idx <= LUA_REGISTRYINDEX){ return idx; }
	return lua_gettop(L) + 1 + idx;
}

int lcl_len(lua_State *L, int idx){
	int len = 0;
#if LUA_VERSION_NUM < 502
	lua_objlen(L, idx);
	len = lua_tointeger(L, -1);
	lua_pop(L, 1);
#else
	lua_len(L, idx);
	len = lua_tointeger(L, -1);
	lua_pop(L, 1);
#endif
	return len;
}
int lcl_geti(lua_State *L, int idx, lua_Integer i){
	idx = lcl_absindex(L, idx);
#if LUA_VERSION_NUM < 503
	lua_pushinteger(L, i);
	lua_gettable(L, idx);
	return lua_type(L, -1);
#else
	return lua_geti(L, idx, i);
#endif
}
int lcl_seti(lua_State *L, int idx, lua_Integer i){
	idx = lcl_absindex(L, idx);
#if LUA_VERSION_NUM < 503
	lua_pushinteger(L, i);
	lua_insert(L, -2);
	lua_settable(L, idx);
#else
	lua_seti(L, idx, i);
#endif
	return 0;
}
int lcl_getfield(lua_State *L, int idx, const char *key){
#if LUA_VERSION_NUM < 503
	lua_getfield(L, idx, key);
	return lua_type(L, -1);
#else
	return lua_getfield(L, idx, key);
#endif
}
int lcl_setfield(lua_State *L, int idx, const char *key){
	lua_setfield(L, idx, key);
	return 0;
}

static void lcl_dumpvalue_indent(FILE *fp, int level){
	while(level --> 0){
		fputc(' ', fp);
		fputc(' ', fp);
	}
}
static void lcl_dumpvalue_r(lua_State *L, int idx, FILE *fp, int level){
	if(lua_istable(L, idx)){
		if(idx < 0){
			idx = lua_gettop(L) + idx + 1;
		}
		fprintf(fp, "{\n");
		lua_pushnil(L);
		while(0 != lua_next(L, idx)){
			lcl_dumpvalue_indent(fp, level);
			fprintf(fp, "  %s : ", lua_tostring(L, -2));
			lcl_dumpvalue_r(L, idx, fp, level+1);
			lua_pop(L, 1);
		}
		lcl_dumpvalue_indent(fp, level);
		fprintf(fp, "}\n");
	}else{
		fprintf(fp, "%s\n", lua_tostring(L, idx));
	}
}
void lcl_dumpvalue(lua_State *L, int idx, FILE *fp){
	lcl_dumpvalue_r(L, idx, fp, 0);
}

int lcl_named_argparse(
	lua_State *L, int idx, const lcl_named_argset *args,
	void *data
){
	static const char *validName = "0123456789"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ" "_"
	"abcdefghijklmnopqrstuvwxyz"
	;
	idx = lcl_absindex(L, idx);
	luaL_argcheck(L, lua_istable(L, idx), idx, "Expected table of named arguments");

	// Keep local buffer of key names
	size_t strbuf_size = 0;
	size_t strbuf_size_alloc = 256;
	char *strbuf = (char*)malloc(sizeof(char)*strbuf_size_alloc);
	size_t keyptr_size = 0;
	size_t keyptr_size_alloc = 8;
	size_t *keyptr = (size_t*)malloc(sizeof(size_t)*keyptr_size_alloc);

	int n_unnamed = 0;
	lua_pushnil(L);  /* first key */
	while(0 != lua_next(L, idx)){
		/* key at -2, value at -1 */
		if(lua_isstring(L, -2)){
			size_t klen ;
			const char *k = lua_tolstring(L, -2, &klen);
			if(keyptr_size >= keyptr_size_alloc){
				keyptr_size_alloc *= 2;
				keyptr = (size_t*)realloc(keyptr, sizeof(size_t)*keyptr_size_alloc);
			}
			if(strbuf_size+klen+1 > strbuf_size_alloc){
				strbuf_size_alloc *= 2;
				strbuf = (char*)realloc(strbuf, sizeof(char)*strbuf_size_alloc);
			}
			keyptr[keyptr_size] = strbuf_size;
			keyptr_size++;
			memcpy(&strbuf[strbuf_size], k, klen);
			strbuf_size += klen;
			strbuf[strbuf_size] = '\0';
			strbuf_size++;
		}else if(lua_isnumber(L, -2)){
			++n_unnamed;
		}
		/* pop value, keep key for next iteration */
		lua_pop(L, 1);
	}

	int *keyflags = (int*)malloc(sizeof(int)*keyptr_size);
	size_t parsed_list_size = 8;
	int *parsed_list = (int*)malloc(sizeof(int)*parsed_list_size);

	// Now go through all the possible sets and try to match
	while(NULL != args->handler && NULL != args->argnames){
		if(n_unnamed > 0 && !args->allow_unnamed){ args++; continue; }
		if('\0' == args->argnames[0]){
			if(0 != keyptr_size){ args++; continue; }
			free(keyflags);
			free(keyptr);
			free(strbuf);
			free(parsed_list);
			// Call the function
			return args->handler(L, lua_gettop(L)+1, &n_unnamed, data);
		}
		int matched = 1;

		// clear the 'seen' flag for each key
		memset(keyflags, 0, sizeof(int)*keyptr_size);
		// Iterate through each comma delimited piece of argnames
		const char *c = args->argnames;
		int ialt = 1;
		int iarg = 0;
		int optional = 0;
		int nexpected = 0;
		do{
			const char *c0 = c;
			// eat name
			c += strspn(c, &validName[10]);
			c += strspn(c, validName);
			const int namelen = c-c0;
			if(namelen <= 0){
				free(keyflags);
				free(keyptr);
				free(strbuf);
				return luaL_error(L, "Error parsing argnames");
			}
			if(1 == ialt && !optional){ nexpected++; }
			// try to find name in the existing set
			int i;
			for(i = 0; i < keyptr_size; ++i){
				if(0 == strncmp(c0, &strbuf[keyptr[i]], namelen) && '\0' == strbuf[keyptr[i]+namelen]){
					keyflags[i] = iarg+1;
					if(iarg >= parsed_list_size){
						parsed_list_size *= 2;
						parsed_list = (int*)realloc(parsed_list, sizeof(int)*parsed_list_size);
					}
					parsed_list[iarg] = ialt;
					break;
				}
			}
			if('|' != *c && !optional && 0 == parsed_list[iarg]){
				// did not match since non-optional argument was not provided
				matched = 0;
				break;
			}
			if('|' == *c){
				ialt++;
				++c;
			}else if(',' == *c){
				ialt = 1;
				iarg++;
				++c;
			}else if(';' == *c){
				ialt = 1;
				iarg++;
				optional = 1;;
				++c;
			}else if('\0' == *c){
				break;
			}else{
				char c_unexpected = *c;
				free(keyflags);
				free(keyptr);
				free(strbuf);
				free(parsed_list);
				return luaL_error(L, "Error parsing argnames; unexpected character: %c", c_unexpected);
			}
		}while('\0' != *c);
		if(!matched){ args++; continue; }
		// At this point, we have flagged every provided key that was found in the argnames set
		// If any were not flagged, this is a non-match
		int i;
		for(i = 0; i < keyptr_size; ++i){
			if(!keyflags[i]){
				matched = 0;
				break;
			}
		}
		if(!matched){ args++; continue; }
		// Now make sure there all non-optional arguments were provided
		for(i = 0; i < nexpected; ++i){
			if(!parsed_list[i]){
				matched = 0;
				break;
			}
		}
		if(!matched){ args++; continue; }

		// We found a match! Push all arguments onto stack
		const int t0 = lua_gettop(L);
		lua_settop(L, t0+keyptr_size);
		for(i = 0; i < keyptr_size; ++i){
			lua_getfield(L, idx, &strbuf[keyptr[i]]);
			lua_replace(L, t0+keyflags[i]);
		}
		free(keyflags);
		free(keyptr);
		free(strbuf);
		// Call the function
		int ret = args->handler(L, t0+1, parsed_list, data);
		free(parsed_list);
		return ret;
	}
	free(parsed_list);
	free(keyflags);
	free(keyptr);
	free(strbuf);
	return luaL_error(L, "Could not match named arguments to any of the following:");
}


int lcl_newtype(lua_State *L, const lcl_usertype *typedata){
	//const int stackTest = lua_gettop(L);

    if(0 == luaL_newmetatable(L, typedata->metatable_name)){
		return luaL_error(L, "metatable '%s' already registered", typedata->metatable_name);
	}
	const int metatable = lua_gettop(L);

	lua_newtable(L);
	const int methodtable = lua_gettop(L);

	// Translate attribute __metatable of meta. This is the value that is
	// returned by getmetatable(obj). Trick to hide real metatable.
    // Does not work if debug.getmetatable is called.
	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, methodtable );
	lua_settable(L, metatable);

	lua_pushliteral(L, "__index");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	for(const luaL_Reg* l = typedata->metamethods; NULL != l && l->name; l++){
		lua_pushstring(L, l->name);
		lua_pushcfunction(L, l->func);
		lua_rawset(L, metatable);
	}

	if(NULL != typedata->methods){
		for(const luaL_Reg* l = typedata->methods; NULL != l && l->name; l++){
			lua_pushstring(L, l->name);
			lua_pushcfunction(L, l->func);
			lua_rawset(L, methodtable);
		}
	}

	return 2;
}


void* lcl_type_push(lua_State *L, const lcl_usertype *typedata, int nbytes){
	void *p = lua_newuserdata(L, nbytes);
	luaL_getmetatable(L, typedata->metatable_name);
	lua_setmetatable(L, -2);
	return p;
}
void* lcl_type_test(lua_State *L, const lcl_usertype *typedata, int idx){
	void *p = lua_touserdata(L, idx);
	if(NULL == p){ return NULL; }
	if(!lua_getmetatable(L, idx)){ return NULL; }
	luaL_getmetatable(L, typedata->metatable_name);
	if(!lua_rawequal(L, -1, -2)){
		p = NULL;
	}
	lua_pop(L, 2);
	return p;
}
void* lcl_type_check(lua_State *L, const lcl_usertype *typedata, int idx){
	void *p = lcl_type_test(L, typedata, idx);
	if(NULL == p){
		luaL_error(L, "Expected type %s at index %d", typedata->metatable_name, idx);
		return NULL;
	}
	return p;
}

int lcl_type_prop_parse(lua_State *L, const luaL_Reg *props){
	size_t lstr = 0;
	const char *prop = luaL_checklstring(L, 2, &lstr);
	for( ; props->name; ++props){
		if(0 == strcmp(props->name, prop)){
			return props->func(L);
		}
	}
	return luaL_error(L, "Unrecognized property: %s", prop);
}


int lcl_newlibrary(lua_State *L, const luaL_Reg *funcs){
#if LUA_VERSION_NUM < 501
	lua_newtable(L);
	for(; funcs->name; funcs++){
		lua_pushstring(L, funcs->name);
		lua_pushcclosure(L, funcs->func, 0);
		lua_settable(L, -3);
	}
	return 1;
#elif LUA_VERSION_NUM == 501
	lua_createtable(L, 0, sizeof(funcs)/sizeof(funcs[0]) - 1);
	for(; funcs->name; funcs++){
		lua_pushcclosure(L, funcs->func, 0);
		lua_setfield(L, -2, funcs->name);
	}
	return 1;
#elif LUA_VERSION_NUM >= 502
	luaL_newlib(L, funcs);
	return 1;
#elif LUA_VERSION_NUM < 501

#else
#error "Unsupported Lua version"
	return 0;
#endif
}





static lua_Number sin2pi(lua_Number x){
	return l_mathop(sin)(2*M_PI*x);
}

void lcl_cossin2pi(lua_Number x, lua_Number *c, lua_Number *s){
	static const lua_Number one = l_mathop(1.0);
	lua_Number intpart;
	unsigned oct = 0, sgn = 0;
	if(x < 0){ sgn = 1; x = -x; }
	x = l_mathop(modf)(x, &intpart);
	oct = 0x7 & ((unsigned)(8*x));
	switch(oct){
	case 0:
		*s = sin2pi(x);
		*c = l_mathop(sqrt)((one+(*s))*(one-(*s)));
		break;
	case 1:
	case 2:
		*c = sin2pi(l_mathop(0.25)-x);
		*s = l_mathop(sqrt)((one+(*c))*(one-(*c)));
		break;
	case 3:
	case 4:
		*s = sin2pi(l_mathop(0.5)-x);
		*c = -l_mathop(sqrt)((one+(*s))*(one-(*s)));
		break;
	case 5:
	case 6:
		*c = sin2pi(x-l_mathop(0.25));
		*s = -l_mathop(sqrt)((one+(*c))*(one-(*c)));
		break;
	default:
		*s = sin2pi(x-one);
		*c = l_mathop(sqrt)((one+(*s))*(one-(*s)));
		break;
	}
	if(sgn){ *s = -(*s); }
}

