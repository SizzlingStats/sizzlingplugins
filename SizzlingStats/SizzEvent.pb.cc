// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: SizzEvent.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "SizzEvent.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
// @@protoc_insertion_point(includes)

namespace SizzEvent {

void protobuf_ShutdownFile_SizzEvent_2eproto() {
  delete SizzEvent::default_instance_;
  delete SizzEvent_EventData::default_instance_;
}

#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
void protobuf_AddDesc_SizzEvent_2eproto_impl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

#else
void protobuf_AddDesc_SizzEvent_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

#endif
  SizzEvent::default_instance_ = new SizzEvent();
  SizzEvent_EventData::default_instance_ = new SizzEvent_EventData();
  SizzEvent::default_instance_->InitAsDefaultInstance();
  SizzEvent_EventData::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_SizzEvent_2eproto);
}

#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AddDesc_SizzEvent_2eproto_once_);
void protobuf_AddDesc_SizzEvent_2eproto() {
  ::google::protobuf::::google::protobuf::GoogleOnceInit(&protobuf_AddDesc_SizzEvent_2eproto_once_,
                 &protobuf_AddDesc_SizzEvent_2eproto_impl);
}
#else
// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_SizzEvent_2eproto {
  StaticDescriptorInitializer_SizzEvent_2eproto() {
    protobuf_AddDesc_SizzEvent_2eproto();
  }
} static_descriptor_initializer_SizzEvent_2eproto_;
#endif

// ===================================================================

bool SizzEvent_EventData_DATA_TYPE_IsValid(int value) {
  switch(value) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      return true;
    default:
      return false;
  }
}

#ifndef _MSC_VER
const SizzEvent_EventData_DATA_TYPE SizzEvent_EventData::TYPE_STRING;
const SizzEvent_EventData_DATA_TYPE SizzEvent_EventData::TYPE_FLOAT;
const SizzEvent_EventData_DATA_TYPE SizzEvent_EventData::TYPE_LONG;
const SizzEvent_EventData_DATA_TYPE SizzEvent_EventData::TYPE_SHORT;
const SizzEvent_EventData_DATA_TYPE SizzEvent_EventData::TYPE_BYTE;
const SizzEvent_EventData_DATA_TYPE SizzEvent_EventData::TYPE_BOOL;
const SizzEvent_EventData_DATA_TYPE SizzEvent_EventData::DATA_TYPE_MIN;
const SizzEvent_EventData_DATA_TYPE SizzEvent_EventData::DATA_TYPE_MAX;
const int SizzEvent_EventData::DATA_TYPE_ARRAYSIZE;
#endif  // _MSC_VER
#ifndef _MSC_VER
const int SizzEvent_EventData::kKeyNameFieldNumber;
const int SizzEvent_EventData::kValueTypeFieldNumber;
const int SizzEvent_EventData::kValueStringFieldNumber;
const int SizzEvent_EventData::kValueFloatFieldNumber;
const int SizzEvent_EventData::kValueLongFieldNumber;
const int SizzEvent_EventData::kValueShortFieldNumber;
const int SizzEvent_EventData::kValueByteFieldNumber;
const int SizzEvent_EventData::kValueBoolFieldNumber;
#endif  // !_MSC_VER

SizzEvent_EventData::SizzEvent_EventData()
  : ::google::protobuf::MessageLite() {
  SharedCtor();
}

void SizzEvent_EventData::InitAsDefaultInstance() {
}

SizzEvent_EventData::SizzEvent_EventData(const SizzEvent_EventData& from)
  : ::google::protobuf::MessageLite() {
  SharedCtor();
  MergeFrom(from);
}

void SizzEvent_EventData::SharedCtor() {
  _cached_size_ = 0;
  key_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  value_type_ = 1;
  value_string_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  value_float_ = 0;
  value_long_ = 0;
  value_short_ = 0;
  value_byte_ = 0;
  value_bool_ = false;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

SizzEvent_EventData::~SizzEvent_EventData() {
  SharedDtor();
}

void SizzEvent_EventData::SharedDtor() {
  if (key_name_ != &::google::protobuf::internal::kEmptyString) {
    delete key_name_;
  }
  if (value_string_ != &::google::protobuf::internal::kEmptyString) {
    delete value_string_;
  }
  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  if (this != &default_instance()) {
  #else
  if (this != default_instance_) {
  #endif
  }
}

void SizzEvent_EventData::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const SizzEvent_EventData& SizzEvent_EventData::default_instance() {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  protobuf_AddDesc_SizzEvent_2eproto();
#else
  if (default_instance_ == NULL) protobuf_AddDesc_SizzEvent_2eproto();
#endif
  return *default_instance_;
}

SizzEvent_EventData* SizzEvent_EventData::default_instance_ = NULL;

SizzEvent_EventData* SizzEvent_EventData::New() const {
  return new SizzEvent_EventData;
}

void SizzEvent_EventData::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (has_key_name()) {
      if (key_name_ != &::google::protobuf::internal::kEmptyString) {
        key_name_->clear();
      }
    }
    value_type_ = 1;
    if (has_value_string()) {
      if (value_string_ != &::google::protobuf::internal::kEmptyString) {
        value_string_->clear();
      }
    }
    value_float_ = 0;
    value_long_ = 0;
    value_short_ = 0;
    value_byte_ = 0;
    value_bool_ = false;
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

bool SizzEvent_EventData::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional string key_name = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_key_name()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(16)) goto parse_value_type;
        break;
      }

      // optional .SizzEvent.SizzEvent.EventData.DATA_TYPE value_type = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_value_type:
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::SizzEvent::SizzEvent_EventData_DATA_TYPE_IsValid(value)) {
            set_value_type(static_cast< ::SizzEvent::SizzEvent_EventData_DATA_TYPE >(value));
          }
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(26)) goto parse_value_string;
        break;
      }

      // optional string value_string = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_value_string:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_value_string()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(37)) goto parse_value_float;
        break;
      }

      // optional float value_float = 4;
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED32) {
         parse_value_float:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   float, ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT>(
                 input, &value_float_)));
          set_has_value_float();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(40)) goto parse_value_long;
        break;
      }

      // optional int32 value_long = 5;
      case 5: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_value_long:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &value_long_)));
          set_has_value_long();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(48)) goto parse_value_short;
        break;
      }

      // optional int32 value_short = 6;
      case 6: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_value_short:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &value_short_)));
          set_has_value_short();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(56)) goto parse_value_byte;
        break;
      }

      // optional int32 value_byte = 7;
      case 7: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_value_byte:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &value_byte_)));
          set_has_value_byte();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(64)) goto parse_value_bool;
        break;
      }

      // optional bool value_bool = 8;
      case 8: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_value_bool:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   bool, ::google::protobuf::internal::WireFormatLite::TYPE_BOOL>(
                 input, &value_bool_)));
          set_has_value_bool();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void SizzEvent_EventData::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // optional string key_name = 1;
  if (has_key_name()) {
    ::google::protobuf::internal::WireFormatLite::WriteString(
      1, this->key_name(), output);
  }

  // optional .SizzEvent.SizzEvent.EventData.DATA_TYPE value_type = 2;
  if (has_value_type()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      2, this->value_type(), output);
  }

  // optional string value_string = 3;
  if (has_value_string()) {
    ::google::protobuf::internal::WireFormatLite::WriteString(
      3, this->value_string(), output);
  }

  // optional float value_float = 4;
  if (has_value_float()) {
    ::google::protobuf::internal::WireFormatLite::WriteFloat(4, this->value_float(), output);
  }

  // optional int32 value_long = 5;
  if (has_value_long()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(5, this->value_long(), output);
  }

  // optional int32 value_short = 6;
  if (has_value_short()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(6, this->value_short(), output);
  }

  // optional int32 value_byte = 7;
  if (has_value_byte()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(7, this->value_byte(), output);
  }

  // optional bool value_bool = 8;
  if (has_value_bool()) {
    ::google::protobuf::internal::WireFormatLite::WriteBool(8, this->value_bool(), output);
  }

}

int SizzEvent_EventData::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // optional string key_name = 1;
    if (has_key_name()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->key_name());
    }

    // optional .SizzEvent.SizzEvent.EventData.DATA_TYPE value_type = 2;
    if (has_value_type()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->value_type());
    }

    // optional string value_string = 3;
    if (has_value_string()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->value_string());
    }

    // optional float value_float = 4;
    if (has_value_float()) {
      total_size += 1 + 4;
    }

    // optional int32 value_long = 5;
    if (has_value_long()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->value_long());
    }

    // optional int32 value_short = 6;
    if (has_value_short()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->value_short());
    }

    // optional int32 value_byte = 7;
    if (has_value_byte()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->value_byte());
    }

    // optional bool value_bool = 8;
    if (has_value_bool()) {
      total_size += 1 + 1;
    }

  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void SizzEvent_EventData::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const SizzEvent_EventData*>(&from));
}

void SizzEvent_EventData::MergeFrom(const SizzEvent_EventData& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_key_name()) {
      set_key_name(from.key_name());
    }
    if (from.has_value_type()) {
      set_value_type(from.value_type());
    }
    if (from.has_value_string()) {
      set_value_string(from.value_string());
    }
    if (from.has_value_float()) {
      set_value_float(from.value_float());
    }
    if (from.has_value_long()) {
      set_value_long(from.value_long());
    }
    if (from.has_value_short()) {
      set_value_short(from.value_short());
    }
    if (from.has_value_byte()) {
      set_value_byte(from.value_byte());
    }
    if (from.has_value_bool()) {
      set_value_bool(from.value_bool());
    }
  }
}

void SizzEvent_EventData::CopyFrom(const SizzEvent_EventData& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool SizzEvent_EventData::IsInitialized() const {

  return true;
}

void SizzEvent_EventData::Swap(SizzEvent_EventData* other) {
  if (other != this) {
    std::swap(key_name_, other->key_name_);
    std::swap(value_type_, other->value_type_);
    std::swap(value_string_, other->value_string_);
    std::swap(value_float_, other->value_float_);
    std::swap(value_long_, other->value_long_);
    std::swap(value_short_, other->value_short_);
    std::swap(value_byte_, other->value_byte_);
    std::swap(value_bool_, other->value_bool_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::std::string SizzEvent_EventData::GetTypeName() const {
  return "SizzEvent.SizzEvent.EventData";
}


// -------------------------------------------------------------------

#ifndef _MSC_VER
const int SizzEvent::kEventTimestampFieldNumber;
const int SizzEvent::kEventNameFieldNumber;
const int SizzEvent::kEventDataFieldNumber;
#endif  // !_MSC_VER

SizzEvent::SizzEvent()
  : ::google::protobuf::MessageLite() {
  SharedCtor();
}

void SizzEvent::InitAsDefaultInstance() {
}

SizzEvent::SizzEvent(const SizzEvent& from)
  : ::google::protobuf::MessageLite() {
  SharedCtor();
  MergeFrom(from);
}

void SizzEvent::SharedCtor() {
  _cached_size_ = 0;
  event_timestamp_ = 0u;
  event_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

SizzEvent::~SizzEvent() {
  SharedDtor();
}

void SizzEvent::SharedDtor() {
  if (event_name_ != &::google::protobuf::internal::kEmptyString) {
    delete event_name_;
  }
  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  if (this != &default_instance()) {
  #else
  if (this != default_instance_) {
  #endif
  }
}

void SizzEvent::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const SizzEvent& SizzEvent::default_instance() {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  protobuf_AddDesc_SizzEvent_2eproto();
#else
  if (default_instance_ == NULL) protobuf_AddDesc_SizzEvent_2eproto();
#endif
  return *default_instance_;
}

SizzEvent* SizzEvent::default_instance_ = NULL;

SizzEvent* SizzEvent::New() const {
  return new SizzEvent;
}

void SizzEvent::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    event_timestamp_ = 0u;
    if (has_event_name()) {
      if (event_name_ != &::google::protobuf::internal::kEmptyString) {
        event_name_->clear();
      }
    }
  }
  event_data_.Clear();
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

bool SizzEvent::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional uint32 event_timestamp = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &event_timestamp_)));
          set_has_event_timestamp();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(18)) goto parse_event_name;
        break;
      }

      // optional string event_name = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_event_name:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_event_name()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(26)) goto parse_event_data;
        break;
      }

      // repeated .SizzEvent.SizzEvent.EventData event_data = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_event_data:
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
                input, add_event_data()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(26)) goto parse_event_data;
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void SizzEvent::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // optional uint32 event_timestamp = 1;
  if (has_event_timestamp()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(1, this->event_timestamp(), output);
  }

  // optional string event_name = 2;
  if (has_event_name()) {
    ::google::protobuf::internal::WireFormatLite::WriteString(
      2, this->event_name(), output);
  }

  // repeated .SizzEvent.SizzEvent.EventData event_data = 3;
  for (int i = 0; i < this->event_data_size(); i++) {
    ::google::protobuf::internal::WireFormatLite::WriteMessage(
      3, this->event_data(i), output);
  }

}

int SizzEvent::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // optional uint32 event_timestamp = 1;
    if (has_event_timestamp()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->event_timestamp());
    }

    // optional string event_name = 2;
    if (has_event_name()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->event_name());
    }

  }
  // repeated .SizzEvent.SizzEvent.EventData event_data = 3;
  total_size += 1 * this->event_data_size();
  for (int i = 0; i < this->event_data_size(); i++) {
    total_size +=
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        this->event_data(i));
  }

  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void SizzEvent::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const SizzEvent*>(&from));
}

void SizzEvent::MergeFrom(const SizzEvent& from) {
  GOOGLE_CHECK_NE(&from, this);
  event_data_.MergeFrom(from.event_data_);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_event_timestamp()) {
      set_event_timestamp(from.event_timestamp());
    }
    if (from.has_event_name()) {
      set_event_name(from.event_name());
    }
  }
}

void SizzEvent::CopyFrom(const SizzEvent& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool SizzEvent::IsInitialized() const {

  return true;
}

void SizzEvent::Swap(SizzEvent* other) {
  if (other != this) {
    std::swap(event_timestamp_, other->event_timestamp_);
    std::swap(event_name_, other->event_name_);
    event_data_.Swap(&other->event_data_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::std::string SizzEvent::GetTypeName() const {
  return "SizzEvent.SizzEvent";
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace SizzEvent

// @@protoc_insertion_point(global_scope)
