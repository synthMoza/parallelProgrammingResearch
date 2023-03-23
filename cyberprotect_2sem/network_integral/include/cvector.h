#ifndef _CVECTOR_H
#define _CVECTOR_H
#include <stdlib.h>
/* Usage:
 * In global space:
 * 	vector_define(my_type);
 * 	or
 * 	typedef vector_define(my_type) my_type_vec_t;
 *	now vector_spec(my_type), my_type_vec_t refer to vector type
 * vector_spec(my_type) vec;
 * or
 * malloc(sizeof(vector_spec(my_type)), 1);
 * vector_init(&vec);
 * vector_reserve+vector_elem/vector_push_back/iterate vector_buf
 * vector_destroy(&vec)
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#define vector_spec(type) struct __vector_spec_##type

#define vector_define(in_type)					\
vector_spec(in_type) {						\
	in_type *buf;						\
	size_t size;						\
	size_t cap;						\
}

#define vector_clear(in_vec)					\
do {											\
	typeof(in_vec) const __vec = (in_vec);		\
	__vec->size = 0;							\
} while (0)

#define vector_init(in_vec)					\
do {								\
	typeof(in_vec) const __vec = (in_vec);			\
	__vec->buf = NULL;					\
	__vec->size = 0;					\
	__vec->cap = 0;						\
} while (0)

/* return -1 on failure */
#define vector_reserve(in_vec, in_cap)				\
({ 								\
	__label__ ret; int rc;					\
	typeof(in_vec) const __vec = (in_vec);			\
	typeof(in_cap) const __cap = (in_cap);			\
	if (__cap <= __vec->cap)				\
		{ rc = 0; goto ret; }				\
	typeof(__vec->buf) ptr;					\
	if (__vec->cap == 0)					\
		ptr = malloc(sizeof(*ptr) * __cap);		\
	else							\
		ptr = realloc(__vec->buf, sizeof(*ptr) * __cap);\
	if (ptr == NULL)					\
		{ rc = -1; goto ret; }				\
	__vec->buf = ptr;					\
	__vec->cap = __cap;					\
	rc = 0;							\
	ret:							\
		rc;						\
})

#define vector_destroy(in_vec)					\
do {								\
	typeof(in_vec) __vec = (in_vec);			\
	if (__vec->buf != NULL)					\
		free(__vec->buf);				\
} while (0)


/* return -1 on failure */
#define vector_push_back(in_vec, in_el)				\
({								\
	__label__ ret; int rc;					\
	typeof(in_vec) const __vec = (in_vec);			\
	if (__vec->size == __vec->cap) {			\
		size_t new_cap;					\
		if (__vec->cap == 0)				\
			new_cap = 16;				\
		else						\
			new_cap = __vec->cap * 2;		\
		if (vector_reserve(__vec, new_cap) < 0)		\
			{ rc = -1; goto ret; }			\
	}							\
	__vec->buf[__vec->size] = (in_el);			\
	__vec->size++;						\
	rc = 0;							\
	ret:							\
		rc;						\
})

#pragma GCC diagnostic pop

/* vector info */
#define vector_value_type(vec) typeof(*(vec)->buf)
#define vector_elem(vec, offs) ((vec)->buf[offs])
#define vector_size(vec) ((vec)->size)
#define vector_capacity(vec) ((vec)->cap)
#define vector_buf(vec) ((vec)->buf)

/* beg/end pointers: for(ptr = beg(&vec); ptr < end(&vec); ++ptr) */
#define vector_beg(vec) vector_buf(vec)
#define vector_end(in_vec)					\
({								\
 	typeof(in_vec) const __vec = (in_vec);			\
	typeof(__vec->buf) rc;					\
	rc = vector_buf(__vec) + vector_size(__vec);		\
	rc;							\
})

#endif /* _CVECTOR_H */
