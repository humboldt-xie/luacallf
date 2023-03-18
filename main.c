#include "lua.h"
#include "lauxlib.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    char type;
    void *value;
}lua_output;

int lua_callf(lua_State *L,char *fn,char *format, ...) {
    int top=lua_gettop(L);

    char fnn[255];
    char *strptr;
    strcpy(fnn,fn);

    char *token = strtok_r(fnn, ".", &strptr);
    int level=0;
    while (token != NULL) {
        if(level==0){
            lua_getglobal(L, token);
        }else{
            lua_getfield(L, -1, token);
            lua_remove(L, -2);
        }
        level++;
        token = strtok_r(NULL, ".", &strptr);
    }

    int inputCount=0;
    int outputCount=0;
    lua_output *outputs[255]={0};
    memset(outputs,0,sizeof(lua_output*)*255);

    va_list ap;
    va_start(ap, format);

    while (*format != '\0') {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd':
                    //count += fprintf(stream, "%d", va_arg(arg, int));
                    lua_pushinteger(L, va_arg(ap, int));
                    inputCount++;
                    break;
                case 's':
                    lua_pushstring(L, va_arg(ap, char *));
                    inputCount++;
                    break;
                case 'b':
                    lua_pushboolean(L, va_arg(ap, int));
                    inputCount++;
                    break;
                case 'f':
                    lua_pushnumber(L, va_arg(ap, double));
                    inputCount++;
                    break;
                // 其他格式化指令
                default:
                    break;
            }
        } 
        if (*format == '&'){
            format++;
            outputCount++;
            lua_output *output=malloc(sizeof(lua_output)); 
            switch (*format) {
            case 'd':
                output->type='d';
                output->value= va_arg(ap, int *);
                break;
            case 's':
                output->type='s';
                output->value= va_arg(ap, char *);
                break;
            case 'b':
                output->type='b';
                output->value= va_arg(ap, int *);
                break;
            case 'f':
                output->type='f';
                output->value= va_arg(ap, double *);
                break;
            default:
                break;
            }
            outputs[outputCount-1]=output;
        }
        format++;
    }

    va_end(ap);

    /*char *arg = va_arg(ap, char *);
    lua_pushstring(L, arg);*/
    printf("inputCount:%d %d\n",inputCount,outputCount);

    lua_pcall(L, inputCount, outputCount, 0);

    for(int i=0;i<outputCount;i++){
        lua_type(L, -(outputCount-i));
        switch(outputs[i]->type){
            case 'd':
                *(int*)outputs[i]->value=lua_tointeger(L, -(outputCount-i));
                break;
            case 'f':
                *(double*)outputs[i]->value=lua_tonumber(L, -(outputCount-i));
                break;
            case 's':
                strcpy(outputs[i]->value,lua_tostring(L, -(outputCount-i)));
                break;
            case 'b':
                *(int*)outputs[i]->value=lua_toboolean(L, -(outputCount-i));
                break;
            default:
                break;
        }
    }
    lua_pop(L,outputCount);

    if(lua_gettop(L) != top){
        //luaL_error(L, "lua_callf: stack size changed")
        printf("lua_callf: stack size changed %d %d",top,lua_gettop(L));
        exit(-1);
    }

    for (int i = 0; i < outputCount; i++)
    {
        free(outputs[i]);
    }
    return 0;


}

int main(){
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_dostring(L,
    "function hello(a,b,c,d)\n"
        "print(\"hello args\",a,b,c,d)\n"
        "return a,b,c,d\n"
    "end\n");
    //luaL_dofile(L, "script.lua");
    double f=0.0;
    lua_callf(L, "math.pow", "%d%d&f", 5,2,&f);
    printf("pow %f \n",f);

    int a=0;
    char b[255];
    lua_callf(L, "hello", "%d%s&d&s", 5,"world",&a,b);
    printf("Hello %d %s",a,b);
    lua_close(L);
    return 0;
}