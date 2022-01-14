#include "Utils.h"
#include <string>
#include <string.h>
#include <openssl/sha.h>
// for pbc library
#include "/usr/local/include/pbc/pbc.h"
// for gmp library
#include <gmp.h>
// for encrypt
#include <openssl/sha.h>

Utils::Utils() {
	// TODO Auto-generated constructor stub

}

Utils::~Utils() {
	// TODO Auto-generated destructor stub
}
/*
 * 哈希签名函数，hash1为映射到G上的hash函数，主要用于对树根进行签名
 * sig为签名的结果，data表示要签名的数据，dataLen为data的长度
 */
int Utils::sign_hash1(element_t sig, char *data, int dataLen, element_t sk, pairing_t pairing){
	element_init_G1(sig, pairing);
	element_t hash;
	element_init_G1(hash, pairing);

	int hash_length = SHA256_DIGEST_LENGTH;
	unsigned char digest[hash_length];
	SHA256((unsigned char*)data, dataLen, (unsigned char*)digest);
	element_from_hash(hash, digest, hash_length);
	element_pow_zn(sig, hash, sk);

	return 0;
}
/*
 * 实现将16进制表示的字符串转换为字节数组，
 * 将密钥从文件中以16进制读进后可调用此函数进行转换
 * input: 结果bits，16进制字符串s，以及s的长度
 */
int Utils::HexToBytes(unsigned char *bits, char *s, int len){
	int i,n = 0;
	for(i = 0; i<len; i += 2) {
		if(s[i] >= 'A' && s[i] <= 'F')
			bits[n] = s[i] - 'A' + 10;
		else bits[n] = s[i] - '0';
		if(s[i + 1] >= 'A' && s[i + 1] <= 'F')
			bits[n] = (bits[n] << 4) | (s[i + 1] - 'A' + 10);
		else bits[n] = (bits[n] << 4) | (s[i + 1] - '0');
		n++;
	}
	return n;
}
int Utils::HexToBytes(unsigned char *bits, std::string s, int len){
	int i,n = 0;
	for(i = 0; i<len; i += 2) {
		if(s[i] >= 'A' && s[i] <= 'F')
			bits[n] = s[i] - 'A' + 10;
		else bits[n] = s[i] - '0';
		if(s[i + 1] >= 'A' && s[i + 1] <= 'F')
			bits[n] = (bits[n] << 4) | (s[i + 1] - 'A' + 10);
		else bits[n] = (bits[n] << 4) | (s[i + 1] - '0');
		n++;
	}
	return n;
}
/*
 * 采用异或方式加密
 * input: 数据data，密钥key，数据长度dataLen，密钥长度keyLen
 * output: 0
 */
int Utils::xor_enc(char *data, unsigned char *key, long dataLen, int keyLen){
	for(int i=0;i<dataLen;i++){
		data[i] = data[i] ^ key[i%keyLen];
	}
	return 0;
}
/*
 * 生成密文ID的函数
 * params: id为输出的id值，*cipher为密文，cipherLen为密文长度
 * 考虑需不需要输出两个版本的id值
 */
int Utils::genCipherID(element_t id, char *cipher, int cipherLen, pairing_t pairing){
	element_init_G1(id, pairing);
	int hash_length = SHA256_DIGEST_LENGTH;
	unsigned char digest[hash_length];
	SHA256((unsigned char*)cipher, cipherLen, (unsigned char*)digest);
	element_from_hash(id, digest, hash_length);
	return 0;
}
/**
 * 生成加密密钥
 * key_out是生成密钥的十六进制版本，digest也是密钥，data是数据块数据，length是data的长度
 */
int Utils::KeyGen(std::string *key_out, unsigned char *digest, unsigned char *data, int length){
	int hash_length = SHA256_DIGEST_LENGTH;
	char mdString[hash_length * 2 + 1];
	SHA256(data, length, digest);
	for(int i = 0; i < hash_length; i++){
		sprintf(&mdString[i*2], "%02X", (unsigned int)digest[i]);
	*key_out = mdString;
	}
	return 0;
}
/*
 * len 是s的长度
 */
int Utils::BytesToHex(char *hex, char *s, int len){
	for(int i = 0; i < len; i++){
		sprintf(&hex[i * 2], "%02X", (unsigned char)s[i]);
	}
	hex[2*len] = '\0';
	return 0;
}
/**
 * string这个会存在问题
 */
int Utils::BytesToHex(char *hex, std::string s, int len){
	for(int i = 0; i < len; i++){
		sprintf(&hex[i * 2], "%02X", (unsigned char)s[i]);
	}
	hex[2*len] = '\0';
	return 0;
}

/**
 * 生成chalNum个vi
 */
int Utils::psiPRF(int blockNum, element_t kPRF, int chalNum, element_t vi[], pairing_t pairing){
	element_t temp;
	element_init_Zr(temp, pairing);
	element_set(temp, kPRF);
	int skLen = element_length_in_bytes(kPRF);
	unsigned char *data_temp = (unsigned char*)pbc_malloc(skLen);

	for(int i = 0; i < chalNum; i++){
		element_to_bytes(data_temp, temp);
		unsigned char *hashTemp = new unsigned char[SHA256_DIGEST_LENGTH];
		SHA256(data_temp, skLen, hashTemp);
		element_init_Zr(vi[i], pairing);
		element_from_hash(vi[i], hashTemp, SHA256_DIGEST_LENGTH);
		element_set(temp, vi[i]);
	}
	return 0;
}
int Utils::psiPRF(int blockNum, char *kPRFChars, int skLen, int chalNum, element_t vi[], pairing_t pairing){
	element_t temp;
	element_init_Zr(temp, pairing);
	unsigned char *data_temp = (unsigned char*)pbc_malloc(skLen);
	memcpy(data_temp, kPRFChars, skLen);
	for(int i = 0; i < chalNum; i++){
		unsigned char *hashTemp = new unsigned char[SHA256_DIGEST_LENGTH];
		SHA256(data_temp, skLen, hashTemp);
		element_init_Zr(vi[i], pairing);
		element_from_hash(vi[i], hashTemp, SHA256_DIGEST_LENGTH);
		element_set(temp, vi[i]);
		element_to_bytes(data_temp, temp);
	}
	return 0;
}
/**
 * 生成chalNum个index
 * prp和prf两个函数的思路不一样
 */
int Utils::piPRP(int blockNum, element_t kPRP, int chalNum, int index[], pairing_t pairing){
	element_t key;
	element_init_Zr(key, pairing);
	element_set(key, kPRP);
	int skLen = element_length_in_bytes(key);
	unsigned char *data_temp = (unsigned char*)pbc_malloc(skLen);
	element_to_bytes(data_temp, key);
	unsigned char *hashTemp = new unsigned char[SHA256_DIGEST_LENGTH];
	SHA256(data_temp, skLen, hashTemp);

	for(int i = 0; i < chalNum; i++){
		SHA256(hashTemp, SHA256_DIGEST_LENGTH, hashTemp);
		char hashHex[2 * SHA256_DIGEST_LENGTH + 1];
		Utils::BytesToHex(hashHex, (char*)hashTemp, SHA256_DIGEST_LENGTH);
		mpz_t bigValue;
		mpz_init_set_str(bigValue, hashHex, 16);
		mpz_mod_ui(bigValue, bigValue, (unsigned int)blockNum);
		index[i] = mpz_get_si(bigValue);
	}
	return 0;
}
/**
 * 生成chalNum个index
 * 但是index是以0开始的索引
 * 需要再加一
 */
int Utils::piPRP(int blockNum, char *kPRPChars, int skLen, int chalNum, int index[], pairing_t pairing){
	unsigned char *data_temp = (unsigned char*)pbc_malloc(skLen);
	memcpy(data_temp, kPRPChars, skLen);
	unsigned char *hashTemp = new unsigned char[SHA256_DIGEST_LENGTH];
	SHA256(data_temp, skLen, hashTemp);

	for(int i = 0; i < chalNum; i++){
		SHA256(hashTemp, SHA256_DIGEST_LENGTH, hashTemp);
		char hashHex[2 * SHA256_DIGEST_LENGTH + 1];
		Utils::BytesToHex(hashHex, (char*)hashTemp, SHA256_DIGEST_LENGTH);
		mpz_t bigValue;
		mpz_init_set_str(bigValue, hashHex, 16);
		mpz_mod_ui(bigValue, bigValue, (unsigned int)blockNum);
		index[i] = mpz_get_si(bigValue);
	}
	return 0;
}
/**
 * 哈希验证函数，主要是验证树根的签名，sign_hash1函数的“逆”函数
 * rootSig为树根的签名，label为树根，labelLen为label的长度
 * pk为公玥
 * 输出1表示验证成功，0表示验证失败
 */
int Utils::verify_hash1(element_t rootSig, std::string label, int labelLen, element_t pk, pairing_t pairing){
	element_t hash;
	element_init_G1(hash, pairing);

	int hash_length = SHA256_DIGEST_LENGTH;
	unsigned char digest[hash_length];
	SHA256((unsigned char*)label.c_str(), labelLen, (unsigned char*)digest);
	element_from_hash(hash, digest, hash_length);
	element_t temp1;
	element_t temp2;
	element_init_GT(temp1, pairing);
	element_init_GT(temp2, pairing);
	if(!element_cmp(temp1, temp2)){
		return 1;
	}else{
		return 0;
	}
}
int Utils::verify_hash1(element_t rootSig, char *label, int labelLen, element_t pk, pairing_t pairing){
	element_t hash;
	element_init_G1(hash, pairing);

	int hash_length = SHA256_DIGEST_LENGTH;
	unsigned char digest[hash_length];
	SHA256((unsigned char*)label, labelLen, (unsigned char*)digest);
	element_from_hash(hash, digest, hash_length);
	element_t temp1;
	element_t temp2;
	element_init_GT(temp1, pairing);
	element_init_GT(temp2, pairing);
	if(!element_cmp(temp1, temp2)){
		return 1;
	}else{
		return 0;
	}

	return 0;
}
