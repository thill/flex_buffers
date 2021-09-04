#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "flexbuf/flexbuf.h"

using namespace flexbuf;

TEST_CASE("Buffer::wrap(shared_ptr<char[]>)") {
  std::shared_ptr<char[]> src{new char[5]};
  src[0] = 'a';
  src[1] = 'b';
  src[2] = 'c';
  src[3] = 'd';
  src[4] = 'e';
  auto buf = Buffer::wrap(src, 1, 3);
  REQUIRE(src.use_count() == 2);
  REQUIRE(buf.str() == "bcd");
  buf[0] = '1';
  buf[1] = '2';
  buf[2] = '3';
  REQUIRE(buf.str() == "123");
  REQUIRE(src[0] == 'a');
  REQUIRE(src[1] == '1');
  REQUIRE(src[2] == '2');
  REQUIRE(src[3] == '3');
  REQUIRE(src[4] == 'e');
}

TEST_CASE("Buffer::wrap(span<uint16_t>)") {
  std::shared_ptr<uint16_t[]> arr{new uint16_t[2]};
  arr.get()[0] = 257;
  arr.get()[1] = 258;
  std::span<uint16_t> src{arr.get(), 2};
  auto buf = Buffer::wrap(src);
  REQUIRE(buf.size() == 4);
  REQUIRE(buf[0] + buf[1] == 2);
  REQUIRE(buf[2] + buf[3] == 3);
}

TEST_CASE("Buffer::wrap(span<const uint16_t, size>)") {
  std::shared_ptr<uint16_t[]> arr{new uint16_t[2]};
  arr.get()[0] = 257;
  arr.get()[1] = 258;
  std::span<const uint16_t, 2> src{arr.get(), 2};
  auto buf = Buffer::wrap(src);
  REQUIRE(buf.size() == 4);
  REQUIRE(buf[0] + buf[1] == 2);
  REQUIRE(buf[2] + buf[3] == 3);
}

TEST_CASE("Buffer.data() from wrapped raw pointer") {
  std::string src{"hello world!"};
  auto buf = Buffer::wrap(src);
  REQUIRE(buf.data()[0] == 'h');
  REQUIRE(buf.data()[2] == 'l');
  REQUIRE(buf.data()[11] == '!');
}

TEST_CASE("Buffer.data() from wrapped shared pointer") {
  std::shared_ptr<char[]> src{new char[3]};
  src[0] = 'a';
  src[1] = 'b';
  src[2] = 'c';
  auto buf = Buffer::wrap(src, 0, 3);
  REQUIRE(src.use_count() == 2);
  REQUIRE(buf.data()[0] == 'a');
  REQUIRE(buf.data()[1] == 'b');
  REQUIRE(buf.data()[2] == 'c');
  buf.data()[0] = '1';
  buf.data()[1] = '2';
  buf.data()[2] = '3';
  // wrapped original source changed
  REQUIRE(src[0] == '1');
  REQUIRE(src[1] == '2');
  REQUIRE(src[2] == '3');
}

TEST_CASE("Buffer.size()") {
  std::string src{"hello world!"};
  auto buf = Buffer::wrap(src);
  REQUIRE(buf.size() == 12);
}

TEST_CASE("Buffer.operator[]") {
  char* src = new char[3];
  src[0] = 'a';
  src[1] = 'b';
  src[2] = 'c';
  auto buf = Buffer::wrap(src, 0, 3);
  REQUIRE(buf[0] == 'a');
  REQUIRE(buf[1] == 'b');
  REQUIRE(buf[2] == 'c');
  delete[] src;
}

TEST_CASE("Buffer.read<>()") {
  char* src = new char[3];
  src[0] = 255;
  src[1] = 1;
  src[2] = 1;
  auto buf = Buffer::wrap(src, 0, 3);
  REQUIRE(buf.read<uint16_t>(1) == static_cast<uint16_t>(257));
}

TEST_CASE("Buffer.span()") {
  std::string src{"hello world!"};
  auto buf = Buffer::wrap(src);
  auto span = buf.span(6);
  REQUIRE(span.str() == "world!");
}

TEST_CASE("Buffer.span() does not dangle") {
  std::string src{"hello world!"};
  auto buf = std::make_shared<Buffer>(Buffer::wrap(src));
  auto span = buf->span(6);
  buf.reset();
  REQUIRE(span.str() == "world!");
}

TEST_CASE("Buffer.hex()") {
  std::shared_ptr<char[]> src{new char[4]};
  src[0] = 1;
  src[1] = 7;
  src[2] = 10;
  src[3] = 33;
  auto span = Buffer::wrap(src, 0, 4);
  REQUIRE(span.hex() == "0x01070a21");
  std::ostringstream oss;
  oss << Buffer::copy_of(span);
  REQUIRE(oss.str() == "0x01070a21");
}

TEST_CASE("Buffer.span() of span") {
  std::shared_ptr<char[]> src{new char[4]};
  src[0] = 1;
  src[1] = 7;
  src[2] = 10;
  src[3] = 33;
  auto span = Buffer::wrap(src, 0, 4);
  auto span1 = span.span(1, 2);
  auto span2 = span1.span(1, 1);
  REQUIRE(span1[0] == 7);
  REQUIRE(span1[1] == 10);
  REQUIRE(span2[0] == 10);
}

TEST_CASE("Buffer(const Buffer& buffer)") {
  std::shared_ptr<char[]> src{new char[3]};
  src[0] = 'a';
  src[1] = 'b';
  src[2] = 'c';
  auto buf = Buffer::copy_of(src.get(), 0, 3);
  Buffer copy{buf};
  copy[1] = '2';
  REQUIRE(buf.str() == "abc");
  REQUIRE(copy.str() == "a2c");
}

TEST_CASE("Buffer& operator=(const Buffer& buffer)") {
  std::shared_ptr<char[]> src{new char[3]};
  src[0] = 'a';
  src[1] = 'b';
  src[2] = 'c';
  auto buf = Buffer::copy_of(src.get(), 0, 3);
  Buffer copy;
  copy = buf;
  copy[1] = '2';
  REQUIRE(buf.str() == "abc");
  REQUIRE(copy.str() == "a2c");
}

TEST_CASE("Buffer.data() from copy") {
  std::shared_ptr<char[]> src{new char[3]};
  src[0] = 'a';
  src[1] = 'b';
  src[2] = 'c';
  auto buf = Buffer::copy_of(src.get(), 0, 3);
  REQUIRE(buf.data()[0] == 'a');
  REQUIRE(buf.data()[1] == 'b');
  REQUIRE(buf.data()[2] == 'c');
  buf.data()[0] = '1';
  buf.data()[1] = '2';
  buf.data()[2] = '3';
  REQUIRE(buf.data()[0] == '1');
  REQUIRE(buf.data()[1] == '2');
  REQUIRE(buf.data()[2] == '3');
  // copied original source not changed
  REQUIRE(src[0] == 'a');
  REQUIRE(src[1] == 'b');
  REQUIRE(src[2] == 'c');
}

TEST_CASE("Buffer.operator[](index)") {
  std::string src{"hello world!"};
  auto buf = Buffer::copy_of(src);
  buf[0] = 'H';
  buf[6] = 'W';
  REQUIRE(buf[0] == 'H');
  REQUIRE(buf[2] == 'l');
  REQUIRE(buf[6] == 'W');
  REQUIRE(buf[11] == '!');
}

TEST_CASE("Buffer to std::span") {
  auto buf = Buffer::allocate(3);
  std::span<char> span = buf;
  REQUIRE(span.size() == 3);
  span[0] = 'a';
  span[1] = 'b';
  span[2] = 'c';
  REQUIRE(buf[0] == 'a');
  REQUIRE(buf[1] == 'b');
  REQUIRE(buf[2] == 'c');
}

TEST_CASE("Buffer to std::span const") {
  auto buf = Buffer::allocate(3);
  buf[0] = 'a';
  buf[1] = 'b';
  buf[2] = 'c';
  std::span<const char> span = buf;
  REQUIRE(span.size() == 3);
  REQUIRE(span[0] == 'a');
  REQUIRE(span[1] == 'b');
  REQUIRE(span[2] == 'c');
}

TEST_CASE("Buffer.write<>(value)") {
  auto buf = Buffer::allocate(4);
  buf.write<uint32_t>(12345, 0);
  REQUIRE(buf.read<uint32_t>(0) == 12345);
}

TEST_CASE("Buffer.write<>(span)") {
  auto buf = Buffer::allocate(8);
  std::vector<uint32_t> vec;
  vec.push_back(12345);
  vec.push_back(67890);
  std::span<uint32_t> span{vec.data(), vec.size()};
  buf.write<uint32_t>(span, 0);
  REQUIRE(buf.read<uint32_t>(0) == 12345);
  REQUIRE(buf.read<uint32_t>(4) == 67890);
}

TEST_CASE("Buffer.write(Buffer&)") {
  auto src1 = Buffer::copy_of(std::string{"my "});
  auto src2 = Buffer::copy_of(std::string{"hello world!"});
  auto buf = Buffer::allocate(8);
  buf.write(src1);
  buf.write(src2.span(6, 5), 3);
  REQUIRE(buf.str() == "my world");
}

TEST_CASE("Buffer.ref<>()") {
  auto buf = Buffer::allocate(8);
  buf.write<uint32_t>(12345, 0);
  buf.write<uint32_t>(67890, 4);
  REQUIRE(buf.ref<uint32_t>(0) == 12345);
  REQUIRE(buf.ref<uint32_t>(4) == 67890);
  buf.ref<uint32_t>(0) = uint32_t{11111};
  buf.ref<uint32_t>(4) = uint32_t{22222};
  REQUIRE(buf.read<uint32_t>(0) == 11111);
  REQUIRE(buf.read<uint32_t>(4) == 22222);
}

TEST_CASE("Buffer.span() shallow") {
  std::string str{"hello world!"};
  auto buf = Buffer::copy_of(str);
  auto span1 = buf.span();
  auto span2 = buf.span(6);
  buf[0] = 'H';
  REQUIRE(buf.str() == "Hello world!");
  REQUIRE(span1.str() == "Hello world!");
  REQUIRE(span2.str() == "world!");
  span1[6] = 'W';
  span2[5] = '?';
  REQUIRE(buf.str() == "Hello World?");
  REQUIRE(span1.str() == "Hello World?");
  REQUIRE(span2.str() == "World?");
}

TEST_CASE("Buffer.copy() deep") {
  std::string str{"hello world!"};
  auto buf = Buffer::copy_of(str);
  auto copy1 = buf.copy();
  auto copy2 = buf.copy(6);
  auto copy3 = buf.copy(6, 3);
  buf[0] = 'H';
  copy1[0] = ' ';
  copy2[0] = 'W';
  REQUIRE(buf.str() == "Hello world!");
  REQUIRE(copy1.str() == " ello world!");
  REQUIRE(copy2.str() == "World!");
  REQUIRE(copy3.str() == "wor");
}

TEST_CASE("Buffer.clear()") {
  std::string src{"hello!"};
  auto buf = Buffer::copy_of(src);
  REQUIRE(buf.str() == "hello!");
  buf.clear();
  REQUIRE(buf.str() == "\0\0\0\0\0\0");
}

TEST_CASE("FlexBuffer(const FlexBuffer&) deep") {
  FlexBuffer buf{8};
  buf << "hello world!";
  FlexBuffer copy;
  copy = buf;
  REQUIRE(copy.capacity() == 16);
  REQUIRE(copy.initial_capacity() == 8);
  buf[0] = 'H';
  REQUIRE(buf.str() == "Hello world!");
  REQUIRE(copy.str() == "hello world!");
  copy[6] = 'W';
  REQUIRE(buf.str() == "Hello world!");
  REQUIRE(copy.str() == "hello World!");
}

TEST_CASE("FlexBuffer operator=(const FlexBuffer&) deep") {
  FlexBuffer buf{128};
  buf << "hello world!";
  FlexBuffer copy;
  copy = buf;
  REQUIRE(copy.capacity() == 128);
  REQUIRE(copy.initial_capacity() == 128);
  buf[0] = 'H';
  REQUIRE(buf.str() == "Hello world!");
  REQUIRE(copy.str() == "hello world!");
  copy[6] = 'W';
  REQUIRE(buf.str() == "Hello world!");
  REQUIRE(copy.str() == "hello World!");
}

TEST_CASE("FlexBuffer.span() does not dangle") {
  auto buf = std::make_shared<FlexBuffer>();
  *buf << "hello world!";
  auto span = buf->span(6);
  buf.reset();
  REQUIRE(span.str() == "world!");
}

TEST_CASE("FlexBuffer.capacity()") {
  FlexBuffer buf{8};
  REQUIRE(buf.capacity() == 8);
  REQUIRE(buf.initial_capacity() == 8);
  buf << "hello world!";
  REQUIRE(buf.capacity() == 16);
  REQUIRE(buf.initial_capacity() == 8);
}

TEST_CASE("FlexBuffer.clear()") {
  FlexBuffer buf;
  buf << "hello!!!";
  buf.resize(4);
  buf.clear();
  buf.resize(8);
  REQUIRE(buf.str() == "\0\0\0\0o!!!");
}

TEST_CASE("FlexBuffer.clear_all()") {
  FlexBuffer buf;
  buf << "hello!!!";
  buf.resize(4);
  buf.clear_all();
  buf.resize(8);
  REQUIRE(buf.str() == "\0\0\0\0\0\0\0\0");
}

TEST_CASE("FlexBuffer.copy_from(index, size)") {
  FlexBuffer buf;
  buf << "hello world!";
  auto copy = buf.copy(4, 3);
  buf.resize(0);
  REQUIRE(copy.str() == "o w");
}

TEST_CASE("FlexBuffer.copy_from(index, size) out of bounds") {
  FlexBuffer buf;
  buf << "hello world!";
  REQUIRE_THROWS(buf.copy(6, 7), "array index out of bounds");
}

TEST_CASE("FlexBuffer.flex_copy()") {
  FlexBuffer buf{8};
  buf << "hello world!";
  auto copy1 = buf.flex_copy();
  auto copy2 = buf.flex_copy(6);
  auto copy3 = buf.flex_copy(6, 3);
  buf.resize(0);
  REQUIRE(copy1.str() == "hello world!");
  REQUIRE(copy2.str() == "world!");
  REQUIRE(copy3.str() == "wor");
  REQUIRE(copy1.capacity() == 16);
  REQUIRE(copy1.initial_capacity() == 8);
  REQUIRE(copy2.capacity() == 8);
  REQUIRE(copy2.initial_capacity() == 8);
  REQUIRE(copy3.capacity() == 8);
  REQUIRE(copy3.initial_capacity() == 8);
}

TEST_CASE("FlexBuffer.resize(shrink, KeepData)") {
  FlexBuffer buf{0};
  buf << "hello world!";
  buf.resize(5);
  REQUIRE(buf.size() == 5);
  REQUIRE(buf.str() == "hello");
}

TEST_CASE("FlexBuffer.resize(shrink, IgnoreData)") {
  FlexBuffer buf{0};
  buf << "hello world!";
  buf.resize(5, ResizeMode::IgnoreData);
  REQUIRE(buf.size() == 5);
  REQUIRE(buf.str() != "hello");
}

TEST_CASE("FlexBuffer.resize(grow, KeepData)") {
  FlexBuffer buf;
  buf << "hello world!";
  buf.resize(100);
  REQUIRE(buf.size() == 100);
  REQUIRE(buf.span(0, 12).str() == "hello world!");
}

TEST_CASE("FlexBuffer.resize(grow, IgnoreData)") {
  FlexBuffer buf;
  buf << "hello world!";
  buf.resize(100, ResizeMode::IgnoreData);
  REQUIRE(buf.size() == 100);
  REQUIRE(buf.span(0, 12).str() != "hello world!");
}

TEST_CASE("FlexBuffer.reserve(size)") {
  FlexBuffer buf;
  auto span1 = buf.reserve(2);
  auto span2 = buf.reserve(2);
  span1[0] = 'a';
  span1[1] = 'b';
  span2[0] = 'c';
  span2[1] = 'd';
  REQUIRE(buf.str() == "abcd");
}

TEST_CASE("FlexBuffer.reserve(size) spans do not dangle") {
  FlexBuffer buf;
  auto span1 = buf.reserve(2);
  auto span2 = buf.reserve(2);
  buf.resize(100);
  buf[0] = '1';
  buf[1] = '2';
  buf[2] = '3';
  buf[3] = '4';
  REQUIRE(span1.str() == "12");
  REQUIRE(span2.str() == "34");
}

TEST_CASE("FlexBuffer << string") {
  FlexBuffer buf;
  buf << "hello";
  buf << " world!";
  REQUIRE(buf.str() == "hello world!");
}

TEST_CASE("FlexBuffer << Buffer") {
  std::string str{"hello world!"};
  auto src = Buffer::wrap(str);
  FlexBuffer dest;
  dest << src;
  dest << " ";
  dest << src;
  REQUIRE(dest.str() == "hello world! hello world!");
}

TEST_CASE("FlexBuffer << FlexBuffer") {
  std::string str{"hello world!"};
  FlexBuffer src;
  src << "hello";
  src << " world!";
  FlexBuffer dest;
  dest << src;
  dest << " ";
  dest << src;
  REQUIRE(dest.str() == "hello world! hello world!");
}

TEST_CASE("FlexBuffer << uint32_t") {
  FlexBuffer buf;
  buf << static_cast<uint32_t>(123456789);
  auto read = buf.read<uint32_t>(0);
  REQUIRE(read == 123456789);
}

TEST_CASE("BufferWriter and BufferReader") {
  auto buf = Buffer::allocate(12);
  BufferWriter writer{buf};
  REQUIRE(writer.remaining() == 12);
  writer << "hello";
  REQUIRE(writer.remaining() == 7);
  writer << " ";
  REQUIRE(writer.remaining() == 6);
  writer << "world!";
  REQUIRE(writer.remaining() == 0);
  REQUIRE_THROWS(writer << "!", "array index out of bounds");
  REQUIRE(writer.remaining() == 0);
  BufferReader reader{buf};
  REQUIRE(reader.remaining() == 12);
  auto peek = reader.peek(6);
  auto span1 = reader.next(6);
  REQUIRE(reader.remaining() == 6);
  auto span2 = reader.next(6);
  REQUIRE(reader.remaining() == 0);
  REQUIRE(peek.str() == "hello ");
  REQUIRE(span1.str() == "hello ");
  REQUIRE(span2.str() == "world!");
  REQUIRE_THROWS(reader.next(1), "array index out of bounds");
  REQUIRE(reader.remaining() == 0);
}

TEST_CASE("BufferWriter and BufferReader fundamental types") {
  auto buf = Buffer::allocate(8);
  BufferWriter writer{buf};
  writer << static_cast<uint32_t>(1234);
  writer << static_cast<uint32_t>(5678);
  BufferReader reader{buf};
  REQUIRE(reader.next<uint32_t>() == 1234);
  REQUIRE(reader.peek<uint32_t>() == 5678);
  REQUIRE(reader.next<uint32_t>() == 5678);
}

TEST_CASE("FlexBuffer.data() const") {
  FlexBuffer buf;
  buf << "abc";
  const FlexBuffer c_buf = buf;
  REQUIRE(c_buf.data()[0] == 'a');
  REQUIRE(c_buf.data()[1] == 'b');
  REQUIRE(c_buf.data()[2] == 'c');
}
