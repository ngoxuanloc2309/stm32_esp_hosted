#ifndef __CTRL_CORE_FIX_H
#define __CTRL_CORE_FIX_H

#ifdef command_log
#undef command_log
#endif
#define command_log(...) do { printf(__VA_ARGS__); printf("\r"); } while(0)

#endif