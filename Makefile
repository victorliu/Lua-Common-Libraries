LUA_CFLAGS = `pkg-config lua --cflags`
LCL_INCLUDE = -I.
LCL_LIBNAME = liblcl.a
SHLIB_EXT = so
SHLIB_FLAGS = -bundle -undefined dynamic_lookup
CFLAGS = -Wall

all: $(LCL_LIBNAME) complex.$(SHLIB_EXT)

clean:
	rm -f $(LCL_LIBNAME) complex.$(SHLIB_EXT)

$(LCL_LIBNAME): lcl.h lcl.c lcl_complex.c lcl_complex.h
	$(CC) $(CFLAGS) $(LCL_INCLUDE) $(LUA_CFLAGS) -c lcl.c -o lcl.o
	$(CC) $(CFLAGS) $(LCL_INCLUDE) $(LUA_CFLAGS) -c lcl_complex.c -o lcl_complex.o
	$(AR) rcs $@ lcl.o lcl_complex.o

complex.$(SHLIB_EXT): lcl_complex_lib.c $(LCL_LIBNAME) lcl.c lcl.h
	$(CC) $(CFLAGS) $(LCL_INCLUDE) $(LUA_CFLAGS) -c lcl_complex_lib.c -o lcl_complex_lib.o
	$(CC) $(SHLIB_FLAGS) lcl_complex_lib.o -L. -llcl -o $@
