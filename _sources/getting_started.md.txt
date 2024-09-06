# Getting Started

AEStream is a powerful C++ library that can be used either directly in the command-line (CLI) or via Python.
You can install it directly via pip or nix: `pip install aestream` ([find more options here](https://aestream.github.io/aestream/install.html)).


AEStream wires arbitrary inputs to arbitrary outputs, allowing you to flexibly move events from files, cameras, and networks to files, networks, or even data types (such as [numpy](https://numpy.org) or [PyTorch](https://pytorch.com)) for further processing.
Here's a visualization of the inputs and outputs.

<img src="https://jegp.github.io/aestream-paper/2212_aestream.svg" />

## Command-line interface (CLI)

> Read more in the [CLI guide](cli.md)

The AEStream CLI is a console interface that avoids the overhead of Python or graphical frontends, and it even works on resource-constrained systems.

AEStream CLI requires a *mandatory* input, but an *optional* output and takes the following form:
```bash
aestream input <input source> [output <output sink>]
```

Here are two concrete examples streaming from an [Inivation](https://inivation.com) camera or file to a UDP sink and standard out, respectively:
```bash
aestream input inivation dvx output udp 10.0.0.1
aestream input file f.aedat4 output stdout 
```

More information about inputs/outputs can be found with a `--help` flag:
```bash
aestream --help
```

## Python API

> Read more in the [Python usage guide](python_usage.md)

The aim of the Python API is to export tensors for further processing with libraries like [Norse](https://github.com/norse/norse) or [Tonic](https://github.com/neuromorphs/tonic).

TBD

### Example: detect edges in real-time with biological neural networks
We stream events from a camera connected via USB and process them on a GPU in real-time using the [spiking neural network library, Norse](https://github.com/norse/norse).
The left panel in the video above shows the raw signal, while the middle and right panels show horizontal and vertical edge detection respectively.
![](https://github.com/aestream/aestream/raw/main/example/usb_edgedetection.gif)
To solve this problem, we need to (1) prepare the neural network, (2) access the camera, (3) inject event "frames" to the neural network, and (4) visualize the results.

```python
import torch, norse, aestream

# Initialize our canvas
from sdl import create_sdl_surface, events_to_bw

window, pixels = create_sdl_surface(640 * 3, 480)

# (1) Prepare neural network
kernel_size = 9
gaussian = torch.sigmoid(torch.linspace(-10, 10, kernel_size + 1))
kernel = (gaussian.diff() - 0.14).repeat(kernel_size, 1)
kernels = torch.stack((kernel, kernel.T))
convolution = torch.nn.Conv2d(1, 2, kernel_size, padding=12, bias=False, dilation=3)
convolution.weight = torch.nn.Parameter(kernels.unsqueeze(1))

net = norse.torch.SequentialState(
    norse.torch.LIFRefracCell(),
    convolution,
)
state = None  # Start with empty state

try:
    # (2) Start streaming from a DVS camera
    with USBInput((640, 480)) as stream:
        while True:  # Loop forever
            # Read a tensor (640, 480) tensor from the camera
            tensor = stream.read()
            # (3) Run the tensor through the network, while updating the state
            with torch.inference_mode():
                filtered, state = net(tensor.view(1, 1, 640, 480), state)

            # (4) Render output
            pixels[0:640] = events_to_bw(tensor)  # Input events
            pixels[640 : 640 * 2] = events_to_bw(filtered[0, 0])  # First channel
            pixels[640 * 2 : 640 * 3] = events_to_bw(filtered[0, 1])  # Second channel
            window.refresh()

finally:
    window.close()
```
The example can also be found in [`example/usb_edgedetection.py`](https://github.com/aestream/aestream/blob/main/example/usb_edgedetection.py)
