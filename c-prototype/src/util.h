#include <openssl/bn.h>
#include <openssl/ec.h>

#pragma once

#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

void print_bignum(const BIGNUM * bn);

void print_ec_point(const EC_POINT * ep);

BIGNUM **read_hashes(const char *filename, const int num_points);

EC_POINT **read_points(const char *filename, EC_GROUP * group,
		       const int num_points);

void free_hashes(BIGNUM ** hashes, const int num_points);

void free_points(EC_POINT ** points, const int num_points);

void store(const char *filename, double value, int comma);

void storeln(const char *filename, double value, int comma);

void export_bignums(const char *filename, BIGNUM ** bignums, int count);

BIGNUM **import_bignums(const char *filename, int count);

#ifdef __cplusplus
}
#endif
#endif				// UTIL_H
