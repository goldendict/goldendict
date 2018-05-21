#ifndef __CPP_HH_INCLUDED__
#define __CPP_HH_INCLUDED__

#if __cplusplus > 199711L
#define THROW_SPEC(...)
#else
#define THROW_SPEC(...) throw(__VA_ARGS__)
#endif

#endif // CPP_HH
