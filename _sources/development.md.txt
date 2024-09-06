# Development documentation

This page gives a few tips on interacting with the code base, for instance to developing your own extensions for AEStream.

For simplicity, we will discuss the native C++ (command-line) and Python API use cases separately.

## Native C++

The native C++ interface can be setup via Cmake (see [Installation](install)).
In essense, AEStream maps streams of input events to streams of output events. 
To add an event pre-processor, for instance, would correspond to the following C++

```c++

#include "generator.hpp"
Generator<AER::Event> some_transformation(Generator<AER::Event> &stream) {
  for (auto event : stream) { // Iterate over incoming events
    // Add logic here
    co_yield event; // Send event downstream
  }
}
```

where `co_yield` emits an event downstream.
Note that the function does not need to return. 
The `co_yield` is novel notation for the [C++20 coroutine standard](https://en.cppreference.com/w/cpp/language/coroutines).

To add your own input, simply produce a `Generator`. To add your own output, simply consume a `Generator`.

The CLI interface is available in `aestream.cpp` and uses the [CLI11 CLI library](https://github.com/CLIUtils/CLI11).

## Python API

The Python interface is a wrapped version of the C++ API.
Meaning, we add python-specific functionality that is exposed in Python using the [nanobind Python binding library](https://nanobind.readthedocs.io/).
Currently, we only support *reading* inputs (such as files, USB devices, network inputs), and exposing them to Python for further processing.
Because some event devices emit millions of events per second (such as event cameras), it would be impractical to expose every single event to Python.
Therefore, most Python wrappers operate on some form of aggregation, such arrays for Numpy, Jax, PyTorch, and Tensorflow.

We make heavy use of the [Python contextlib](https://docs.python.org/3/library/contextlib.html) for resource management (the `with ... as ...` notation). The benefit is that the user doesn't have to worry about opening/closing resources correctly.
Example code can be found in [module.cpp in the `src/pybind` package](https://github.com/aestream/aestream/blob/main/src/pybind/module.cpp)