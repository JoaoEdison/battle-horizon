/*
  Battle Horizon is a 3D space battle game in Raylib
  Copyright (C) 2023-2025  João E. R. Manica
  
  Battle Horizon is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  Battle Horizon is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; If not, see <http://www.gnu.org/licenses/>
*/
#include <stdlib.h>

typedef struct {
	void *base;
	size_t sizememb, nmemb, len;
} vector_d;

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
