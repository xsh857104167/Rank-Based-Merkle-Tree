#include <string>
#include <openssl/sha.h>
#ifndef BTREE_H_
#define BTREE_H_
// 位置，表示节点之间的关系
typedef struct Pos{
	int leftPos; // -1表示null
	int rightPos;
	int nodeNum;
	int rank;
}Pos;

class BTree {
public:
	int rank; // 叶节点的个数
	int nodeNum; // 节点的个数
	int flag; // 左右标识位，0表示left，1表示right
	unsigned char label[SHA256_DIGEST_LENGTH + 1];// 标签值
	BTree *left;
	BTree *right;

	BTree();
	BTree(int rank, int nodeNum, unsigned char* label);
	virtual ~BTree();
	/**
	 * 通过数据初始化构造树
	 */
	static BTree* buildTree(std::string data[], int len, int dataLen);
	/**
	 * 通过已保存的数组和位置重新构造树
	 */
	static BTree* reBuildTree(std::string data[], Pos pos[], int len);
	void serializeToArrays(std::string data[], Pos pos[]);
};

#endif /* BTREE_H_ */
