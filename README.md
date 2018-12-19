# MK14_Windows_tinyC
MK14 built for windows 10 using TinyC

Tiny C is available from https://bellard.org/tcc/ and comes with a minimum windows example
Paul Robson's MK14 had a version that build under Borland C++ 4.5

This builds using Tiny C
One or two bugs have been fixed.

The CPU.C did jmp 80 using E. this prevented version 1 ROM monitor from working

memory.c now has both versions of the Monitor ROM.
You can preload RAM 

I added a timer to run the CPU in a time slice
I added some LEDs connected to the FLAG outputs
