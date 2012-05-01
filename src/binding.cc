#include <v8.h>
#include <node.h>
#include <node_buffer.h>

using namespace v8;
using namespace node;

namespace {


static Persistent<Object> null_pointer_buffer;

/*
 * Returns the pointer address as a Number of the given Buffer instance
 *
 * args[0] - Buffer - the Buffer instance get the memory address of
 * args[1] - Number - optional (0) - the offset of the Buffer start at
 */

Handle<Value> Address(const Arguments& args) {
  HandleScope scope;

  Local<Value> buf = args[0];
  if (!Buffer::HasInstance(buf)) {
    return ThrowException(Exception::TypeError(
          String::New("address: Buffer instance expected")));
  }

  size_t address = (size_t)Buffer::Data(buf.As<Object>());
  Local<Number> ret = Number::New(address);

  return scope.Close(ret);
}

/*
 * Returns "true" if the given Buffer points to NULL, "false" otherwise.
 *
 * args[0] - Buffer - the Buffer instance to check for NULL
 * args[1] - Number - optional (0) - the offset of the Buffer start at
 */

Handle<Value> IsNull(const Arguments& args) {
  HandleScope scope;

  Local<Value> buf = args[0];
  if (!Buffer::HasInstance(buf)) {
    return ThrowException(Exception::TypeError(
          String::New("isNull: Buffer instance expected")));
  }

  bool isNull = Buffer::Data(buf.As<Object>()) == NULL;
  Handle<Value> ret = Boolean::New(isNull);

  return scope.Close(ret);
}

/*
 * A callback that should never be invoked since the NULL pointer
 * wrapper Buffer should never be collected
 */

void unref_null_cb(char *data, void *hint) {
  assert(0 && "NULL Buffer should never be garbage collected");
  fprintf(stderr, "FATAL: NULL Buffer should never be garbage collected");
}

/*
 * Creates the "null_pointer_buffer" Buffer instance that points to NULL.
 * It has a length of 0 so that you don't accidentally try to deref the NULL
 * pointer in JS-land by doing something like: `ref.NULL[0]`.
 */

Persistent<Object> WrapNullPointer () {
  size_t buf_size = 0;
  char *ptr = (char *)NULL;
  void *user_data = NULL;
  Buffer *buf = Buffer::New(ptr, buf_size, unref_null_cb, user_data);
  null_pointer_buffer = Persistent<Object>::New(buf->handle_);
  return null_pointer_buffer;
}

/*
 * Retreives a JS Object instance that was previously stored in
 * the given Buffer instance at the given offset.
 */

Handle<Value> ReadObject(const Arguments& args) {
  HandleScope scope;

  Local<Value> buf = args[0];
  if (!Buffer::HasInstance(buf)) {
    return ThrowException(Exception::TypeError(
          String::New("address: Buffer instance expected")));
  }

  size_t offset = args[1]->Uint32Value();
  char *ptr = Buffer::Data(buf.As<Object>()) + offset;
  Persistent<Value> rtn = *reinterpret_cast<Persistent<Value>*>(ptr);

  return scope.Close(rtn);
}

/*
 * Writes a Persistent reference to given Object to the given Buffer
 * instance and offset.
 */

Handle<Value> WriteObject(const Arguments& args) {
  HandleScope scope;

  Local<Value> buf = args[0];
  if (!Buffer::HasInstance(buf)) {
    return ThrowException(Exception::TypeError(
          String::New("address: Buffer instance expected")));
  }

  size_t offset = args[1]->Uint32Value();
  char *ptr = Buffer::Data(buf.As<Object>()) + offset;

  Local<Value> obj = args[2];
  *reinterpret_cast<Persistent<Value>*>(ptr) = Persistent<Value>::New(obj);

  return Undefined();
}



} // anonymous namespace

void init (Handle<Object> target) {
  HandleScope scope;

  // "sizeof" map
  Local<Object> smap = Object::New();
  smap->Set(String::NewSymbol("byte"),      Integer::New(sizeof(unsigned char)));
  smap->Set(String::NewSymbol("int8"),      Integer::New(sizeof(int8_t)));
  smap->Set(String::NewSymbol("uint8"),     Integer::New(sizeof(uint8_t)));
  smap->Set(String::NewSymbol("int16"),     Integer::New(sizeof(int16_t)));
  smap->Set(String::NewSymbol("uint16"),    Integer::New(sizeof(uint16_t)));
  smap->Set(String::NewSymbol("int32"),     Integer::New(sizeof(int32_t)));
  smap->Set(String::NewSymbol("uint32"),    Integer::New(sizeof(uint32_t)));
  smap->Set(String::NewSymbol("int64"),     Integer::New(sizeof(int64_t)));
  smap->Set(String::NewSymbol("uint64"),    Integer::New(sizeof(uint64_t)));
  smap->Set(String::NewSymbol("char"),      Integer::New(sizeof(char)));
  smap->Set(String::NewSymbol("uchar"),     Integer::New(sizeof(unsigned char)));
  smap->Set(String::NewSymbol("short"),     Integer::New(sizeof(short)));
  smap->Set(String::NewSymbol("ushort"),    Integer::New(sizeof(unsigned short)));
  smap->Set(String::NewSymbol("int"),       Integer::New(sizeof(int)));
  smap->Set(String::NewSymbol("uint"),      Integer::New(sizeof(unsigned int)));
  smap->Set(String::NewSymbol("long"),      Integer::New(sizeof(long)));
  smap->Set(String::NewSymbol("ulong"),     Integer::New(sizeof(unsigned long)));
  smap->Set(String::NewSymbol("longlong"),  Integer::New(sizeof(long long)));
  smap->Set(String::NewSymbol("ulonglong"), Integer::New(sizeof(unsigned long long)));
  smap->Set(String::NewSymbol("float"),     Integer::New(sizeof(float)));
  smap->Set(String::NewSymbol("double"),    Integer::New(sizeof(double)));
  smap->Set(String::NewSymbol("pointer"),   Integer::New(sizeof(unsigned char *)));
  //smap->Set(String::NewSymbol("string"),    Integer::New(sizeof(char *)));
  smap->Set(String::NewSymbol("size_t"),    Integer::New(sizeof(size_t)));
  // size of a Persistent handle to a JS object
  smap->Set(String::NewSymbol("Object"),    Integer::New(sizeof(Persistent<Object>)));


  // exports
  target->Set(String::NewSymbol("sizeof"), smap);
  target->Set(String::NewSymbol("NULL"), WrapNullPointer());
  NODE_SET_METHOD(target, "address", Address);
  NODE_SET_METHOD(target, "isNull", IsNull);
  NODE_SET_METHOD(target, "readObject", ReadObject);
  NODE_SET_METHOD(target, "writeObject", WriteObject);
}
NODE_MODULE(binding, init);
