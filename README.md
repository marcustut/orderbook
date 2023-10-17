# Orderbook

This project started as an endeavor for myself to learn about these 3 main things:

1. FIX Protocol
2. CMake
3. Writing TUI

This project basically provides a Terminal-based UI (TUI) client for viewing order book data across different instruments 
on the [Deribit](https://www.deribit.com) exchange. Deribit was chosen because it is one of the very few crypto exchanges 
that provides a Financial information eXchange (FIX) API.

### Features

- [x] Connect Deribit through FIX
- [x] Receive raw orderbook market data
- [x] Maintain an in-memory local order book (allows to view best bid, best ask)
- [ ] Supports multi instruments in local order book
- [ ] Terminal UI Client

### Dependencies

The project depends on several dependencies:

- [OpenSSL](https://github.com/openssl/openssl)
- [Quickfix](https://github.com/quickfix/quickfix)
- [Google Test](https://github.com/google/googletest)

CMake automatically find an installation of OpenSSL if it exists in the operating system. 

Quickfix can be installed by building from source following instructions on their repo or through the package manager [vcpkg](https://github.com/microsoft/vcpkg) 
by running `vcpkg install quickfix`. 

Google Test can be installed by building from source or vcpkg as well by doing `vcpkg install gtest`.

### Building and running

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

After running the command above you should see that there is a binary `orderbook_cli` being built. The binary can be invoked 
by doing running `./orderbook_cli`, make sure that you have a `fix_settings.cfg` file alongside the binary. The config file 
should resembles `example_fix_settings.cfg`.