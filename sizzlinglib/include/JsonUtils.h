
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#pragma once

#include <cstdint>

namespace json
{
	struct JsonContext;

	class JsonWriter
	{
	public:
		JsonWriter();
		~JsonWriter();
		
		void StartObject(const char* name = nullptr);
		void EndObject();

		void StartArray(const char* name = nullptr);
		void EndArray();

		void InsertKV(const char* key, const char* value);
		void InsertKV(const char* key, std::int32_t value);
		void InsertKV(const char* key, std::uint32_t value);
		void InsertKV(const char* key, std::uint64_t value);

		const char* GetJsonString() const;

	private:
		JsonContext* m_context;
	};
}
