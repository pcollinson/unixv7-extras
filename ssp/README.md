# The ssp command

_ssp_ is  program written by Bill Joy, and was extracted from University of California’s 1BSD release. _ssp_ stands for - single space output, and removes successive blank lines from its standard input. The program is used automatically when output is to the terminal, or not otherwise.

### Compilation

To compile use the _makefile_:

``` sh
make
```
### Installation

Again use the _makefile_ to install the binary and the manual page:

``` sh
make install
```

### Man scripts

I've install the _man_ script here into my own private _bin_ directory. The _man7_ command is useful it you have a manual page in a file and you want to see what it says.
