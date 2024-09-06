# Installation

AEStream is usable both as a command-line binary or Python tool.

| **Source** | **Installation**
| -------------------- | --- |
| [pip](https://pypi.org/) | <code>pip install aestream |
| [nix](https://nixos.org/) | <code>nix run github:aestream/aestream</code> (CLI) <br/> <code>nix develop github:aestream/aestream</code> (Python environment) |
| [docker](https://docker.com/) | See [Installation documentation](https://aestream.github.io/aestream/install.html) |

Contributions to support AEStream on additional platforms are always welcome.

## Installing via pip
Run `pip install aestream`

Installing via pip is the most convenient method for installing AEStream and gives you both the [command-line interface](cli) and the [Python API](python_usage).
Pip is the [Python package manager](https://pip.pypa.io/en/stable/installation/) and is accessible on most computers.

### Installing with CUDA support
If you want to use AEStream with CUDA support, you likely need to build the package yourself. This is because the CUDA version must match the one installed on your system.
To do so, simply run `pip install aestream --no-binary aestream` to avoid using the binary cache.
It will take a few minutes to compile, but if you have an NVIDIA GPU, you'll see dramatic performance improvements.
Note that you can provide a `-v` flag to enable verbose output, which will show you if the CUDA drivers were detected (look for `CUDA found`).
If you need to specify a different C++ compiler, be sure to also specify it for NVCC:

```CXX=/path/to/g++-10 NVCC_PREPEND_FLAGS='-ccbin /path/to/g++-10' pip install aestream --no-binary aestream```

### Event camera drivers
AEStream can read from [Inivation](https://gitlab.com/inivation/dv/libcaer/) or [Prophesee](https://github.com/prophesee-ai/openeb/) event cameras, *given that the drivers are installed*.
**Please make sure the drivers are installed before installing aestream** (step 1 below).

1. Follow the instructions at [Inivation libcaer](https://gitlab.com/inivation/dv/libcaer/) to install the Inivation drivers and/or [Prophesee Openeb](https://github.com/prophesee-ai/openeb/) to install the Prophesee drivers
2. Install AEStream with pip: `pip install aestream --no-binary aestream -v`
    * The `--no-binary` flag forces pip to recompile aestream for your system, which will detect the event camera drivers, if present.
    * The `-v` flag enables verbose output, which will show you if the drivers were detected
    * If you already have an installation, you need to re-install
3. Ensure that the drivers were detected by inspecting the installation log
    * If successful, you should see messages like `-- Inivation dependencies (libcaer) found at /usr/local/lib/cmake/libcaer`
4. Test your installation by running the [`usb_video.py` example](https://github.com/aestream/aestream/blob/main/example/usb_video.py). If a window with streaming events pops up, you are successful!

Once installed, the cameras can be used in both [CLI](cli) or [Python](python_usage).

We are working to automate the driver installation as a part of AEStream. Help is welcome and appreciated!

## Installing via Docker
A `Dockerfile` is available in the root directory and can be built with the `Dockerfile` available in the aestream root directory as follows:
```bash
docker build -t aestream .
```

## Installing via source
Source installations can either be done via pip, which wraps the C++ code, or CMake, where you have to manually configure the project.

### Python source install and development
```bash
git clone https://github.com/aestream/aestream
cd aestream
pip install .
```

Note that this will setup the install in an isolated virtual environment, which is slow and can cause problems if you would like to work with the source code.
To avoid this, we recomment using `--no-build-isolation` in the pip install, but that requires manually installing the necessary packages for the build system, shown below.

```bash
# Pull the source code
git clone https://github.com/aestream/aestream
cd aestream
# Install build dependencies
pip install scikit-build-core setuptools_scm pathspec nanobind
# Build aestream, but without build isolation
# Any future compilation will *only* compile the files you changed
pip install --no-build-isolation .
```


### CMake source install
You can also install the C++ code directly using [CMake](https://cmake.org/).
```bash
git clone https://github.com/aestream/aestream
cd aestream
mkdir build
cd build
cmake -GNinja .. 
ninja install
```
Note that this will only install the CLI version of AEStream.
To have access to the Python features, you must enable the `USE_PYTHON` config in CMAKE by running `cmake -GNinja -DUSE_PYTHON=ON ..` instead.

If your default C++ compiler doesn't support C++ 20, you will have to install an up-to-date compiler and provide the environmental variable `CXX`.
For instance like this: `CXX=/path/to/g++-10 cmake -GNinja ..`

## Requirements
AEStream relies on modern compiler features and require at least [GCC 10](https://gcc.gnu.org/) and [CMake 3.20](https://cmake.org/).

If you are on older machines that default to GCC versions *below* 10, you can enforce a specific version by setting the environmental variable `CXX`. Here is an example for pip:

```CXX=/path/to/g++-10 pip install aestream```

If you have problems with CMake, try installing the latest version from pip: `pip install cmake`.

## Common issues

Here, we list some common issues that users encounter during installation.
If you run into problems that is not listed here, please [open an issue](https://github.com/aestream/aestream/issues/new).

```{admonition} CMake Error: Could not find a package configuration file provided by "nanobind" with any of the following names: ...
    :class: warning

This happens when the `nanobind` dependency could not be found. Run `pip install nanobind` to install it and retry.

```