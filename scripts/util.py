import math, hashlib, hmac, struct
from datetime import datetime

ALL, DEBUG, INFO, STATUS, WARNING, ERROR = range(6)
COLORCODES = { "gray"  : "\033[0;37m",
               "green" : "\033[0;32m",
               "orange": "\033[0;33m",
               "red"   : "\033[0;31m" }

global_log_level = INFO
def log(level, msg, color=None, showtime=True):
	if level < global_log_level: return
	if level == DEBUG   and color is None: color="gray"
	if level == WARNING and color is None: color="orange"
	if level == ERROR   and color is None: color="red"
	msg = (datetime.now().strftime('[%H:%M:%S] ') if showtime else " "*11) + COLORCODES.get(color, "") + msg + "\033[1;0m"
	print(msg)

def set_log_level(level):
	global global_log_level
	global_log_level = level

def invert_mod(a, p):
	return pow(a, -1, p)

def legendre_symbol(a, p):
	if a % p == 0: return 0

	ls = pow(a, (p - 1)//2, p)
	return -1 if ls == p - 1 else ls

def is_quadratic_residue(a, p):
	return legendre_symbol(a, p) == 1

def modular_sqrt(a, p):
	if legendre_symbol(a, p) != 1:
		return None
	if a == 0:
		return 0
	if p % 4 == 3:
		return pow(a, (p + 1) // 4, p)
	s, e = p - 1, 0
	while s % 2 == 0:
		s //= 2
		e += 1
	n = 2
	while legendre_symbol(n, p) != -1:
		n += 1
	x = pow(a, (s + 1) // 2, p)
	b = pow(a, s, p)
	g = pow(n, s, p)
	r = e
	while True:
		t = b
		m = 0
		for m in range(r):
			if t == 1:
				break
			t = pow(t, 2, p)
		if m == 0:
			return x
		gs = pow(g, 2 ** (r - m - 1), p)
		g = gs * gs % p
		x = x * gs % p
		b = b * g % p
		r = m

def int_to_bytes(i, length):
	return i.to_bytes(length, byteorder='big')

def point_to_bytes(p, length):
	return int_to_bytes(p.x, length) + int_to_bytes(p.y, length)

def bytes_to_int(b):
	return int.from_bytes(b, byteorder='big')

def ceq(x, y):
	return x == y

def csel(cond, x, y):
	return x if cond else y

def lsb(x):
	return x & 1

def HMAC256(pw, data):
	return hmac.new(pw, data, hashlib.sha256).digest()

def hkdf_extract(salt, ikm, hash_algo=hashlib.sha256):
	return hmac.new(salt, ikm, hash_algo).digest()

def hkdf_expand(prk, info, length, hash_algo=hashlib.sha256):
	hash_len = hash_algo().digest_size
	n = math.ceil(length / hash_len)
	okm = b""
	output_block = b""
	for i in range(1, n + 1):
		output_block = hmac.new(prk, output_block + info + struct.pack('B', i), hash_algo).digest()
		okm += output_block
	return okm[:length]

def KDF_Length(data, label, context, length):
	# TODO: Support different lengths / hash functions. Need to switch to HMAC224?
	iterations = int(math.ceil(length / 256.0))
	result = b""
	for i in range(1, iterations + 1):
		hash_data = struct.pack("<H", i) + label + context + struct.pack("<H", length)
		result += HMAC256(data, hash_data)
	return result

def hash_pw_mod(password, modulo):
	"""Hash the password and return the result as an integer"""
	sha256_hash = hashlib.sha256()
	sha256_hash.update(password.encode('utf-8'))
	return bytes_to_int(sha256_hash.digest()) % modulo

