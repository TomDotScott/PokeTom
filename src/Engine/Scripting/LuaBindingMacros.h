#ifndef LUABINDINGMACROS_H
#define LUABINDINGMACROS_H

#define LUA_API_BEGIN(TableName, LuaState) \
	{ \
		sol::table TableName##_table = (LuaState).create_named_table(#TableName);

#define LUA_FUNC(TableName, FuncName, Lambda) \
	TableName##_table.set_function(#FuncName, Lambda);

#define LUA_CONST(TableName, ConstName, Value) \
	TableName##_table[#ConstName] = (Value);

#define LUA_API_END() \
	}

#endif