#include "encode.h"
#include "weaver.h"
#include "util.h"
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <time.h>

#define MAX_NUM_POINTS 1000

#define CONFIG_PRECOMPUTE
#define CONFIG_EXPORT_MATRIX

#define A                                                                        \
    "11579208921035624876269744694940757353008614341529031419553363130886709785" \
    "3948"
#define B                                                                        \
    "41058363725152142129326129780047268409114441015993725554835256314039467401" \
    "291"
#define P                                                                        \
    "11579208921035624876269744694940757353008614341529031419553363130886709785" \
    "3951"

// Helper function to calculate elapsed time in nanoseconds
double elapsed_ns(struct timespec start, struct timespec end)
{
	return (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec -
						    start.tv_nsec);
}

int measure_operations(int num_points, int num_iterations, int num_warmup)
{
	EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
	BN_CTX *ctx = BN_CTX_new();

	char filename[256] = { 0 };
#ifdef CONFIG_PRECOMPUTE
	sprintf(filename, "../testing/raw_algo_measurements/measurements_%dp.csv", num_points);
	FILE *fp = fopen(filename, "a");
	fprintf(fp,
		"encoding,precomputation,weaver,evaluation,decoding\n");
#else
	sprintf(filename, "../testing/raw_algo_measurements/measurements_%d.csv", num_points);
	FILE *fp = fopen(filename, "a");
	fprintf(fp, "encoding,weaver,evaluation,decoding\n");
#endif
	fclose(fp);

	EC_POINT **points = read_points("points.txt", group, num_points);
	if (points == NULL)
		goto return_free_group_ctx;

	BIGNUM **hashes = read_hashes("hashes.txt", num_points);
	if (hashes == NULL) {
		free_points(points, num_points);
		goto return_free_group_ctx;
	}

	BIGNUM *a = BN_new();
	BN_dec2bn(&a, A);
	BIGNUM *b = BN_new();
	BN_dec2bn(&b, B);
	BIGNUM *prime = BN_new();
	BN_dec2bn(&prime, P);

	// Calculate the scaling correction factor, this is necessary as more
	// passwords runs the encoding this has to be compensated, so testcases with
	// a low amount of passwords do almost as much runs of the encoding as the
	// high amount of passwords, we assume a max of 1000 points for current
	// measurements
	int scaling_correction = MAX_NUM_POINTS / num_points;
	if (scaling_correction < 1) {
		scaling_correction = 1;
	}
	printf("Scaling correction: %d\n", scaling_correction);

	for (int i = 0; i < num_iterations; i++) {
		BIGNUM **u_values =
		    (BIGNUM **) malloc(num_points * sizeof(BIGNUM *));
		BIGNUM **v_values =
		    (BIGNUM **) malloc(num_points * sizeof(BIGNUM *));

		struct timespec start_time, end_time;
		double time_spent;

		for (int j = 0; j < num_warmup; j++)
			es_encode(points[0], group, a, b, prime);

		// Encoding         (FOR ALL POINTS)
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time);
		for (int x = 0; x < scaling_correction; x++) {
			for (int j = 0; j < num_points; j++) {
				BIGNUM **pair =
				    es_encode(points[j], group, a, b,
					      prime);
				u_values[j] = pair[0];
				v_values[j] = pair[1];
				free(pair);
			}
		}
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time);
		time_spent = elapsed_ns(start_time, end_time);
		store(filename,
		      time_spent / (double)scaling_correction /
		      1000000.0, 0);

		char matrix_filename[256] = { 0 };
		sprintf(matrix_filename, "../matrices/matrix_%d.txt", num_points);

#ifdef CONFIG_PRECOMPUTE
		// Precomputation   (FOR ALL POINTS)
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time);
		BIGNUM **matrix =
		    precompute(hashes, num_points, prime, ctx);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time);
		time_spent = elapsed_ns(start_time, end_time);
		store(filename, time_spent / 1000000.0, 1);
#ifdef CONFIG_EXPORT_MATRIX
		export_bignums(matrix_filename, matrix,
			       num_points * num_points);
#endif
#else
		BIGNUM **matrix =
		    import_bignums(matrix_filename,
				   num_points * num_points);
#endif

		// Weaver    (FOR ALL POINTS)
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time);
		BIGNUM **c_u =
		    weave(u_values, matrix, num_points, prime,
				ctx);
		BIGNUM **c_v =
		    weave(v_values, matrix, num_points, prime,
				ctx);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time);
		time_spent = elapsed_ns(start_time, end_time);
		store(filename, time_spent / 1000000.0, 1);

		// Evaluation       (FOR ONE POINT)
		BIGNUM ***pairs =
		    (BIGNUM ***) malloc(num_points * sizeof(BIGNUM **));
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time);
		BIGNUM *rec_u =
		    evaluate(c_u, hashes[0], num_points, prime, ctx);
		BIGNUM *rec_v =
		    evaluate(c_v, hashes[0], num_points, prime, ctx);
		BIGNUM **pair =
		    (BIGNUM **) malloc(2 * sizeof(BIGNUM *));
		pair[0] = rec_u;
		pair[1] = rec_v;
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time);
		time_spent = elapsed_ns(start_time, end_time);
		store(filename, time_spent / 1000000.0, 1);

		// Decoding         (FOR ONE POINT)
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time);
		EC_POINT *recovered_point;
		for (int x = 0; x < MAX_NUM_POINTS; x++)
			recovered_point =
			    es_decode(pair, group, a, b, prime);
		BN_free(pair[0]);
		BN_free(pair[1]);
		free(pair);
		EC_POINT_free(recovered_point);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time);
		time_spent = elapsed_ns(start_time, end_time);
		storeln(filename,
			time_spent / MAX_NUM_POINTS / 1000000.0, 1);

		// CLEANUP MEMORY
		for (int j = 0; j < num_points; j++) {
			BN_free(u_values[j]);
			BN_free(v_values[j]);
		}
		free(u_values);
		free(v_values);

		for (int j = 0; j < num_points * num_points; j++)
			BN_free(matrix[j]);
		free(matrix);

		for (int j = 0; j < num_points; j++)
			BN_free(c_u[j]);
		free(c_u);

		for (int j = 0; j < num_points; j++)
			BN_free(c_v[j]);
		free(c_v);

		free(pairs);
	}

	BN_free(a);
	BN_free(b);
	BN_free(prime);

	free_points(points, num_points);
	free_hashes(hashes, num_points);

return_free_group_ctx:
	EC_GROUP_free(group);
	BN_CTX_free(ctx);
}

int generate_encoded_points()
{
	EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);

	BIGNUM *a = BN_new();
	BN_dec2bn(&a, A);
	BIGNUM *b = BN_new();
	BN_dec2bn(&b, B);
	BIGNUM *prime = BN_new();
	BN_dec2bn(&prime, P);

	EC_POINT **points = read_points("points.txt", group, 10000);

	FILE *fp = fopen("encoded_points.txt", "w");
	if (fp == NULL) {
		printf("Error opening file for writing\n");
		return 1;
	}

	printf("Encoding points in points.txt and saving them to encoded_points.txt ...\n");

	for (int i = 0; i < 10000; i++) {
		BIGNUM **pair = es_encode(points[i], group, a, b, prime);

		char *u_str = BN_bn2dec(pair[0]);
		char *v_str = BN_bn2dec(pair[1]);

		fprintf(fp, "%s\n", u_str);
		fprintf(fp, "%s\n", v_str);

		OPENSSL_free(u_str);
		OPENSSL_free(v_str);
		BN_free(pair[0]);
		BN_free(pair[1]);
		free(pair);
	}

	fclose(fp);
}

int main(int argc, char **argv)
{
	int num_points, num_iterations, num_warmup;

	if (argc == 1) {
		generate_encoded_points();
		return 0;
	}
	else if (argc < 4) {
		printf
		    ("Usage: %s <num_points> <num_iterations> <num_warmups>\n",
		     argv[0]);
		return 1;
	}

	num_points = atoi(argv[1]);
	if (num_points <= 0) {
		printf("Error: Number of points must be a positive integer\n");
		return 1;
	}

	num_iterations = atoi(argv[2]);
	if (num_iterations <= 0) {
		printf
		    ("Error: Number of iterations must be a positive integer\n");
		return 1;
	}

	num_warmup = atoi(argv[3]);
	if (num_warmup <= 0) {
		printf
		    ("Error: Number of iterations must be a positive integer\n");
		return 1;
	}

	measure_operations(num_points, num_iterations, num_warmup);
}
