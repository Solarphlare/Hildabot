#pragma once

#define VERSION_NUMBER "3.2.2"
#define BUILD_NUMBER "12B018"

#ifdef __aarch64__
#define VERSION VERSION_NUMBER "-arm64"
#else
#ifdef __x86_64__
#define VERSION VERSION_NUMBER "-amd64"
#else
#define VERSION VERSION_NUMBER
#endif
#endif
