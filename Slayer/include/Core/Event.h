#pragma once

#include "Core/Core.h"

/*
 * Drew insperation for this in The Cherno's implementation of an event system. See link:
 * https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Events/
 */

#define SL_EVENT_TYPE(in_type) static const std::string GetClassType() { return #in_type; }\
							std::string type = #in_type;\
							virtual std::string& GetType() override { return type; }

#define SL_EVENT_DISPATCH(type, func) Slayer::Event::Dispatch<type>(e, std::bind(&func, this, std::placeholders::_1))

#define SL_EVENT_BIND(func) std::bind(&func, this, std::placeholders::_1)


namespace Slayer {

	class Event
	{
	public:
		virtual const std::string& GetType() { return nullptr; };

		template <typename T>
		static bool Dispatch(Event& e, std::function<bool(T&)> func)
		{
			if (e.GetType() == T::GetClassType())
			{
				return func(*(T*)&e);
			}
			return false;
		}
	};

	using EventCallback = std::function<void(Event&)>;
}
