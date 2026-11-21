#include "working_env_c.h"

const char* GetCStandard(void)
{
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    return "C23";
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201710L
    return "C17/C18";
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    return "C11";
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    return "C99";
#elif defined(__STDC_VERSION__)
    return "C90";
#else
    return "C89/C90";
#endif
}
