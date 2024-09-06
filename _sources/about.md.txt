# About AEStream

AEStream aims to **simplify access to event-based neuromorphic technologies**, motivated by two problems:
1. the overhead to interact with any neuromorphic technologies (event cameras, neuromorphic chips, machine learning models, etc.) and
2. incompatibilities between neuromorphic systems (e. g. streaming cameras to [SpiNNaker](https://spinnakermanchester.github.io/)).

**The biggest innovation in AEStream is to converge on a single, unifying data structure: address-event representations (AER).**
By insisting that each input and output operates on the same notion of event, we are free to compose any input with any output.
And adding any peripheral is as simple as translating to or from AER.

## Related work
We are not the first to work towards user-friendly neuromorphic device libraries.
However, most of the libraries are developed from the vantage point of a hardware producer, single lab, or even the occasional lonely student who needs to solve a problem.
The figure below shows the current landscape (February 2023) of event-based libraries for cameras and AER files.
While there are much great software out there, AEStream is, at the moment of writing, supporting more peripherals than other libraries.

![](https://jegp.github.io/aestream-paper/2212_table.png)
<div style="text-align: center; color: #444; margin-top:0; font-size: 80%;">
An overview of open-source libraries for event-based processing based on the underlying code, python bindings, and native I/O support. Icons indicate support for GPUs, event-based cameras, files, and network transmission. “N/A” shows that no native outputs are supported. * Sepia supports cameras via extensions.
</div>

## AEStream 
A paper on AEStream was published at the Neuro Inspired Computational Elements Conference in 2023, available as a preprint on arXiv: [https://arxiv.org/abs/2212.10719](https://arxiv.org/abs/2212.10719).
In a streaming setting, the paper concludes:

> In sum, AEStream reduces copying operations to the GPU by a factor of at least 5 and allows processing at around 30% more frames (65k versus 5k in total) over a period of around 25 seconds compared to conventional synchronous processing. 

## Developer affilications and acknowledgements
AEStream is an inter-institutional and international effort, spanning several research institutions and compa.
Particularly, AEStream has received contributions from 
* KTH Royal Institute of Technology
* University of Heidelberg
* University of Groningen
* Heriot-Watt University
* European Space Agency

The developers would like to thank Anders Bo Sørensen for his friendly and invaluable help with CUDA and GPU profiling. Emil Jansson deserves our gratitude for scrutinizing and improving the coroutine benchmark C++ code.

We gracefully recognize funding from the EC Horizon 2020 Framework Programme under Grant Agreements 785907 and 945539 (HBP). Our thanks also extend to the Pioneer Centre for AI, under the Danish National Research Foundation grant number P1, for hosting us. 