# Change to the _man_ macros

The _man_ command uses _/usr/lib/tmac/tmac.an_ as its basic troff/nroff macro package. It assumes that the value of the year coming out of the _localtime_ routine is in the 1900's.  This change adds 1900 to the value of the two year number registers and removes '19' from the heading definitions.
