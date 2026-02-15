# IoError BBS Client

This fork is **2.3.10-Stilgar** by IO ERROR, based on the ISCABBS client 1.5.1
(stdio patch) by Serendipity.

Project page: <http://ioerror.bbsclient.net/>

## Quick Start

```bash
autoreconf -i
./configure
make
make install
```

## Formatting and Linting

```bash
# Generate/refresh compile_commands.json for clang-tidy
bear -- make clean all -j4

# Run static analysis with cppcheck
make cppcheck

# Run full analysis (cppcheck + clang-tidy when available)
make analyze
```

```bash
# Add required braces to control statements
clang-tidy -p . -fix -fix-errors -format-style=file \
  -checks='-*,readability-braces-around-statements' \
  --extra-arg=-isysroot \
  --extra-arg="$(xcrun --sdk macosx --show-sdk-path)" \
  $(git ls-files '*.c')
```

```bash
# Apply repository formatting rules (.clang-format)
clang-format -i $(git ls-files '*.c' '*.h')

# Verify build
make clean
make -j4
```

## License

This software is OSI Certified Open Source Software. OSI Certified is a
certification mark of the Open Source Initiative.

Copyright (C) 1995-2003 Michael Hampton.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

Additional grant of rights: You are expressly permitted to link this software
to the OpenSSL library, so long as in all other respects you remain in
compliance with both the GNU General Public License and the OpenSSL license.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 675 Mass
Ave, Cambridge, MA 02139, USA.
