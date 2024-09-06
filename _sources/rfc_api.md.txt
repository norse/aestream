# API Request For Comment

Overall, the goal is to provide a unified library that helps “route” events from sources to sinks, inputs to outputs. This happens in two modes: synchronous or asynchronous. The synchronous case addresses reproducible processing (ML/DL) while the asynchronous mode targets real-time streaming of events. The two modes share some I/O primitives such as files and network I/O, but peripherals such as event cameras only make sense in the streaming mode. See table below for a complete list of I/O for the two modes.

## Synchronous mode

In synchronous mode, events can be served one-by-one or **aggregated into precisely defined containers** such as chunks of N events or of (X,Y) frames of a specific time width. These “parts” can either be read all at once (at the risk of memory overflowing) or as an iterator (promising partial processing).

## Asynchronous mode

Asynchronous processing uses parallelizes I/O to improve throughput, and **cannot guarantee deterministic containers** due to the inversion of control. Practically, asynchronous processing happens by first 1) starting one or more background threads that “dump” events into a container, and 2) users “scraping” or “reading” that container at a chosen interval in a separate thread. Example: USB camera streaming events to a GPU tensor.

## Use cases

| **Mode** | **Grouping**         | **Format** | **Output type**  | **Code**                                                     |
| -------- | -------------------- | ---------- | ---------------- | ------------------------------------------------------------ |
| Read     | All                  | Event      | 1d event array   | .get_events() -> 1d                                          |
|          |                      | Frame      | 2d frame         | .get_frame() -> 2d                                           |
|          | Part (event count)   | Event      | 2d event array   | ?.get_events(n_events=X) -> 2d                               |
|          |                      | Frame      | 3d frame         | .get_frames(n_events=X)                                      |
|          | Part (time interval) | Event      | 2d event array   | ?.get\_                                                      |
|          |                      | Frame      | 3d frame         | ?.get\_                                                      |
| Iterate  | Single events        | Event      | 1 (single) Event | for e in input                                               |
|          | Part (event count)   | Event      | 1d event array   | EventIterator(input, n_events=X)<br>input.events(n_events=X) |
|          |                      | Frame      | 2d frame         | FrameIterator(input, n_events=X)                             |
|          | Part (time interval) | Event      | 1d event array   | EventIterator(input, interval=Y)                             |
|          |                      | Frame      | 2d frame         | FrameIterator(input, interval=Y)                             |
|          | Part (time + count)  |            |                  | EventIterator(input, interval=Y, n_events=X)                 |
| Stream   | Frames               | Frame      | 2d event array   | with Input() as x:<br>&nbsp;&nbsp;&nbsp;&nbsp;x.frame()      |

**Note**: UDP input/output is only supported from CLI at the moment.

## I/O sources and sinks

| Peripheral               | Input | Output            | Sync                | Async |
| ------------------------ | ----- | ----------------- | ------------------- | ----- |
| File<br>.dat, .aedat4, … | Yes   | Yes (CLI only)    | Yes                 | Yes   |
| Network                  | Yes   | Yes (CLI only)    | No (Yes? Blocking?) | Yes   |
| Event cameras            | Yes   | No                | No                  | Yes   |
| STDIO/STDOUT             | Yes   | Yes (CLI only)    | No                  | Yes   |
| Numpy arrays             | No    | Yes (Python only) | Yes                 | Yes   |
| PyTorch tensors          | No    | Yes (Python only) | Yes                 | Yes   |
| Anything else?           |


The package in its basic form would offer file+network reading and Numpy tensors, but with [pip extensions](https://setuptools.pypa.io/en/latest/userguide/dependency_management.html) it can detect and support PyTorch `pip install aestream[torch]` and different cameras `pip install aestream[inivation]`, `pip install aestream[prophesee]`.

