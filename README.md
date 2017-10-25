Valgrind: an enhanced version for pmem
======================================

[![Build Status](https://travis-ci.org/pmem/valgrind.svg)](https://travis-ci.org/pmem/valgrind)

This is the top-level README.md for the pmem-enhanced version of Valgrind.
This version has support for the new CLFLUSHOPT, PCOMMIT and CLWB
instructions. It also introduces a new tool called **pmemcheck**, which
validates the correctness of stores made to persistent memory. Be aware
that this is still a prototype tool.

Please see the file COPYING for information on the license.

The layout is identical to the original Valgrind.
The new tool is available in:

* **pmemcheck** -- the new persistent memory aware tool

All packages necessary to build this modified version of Valgrind are
the same as for the original version.

Note: This branch uses a git submodule for the VEX subproject. You can use git
clone --recursive to download both or execute these commands on an existing
repository:
```
$ git submodule init
$ git submodule update
```
A submodule update should be performed after each branch switch.

Valgrind is built using these commands at the top level:
```
	$ ./autogen.sh
	$ ./configure [--prefix=/where/to/install]
	$ make
```

To build tests:
```
	$ make check
```

To run all regression tests:
```
	$ make regtest
```

To run pmemcheck tests only:
```
	$ perl tests/vg_regtest pmemcheck
```

To install Valgrind run (possibly as root if destination permissions
require that):
```
	$ make install
```

The pmem-aware version of Valgrind is also supported on FreeBSD. Supplemental
package and build requirements are the same as those for
[PMDK](https://github.com/pmem/pmdk/blob/master/README.md).

For more information on Valgrind please refer to the original README
files and the documentation which is available at:
```
	$PREFIX/share/doc/valgrind/manual.html
```
Where $PREFIX is the path specified with --prefix to configure.

For information on how to run the new tool refer to the appropriate
part of the documentation or type:
```
	$ valgrind --tool=pmemcheck --help
```

For more information on the modifications made to Valgrind
contact Andy Rudoff (andy.rudoff@intel.com),
Tomasz Kapela (tomasz.kapela@intel.com) or
Krzysztof Czurylo (krzysztof.czurylo@intel.com).
