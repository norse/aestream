#include "tensor_iterator.hpp"

TensorIterator::TensorIterator(Generator<AER::Event> &generator,
                               py_size_t shape, size_t time_window)
    : generator(generator), shape(shape), time_window(time_window){};

template <typename T>
inline void TensorIterator::assign_event(T *array, int16_t x, int16_t y) {
  (*(array + shape[1] * x + y))++;
}

tensor_t TensorIterator::next() {
  const size_t size = shape[0] * shape[1];
  float array[size];
  for (const auto &event : generator) {
    assign_event(array, event.x, event.y);
    if (event.timestamp >= current_timestamp + time_window) {
      current_timestamp = event.timestamp;
      break;
    }
  }
#ifdef USE_TORCH
  auto t = torch::from_blob(array, {shape[0], shape[1]});
#else
  auto t = tensor_t(size, array);
  t.resize(shape);
  t.owndata();
#endif
  return t;
}
