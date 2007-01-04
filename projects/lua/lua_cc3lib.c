#include <stdlib.h>
#include <stdio.h>

#include "cc3.h"

#include "lua.h"
#include "lauxlib.h"

#include "lua_cc3lib.h"



static int cc3_load (lua_State *L) {
  cc3_pixbuf_load();
  return 0;
}

static int cc3_rewind (lua_State *L) {
  cc3_pixbuf_rewind();
  return 0;
}




static const luaL_Reg cc3lib[] = {
  {"load", cc3_load},
  {"rewind", cc3_rewind},
  {NULL, NULL}
};

int open_cc3lib (lua_State *L) {
  luaL_register(L, "cc3", cc3lib);

  return 1;
}
