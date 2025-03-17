from Cryptodome.PublicKey import ECC

# Parameters of the curve P-256, the default curve used in WPA3.
# Prime modulus p: determined valid range of x and y coordinates.
secp256r1_p = 0xffffffff00000001000000000000000000000000ffffffffffffffffffffffff
# The parameters a and b in the elliptic curve formula Y^2 = X^3 + a * x + b.
# Note that a equals (-3 mod p) which here is represented as (secp256r1_p - 3).
secp256r1_a = 0xFFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC
secp256r1_b = 0x5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B
# Order of the base point = number of points on the curve.
secp256r1_r = 0xffffffff00000000ffffffffffffffffbce6faada7179e84f3b9cac2fc632551
# The paramater z is used for the SSWU hash-to-curve method. How this number is calculated,
# and values for some curves, is explained in IEEE 802.11 2020 section 12.4.4.2.3.
# The algorithm for finding z is also covered in RFC 9380.
secp256r1_z = -10

# Same parameters as above, but now for the curve P-224. This curve is weak though.
secp224r1_p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000001
secp224r1_a = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFE
secp224r1_b = 0xB4050A850C04B3ABF54132565044B0B7D7BFD8BA270B39432355FFB4
secp224r1_r = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFF16A2E0B8F03E13DD29455C5C2A3D
secp224r1_z = 31

curve_params = {
	'secp256r1': (secp256r1_p, secp256r1_a, secp256r1_b, secp256r1_r, secp256r1_z),
	'secp224r1': (secp224r1_p, secp224r1_a, secp224r1_b, secp224r1_r, secp224r1_z)
}

# Pycryptodome also provides some of the constants. Verify them here.
for curve in curve_params:
	p, a, b, r, z = curve_params[curve]
	assert ECC._curves[curve].p == p
	assert ECC._curves[curve].b == b
	assert ECC._curves[curve].order == r

