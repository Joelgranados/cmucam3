/*
** $Id: lparser.c,v 2.42a 2006/06/05 15:57:59 roberto Exp $
** Lua Parser
** See Copyright Notice in lua.h
*/


#define lparser_c
#define LUA_CORE

#include "lua.h"

#include "lparser.h"

Proto *luaY_parser (lua_State *L, ZIO *z, Mbuffer *buff, const char *name) {
  UNUSED(z);
  UNUSED(buff);
  UNUSED(name);
  lua_pushliteral(L,"parser not loaded");
  lua_error(L);
  return NULL;
}

/* }====================================================================== */
