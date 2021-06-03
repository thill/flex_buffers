#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "flexbuf/flexbuf.h"

using namespace flexbuf;

TEST_CASE("BufferView::wrap(shared_ptr<char[]>)") {
  std::shared_ptr<char[]> src{new char[5]};
  src[0] = 'a';
  src[1] = 'b';
  src[2] = 'c';
  src[3] = 'd';
  src[4] = 'e';
  auto buf = BufferView::wrap(src, 1, 3);
  REQUIRE(src.use_count() == 2);
  REQUIRE(buf.str() == "bcd");
}

TEST_CASE("BufferView(const BufferView&) shallow") {
  std::string str{"hello world!"};
  auto src = Buffer::copy_of(str);
  BufferView buf = src.view();
  BufferView copy{buf};
  src[0] = 'H';
  src[6] = 'W';
  REQUIRE(src.str() == "Hello World!");
  REQUIRE(buf.str() == "Hello World!");
  REQUIRE(copy.str() == "Hello World!");
}

TEST_CASE("BufferView operator=(const BufferView&) shallow") {
  std::string str{"hello world!"};
  auto src = Buffer::copy_of(str);
  BufferView buf = src.view();
  BufferView copy;
  copy = buf;
  src[0] = 'H';
  src[6] = 'W';
  REQUIRE(src.str() == "Hello World!");
  REQUIRE(buf.str() == "Hello World!");
  REQUIRE(copy.str() == "Hello World!");
}

TEST_CASE("BufferView.data() from wrapped raw pointer") {
  std::string src{"hello world!"};
  auto buf = BufferView::wrap(src);
  REQUIRE(buf.data()[0] == 'h');
  REQUIRE(buf.data()[2] == 'l');
  REQUIRE(buf.data()[11] == '!');
}

TEST_CASE("BufferView.data() from wrapped shared pointer") {
  std::shared_ptr<char[]> src{new char[3]};
  src[0] = 'a';
  src[1] = 'b';
  src[2] = 'c';
  auto buf = BufferView::wrap(src, 0, 3);
  REQUIRE(src.use_count() == 2);
  src = nullptr;
  REQUIRE(buf.data()[0] == 'a');
  REQUIRE(buf.data()[1] == 'b');
  REQUIRE(buf.data()[2] == 'c');
}

TEST_CASE("BufferView.size()") {
  std::string src{"hello world!"};
  auto buf = BufferView::wrap(src);
  REQUIRE(buf.size() == 12);
}

TEST_CASE("BufferView.operator[]") {
  char* src = new char[3];
  src[0] = 'a';
  src[1] = 'b';
  src[2] = 'c';
  auto buf = BufferView::wrap(src, 0, 3);
  REQUIRE(buf[0] == 'a');
  REQUIRE(buf[1] == 'b');
  REQUIRE(buf[2] == 'c');
  delete[] src;
}

TEST_CASE("BufferView.view()") {
  std::string src{"hello world!"};
  auto buf = BufferView::wrap(src);
  auto view = buf.view(6);
  REQUIRE(view.str() == "world!");
}

TEST_CASE("BufferView.view() does not dangle") {
  std::string src{"hello world!"};
  auto buf = std::make_shared<BufferView>(BufferView::wrap(src));
  auto view = buf->view(6);
  buf.reset();
  REQUIRE(view.str() == "world!");
}

TEST_CASE("BufferView.hex()") {
  std::shared_ptr<char[]> src{new char[4]};
  src[0] = 1;
  src[1] = 7;
  src[2] = 10;
  src[3] = 33;
  auto view = BufferView::wrap(src, 0, 4);
  REQUIRE(view.hex() == "0x01070a21");
  std::ostringstream oss;
  oss << Buffer::copy_of(view);
  REQUIRE(oss.str() == "0x01070a21");
}

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

TEST_CASE("FlexBuffer.view() does not dangle") {
  auto buf = std::make_shared<FlexBuffer>();
  *buf << "hello world!";
  auto view = buf->view(6);
  buf.reset();
  REQUIRE(view.str() == "world!");
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
  REQUIRE(buf.view(0, 12).str() == "hello world!");
}

TEST_CASE("FlexBuffer.resize(grow, IgnoreData)") {
  FlexBuffer buf;
  buf << "hello world!";
  buf.resize(100, ResizeMode::IgnoreData);
  REQUIRE(buf.size() == 100);
  REQUIRE(buf.view(0, 12).str() != "hello world!");
}

TEST_CASE("FlexBuffer.reserve(size)") {
  FlexBuffer buf;
  auto view1 = buf.reserve(2);
  auto view2 = buf.reserve(2);
  view1[0] = 'a';
  view1[1] = 'b';
  view2[0] = 'c';
  view2[1] = 'd';
  REQUIRE(buf.str() == "abcd");
}

TEST_CASE("FlexBuffer.reserve(size) views do not dangle") {
  FlexBuffer buf;
  auto view1 = buf.reserve(2);
  auto view2 = buf.reserve(2);
  buf.resize(100);
  buf[0] = '1';
  buf[1] = '2';
  buf[2] = '3';
  buf[3] = '4';
  REQUIRE(view1.str() == "12");
  REQUIRE(view2.str() == "34");
}

TEST_CASE("FlexBuffer << string") {
  FlexBuffer buf;
  buf << "hello";
  buf << " world!";
  REQUIRE(buf.str() == "hello world!");
}

TEST_CASE("FlexBuffer << BufferView") {
  std::string str{"hello world!"};
  auto src = BufferView::wrap(str);
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
  auto view1 = reader.next(6);
  REQUIRE(reader.remaining() == 6);
  auto view2 = reader.next(6);
  REQUIRE(reader.remaining() == 0);
  REQUIRE(view1.str() == "hello ");
  REQUIRE(view2.str() == "world!");
  REQUIRE_THROWS(reader.next(1), "array index out of bounds");
  REQUIRE(reader.remaining() == 0);
}

TEST_CASE("FlexBuffer.data() const") {
  FlexBuffer buf;
  buf << "abc";
  const FlexBuffer c_buf = buf;
  REQUIRE(c_buf.data()[0] == 'a');
  REQUIRE(c_buf.data()[1] == 'b');
  REQUIRE(c_buf.data()[2] == 'c');
}
