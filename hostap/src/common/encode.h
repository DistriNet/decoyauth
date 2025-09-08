#include <openssl/ec.h>
#include <stdbool.h>

#pragma once

#ifndef ENCODE_H
#define ENCODE_H

#ifdef __cplusplus
extern "C" {
#endif

BIGNUM* X_0(BIGNUM* u, BIGNUM* a, BIGNUM* b, BIGNUM* prime);

BIGNUM* X_1(BIGNUM* u, BIGNUM* a, BIGNUM* b, BIGNUM* prime);

EC_POINT* f(BIGNUM* u, EC_GROUP* group, BIGNUM* a, BIGNUM* b, BIGNUM* prime);

BIGNUM* g(BIGNUM* x, BIGNUM* a, BIGNUM* b, BIGNUM* prime);

BIGNUM* calc_v(EC_POINT* q, int j, EC_GROUP* group, BIGNUM* a, BIGNUM* b, BIGNUM* prime);

BIGNUM** point_to_values(EC_POINT* point, EC_GROUP* group, BIGNUM* a, BIGNUM* b, BIGNUM* prime);

EC_POINT* values_to_point(BIGNUM** encoded_point, EC_GROUP* group, BIGNUM* a, BIGNUM* b, BIGNUM* p);

#ifdef __cplusplus
}
#endif

#endif // ENCODE_H
