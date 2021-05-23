# libdiscord

A C Library for building discord bots.

# Requirements

- GCC
- Make
- OpenSSL
- Pthreads
- jansson

# Installation libdiscord

- Create your project folder and open a terminal in that folder.
- `git clone https://github.com/mall0cd/libdiscord.git ./`
- `sudo make libs`

The above steps will build all the files and compile them on your machine. Afterwards it will be ready for use, Some examples are present in `examples/`

# How to use the library?

If you've installed libdiscord correctly you should be able to include the following headers in a `.c` or `.h` file.

```c
#include <libdiscord/gateway.h>
#include <libdiscord/wsc.h>
#include <libdiscord/bot.h>
#include <libdiscord/rest.h>
#include <libdiscord/types.h>
```

These are the headers required to create a very basic bot
```c
#include <libdiscord/gateway.h>
#include <libdiscord/wsc.h>
#include <libdiscord/bot.h>
#include <libdiscord/rest.h>
#include <libdiscord/types.h>
#include <string.h>
#include <sys/time.h>
```

Please read the source code and the examples for more information and usage. Currently the library is still in work so there is not much documentation created yet.
Soon all the methods will be described in the repo's wiki page.
