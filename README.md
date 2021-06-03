# flex_buffers
Safe, flexible buffers for modern C++.


## Class Overview
* `BufferView` - A const-only, fixed-length buffer that wraps existing memory. Pass-by-value continues to point to the same underlying memory.
* `Buffer` - A mutable, fixed-length buffer that can either allocate memory or wrap existing memory. Pass-by-value will deep copy. Extends `BufferView`.
* `FlexBuffer` - A mutable, growable buffer that always allocates. Pass-by-value will deep copy. Extends `Buffer`.
* `BufferReader` - Wraps a `BufferView` to provide linear reads.
* `BufferWriter` - Wraps a `Buffer` to provide linear writes.


## BufferView
* A const-only, fixed-length buffer that wraps existing memory. 
* Pass-by-value continues to point to the same underlying memory.

### BufferView Usage
Factory Functions:
* `static BufferView wrap(const char* data, size_t offset, size_t size)` - Wrap the given raw pointer at the given offset/size.
* `static BufferView wrap(std::shared_ptr<const char[]> data, size_t offset, size_t size)` - Wrap the given shared_ptr buffer at the given offset/size.
* `static BufferView wrap(const std::string_view& string)` - Wrap the given string_view.

Member Functions:
* `const char* data()` - Get the raw pointer to the start of the wrapped data.
* `size_t size()` - Get the buffer size.
* `BufferView view(size_t index = 0, size_t size = BufferView::npos)` - Get a sub-view of this buffer.

Member Operators:
* `const char& operator[](size_t index)` - Get the byte at the given index.

Stringification Functions:
* `std::string BufferView::str()` - Convert the contents to a std::string
* `std::string BufferView::hex()` - Convert the contents to a hex string
* `std::ostream& operator<<(std::ostream& os, const flexbuf::BufferView& view)` - Convert the content to hex and append to the `ostream`

### BufferView Memory Wrapping
To wrap a raw pointer, use the factory method: `BufferView BufferView::wrap(const char* ptr, size_t offset, size_t length)`
```
std::string src{"hello world!"};
auto buffer_view = BufferView::wrap(src.data(), 0, src.size());
```

To wrap a shared pointer, use the factory method: `BufferView BufferView::wrap(std::shared_ptr<const char[]> ptr, size_t offset, size_t length)`
```
std::shared_ptr<char[]> src{new char[3]};
src[0] = 'a';
src[1] = 'b';
src[2] = 'c';
auto buffer_view = BufferView::wrap(src, 0, 3);
```


### BufferView Data Access
Example:
```
std::string src{"hello world!"};
auto buffer_view = BufferView::wrap(src);
// stringification
std::cout << "str(): " << buffer_view.str() << std::endl;
std::cout << "hex(): " << buffer_view.hex() << std::endl;
// operator[] (provide bounds checking)
std::cout << "buffer_view[0]: " << buffer_view[0] << std::endl;
// raw pointer access
const char* buffer_view_raw = buffer_view.data();
std::cout << "buffer_view_raw[0]: " << buffer_view_raw[0] << std::endl;
```
Output:
```
str(): hello world!
hex(): 0x68656c6c6f20776f726c6421
buffer_view[0]: h
buffer_view_raw[0]: h
```


### BufferView Child View
Example:
```
std::string src{"hello world!"};
auto buffer_view = BufferView::wrap(src);
// create a subview from `index=6` of `size=5`
auto child_view = buffer_view.view(6, 5);
// stringification of child view
std::cout << "child_view.str(): " << child_view.str() << std::endl;
// raw pointer points directly to the beginning of the child view
const char* child_view_raw = child_view.data();
std::cout << "child_view_raw[0]: " << child_view_raw[0] << std::endl;
```
Output:
```
child_view.str(): world
child_view_raw[0]: w
```


## Buffer
* A mutable, fixed-length buffer that can either allocate memory or wrap existing memory. 
* Pass-by-value will deep copy. 
* Extends `BufferView`.


### Buffer Usage
Factory Functions:
* `static Buffer wrap(char* data, size_t offset, size_t size)` - Wrap the given raw pointer at the given offset/size.
* `static Buffer wrap(std::shared_ptr<char[]> data, size_t offset, size_t size)` - Wrap the given shared_ptr buffer at the given offset/size.
* `static Buffer copy_of(const char* data, size_t offset, size_t size)` - Allocate a new Buffer and copy the contents of the given data into it.
* `static Buffer copy_of(const std::string_view& string)` - Allocate a new Buffer and copy the contents of the given string_view into it.
* `static Buffer copy_of(const BufferView& buffer_view)` - Allocate a new Buffer and copy the contents of the given BufferView into it.

Member Functions:
* `void clear()` - Fill the data with 0's
* `Buffer copy(size_t index = 0, size_t size = Buffer::npos)` - Allocate a new Buffer consisting of the contents of this buffer for the given range.
* `char* data()` - Get the raw pointer to the start of the wrapped data.
* `Buffer span(size_t index = 0, size_t size = Buffer::npos)` - Get a mutable buffer that wraps the same underlying data for the given range.
* `size_t size()` - Get the buffer size.
* `BufferView view(size_t index = 0, size_t size = BufferView::npos)` - Get a sub-view of this buffer.

Member Operators:
* `char& operator[](size_t index)` - Get the byte at the given index.

Stringification Functions:
* `std::string Buffer::str()` - Convert the contents to a std::string
* `std::string Buffer::hex()` - Convert the contents to a hex string
* `std::ostream& operator<<(std::ostream& os, const flexbuf::Buffer& view)` - Convert the content to hex and append to the `ostream`

### Buffer Memory Wrapping
To wrap a raw pointer, use the factory method: `Buffer Buffer::wrap(char* ptr, size_t offset, size_t length)`
To wrap a shared pointer, use the factory method: `Buffer Buffer::wrap(std::shared_ptr<char[]> ptr, size_t offset, size_t length)`


### Buffer Memory Allocation
To allocate a `Buffer` of a specific size, use the factory method: `Buffer::allocate(size_t size)`


### Buffer Memory Copying
To copy data into a new `Buffer`, use the factory methods:
* `Buffer Buffer::copy_of(const char* data, size_t offset, size_t size)`
* `Buffer Buffer::copy_of(const BufferView& view)`


### Buffer Data Read Access
`Buffer` extends `BufferView` to provide the same read patterns.


### Buffer Data Write Access
`Buffer` provides `char* data()` and `char& operator[](size_t index)` to mutate the underlying data


### Buffer Child Span
`Buffer` extends `BufferView` to provide `const` child views.
Additionally, `Buffer` provides `span` methods that return a mutable child `Buffer` object that wraps a portion of the underlying memory.
Example:
```
std::string str{"hello world!"};
auto buf = Buffer::copy_of(str);
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
Note: Parent and child views use a `shared_ptr` to manage underlying buffer ownership, so it is a completely valid for a child view to outlive the parent view. (Beware wrapping user-provided raw pointers, as they must remain valid as long as any parent or child view continues to use it! Consider using `shared_ptr<const char[]>` when possible.)


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
* `Buffer span(size_t index = 0, size_t size = Buffer::npos)` - Get a mutable buffer that wraps the same underlying data for the given range.
* `size_t size()` - Get the buffer size.
* `BufferView view(size_t index = 0, size_t size = BufferView::npos)` - Get a sub-view of this buffer.

Member Operators:
* `char& operator[](size_t index)` - Get the byte at the given index.
* `FlexBuffer& operator<<(const BufferView& buffer)` - Append the given buffer to the end of this FlexBuffer, growing by the given buffer's size.
* `FlexBuffer& operator<<(const std::string_view& string)` - Append the given string to the end of this FlexBuffer, growing by the given string's size.

Stringification Functions:
* `std::string FlexBuffer::str()` - Convert the contents to a std::string
* `std::string FlexBuffer::hex()` - Convert the contents to a hex string
* `std::ostream& operator<<(std::ostream& os, const flexbuf::FlexBuffer& view)` - Convert the content to hex and append to the `ostream`


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
The `<<` operator accepts a `const BufferView&` or a `const std::string_view&`.
Example:
```
std::string world_str = "world";
auto world_view = BufferView::wrap(world_str);
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
Wraps a `BufferView` to provide linear reads by advancing a `position`. 


### BufferReader Usage
Constructors:
* `BufferReader(const BufferView& view)`

Member Functions:
* `BufferView next(size_t size)` - Get a BufferView of the next `size` elements and advance the `position`
* `BufferView peek(size_t size)` - Get a BufferView of the next `size` elements without advancing the `position`
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
* `BufferWriter& operator<<(const BufferView& buffer)` - Write the given buffer at the current position, advancing the position by given buffer's size.
* `BufferWriter& operator<<(const std::string_view& string)` - Write the given string at the current position, advancing the position by the given string's size.