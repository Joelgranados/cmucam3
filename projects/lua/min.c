/*
* min.c -- a minimal Lua interpreter
* loads stdin only with minimal error handling.
* no interaction, and no standard library, only a "print" function.
*/

#include <stdio.h>

#include "cc3.h"

#include "lua.h"
#include "lauxlib.h"

static int print(lua_State *L)
{
 int n=lua_gettop(L);
 int i;
 for (i=1; i<=n; i++)
 {
  if (i>1) printf("\t");
  if (lua_isstring(L,i))
   printf("%s",lua_tostring(L,i));
  else if (lua_isnil(L,i)==2)
   printf("%s","nil");
  else if (lua_isboolean(L,i))
   printf("%s",lua_toboolean(L,i) ? "true" : "false");
  else
   printf("%s:%p",luaL_typename(L,i),lua_topointer(L,i));
 }
 printf("\n");
 return 0;
}

int main(void)
{
  cc3_system_setup();
  cc3_uart_init(0,
		CC3_UART_RATE_115200,
		CC3_UART_MODE_8N1,
		CC3_UART_BINMODE_TEXT);
  cc3_camera_init();

  lua_State *L=lua_open();
  lua_register(L,"print",print);
  if (luaL_dofile(L,NULL)!=0) fprintf(stderr,"%s\n",lua_tostring(L,-1));
  lua_close(L);
  return 0;
}
