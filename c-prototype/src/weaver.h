#include <openssl/bn.h>

#pragma once

#ifndef WEAVER_H
#define WEAVER_H

#ifdef __cplusplus
extern "C" {
#endif

	BIGNUM **precompute(BIGNUM ** x_values, int num_elements,
			    const BIGNUM * prime, BN_CTX * ctx);

	BIGNUM **weave(BIGNUM ** y_values, BIGNUM ** matrix,
			     int num_elements, BIGNUM * prime, BN_CTX * ctx);

	BIGNUM *evaluate(BIGNUM ** vals, BIGNUM * x, int num_elements,
			 BIGNUM * prime, BN_CTX * ctx);

#ifdef __cplusplus
}
#endif
#endif				// WEAVER_H
