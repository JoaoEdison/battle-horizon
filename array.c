#include <stdlib.h>

struct vector_d {
	void *base;
	size_t sizememb, nmemb, len;
};

#define ALLOCATE_LOCAL(V, SIZEMEMB, LEN) \
	V.sizememb = SIZEMEMB; \
	V.len = LEN; \
	V.base = malloc(V.sizememb * V.len); \
	V.nmemb = 0;
#define LAST_SPACE_LOCAL(V) (V.base + V.sizememb * V.nmemb)
#define AT_PTR(V, I) (V->base + I * V->sizememb)
#define AT(V, I) (V.base + I * V.sizememb)
