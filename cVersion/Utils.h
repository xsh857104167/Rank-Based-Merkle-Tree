#include <openssl/sha.h>
// for pbc library
#include "/usr/local/include/pbc/pbc.h"
// for gmp library
#include <gmp.h>

#ifndef UTILS_H_
#define UTILS_H_


// 客户端请求包
struct requestPack{
	int flag; // 用于标识客户端操作：1表示上传，2表示验证,3表示修改，4表示插入，5表示删除
	char fileID[SHA256_DIGEST_LENGTH + 1];
	char clientID[100];
	// 如果是修改插入删除，blockNum表示which
	// 如果是验证操作，blockNum表示chalNum
	int blockNum;
	int blockSize; // 仅用于修改插入操作中
	unsigned char kPRFChars[100];// 验证操作中存储prf的密钥，而在修改插入操作中存储sigma
	unsigned char kPRPChars[64];// 验证操作中存储prp的密钥，而在修改插入操作中存储密文id
};
// 上传请求的回复包
struct upResponse{
	int flag; // 0表示退出（自己已经存过了）,1表示已经有用户存过了,2表示没有用户存储过
	char ida[SHA256_DIGEST_LENGTH + 1];
};
// 数据块信息
struct blockInfo{
	int whichBlock;// 属于第几个块
	int blockSize;// 快大小
	char sigma[100];// 验证标签
};
struct verifyResponse{
	int tupleNum;
	unsigned char rootSig[100];
};
struct muResponse{
	int thetaSize;
	unsigned char mu[200][20]; // 这里写死了s=200,只能用a曲线
};
struct updateResponse{
	int flag; // 0表示没有人存需要上传密文，1表示已经有人存了不需要上传密文
	int nodeNum;
	unsigned char rootSig[100];
};
class Utils {
public:
	Utils();
	virtual ~Utils();
	static int sign_hash1(element_t sig, char *data, int dataLen, element_t sk, pairing_t pairing);
	static int verify_hash1(element_t rootSig, std::string label, int labelLen, element_t pk, pairing_t pairing);
	static int verify_hash1(element_t rootSig, char *label, int labelLen, element_t pk, pairing_t pairing);
	static int HexToBytes(unsigned char *bits, char *s, int len);
	static int HexToBytes(unsigned char *bits, std::string s, int len);
	static int xor_enc(char *data, unsigned char *key, long dataLen, int keyLen);
	static int genCipherID(element_t id, char *cipher, int cipherLen, pairing_t pairing);
	static int KeyGen(std::string *key_out, unsigned char *digest, unsigned char *data, int length);
	static int BytesToHex(char *hex, char *s, int len);
	static int BytesToHex(char *hex, std::string s, int len);
	static int psiPRF(int blockNum, element_t kPRF, int chalNum, element_t vi[], pairing_t pairing);
	static int psiPRF(int blockNum, char *kPRFChars, int skLen, int chalNum, element_t vi[], pairing_t pairing);
	static int piPRP(int blockNum, element_t kPRP, int chalNum, int index[], pairing_t pairing);
	static int piPRP(int blockNum, char *kPRPChars, int skLen, int chalNum, int index[], pairing_t pairing);
};

#endif /* UTILS_H_ */
