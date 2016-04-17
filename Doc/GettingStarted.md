### Getting started

On recent Raspbian distributions, QPULib should work out-of-the-box.
It's been tested on the Pi 1 Model B, the Pi 2, but not yet the Pi
3.

##### Building and running the GCD example

Try the commands below to build and run the `GCD` example.

```
sudo apt-get install git
git clone https://github.com/mn416/QPULib
cd QPULib/Tests
make QPU=1 GCD
sudo ./GCD
```

The `QPU=1` flag to `make` indicates that the physical QPUs
on the Raspberry Pi should be used.  Simply using `make` without
`QPU=1` will default to **emulation mode**.  As the name suggests,
this means that QPU code will be emulated in software.  This is useful
for debugging, and also allows you to run QPULib programs on a PC if
your Pi is not to hand.

Strictly speaking, any program that works in emulation mode but not on
the Pi's physical QPUs is probably a bug in QPULib and should be
reported, although there may be valid explanations for such
differences.

##### Other examples

Here is a quick overview of some the other examples, which can be
built in the same way as GCD:

  Example   | Description | Output
  --------- | ----------- | ------
  GCD       | [Euclid's algorithm](https://en.wikipedia.org/wiki/Euclidean_algorithm) | The GCD's of some random pairs of integers
  Tri       | Computes [triangular numbers](https://en.wikipedia.org/wiki/Triangular_number) | The first 16 triangular numbers
  MultiTri  | Like Tri, but uses 4 QPUs instead of 1 | The first 64 triangular numbers
  TriFloat  | Like Tri but casts floats to integers and back | The first 16 triangular numbers
  OET       | [Odd-even transposition sorter](https://en.wikipedia.org/wiki/Odd%E2%80%93even_sort) | 32 integers, sorted
  HeatMap   | Modelling heat flow across a 2D surface | An image in [pgm](http://netpbm.sourceforge.net/doc/pgm.html) format, and the time taken
  Rot3D     | 3D rotation of a random object | The time taken

##### AutoTest: specification-based testing of the compiler

Another program in the `Tests` directory worth mentioning is
`AutoTest`: it generates random QPULib programs, runs them on the both
source language interpreter and the target language emulator, and
checks for equivalance.  Currently, it only works in emulation mode.

##### CPU/GPU memory split

Depending on your plans, it may be useful to ensure that plenty of
memory is available to the GPU.  This can be done by using
`raspi-config`, selecting `Advanced Options` and then `Memory Split`:
(On a Raspberry Pi 1 Model B, 32M seems to be the minimum that works
for me.)
