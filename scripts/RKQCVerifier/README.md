# RKQCVerifier

RKQCVerifier is a tool that helps verify the correctness of RKQC applications.
In this folder, there are three files: example.scaffold, prebuilt.c, and rkqc_verify.py.

- example.scaffold is a scaffold file that we use it to demonstrate how this tool works.
- prebuilt.c is a set of premitive functions.
- rkqc_verifyy.py is used to translate scaffold file into c file, and it also automatically uses g++ compiler to compile it into a classical executable file. This tool will generate an executable file called rkqc_executable. You can verify the correctness by simply running this executable file.

### Usage:

```sh
$ python rkqc_verify.py example.scaffold
```
