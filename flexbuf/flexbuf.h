#pragma once

#include <compare>
#include <cstddef>
#include <cstring>
#include <memory>
#include <ostream>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace flexbuf {

enum class ResizeMode { KeepData, IgnoreData };
class BufferView;
class Buffer;
class FlexBuffer;
class BufferReader;
class BufferWriter;

/**
 * Internal namespace, never exposed via the API.
 * Behavior is undefined and can change any time without warning.
 */
namespace internal {
class BufferData {
private:
  std::shared_ptr<char[]> _ptr; // optional shared ownership
  char* _data;
  size_t _capacity;

public:
  BufferData() : _ptr{nullptr}, _data{nullptr}, _capacity{0} {};
  BufferData(size_t capacity) : _ptr{new char[capacity]}, _data{_ptr.get()}, _capacity{capacity} {};
  BufferData(std::shared_ptr<char[]> data, size_t offset, size_t size)
      : _ptr{data}, _data{reinterpret_cast<char*>(_ptr.get() + offset)}, _capacity{size} {};
  BufferData(char* data, size_t offset, size_t size)
      : _ptr{nullptr}, _data{reinterpret_cast<char*>(data + offset)}, _capacity{size} {};
  BufferData(const BufferData&) = delete;
  BufferData& operator=(const BufferData&) = delete;
  BufferData(BufferData&&) = default;
  BufferData& operator=(BufferData&&) = default;
  ~BufferData() = default;

  char* data() {
    return _data;
  }

  const char* data() const {
    return _data;
  }

  size_t capacity() const {
    return _capacity;
  }

  void resize(ResizeMode mode, size_t new_capacity) noexcept {
    auto old_ptr = _ptr;
    auto old_data = _data;
    _ptr = std::shared_ptr<char[]>{new char[new_capacity]};
    _data = _ptr.get();
    if (mode == ResizeMode::KeepData) {
      memcpy(_data, old_data, std::min(_capacity, new_capacity));
    }
    _capacity = new_capacity;
  }
};
} // namespace internal

/**
 * A const view of a buffer that wraps existing memory.
 * Pass-by-value semantics will point at the same underlying memory - O(1).
 * The default constructor wraps a nullptr and has a size of 0.
 * Static factory methods are used to keep consistent semantics with the Buffer class.
 */
class BufferView {
private:
  friend class Buffer;
  friend class FlexBuffer;

  using BufferData = flexbuf::internal::BufferData;
  using BufferDataPtr = std::shared_ptr<BufferData>;
  BufferDataPtr _data;
  size_t _offset;
  size_t _size;

  BufferView(BufferDataPtr& data, size_t offset, size_t size) : _data{data}, _offset{offset}, _size{size} {};
  BufferView(BufferDataPtr&& data, size_t offset, size_t size) : _data{data}, _offset{offset}, _size{size} {};

  inline void check_bounds(size_t index, size_t size) const {
    auto end = index + size;
    // if "beyond size of this view" or "beyond capacity of underlying data"
    if (end > _size || end > _data->capacity() - _offset)
      throw std::range_error{"array index out of bounds"};
  }

  inline char* raw_data() noexcept {
    return reinterpret_cast<char*>(_data->data() + _offset);
  }

  inline const char* raw_data() const noexcept {
    return reinterpret_cast<char*>(_data->data() + _offset);
  }

public:
  static const size_t npos = -1;

  /**
   * Wrap the given raw pointer at the given offset/size.
   * Beware ownership: you must ensure the raw pointer remains valid.
   * Consider wrap(std::shared_ptr<const char[]> data, size_t offset, size_t size) for safety if possible.
   */
  static BufferView wrap(const char* data, size_t offset, size_t size) {
    return BufferView{std::make_shared<BufferData>(const_cast<char*>(data), offset, size), 0, size};
  }

  /**
   * Wrap the given shared_ptr buffer at the given offset/size.
   */
  static BufferView wrap(std::shared_ptr<const char[]> data, size_t offset, size_t size) {
    return BufferView{std::make_shared<BufferData>(std::const_pointer_cast<char[]>(data), offset, size), 0, size};
  }

  /**
   * Wrap the given string.
   * Beware ownership: you must ensure the referenced string remains valid.
   * Consider wrap(std::shared_ptr<const char[]> data, size_t offset, size_t size) for safety if possible.
   */
  static BufferView wrap(const std::string& string) {
    return wrap(string.data(), 0, string.size());
  }

  /**
   * Wrap the given string_view.
   * Beware ownership: you must ensure the referenced string remains valid.
   * Consider wrap(std::shared_ptr<const char[]> data, size_t offset, size_t size) for safety if possible.
   */
  static BufferView wrap(const std::string_view& string) {
    return wrap(string.data(), 0, string.size());
  }

  /**
   * Wrap the given span.
   * Beware ownership: you must ensure the span's underlying data remains valid.
   * Consider wrap(std::shared_ptr<const char[]> data, size_t offset, size_t size) for safety if possible.
   */
  template <typename T, typename = typename std::enable_if_t<std::is_fundamental_v<T>>>
  static BufferView wrap(const std::span<const T>& span) {
    auto data = const_cast<char*>(reinterpret_cast<const char*>(span.data()));
    auto size = span.size() * sizeof(T);
    return BufferView{std::make_shared<BufferData>(data, 0, size), 0, size};
  }

  /**
   * Wrap the given span.
   * Beware ownership: you must ensure the span's underlying data remains valid.
   * Consider wrap(std::shared_ptr<const char[]> data, size_t offset, size_t size) for safety if possible.
   */
  template <typename T, typename = typename std::enable_if_t<std::is_fundamental_v<T>>>
  static BufferView wrap(const std::span<T>& span) {
    auto data = reinterpret_cast<char*>(span.data());
    size_t size = span.size() * sizeof(T);
    return BufferView{std::make_shared<BufferData>(data, 0, size), 0, size};
  }

  /**
   * Wrap the given span.
   * Beware ownership: you must ensure the span's underlying data remains valid.
   * Consider wrap(std::shared_ptr<const char[]> data, size_t offset, size_t size) for safety if possible.
   */
  template <typename T, size_t N, typename = typename std::enable_if_t<std::is_fundamental_v<T>>>
  static BufferView wrap(const std::span<const T, N>& span) {
    auto data = const_cast<char*>(reinterpret_cast<const char*>(span.data()));
    size_t size = span.size() * sizeof(T);
    return BufferView{std::make_shared<BufferData>(data, 0, size), 0, size};
  }

  /**
   * Wrap the given span.
   * Beware ownership: you must ensure the span's underlying data remains valid.
   * Consider wrap(std::shared_ptr<const char[]> data, size_t offset, size_t size) for safety if possible.
   */
  template <typename T, size_t N, typename = typename std::enable_if_t<std::is_fundamental_v<T>>>
  static BufferView wrap(const std::span<T, N>& span) {
    auto data = reinterpret_cast<char*>(span.data());
    size_t size = span.size() * sizeof(T);
    return BufferView{std::make_shared<BufferData>(data, 0, size), 0, size};
  }

  /**
   * Create a buffer with size=0 and set underlying data to nullptr
   */
  BufferView() : _data{std::make_shared<BufferData>()}, _offset{0}, _size{0} {};

  /**
   * Shallow copy
   */
  BufferView(const BufferView& rhs) = default;

  /**
   * Shallow copy
   */
  BufferView& operator=(const BufferView&) = default;

  /**
   * Move
   */
  BufferView(BufferView&& rhs) : _data{std::move(rhs._data)}, _offset{rhs._offset}, _size{rhs._size} {
    rhs._offset = 0;
    rhs._size = 0;
  }

  /**
   * Move
   */
  BufferView& operator=(BufferView&& rhs) {
    _data = std::move(rhs._data);
    _offset = rhs._offset;
    _size = rhs._size;
    rhs._offset = 0;
    rhs._size = 0;
    return *this;
  }

  ~BufferView() = default;

  /**
   * Get the raw pointer to the start of the underlying data.
   */
  inline const char* data() const {
    check_bounds(0, 0);
    return raw_data();
  }

  /**
   * Get the buffer size.
   */
  inline size_t size() const noexcept {
    return _size;
  }

  /**
   * Get the byte at the given index.
   * Throws on array index out of bounds.
   */
  const char& operator[](size_t index) const {
    check_bounds(index, 1);
    return raw_data()[index];
  }

  /**
   * Convert this BufferView to a std::span
   */
  operator std::span<const char>() const {
    check_bounds(0, size());
    return std::span{raw_data(), size()};
  }

  /**
   * Return a copy of any fundamental type from the given index.
   */
  template <typename T, typename = std::enable_if_t<std::is_fundamental_v<T>>>
  const T read(size_t index) const {
    check_bounds(index, sizeof(T));
    T v;
    memcpy(&v, reinterpret_cast<const char*>(raw_data() + index), sizeof(T));
    return v;
  }

  /**
   * Get a sub-view of this buffer.
   * Throws on array index out of bounds.
   * The returned buffer may outlive the source buffer.
   */
  BufferView subview(size_t index = 0, size_t size = BufferView::npos) {
    if (size == BufferView::npos)
      size = _size - index;
    check_bounds(index, size);
    return BufferView{_data, index, size};
  }

  /**
   * Get a sub-view of this buffer.
   * Throws on array index out of bounds.
   * The returned buffer may outlive the source buffer.
   */
  const BufferView subview(size_t index = 0, size_t size = BufferView::npos) const {
    return const_cast<BufferView&>(*this).subview(index, size);
  }

  /**
   * Convert the contents to a std::string
   */
  std::string str() const {
    check_bounds(0, _size);
    return std::string{raw_data(), 0, _size};
  }

  /**
   * Convert the contents to a hex string
   */
  std::string hex() const {
    check_bounds(0, _size);
    std::ostringstream oss;
    oss << "0x";
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < _size; ++i)
      oss << std::setw(2) << static_cast<int>(raw_data()[i]);
    return oss.str();
  }
};

/**
 * A fixed-size buffer that can either allocate memory or wrap existing memory.
 * Pass-by-value semantics will deep copy the underlying data - O(n).
 * The default constructor wraps a nullptr and has a size of 0.
 * Static factory methods are used for differentiating wrap, allocation, and copy semantics.
 * This class extends BufferView.
 */
class Buffer : public BufferView {
private:
  friend class FlexBuffer;

  Buffer(BufferDataPtr& data, size_t offset, size_t size) : BufferView{data, offset, size} {};
  Buffer(BufferDataPtr&& data, size_t offset, size_t size) : BufferView{data, offset, size} {};

public:
  static Buffer allocate(size_t size) {
    return Buffer{std::make_shared<BufferData>(size), 0, size};
  }

  /**
   * Wrap the given raw pointer at the given offset/size.
   * Beware ownership: you must ensure the raw pointer remains valid.
   * Consider wrap(std::shared_ptr<char[]> data, size_t offset, size_t size) for safety if possible.
   */
  static Buffer wrap(char* data, size_t offset, size_t size) {
    return Buffer{std::make_shared<BufferData>(data, offset, size), 0, size};
  }

  /**
   * Wrap the given shared_ptr buffer at the given offset/size.
   */
  static Buffer wrap(std::shared_ptr<char[]> data, size_t offset, size_t size) {
    return Buffer{std::make_shared<BufferData>(data, offset, size), 0, size};
  }

  /**
   * Allocate a new Buffer and copy the contents of the given data into it.
   */
  static Buffer copy_of(const char* data, size_t offset, size_t size) {
    auto buffer = Buffer::allocate(size);
    memcpy(buffer.raw_data(), reinterpret_cast<const char*>(data + offset), size);
    return buffer;
  }

  /**
   * Allocate a new Buffer and copy the contents of the given string into it.
   */
  static Buffer copy_of(const std::string& string) {
    return copy_of(string.data(), 0, string.size());
  }

  /**
   * Allocate a new Buffer and copy the contents of the given string_view into it.
   */
  static Buffer copy_of(const std::string_view& string) {
    return copy_of(string.data(), 0, string.size());
  }

  /**
   * Allocate a new Buffer and copy the contents of the given span into it.
   */
  template <typename T, typename = typename std::enable_if_t<std::is_fundamental_v<T>>>
  static BufferView copy_of(const std::span<const T>& span) {
    auto data = const_cast<char*>(reinterpret_cast<const char*>(span.data()));
    auto size = span.size() * sizeof(T);
    return copy_of(data, 0, size);
  }

  /**
   * Allocate a new Buffer and copy the contents of the given span into it.
   */
  template <typename T, typename = typename std::enable_if_t<std::is_fundamental_v<T>>>
  static BufferView copy_of(const std::span<T>& span) {
    auto data = reinterpret_cast<char*>(span.data());
    size_t size = span.size() * sizeof(T);
    return copy_of(data, 0, size);
  }

  /**
   * Allocate a new Buffer and copy the contents of the given span into it.
   */
  template <typename T, size_t N, typename = typename std::enable_if_t<std::is_fundamental_v<T>>>
  static BufferView copy_of(const std::span<const T, N>& span) {
    auto data = reinterpret_cast<const char*>(span.data());
    size_t size = span.size() * sizeof(T);
    return copy_of(data, 0, size);
  }

  /**
   * Allocate a new Buffer and copy the contents of the given span into it.
   */
  template <typename T, size_t N, typename = typename std::enable_if_t<std::is_fundamental_v<T>>>
  static BufferView copy_of(const std::span<T, N>& span) {
    auto data = reinterpret_cast<char*>(span.data());
    size_t size = span.size() * sizeof(T);
    return copy_of(data, 0, size);
  }

  /**
   * Allocate a new Buffer and copy the contents of the given BufferView into it.
   */
  static Buffer copy_of(const BufferView& buffer_view) {
    return copy_of(buffer_view.data(), 0, buffer_view.size());
  }

  /**
   * Create a buffer with size=0 and set underlying data to nullptr
   */
  Buffer() : BufferView{} {};

  /**
   * Deep copy
   */
  Buffer(const Buffer& rhs) : BufferView{std::make_shared<BufferData>(rhs.size()), 0, rhs.size()} {
    memcpy(raw_data(), rhs.raw_data(), rhs.size());
  }

  /**
   * Deep copy
   */
  Buffer& operator=(const Buffer& rhs) {
    _data = std::make_shared<BufferData>(rhs.size());
    _size = rhs._size;
    memcpy(raw_data(), rhs.raw_data(), rhs.size());
    return *this;
  }

  Buffer(Buffer&& rhs) = default;
  Buffer& operator=(Buffer&& rhs) = default;
  ~Buffer() = default;

  /**
   * Get the raw pointer to the start of the underlying data.
   */
  inline char* data() {
    check_bounds(0, 0);
    return raw_data();
  }

  /**
   * Get the raw pointer to the start of the underlying data.
   */
  inline const char* data() const {
    return const_cast<Buffer&>(*this).data();
  }

  /**
   * Get the byte at the given index.
   * Throws on array index out of bounds.
   */
  char& operator[](size_t index) {
    check_bounds(index, 1);
    return raw_data()[index];
  }

  /**
   * Get the byte at the given index.
   * Throws on array index out of bounds.
   */
  const char& operator[](size_t index) const {
    return const_cast<Buffer&>(*this)[index];
  }

  /**
   * Convert this Buffer to a std::span
   */
  operator std::span<char>() {
    check_bounds(0, size());
    return std::span{raw_data(), size()};
  }

  /**
   * Convert this Buffer to a std::span
   */
  operator std::span<const char>() const {
    check_bounds(0, size());
    return std::span{raw_data(), size()};
  }

  /**
   * Write any trivially copyable type to the given index.
   */
  template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
  void write(const T& src, size_t index = 0) {
    check_bounds(index, sizeof(T));
    memcpy(reinterpret_cast<char*>(raw_data() + index), &src, sizeof(T));
  }

  /**
   * Get a mutable buffer that wraps the same underlying data for the given range.
   * The returned buffer may outlive the source buffer.
   */
  Buffer subspan(size_t index = 0, size_t size = Buffer::npos) {
    if (size == Buffer::npos)
      size = _size - index;
    check_bounds(index, size);
    return Buffer{_data, index, size};
  }

  /**
   * Get a const buffer that wraps the same underlying data for the given range.
   * The returned buffer may outlive the source buffer.
   */
  const Buffer subspan(size_t index = 0, size_t size = Buffer::npos) const {
    return const_cast<Buffer&>(*this).subspan(index, size);
  }

  /**
   * Allocate a new Buffer consisting of the contents of this buffer for the given range.
   */
  Buffer copy(size_t index = 0, size_t size = Buffer::npos) const {
    if (size == Buffer::npos)
      size = _size - index;
    check_bounds(index, size);
    auto result = Buffer::allocate(size);
    memcpy(result.raw_data(), reinterpret_cast<const char*>(raw_data() + index), size);
    return result;
  }

  /**
   * Fill the data with 0's
   */
  void clear() {
    check_bounds(0, _size);
    memset(raw_data(), 0, _size);
  }
};

/**
 * A buffer that always allocates its own memory, can be resized, and can continuously grow to fit more data.
 * Pass-by-value semantics will deep copy the underlying data - O(n).
 * The default constructor allocates an initial capacity of the system's byte-alignment length and sets the size to 0.
 * The size will automatically double and halve as necessary as the buffer is resized or data is appended.
 * This class extends Buffer.
 */
class FlexBuffer : public Buffer {
private:
  size_t _initial_capacity;
  FlexBuffer(size_t initial_capacity, size_t allocate_size)
      : Buffer{std::make_shared<BufferData>(allocate_size), 0, 0}, _initial_capacity{initial_capacity} {};

  static size_t capacity_for(const size_t size, const size_t min_capacity) {
    auto capacity = std::max(static_cast<size_t>(1), min_capacity);
    while (size > capacity && capacity != 0) {
      capacity <<= 1;
    }
    if (capacity == 0) {
      // capacity can no longer double on architecture, set capacity to size
      capacity = size;
    }
    return capacity;
  }

public:
  /**
   * Sets size=0 and pre-allocates a buffer to the given initial_capacity,
   * which defaults to the system's byte-alignment size.
   */
  FlexBuffer(size_t initial_capacity = __STDCPP_DEFAULT_NEW_ALIGNMENT__)
      : FlexBuffer{initial_capacity, initial_capacity} {};

  /**
   * Deep copy
   */
  FlexBuffer(const FlexBuffer& rhs) : FlexBuffer{rhs._initial_capacity, rhs.capacity()} {
    memcpy(raw_data(), rhs.raw_data(), rhs.size());
  }

  /**
   * Deep copy
   */
  FlexBuffer& operator=(const FlexBuffer& rhs) {
    _data = std::make_shared<BufferData>(rhs.capacity());
    _size = rhs._size;
    _initial_capacity = rhs._initial_capacity;
    memcpy(raw_data(), rhs.raw_data(), rhs.size());
    return *this;
  }

  /**
   * Move
   */
  FlexBuffer(FlexBuffer&& rhs)
      : Buffer{std::move(rhs._data), rhs._offset, rhs._size}, _initial_capacity(rhs._initial_capacity) {
    rhs._offset = 0;
    rhs._size = 0;
  }

  /**
   * Move
   */
  FlexBuffer& operator=(FlexBuffer&& rhs) {
    _data = std::move(rhs._data);
    _offset = rhs._offset;
    _size = rhs._size;
    _initial_capacity = rhs._initial_capacity;
    rhs._offset = 0;
    rhs._size = 0;
    return *this;
  }
  ~FlexBuffer() = default;

  /**
   * Get the initial capacity.
   * The underlying memory will never reallocate smaller than this size.
   */
  inline size_t initial_capacity() const noexcept {
    return _initial_capacity;
  }

  /**
   * Get the current capacity of this buffer.
   * This represents the maximum this buffer can be resized without undergoing a reallocation and copy.
   */
  inline size_t capacity() const noexcept {
    return _data->capacity();
  }

  /**
   * Clear the entirety of the underlying allocated memory.
   */
  inline void clear_all() noexcept {
    memset(_data->data(), 0, _data->capacity());
  }

  /**
   * Allocate a new FlexBuffer consisting of the contents of this buffer for the given range.
   */
  inline FlexBuffer flex_copy(size_t index = 0, size_t size = FlexBuffer::npos) const {
    if (size == FlexBuffer::npos)
      size = _size - index;
    check_bounds(index, size);
    auto allocate_size = capacity_for(size, _initial_capacity);
    FlexBuffer result{_initial_capacity, allocate_size};
    result.append(raw_data(), index, size);
    return result;
  }

  /**
   * Set the current size, and grow or shrink the underlying memory by factors of two as necessary.
   * By default all data through the current size is copied.
   * Optionally, setting mode=ResizeMode::IgnoreData can disable the copy behavior.
   */
  void resize(size_t size, ResizeMode mode = ResizeMode::KeepData) noexcept {
    auto new_capacity = size > _size ? capacity_for(size, _data->capacity()) : capacity_for(size, _initial_capacity);
    if (new_capacity != _data->capacity()) {
      _data->resize(mode, new_capacity);
    }
    _size = size;
  }

  /**
   * Increment the total size by the given size, and return a writable Buffer that wraps this new memory.
   */
  inline Buffer reserve(size_t size) noexcept {
    auto offset = _size;
    resize(_size + size);
    return Buffer{_data, offset, size};
  }

  /**
   * Append the given buffer to the end of this FlexBuffer, growing by the given buffer's size.
   */
  FlexBuffer& operator<<(const BufferView& buffer) noexcept {
    return append(buffer.data(), 0, buffer.size());
  }

  /**
   * Append the given string to the end of this FlexBuffer, growing by the given string's size.
   */
  FlexBuffer& operator<<(const std::string_view& string) noexcept {
    return append(string.data(), 0, string.size());
  }

  /**
   * Append any fundamental type to the end of this FlexBuffer, growing by the given type's size.
   */
  template <typename T, typename = typename std::enable_if_t<std::is_fundamental_v<T>>>
  FlexBuffer& operator<<(const T& src) {
    auto dest = reserve(sizeof(T));
    memcpy(dest.data(), &src, sizeof(T));
    return *this;
  }

private:
  inline FlexBuffer& append(const char* src, size_t offset, size_t size) noexcept {
    auto dest = reserve(size);
    memcpy(dest.data(), reinterpret_cast<const char*>(src + offset), size);
    return *this;
  }
};

class BufferReader {
private:
  const BufferView _view;
  size_t _position = 0;

public:
  BufferReader() = delete;
  BufferReader(const BufferView& view) : _view(view){};
  BufferReader(const BufferReader&) = default;
  BufferReader& operator=(const BufferReader&) = default;
  BufferReader(BufferReader&&) = default;
  BufferReader& operator=(BufferReader&&) = default;

  /**
   * Get the read position
   */
  size_t position() const noexcept {
    return _position;
  }

  /**
   * Set the read position.
   */
  void position(size_t position) noexcept {
    _position = position;
  }

  /**
   * Get the remaining bytes to be read.
   */
  size_t remaining() const noexcept {
    return _position < _view.size() ? _view.size() - _position : 0;
  }

  /**
   * Get a view of the next "size" bytes of the underlying BufferView from the current position.
   * After creating the view, this Reader's position remains unchanged.
   */
  const BufferView peek(size_t size) const {
    return _view.subview(_position, size);
  }

  /**
   * Get a copy of any fundemental type from the underlying BufferView from the current position.
   * After reading the value, this Reader's position remains unchanged.
   */
  template <typename T, typename = typename std::enable_if_t<std::is_fundamental_v<T>>>
  T peek() const {
    return _view.read<T>(_position);
  }

  /**
   * Get a view of the next "size" bytes of the underlying BufferView from the current position.
   * After creating the view, this Reader's position is advanced by the size.
   */
  const BufferView next(size_t size) {
    BufferView result = _view.subview(_position, size);
    _position += size;
    return result;
  }

  /**
   * Get a copy of any fundemental type from the underlying BufferView from the current position.
   * After reading the value, this Reader's position is advanced by the size.
   */
  template <typename T, typename = typename std::enable_if_t<std::is_fundamental_v<T>>>
  T next() {
    auto result = _view.read<T>(_position);
    _position += sizeof(T);
    return result;
  }
};

class BufferWriter {
private:
  Buffer& _buf;
  size_t _position = 0;

public:
  BufferWriter() = delete;
  BufferWriter(Buffer& buf) : _buf(buf){};
  BufferWriter(const BufferWriter&) = delete;
  BufferWriter& operator=(const BufferWriter&) = delete;
  BufferWriter(BufferWriter&) = default;
  BufferWriter& operator=(BufferWriter&) = default;
  BufferWriter(BufferWriter&&) = default;
  BufferWriter& operator=(BufferWriter&&) = default;

  /**
   * Get the write position
   */
  size_t position() const noexcept {
    return _position;
  }

  /**
   * Set the write position.
   */
  void position(size_t position) noexcept {
    _position = position;
  }

  /**
   * Get the remaining bytes that may be written before overflowing the buffer.
   */
  size_t remaining() const noexcept {
    return _position < _buf.size() ? _buf.size() - _position : 0;
  }

  /**
   * Get a span of the next "size" bytes of the underlying Buffer from the current position.
   * After creating the span, this Reader's position remains unchanged.
   */
  Buffer peek(size_t size) {
    return _buf.subspan(_position, size);
  }

  /**
   * Get a span of the next "size" bytes of the underlying Bufer from the current position.
   * After creating the span, this Reader's position is advanced by the size.
   */
  Buffer next(size_t size) {
    Buffer result = _buf.subspan(_position, size);
    _position += size;
    return result;
  }

  /**
   * Write the given buffer at the current position, advancing the position by given buffer's size.
   */
  BufferWriter& operator<<(const BufferView& buffer) {
    return write(buffer.data(), 0, buffer.size());
  }

  /**
   * Write the given string at the current position, advancing the position by the given string's size.
   */
  BufferWriter& operator<<(const std::string_view& string) {
    return write(string.data(), 0, string.size());
  }

  /**
   * Write the given fundamental type's value at the current position, advancing the offset by the given type's size.
   */
  template <typename T, typename = typename std::enable_if_t<std::is_fundamental_v<T>>>
  BufferWriter& operator<<(const T& src) {
    if (_position + sizeof(T) > _buf.size())
      throw std::runtime_error{"array index out of bounds"};
    memcpy(reinterpret_cast<char*>(_buf.data() + _position), &src, sizeof(T));
    _position += sizeof(T);
    return *this;
  }

private:
  inline BufferWriter& write(const char* src, size_t offset, size_t size) {
    if (_position + size > _buf.size())
      throw std::runtime_error{"array index out of bounds"};
    memcpy(reinterpret_cast<char*>(_buf.data() + _position), reinterpret_cast<const char*>(src + offset), size);
    _position += size;
    return *this;
  }
};

} // namespace flexbuf

std::ostream& operator<<(std::ostream& os, const flexbuf::BufferView& view) {
  os << view.hex();
  return os;
}
