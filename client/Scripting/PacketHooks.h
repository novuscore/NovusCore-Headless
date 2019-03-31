#pragma once
#include "../Utils/DebugHandler.h"
#include "../Utils/Any.h"
#include "ScriptEngine.h"
#include "AngelBinder.h"
#include <array>
#include <memory>

class PacketHooks
{
public:
	enum Hooks
	{
		HOOK_ONLOGIN_CHALLENGE,

		COUNT
	};

	inline static void Register(Hooks id, asIScriptFunction* func)
	{
		_hooks[id].push_back(func);
	}

	inline static std::vector<asIScriptFunction*>& GetHooks(Hooks id)
	{
		return _hooks[id];
	}

	template <typename... Args>
	inline static void CallHook(Hooks id, Args... args)
	{
		for (auto function : _hooks[id])
		{
			AngelBinder::Context* context = ScriptEngine::GetScriptContext();
			if (context)
			{
				context->prepare(function);

				std::vector<any> arguments = { args... };
				for (auto argument : arguments)
				{
					switch (argument.get_type())
					{
						case any::U8: context->setByte(argument.get_u8()); break;
						case any::U16: context->setWord(argument.get_u16()); break;
						case any::U32: context->setDWord(argument.get_u32()); break;
						case any::U64: context->setQWord(argument.get_u64()); break;
						case any::I8: context->setByte(argument.get_i8()); break;
						case any::I16: context->setWord(argument.get_i16()); break;
						case any::I32: context->setDWord(argument.get_i32()); break;
						case any::I64: context->setQWord(argument.get_i64()); break;
						case any::F32: context->setFloat(argument.get_f32()); break;
						case any::F64: context->setDouble(argument.get_f64()); break;
						case any::Bool: context->setBool(argument.get_bool()); break;
					}
				}
				
				context->execute();
				context->release();
			}
		}
		return;
	}

private:
	static std::array<std::vector<asIScriptFunction*>, Hooks::COUNT> _hooks;
};