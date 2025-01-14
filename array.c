#include <stdlib.h>

struct vector_d {
	void *base;
	size_t sizememb, nmemb, len;
};

#define ALLOCATE_PTR(V, SIZEMEMB, LEN) \
	V->sizememb = SIZEMEMB; \
	V->len = LEN; \
	V->base = malloc(V->sizememb * V->len); \
	V->nmemb = 0;
#define ALLOCATE(V, SIZEMEMB, LEN) \
	V.sizememb = SIZEMEMB; \
	V.len = LEN; \
	V.base = malloc(V.sizememb * V.len); \
	V.nmemb = 0;

#define LAST_SPACE_PTR(V) (V->base + V->sizememb * V->nmemb)
#define LAST_SPACE(V) (V.base + V.sizememb * V.nmemb)

#define AT_PTR(V, I) (V->base + I * V->sizememb)
#define AT(V, I) (V.base + I * V.sizememb)

#define EXPAND_PTR(V, INC) \
	if (V->nmemb == V->len) \
		V->base = realloc(V->base, V->sizememb * (V->len += INC));
#define EXPAND(V, INC) \
	if (V.nmemb == V.len) \
		V.base = realloc(V.base, V.sizememb * (V.len += INC));
