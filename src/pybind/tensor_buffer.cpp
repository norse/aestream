#include "tensor_buffer.hpp"
#include <iostream>

namespace nb = nanobind;

inline buffer_t allocate_buffer(const size_t &length, std::string device) {
#ifdef USE_CUDA
  if (device == "cuda") {
    // Thanks to https://stackoverflow.com/a/47406068
    buffer_t buffer_ptr(alloc_memory_cuda_float(length), BufferDeleter());
    return buffer_ptr;
  }
#endif
  return buffer_t(new float[length]{0}, BufferDeleter());
}

// TensorBuffer constructor
TensorBuffer::TensorBuffer(py_size_t size, std::string device,
                           size_t buffer_size)
    : shape(size), device(device) {
#ifdef USE_CUDA
  if (device == "cuda") {
    cuda_device_pointer = alloc_memory_cuda_int(buffer_size);
    offset_buffer = std::vector<int>(buffer_size);
  }
#endif
  buffer1 = allocate_buffer(size[0] * size[1], device);
  buffer2 = allocate_buffer(size[0] * size[1], device);
}

TensorBuffer::~TensorBuffer() {}

void TensorBuffer::set_buffer(uint16_t data[], int numbytes) {
  const auto length = numbytes >> 1;
  const std::lock_guard lock{buffer_lock};
#ifdef USE_CUDA
  if (device == "cuda") {
    offset_buffer.clear();
    for (int i = 0; i < length; i = i + 2) {
      // Decode x, y
      const uint16_t y_coord = data[i] & 0x7FFF;
      const uint16_t x_coord = data[i + 1] & 0x7FFF;
      offset_buffer.push_back(shape[1] * x_coord + y_coord);
    }
    index_increment_cuda(buffer1.get(), offset_buffer, cuda_device_pointer);
    return;
  }
#endif
  for (int i = 0; i < length; i = i + 2) {
    // Decode x, y
    const int16_t y_coord = data[i] & 0x7FFF;
    const int16_t x_coord = data[i + 1] & 0x7FFF;
    assign_event(buffer1.get(), x_coord, y_coord);
  }
}

void TensorBuffer::set_vector(std::vector<AER::Event> events) {
  const std::lock_guard lock{buffer_lock};
#ifdef USE_CUDA
  if (device == "cuda") {
    for (size_t i = 0; i < events.size(); i++) {
      offset_buffer[i] = shape[1] * events[i].x + events[i].y;
    }
    index_increment_cuda(buffer1.get(), offset_buffer, cuda_device_pointer);
    return;
  }
#endif
  for (auto event : events) {
    assign_event(buffer1.get(), event.x, event.y);
  }
}

template <typename R>
inline void TensorBuffer::assign_event(R *array, int16_t x, int16_t y) {
  (*(array + shape[1] * x + y))++;
}

BufferPointer TensorBuffer::read() {
  // Swap out old pointer
  {
    const std::lock_guard lock{buffer_lock};
    buffer1.swap(buffer2);
  }
  // Create new buffer and swap with old
  buffer_t buffer3 = allocate_buffer(shape[0] * shape[1], device);
  buffer2.swap(buffer3);
  // Return pointer
  return BufferPointer(std::move(buffer3), shape, device);
}

BufferPointer::BufferPointer(buffer_t data, const std::vector<int64_t> &shape,
                             std::string device)
    : data(std::move(data)), shape(shape), device(device) {}

tensor_numpy BufferPointer::to_numpy() {
  const size_t s[2] = {shape[0], shape[1]};
  return tensor_numpy(data.release(), 2, s);
}

tensor_torch BufferPointer::to_torch() {
  const size_t s[2] = {shape[0], shape[1]};
  int32_t device_type =
      device == "cuda" ? nb::device::cuda::value : nb::device::cpu::value;
  return tensor_torch(data.release(), 2, s, nanobind::handle(), /* owner */
                      nullptr,                                  /* strides */
                      nanobind::dtype<float>(), device_type);
}