![Sceneri_Banner](https://github.com/user-attachments/assets/2f9cc95e-b40d-4c29-86e0-7b9b3cdff32a)

![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/nginetechnologies/sceneri-common/.github%2Fworkflows%2Fworkflow.yml)
[![Discord](https://img.shields.io/discord/842853727606013963?style=plastic&logo=discord&logoColor=white&label=Discord&link=https%3A%2F%2Fdiscord.gg%2Fsceneriapp)](https://discord.gg/sceneriapp)

This is the Common API (aka standard library) of the [Sceneri](http://sceneri.com) game / interactive media engine supporting creation & play of 2D and 3D experiences on and for any device. This API drives the low-level core of Sceneri, and does not include the engine itself.

The common library is routinely tested on:
- Windows (x64)
- Linux (x64)
- macOS (x64 & arm64)
- iOS (arm64)
- visionOS (arm64)
- Android (arm64)
- Web (WebAssembly, via Emscripten)

The library is currently provided as-is, but we encourage using it in your projects and bringing improvements back via pull requests.

Keep in mind that Sceneri is optimized for performance, and this standard library does take shortcuts where possible - don't expect full conformance to the C++ std library. We follow the principle of breaking often until Sceneri is where we want it to be, so no guarantees of API & ABI stability for now.

We are currently compiling with C++17, but aim to upgrade to C++20 soon - and will at some point also start utilizing C++20 features such as modules. Style-wise we currently differ from std's lower-case style but are considering switching over.

Notable functionality included is:
- Assert implementation
- Function / Event implementations (including FlatFunction and more)
- IO utilities such as File wrappers, file change listeners, library loading and more.
- Equivalent to std::filesystem with extensions (see IO::Path, IO::PathView, IO::File, IO::FileView etc)
- IO::URI implementation, extending on IO::Path (see above) for generic URI handling
- Extensive math library focusing on performance and vectorization (SSE, AVX, NEON, WASM SIMD)
- Custom allocators and containers such as vectors, flat vectors, inline vectors, maps and more.
- Implementation of Any, AnyView, TypeDefinition extending the core concept of std::any
- Custom Tuple and Variant
- Custom Optional implementation, with a focus on overloads for types that can avoid an extra boolean
- Serialization support, wrapped on top of rapidjson (with the intention to use simdjson at some point)
- Atomics & 128-bit numerics
- Threading utilties to multi-thread across all platforms including web via web workers
- Mutexes and shared mutexes across all platforms including web workers
- Type traits
- High precision timestamps, stopwatches and more.
- libfmt driven formatting of custom types
