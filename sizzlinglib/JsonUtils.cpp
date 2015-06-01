
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "JsonUtils.h"
#include <rapidjson/writer.h>

namespace json
{
	struct JsonContext
	{
		rapidjson::StringBuffer stringBuf;
		rapidjson::Writer<rapidjson::StringBuffer> jsonWriter;
	};

	JsonWriter::JsonWriter():
		m_context(new JsonContext)
	{
		m_context->jsonWriter.Reset(m_context->stringBuf);
	}

	JsonWriter::~JsonWriter()
	{
		delete m_context;
	}

	void JsonWriter::StartObject(const char* name /*= nullptr*/)
	{
		auto& writer = m_context->jsonWriter;
		if (name)
		{
			writer.String(name);
		}
		writer.StartObject();
	}

	void JsonWriter::EndObject()
	{
		m_context->jsonWriter.EndObject();
	}

	void JsonWriter::StartArray(const char* name /*= nullptr*/)
	{
		auto& writer = m_context->jsonWriter;
		if (name)
		{
			writer.String(name);
		}
		writer.StartArray();
	}

	void JsonWriter::EndArray()
	{
		m_context->jsonWriter.EndArray();
	}

	void JsonWriter::InsertKV(const char* key, const char* value)
	{
		auto& writer = m_context->jsonWriter;
		writer.String(key);
		writer.String(value);
	}

	void JsonWriter::InsertKV(const char* key, std::int32_t value)
	{
		auto& writer = m_context->jsonWriter;
		writer.String(key);
		writer.Int(value);
	}

	void JsonWriter::InsertKV(const char* key, std::uint32_t value)
	{
		auto& writer = m_context->jsonWriter;
		writer.String(key);
		writer.Uint(value);
	}

	void JsonWriter::InsertKV(const char* key, std::uint64_t value)
	{
		auto& writer = m_context->jsonWriter;
		writer.String(key);
		writer.Uint64(value);
	}

	const char* JsonWriter::GetJsonString() const
	{
		return m_context->stringBuf.GetString();
	}
}
