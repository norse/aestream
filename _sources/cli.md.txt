# Command-line interface

The AEStream command-line interface (CLI) provides a mean to **read** some input and **stream** to some output.
We support a wide range of inputs and outputs.

The CLI interface *requires* an input, but an *optional* output and takes the following form:
```bash
aestream input <input source> [output <output sink>]
```

## Supported Inputs

| Input | Description | Usage |
| --------- | :----------- | ----- |
| DAVIS, DVXplorer | [Inivation](https://inivation.com/) DVS Camera over USB | `input inivation` |
| EVK Cameras      | [Prophesee](https://www.prophesee.ai/) DVS camera over USB  | `input prophesee` |
| File             | [AEDAT file format](https://gitlab.com/inivation/inivation-docs/blob/master/Software%20user%20guides/AEDAT_file_formats.md) as `.aedat`, `.aedat4`, `.dat`, `.raw`, or `.csv` | `input file x.aedat4` |
| ZMQ              | [ZeroMQ](https://zeromq.org/) input | `input zmq`


### Inivation and Prophesee cameras
Streams camera data from Inivation or Prophesee cameras via USB.
Note that this requires that you installed and configured the appropriate drivers, see the [installation instructions](install).

### File inputs
Streams data from a file. The file type is inferred from the file extension. Supported file types are `.aedat`, `.aedat4`, `.dat`, `.raw`, and `.csv`.

By default, the files will be played back at the same speed as they were recorded.
We assume events are streamed with microsecond time resolution, but this can be changed by specifying `--time-unit` with either `us`, `ms`, or `s`, e.g. `--time-unit ms`.
If you wish to stream the events as fast as possible, simply add the `--ignore-time` flag.

### ZMQ inputs
Streams data from a ZeroMQ socket. The socket defaults to `tcp://0.0.0.0:40001`, but can be customized with the `sock` option in the CLI, e.g. `input zmq sock tcp://0.0.0.0:40002`.

## Supported outputs

| Output | Description | Usage |
| --------- | ----------- | ----- |
| STDOUT    | Standard output (default output) | `output stdout`
| Ethernet over UDP | Outputs to a given IP and port using the [SPIF protocol](https://github.com/SpiNNakerManchester/spif)  | `output udp 10.0.0.1 1234` |
| File  | Output to [`.aedat4`](https://gitlab.com/inivation/inivation-docs/blob/master/Software%20user%20guides/AEDAT_file_formats.md#aedat-40) or comma-separated-value files (CSV) | `output file my_file.aedat4` |

### Ethernet over UDP
Streams data to a given IP and port using the SPIF protocol. The IP and port are specified as arguments to the `output udp` command. You can modify the buffer size with the `--buffer-size` option, e.g. `--buffer-size 1024` (default). This is handy when working with high-speed or resource constrained networks.

### File outputs
Saves events to a file, whose format is inferred from the file extension. Supported file types are `.aedat4` and `.csv`/`.txt`. Example: `... output file my_file.aedat4`.