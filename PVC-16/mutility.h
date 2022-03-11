#pragma once

#ifndef _DEBUG
#if defined(_MSC_VER)
#define UNREACHABLE __assume(0)
#elif defined(__GNUC__)
#define UNREACHABLE __buitin_unreachable()
#else
#define UNREACHABLE abort()
#endif
#else
#define UNREACHABLE abort()
#endif