# flex_buffers
Safe, flexible buffers for modern C++.


## Include
Flex Buffers are provided in a single header file at `flexbuf/flexbuf.h`.
Bazel users can utilize `//:flexbuf` from the root `BUILD` file.


## Class Overview
* `Buffer` - A mutable, fixed-length buffer that wraps or allocates memory. Pass-by-value will deep copy.
* `FlexBuffer` - A mutable, growable buffer that always allocates. Pass-by-value will deep copy. Extends `Buffer`.
* `BufferReader` - Wraps a `Buffer` to provide linear reads.
* `BufferWriter` - Wraps a `Buffer` to provide linear writes.


## Buffer
* A mutable, fixed-length buffer that can wrap existing memory or allocate new memory.
* Pass-by-value will deep copy. 

### Buffer Usage
Factory Functions:
* `static Buffer allocate(size_t size)` - Allocate a buffer of a specific size
* `static Buffer copy_of(const char* data, size_t offset, size_t size)` - Allocate a new Buffer and copy the contents of the given data into it.
* `static Buffer copy_of(const std::string_view& string)` - Allocate a new Buffer and copy the contents of the given string_view into it.
* `static Buffer copy_of(const Buffer& buffer_span)` - Allocate a new Buffer and copy the contents of the given Buffer into it.
* `static Buffer copy_of(const span<T>& span)` - Allocate a new Buffer and copy the contents of the given span of a fundamental type into it.
* `static Buffer wrap(char* data, size_t offset, size_t size)` - Wrap the given raw pointer at the given offset/size.
* `static Buffer wrap(std::shared_ptr<char[]> data, size_t offset, size_t size)` - Wrap the given shared_ptr buffer at the given offset/size.
* `static Buffer wrap(std::shared_ptr<const char[]> data, size_t offset, size_t size)` - Wrap the given shared_ptr buffer at the given offset/size.
* `static Buffer wrap(const std::string& string)` - Wrap the given string.
* `static Buffer wrap(const std::span<T>& span)` - Wrap the given span of a fundamental type.

Member Functions:
* `size_t size()` - Get the buffer size.
* `char* data()` - Get the raw pointer to the start of the wrapped data.
* `T read<T>(size_t index)` - Return a copy of any fundamental type from the given index.
* `T& ref<T>(size_t index)` - Return a reference of any fundamental type that is backed by the buffer at the given index.
* `void write<T>(const T& src, size_t index = 0)` - Write any fundamental copyable type to the given index.
* `void write<T>(const std::span<T>& src, size_t index = 0)` - Write a span of any fundamental copyable type to the given index.
* `void write(const Buffer& src, size_t index = 0)` - Write another Buffer to the given index.
* `void clear()` - Fill the data with 0's
* `Buffer span(size_t index = 0, size_t size = Buffer::npos)` - Get a mutable buffer that wraps the same underlying data for the given range.

Member Operators:
* `char& operator[](size_t index)` - Get the byte at the given index.

Stringification Functions:
* `std::string Buffer::str()` - Convert the contents to a std::string
* `std::string Buffer::hex()` - Convert the contents to a hex string
* `std::ostream& operator<<(std::ostream& os, const flexbuf::Buffer& span)` - Convert the content to hex and append to the `ostream`

### Buffer Memory Wrapping
To wrap a raw pointer, use the factory method: `Buffer Buffer::wrap(const char* ptr, size_t offset, size_t length)`
```
std::string src{"hello world!"};
auto buffer_span = Buffer::wrap(src.data(), 0, src.size());
```

To wrap a shared pointer, use the factory method: `Buffer Buffer::wrap(std::shared_ptr<const char[]> ptr, size_t offset, size_t length)`
```
std::shared_ptr<char[]> src{new char[3]};
src[0] = 'a';
src[1] = 'b';
src[2] = 'c';
auto buffer_span = Buffer::wrap(src, 0, 3);
```

### Buffer Data Access
Example:
```
std::string src{"hello world!"};
auto buffer_span = Buffer::wrap(src);
// stringification
std::cout << "str(): " << buffer_span.str() << std::endl;
std::cout << "hex(): " << buffer_span.hex() << std::endl;
// operator[] (provide bounds checking)
std::cout << "buffer_span[0]: " << buffer_span[0] << std::endl;
// raw pointer access
const char* buffer_span_raw = buffer_span.data();
std::cout << "buffer_span_raw[0]: " << buffer_span_raw[0] << std::endl;
```
Output:
```
str(): hello world!
hex(): 0x68656c6c6f20776f726c6421
buffer_span[0]: h
buffer_span_raw[0]: h
```

### Reading Fundamental Types
Example:
```
char* src = new char[2];
src[0] = 1;
src[1] = 1;
auto buf = Buffer::wrap(src, 0, 2);
std::cout << buf.read<uint16_t>(0) << std::endl;
```
Output:
```
257
```

### Writing Fundamental Types
Example:
```
auto buf = Buffer::wrap(...);
buf.write<uint32_t>(12345, 0);
std::cout << buf.read<uint32_t>(0) << std::endl;
```
Output:
```
12345
```

### Buffer Child Span
`Buffer` provides `span` methods that return a mutable child `Buffer` object that wraps a portion of the underlying memory.
Example:
```
std::string str{"hello world!"};
auto buf = Buffer::wrap(str.data(), 0, str.length());
auto span1 = buf.span(0, 6); // span from index 0 of size 6
auto span2 = buf.span(6); // span from index 6 to end of buffer
span1[0] = 'H';
span2[0] = 'W';
std::cout << buf.str() << std::endl;
```
Output:
```
Hello World!
```
Note: Parent and child spans use a `shared_ptr` to manage underlying buffer ownership, so it is a completely valid for a child span to outlive the parent span. (Beware wrapping user-provided raw pointers, as they must remain valid as long as any parent or child span continues to use it! Consider using `shared_ptr<const char[]>` when possible.)


## FlexBuffer
* A mutable, growable buffer that always allocates.
* Pass-by-value will deep copy.
* Extends `Buffer`.

### FlexBuffer Usage
Constructors:
* `FlexBuffer()` - Sets size=0 and pre-allocates a buffer to the size of the system's byte-alignment length
* `FlexBuffer(size_t initial_capacity)` - Sets size=0 and pre-allocates a buffer to the given initial_capacity

Member Functions:
* `size_t capacity()` - Get the current capacity of this buffer.
* `void clear()` - Fill the data with 0's to the current size.
* `void clear_all()` - Fill the data with 0's to the current capacity.
* `Buffer copy(size_t index = 0, size_t size = Buffer::npos)` - Allocate a new Buffer consisting of the contents of this buffer for the given range.
* `char* data()` - Get the raw pointer to the start of the wrapped data.
* `FlexBuffer flex_copy(size_t index = 0, size_t size = FlexBuffer::npos)` - Allocate a new FlexBuffer consisting of the contents of this buffer for the given range.
* `size_t initial_capacity()` - Get the initial capacity. The underlying memory will never reallocate smaller than this size.
* `Buffer reserve(size_t size)` - Increment the total size by the given size, and return a writable Buffer that wraps this new memory.
* `void resize(size_t size, ResizeMode mode = ResizeMode::KeepData)` - Set the current size, and grow or shrink the underlying memory by factors of two as necessary.
* `size_t size()` - Get the buffer size.
* `Buffer span(size_t index = 0, size_t size = Buffer::npos)` - Get a Buffer that wraps the same underlying data for the given range.

Member Operators:
* `char& operator[](size_t index)` - Get the byte at the given index.
* `FlexBuffer& operator<<(const Buffer& buffer)` - Append the given buffer to the end of this FlexBuffer, growing by the given buffer's size.
* `FlexBuffer& operator<<(const std::string_view& string)` - Append the given string to the end of this FlexBuffer, growing by the given string's size.
* `FlexBuffer& operator<< <T>(const T& value)` - Append any fundamental type to the end of this FlexBuffer, growing by the given type's size.

Stringification Functions:
* `std::string FlexBuffer::str()` - Convert the contents to a std::string
* `std::string FlexBuffer::hex()` - Convert the contents to a hex string
* `std::ostream& operator<<(std::ostream& os, const flexbuf::FlexBuffer& buffer)` - Convert the content to hex and append to the `ostream`

### FlexBuffer Allocation
FlexBuffers start empty and can have data appended to them. 
They grow dynamically as needed.
The default constructor will create a FlexBuffer with a size of 0.
By default, the initial capacity of a buffer is the system's byte alignment size.
The underlying memory will double or halve as needed when the buffer is resized.

### FlexBuffer Resizing
The `resize` method can be used to change the size of the `FlexBuffer`.
By default, data is preserved. 
Data preservation can be disabled by optionally passing `ResizeMode::IgnoreData` to the `resize` method.
Example:
```
FlexBuffer buf;
buf << "hello world!";
std::cout << "initial size: " << buf.size() << std::endl;
std::cout << "initial str: " << buf.str() << std::endl;
buf.resize(5);
std::cout << "final size: " << buf.size() << std::endl;
std::cout << "final str: " << buf.str() << std::endl;
```
Output:
```
initial size: 12
initial str: hello world!
final size: 5
final str: hello
```

### FlexBuffer Appending
Data can be appended to a `FlexBuffer` with the `<<` operator.
The `FlexBuffer` will automatically resize and grow the underlying memory as needed to fit appended data.
The `<<` operator accepts a `const Buffer&` or a `const std::string_view&`.
Example:
```
std::string world_str = "world";
auto world_view = Buffer::wrap(world_str);
FlexBuffer buf;
buf << "hello";
buf << world_view;
std::cout << buf.str() << std::endl;
```
Output:
```
hello world
```

### FlexBuffer Reservation
`FlexBuffer`'s `reserve(size_t size)` method can be used to reserve and preallocate memory to be written later.
The `reserve` method increments the total FlexBuffer size by the given size, and return a writable `Buffer` that wraps this new portion of memory. 
This is especially useful for common encoding logic that can operate on a `Buffer` and always assume `offset=0`.
Example:
```
FlexBuffer buf;
auto span1 = buf.reserve(2);
auto span2 = buf.reserve(2);
// write to span1
span1[0] = 'a';
span1[1] = 'b';
// write to span2
span2[0] = 'c';
span2[1] = 'd';
// print original flex buffer
std::cout << buf.str() << std::endl;
```
Output:
```
abcd
```


## BufferReader
Wraps a `Buffer` to provide linear reads by advancing a `position`. 

### BufferReader Usage
Constructors:
* `BufferReader(const Buffer& view)`

Member Functions:
* `Buffer next(size_t size)` - Get a Buffer of the next `size` bytes and advance the `position`
* `T next<T>()` - Read a fundamental type and advance the `position` by the size of the type
* `Buffer peek(size_t size)` - Get a Buffer of the next `size` bytes without advancing the `position`
* `T peek<T>()` - Read a fundamental type without advancing the `position`
* `size_t position()` - Get the current position
* `void position(size_t position)` - Set the current position
* `size_t remaining()` - Get the remaining bytes that can be read (`view.size() - position()`)


## BufferWriter
Wraps a `Buffer` to provide linear write by advancing a `position`. 

### BufferWriter Usage
Constructor:
* `BufferWriter(Buffer& buffer)`

Member Functions:
* `Buffer next(size_t size)` - Get a wrapped Buffer of the next `size` elements and advance the `position`
* `Buffer peek(size_t size)` - Get a wrapped Buffer of the next `size` elements without advancing the `position`
* `size_t position()` - Get the current position
* `void position(size_t position)` - Set the current position
* `size_t remaining()` - Get the remaining bytes that can be written (`buffer.size() - position()`)

Member Operators:
* `BufferWriter& operator<<(const Buffer& buffer)` - Write the given buffer at the current position, advancing the position by given buffer's size.
* `BufferWriter& operator<<(const std::string_view& string)` - Write the given string at the current position, advancing the position by the given string's size.
* `FlexBuffer& operator<< <T>(const T& value)` - Write the given fundamental type's value at the current position, advancing the offset by the given type's size.


## Developing
For containerized development with VS Code IDE:
* `ide/linux-amd64` for Linux, WSL, and MacOS on x86_64
* `ide/selinux-amd64` for Security-Enhanded Linux

```
$ code ./ide/linux-amd64
```

Then follow the "Reopen in container" prompts.
