import random

from curves import *

from encode import point_to_values, values_to_point, point_to_values_for_u_j
from weaver import precompute, weaver, eval_weave
from decoyauth import simulate_decoyauth_handshake


# -------------------- Point Encoding --------------------------

def test_point_to_values_vectors():
	P1 = (	0x12D4D7A6C9F0AB5AEFA23333BC1CDF1C74BAF471A609BE195FF1DCDF159F25EA,
		0x1FC1DE07FD09F9C252522FD3C3FEF274B6063EA46D6BD4123870B052692C43BD)
	u, j = 0x1eec2cac75144e8ee1ea7e265cc2037d05fba6d8dcd0eae2282f9cfe7f4ff34e, 3
	v = point_to_values_for_u_j(P1, secp256r1_a, secp256r1_b, secp256r1_p, u, j)
	assert v == 0x6d0b798762d5d606fb4605a3dc298b58034bdb171fc9a4b221bcabcdf4431c3d

	P2 = (	0x82A689728B6991B3DE5EB4363D8DA7D95D14691B4D220E9FD2375FE239B17AB0,
		0x7C4B4A95926581BD9328B0AFC2AF217214FD6FA59F55BB06091EDEA866175173)
	u, j = 0x2794a1cb1c776c726fd8fde0da43d008bc57aac937168c9554343586a1051a72, 3
	v = point_to_values_for_u_j(P2, secp256r1_a, secp256r1_b, secp256r1_p, u, j)
	assert v == 0x26735102b201ecbb88da7657444819d07b164b9a22f8aeefe334329250578167

	P3 = (	0x888837674F7254D03F64401EB23431DDC554BAB34B7A87A2606C332CA2E6CDC2,
		0x195AC98C52F5FED05C7008D35277AB68396CCCABA37EE530DB0735621A864FA7)
	u, j = 0xf93c890af49b337e28e5d12daaf54f9a27392eaeca5bfd57d4350360137317b7, 0
	v = point_to_values_for_u_j(P3, secp256r1_a, secp256r1_b, secp256r1_p, u, j)
	assert v == 0xf04d4b87921f04c93f6b31bdccf7c9cec08df9747fbafca22b142be85da5a99b


def test_values_to_point_vectors():
	u = 0x1eec2cac75144e8ee1ea7e265cc2037d05fba6d8dcd0eae2282f9cfe7f4ff34e
	v = 0x6d0b798762d5d606fb4605a3dc298b58034bdb171fc9a4b221bcabcdf4431c3d
	P = values_to_point(u, v, secp256r1_a, secp256r1_b, secp256r1_p)
	assert P[0] == 0x12D4D7A6C9F0AB5AEFA23333BC1CDF1C74BAF471A609BE195FF1DCDF159F25EA
	assert P[1] == 0x1FC1DE07FD09F9C252522FD3C3FEF274B6063EA46D6BD4123870B052692C43BD

	u = 0x2794a1cb1c776c726fd8fde0da43d008bc57aac937168c9554343586a1051a72
	v = 0x26735102b201ecbb88da7657444819d07b164b9a22f8aeefe334329250578167
	P = values_to_point(u, v, secp256r1_a, secp256r1_b, secp256r1_p)
	assert P[0] == 0x82A689728B6991B3DE5EB4363D8DA7D95D14691B4D220E9FD2375FE239B17AB0
	assert P[1] == 0x7C4B4A95926581BD9328B0AFC2AF217214FD6FA59F55BB06091EDEA866175173

	u = 0xf93c890af49b337e28e5d12daaf54f9a27392eaeca5bfd57d4350360137317b7
	v = 0xf04d4b87921f04c93f6b31bdccf7c9cec08df9747fbafca22b142be85da5a99b
	P = values_to_point(u, v, secp256r1_a, secp256r1_b, secp256r1_p)
	assert P[0] == 0x888837674F7254D03F64401EB23431DDC554BAB34B7A87A2606C332CA2E6CDC2
	assert P[1] == 0x195AC98C52F5FED05C7008D35277AB68396CCCABA37EE530DB0735621A864FA7


def test_point_conversion():
	Ps = [(	0x12D4D7A6C9F0AB5AEFA23333BC1CDF1C74BAF471A609BE195FF1DCDF159F25EA,
		0x1FC1DE07FD09F9C252522FD3C3FEF274B6063EA46D6BD4123870B052692C43BD),
	      (	0x82A689728B6991B3DE5EB4363D8DA7D95D14691B4D220E9FD2375FE239B17AB0,
		0x7C4B4A95926581BD9328B0AFC2AF217214FD6FA59F55BB06091EDEA866175173),
	      (	0x888837674F7254D03F64401EB23431DDC554BAB34B7A87A2606C332CA2E6CDC2,
		0x195AC98C52F5FED05C7008D35277AB68396CCCABA37EE530DB0735621A864FA7)]
	for Pi in Ps:
		u, v = point_to_values(Pi, secp256r1_a, secp256r1_b, secp256r1_p)
		recovered_point = values_to_point(u, v, secp256r1_a, secp256r1_b, secp256r1_p)
		assert Pi[0] == recovered_point[0], f"Expected {hex(P[0])}, got {hex(recovered_point[0])})"
		assert Pi[1] == recovered_point[1], f"Expected {hex(P[0])}, got {hex(recovered_point[0])})"


# -------------------- Point Weaving --------------------------

def test_precompute_vectors():
	xs = [0, 1]
	matrix = precompute(xs, 13)
	assert matrix == [[1, 0], [12, 1]]

	xs = [0, 1, 7, 12]
	matrix = precompute(xs, 13)
	assert matrix == [[1, 0, 0, 0], [11, 6, 7, 2], [12, 7, 0, 7], [2, 1, 6, 4]]

	xs = [	0xb978ded97cd42d43de79f385bbb5a30f6fde475bef67f42f2545adc1b8de48ae,
		0xb38725b87bb746384b1b1fb7999c5a4527cb34655b5fc7064ce2e48ba4c580d5,
		0xac475b1db500eb3d257ddb79b749294b4748f80a718059630ebca92974fe20dc]
	matrix = precompute(xs, secp256r1_p)
	assert matrix == [
		[0x36af9b2504a2cef45b428a73450417dd5ba4e3b0b58d8c780179cdfc47ba9a5f,
		 0xfdeabbfcfcf4790c5a8760d1d9bcbbf1953d91d11361f1cc0d3911146492e838,
		 0xcb65a8dbfe68b8014a3614bae13f2c310f1d8a80371081bbf14d20ef53b27d68], 
		[0xce28e0b1b7e0fb871a1718f1b2470620378a71c3866e1cb05f1101019646d50f,
		 0x58e141785963f7e44bc6aaa1aed02f108260d6a8729a97987c2f6e60badc8f8d,
		 0xd8f5ddd3eebb0c969a223c6c9ee8cacf4614b79606f74bb724bf909daedc9b62], 
		[0x9cac2bc76af618be4b50c55411974c2026b85331ced8ef82cd4f8e5d341f5407,
		 0x7f1b9d6ef0861e6d222281e3b1a6d164f782a03bdba2987567964cf553c1be33,
		 0xe43836c7a483c8d6928cb8c83cc1e27ae1c50c9455847807cb1a24ad781eedc4]]


def test_weaver_vectors():
	ys = [4, 8]
	matrix = [[1, 0], [12, 1]]
	vals = weaver(ys, matrix, 13)
	assert vals == [4, 4]
	
	ys = [0, 4, 9, 11]
	matrix = [[1, 0, 0, 0], [11, 6, 7, 2], [12, 7, 0, 7], [2, 1, 6, 4]]
	vals = weaver(ys, matrix, 13)
	assert vals == [0, 5, 1, 11]

	ys = [	0x1eec2cac75144e8ee1ea7e265cc2037d05fba6d8dcd0eae2282f9cfe7f4ff34e,
		0x2794a1cb1c776c726fd8fde0da43d008bc57aac937168c9554343586a1051a72,
		0xf93c890af49b337e28e5d12daaf54f9a27392eaeca5bfd57d4350360137317b7]
	matrix = [
		[0x36af9b2504a2cef45b428a73450417dd5ba4e3b0b58d8c780179cdfc47ba9a5f,
		 0xfdeabbfcfcf4790c5a8760d1d9bcbbf1953d91d11361f1cc0d3911146492e838,
		 0xcb65a8dbfe68b8014a3614bae13f2c310f1d8a80371081bbf14d20ef53b27d68], 
		[0xce28e0b1b7e0fb871a1718f1b2470620378a71c3866e1cb05f1101019646d50f,
		 0x58e141785963f7e44bc6aaa1aed02f108260d6a8729a97987c2f6e60badc8f8d,
		 0xd8f5ddd3eebb0c969a223c6c9ee8cacf4614b79606f74bb724bf909daedc9b62], 
		[0x9cac2bc76af618be4b50c55411974c2026b85331ced8ef82cd4f8e5d341f5407,
		 0x7f1b9d6ef0861e6d222281e3b1a6d164f782a03bdba2987567964cf553c1be33,
		 0xe43836c7a483c8d6928cb8c83cc1e27ae1c50c9455847807cb1a24ad781eedc4]]
	vals = weaver(ys, matrix, secp256r1_p)
	assert vals == [
		0xe222adae266aa3160866ae3b651f7cd2f0a889784ae15ec54618cd18a7d101b4,
		0x6bf2dfef3535033a07a42c805fc6af78a91a4d329c6ceae41ec2737e4a6a0dca,
		0xbae617cc1945320c9275536eaa014fe49e9a15b322b3cf5ba00618c76befd667]


def test_eval_weave_vectors():
	vals = [4, 4]
	assert eval_weave(vals, 0, 13) == 4
	assert eval_weave(vals, 1, 13) == 8

	vals = [0, 5, 1, 11]
	assert eval_weave(vals, 0, 13) == 0
	assert eval_weave(vals, 1, 13) == 4
	assert eval_weave(vals, 7, 13) == 9
	assert eval_weave(vals, 12, 13) == 11

	vals = [0xe222adae266aa3160866ae3b651f7cd2f0a889784ae15ec54618cd18a7d101b4,
		0x6bf2dfef3535033a07a42c805fc6af78a91a4d329c6ceae41ec2737e4a6a0dca,
		0xbae617cc1945320c9275536eaa014fe49e9a15b322b3cf5ba00618c76befd667]
	assert eval_weave(vals, 0xb978ded97cd42d43de79f385bbb5a30f6fde475bef67f42f2545adc1b8de48ae, secp256r1_p) == \
			0x1eec2cac75144e8ee1ea7e265cc2037d05fba6d8dcd0eae2282f9cfe7f4ff34e
	assert eval_weave(vals, 0xb38725b87bb746384b1b1fb7999c5a4527cb34655b5fc7064ce2e48ba4c580d5, secp256r1_p) == \
			0x2794a1cb1c776c726fd8fde0da43d008bc57aac937168c9554343586a1051a72
	assert eval_weave(vals, 0xac475b1db500eb3d257ddb79b749294b4748f80a718059630ebca92974fe20dc, secp256r1_p) == \
			0xf93c890af49b337e28e5d12daaf54f9a27392eaeca5bfd57d4350360137317b7


def test_weaver_random():
	for testid in range(100):
		prime = random.choice([13, 59, 97])
		xs_len = random.randint(2, prime - 1)			# random number between [2, prime[
		xs = random.sample(range(0, prime - 1), xs_len)		# xs_len unique values betwen [0, prime[
		ys = [random.randint(0, prime - 1) for _ in range(xs_len)] # xs_len values between [0, prime[
		matrix = precompute(xs, prime)
		vals = weaver(ys, matrix, prime)
		for i in range(len(xs)):
			assert eval_weave(vals, xs[i], prime) == ys[i]


# -------------------- Handshake Execution --------------------------

def test_multikey_decoyauth():
	for i in range(20):
		simulate_decoyauth_handshake()

