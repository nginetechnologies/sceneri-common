#pragma once

#if __cplusplus == 202002L
#define CPP_VERSION 20
#elif __cplusplus == 201703L
#define CPP_VERSION 17
#else
#error "Unknown or unsupported C++ language version detected!"
#endif
