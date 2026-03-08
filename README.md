# IoError BBS Client

This fork is **2.3.10-Stilgar** by Stilgar, based on the ISCABBS client 1.5.1
(stdio patch) by Serendipity.

Project page: <https://github.com/StilgarISCA/IoErrorBbsClient>
ISCA BBS site: <https://iscabbs.rocks/>

## Historical Contributors

- Michael Hampton (IO ERROR)
- Doug Siebert (Serendipity)
- Marc Dionne (Marco Polo)
- David Bailey
- Dave Lacey
- Dave (Isoroku)
- Client 9 / Blackout Group

More background is documented in `history.md`.

## Quick Start

```bash
make clean
autoreconf -i
./configure
make -j4
make check
make cppcheck
make install
```

### Build Modes

The default configure path produces a development build.

- Dev build is the default
- Dev build uses debug-friendly flags
- macOS builds still add host-appropriate tuning flags by default:
  - Apple Silicon builds use Apple Silicon tuning flags
  - Intel Mac builds use Intel tuning flags

Default dev build:

```bash
make clean
autoreconf -i
./configure
make -j4
make check
make cppcheck
```

Release build:

```bash
make clean
autoreconf -i
./configure --enable-release-build
make -j4
make check
make cppcheck
```

Universal macOS build for both Apple Silicon and Intel Macs:

```bash
make clean
autoreconf -i
./configure --enable-universal-binary
make -j4
make check
make cppcheck
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
/opt/homebrew/opt/llvm/bin/clang-format -i $(git ls-files '*.c' '*.h')

# Verify build
make clean
autoreconf -i
./configure
make -j4
make check
make cppcheck
```

## License

- See `LICENSE` for the GNU GPL v2.0-or-later terms used by this project.
- OpenSSL exception: linking this software to OpenSSL is explicitly permitted,
  provided all other GPL and OpenSSL license obligations are satisfied.
