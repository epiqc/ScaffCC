# Rotation Library Generator

This is a C++ implemetation of the *library construction* method for generating *Rz* rotations. The main features of this generator include:
  - Powered by [gridsynth](http://www.mathstat.dal.ca/~selinger/newsynth/), the package can generate rotation sequences that approximate arbitray *Rz* angles, up to given precision.
  - Generate libraries of rotation sequences given use-defined precision and storage requirements, trading storage for execution time.
  - Dynamically concatenate rotation sequences at run time using generated libraries.

### Library Construction Method

The library construction method stems from optimizations for dynamic code generation. Specifically, many quantum algorithms, like Ground State Estimation, require arbitrary rotation angles (either known or unknown at compile time), which is typically realized/approximated by a sequence of operations, such as the Cifford+T gates. Rather than calling core generator for all rotations at run time, the library construction method uses a pre-compiled library (of some basis angles), and rapidly assembles the library rotations to construct the desired angle. 

### Example

We have provided an example code to help you getting started with the tool. To compile the example code, run
```sh
$ make
$./example 2 2 3 110 1
```
which will produce a library with precision up to 3 binrary floating point digits. It will not exceed 110 bytes in size.

More concretely, when generating a rotation library for the first time, you would need to specify the following parameters as input:
- The library will then consist of angles: {pi, pi/b, pi/b<sup>2</sup>, pi/b<sup>3</sup>, ... }, until the required precision can be achieved.
  - *b*: basis of the angles. 
- The guaranteed precision will be *k* base-*n* places. E.g. if *n=10, k=10*, then the angles generated is guaranteed to have precision up to 10 decimal places.
  - *n*: base of precision (can be 2 or 10).
  - *k*: number of places in base-*n* precision
- For storage, you would need to provide the size and the unit, such as 100 KB. 
- You may also specify whether to have the rotations decomposed up to global phase or not.

Some important functions from RotLib class you may take advantage of are:
- RotLib::generate() - which envokes the core rotation generator and decompresses rotation sequences with Huffman encoding.
- RotLib::save(filename) - which writes the encoded library into output file.
- RotLib::load(filename) - which loads previously saved library from file.
- RotLib::concatenate(angle[, factor]) - which automatically assembles library angles for the desired angle, optionally with factor = "pi". Note that angle is in RotLib::Rz type, whose members include:
  - angle -> theta: rotation angle in radian
  - angle -> gates: the decomposed rotation sequence
  - angle -> length: number of operations in sequence

Yongshan (yongshan@uchicago.edu)
