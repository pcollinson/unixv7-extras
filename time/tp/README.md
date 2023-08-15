# Small change to the _tp_ command

The _tp_ command uses _localtime_ to get a formatted date string, but needs changing to understand that year values greater than or equal to 100 should be interpreted in the 21st century and should have 100 removed from the value. This change is in _tp3.c_.

## Compiling

This should be done after the library is updated.

Type ```make``` to compile and ```make install``` to install.
