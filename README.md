# MK14_Windows_tinyC
MK14 built for windows 10 using TinyC

Tiny C is available from https://bellard.org/tcc/ and comes with a minimum windows example

Paul Robson's MK14 had a version that build under Borland C++ 4.5

This builds using Tiny C

One or two bugs have been fixed.

The CPU.C did jmp 80 using E. This prevented version 1 ROM monitor from working.

memory.c now has both versions of the Monitor ROM.

I added a timer to run the SC/MP CPU in a time slice.

I added some LEDs connected to the FLAG outputs.

You can preload RAM. 0fA0 has some code to write to the flag LEDs. 
