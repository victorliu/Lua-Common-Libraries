LCL: Lua Common Libraries
=========================

Motivation
----------

The most widely used Lua versions (5.0 and 5.1) and the latest versions
that are distributed (5.2 and 5.3) have subtly different APIs, making it
difficult to write cross-platform libraries using the C API. The LCL
includes a small C/C++ library to augment the Lua C API to standardize
some parts of it that has changed over the various 5.x versions.

Additionally, the included libraries are very barebones, making Lua a
formidable language to get started with. The main difficulty is the lack
of scientific computing functionality, which forms the basis of many
higher level functionalities. The LCL includes additional libraries for
performing linear algebra and geometric predicates.

Supported Lua versions
----------------------

The current libraries support 5.0, 5.1, 5.2, and 5.3. These libraries
should also be compatible with LuaJIT 2.0 (similar to Lua 5.1).

