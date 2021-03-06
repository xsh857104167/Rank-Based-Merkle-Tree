#include "BTree.h"
#include "Utils.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>
#include <openssl/sha.h>

//using namespace std;

BTree::BTree(){
	rank = 1;
	nodeNum = 1;
	flag = -1;
//	label = NULL;
	left = NULL;
	right = NULL;
}
BTree::BTree(int rank, int nodeNum, unsigned char label[]) {
	// TODO Auto-generated constructor stub
	this->rank = rank;
	this->nodeNum = nodeNum;
	flag = -1;
	strcpy((char*)this->label, (char*)label);
	left = NULL;
	right = NULL;
}

BTree::~BTree() {
	// TODO Auto-generated destructor stub
}
/**
 * 静态函数
 * 通过数据初始化构造树
 * hash[]:数据hash值的数组，len：数据块的数量
 */
BTree* BTree::buildTree(std::string hash[], int len){
	int hash_length = SHA256_DIGEST_LENGTH;
	BTree *trees[len];
//				= new BTree();
	// 1. 数据全部hash，并new叶子节点
	for(int i = 0; i < len; i++){
		trees[i] = new BTree();
		std::string str = hash[i] + "1";
		SHA256((unsigned char*)str.c_str(), str.length(), trees[i]->label);
		trees[i]->label[hash_length] = '\0';
	}

	int levelLen = len;
	// 自底向上构建树
	while(levelLen > 1){
		for(int i = 0; i < levelLen; i += 2){
			int next = i + 1;
			if(next >= levelLen){ // 若出现奇数
				next = -1;
			}
			if(next != -1){
				int rank = trees[i]->rank + trees[next]->rank;
				std::string str = "";
				str = str + (char*)trees[i]->label;
				str = str + (char*)trees[next]->label; // 这里应该把自己的flage也连接到hash中
				std::stringstream ss;
				ss << rank;
				str = str + ss.str();

				BTree *node = new BTree();
				SHA256((unsigned char*)str.c_str(), str.length(), node->label);
				node->label[hash_length] = '\0';
				node->rank = rank;
				node->nodeNum = trees[i]->nodeNum + trees[next]->nodeNum + 1;

				trees[i]->flag = 0;
				trees[next]->flag = 1;
				node->left = trees[i];
				node->right = trees[next];
				trees[i / 2] = node;
			}else{
				trees[i / 2] = trees[i]; // 奇数时，最后一个直接进入下一层
			}
		}
		levelLen = (levelLen + 1) / 2;
	}

	return trees[0];
}

/**
 * 静态函数
 * 通过已保存的数组和位置重新构造树
 * data[]表示存储hash值的数据，pos为位置数据， len表示数据的个数
 */
BTree* BTree::reBuildTree(std::string data[], Pos pos[], int len){
	BTree *trees[len];
	for(int i = 0; i < len; i++){
		trees[i] = new BTree(pos[i].rank, pos[i].nodeNum, (unsigned char*)data[i].c_str());
	}
	for(int i = 0; i < len; i++){
		if(pos[i].leftPos != -1){
			trees[i]->left = trees[pos[i].leftPos];
			trees[pos[i].leftPos]->flag = 0;
		}else{
			trees[i]->left = NULL;
		}
		if(pos[i].rightPos != -1){
			trees[i]->right = trees[pos[i].rightPos];
			trees[pos[i].rightPos]->flag = 1;
		}else{
			trees[i]->right = NULL;
		}
	}
	return trees[0];
}
BTree* BTree::reBuildTree(std::string path, int len){
	std::string treePathNode = path + "-node";
	std::string treePathPos = path + "-pos";
	std::fstream in_node;
	std::fstream in_pos;
	in_node.open(treePathNode.c_str(), std::ios::in|std::ios::binary);
	in_pos.open(treePathPos.c_str(), std::ios::in|std::ios::binary);
	std::string data[len];
	Pos pos[len];
	char buff[512];
	for(int i = 0; i < len; i++){
		in_node>>buff;
		char *temp = new char[SHA256_DIGEST_LENGTH + 1];
		Utils::HexToBytes((unsigned char*)temp, buff, 2*SHA256_DIGEST_LENGTH);
		temp[SHA256_DIGEST_LENGTH] = '\0';
		data[i] = std::string(temp);
	}
	in_node.close();
	for(int i = 0; i < len; i++){
		in_pos>>buff;
	    char *token;
	    token = strtok(buff, ",");
	    pos[i].leftPos = atoi(token);
	    token = strtok(NULL,",");
	    pos[i].rightPos = atoi(token);
	    token = strtok(NULL,",");
	    pos[i].nodeNum = atoi(token);
	    token = strtok(NULL,",");
	    pos[i].rank = atoi(token);
	}
	in_pos.close();
	return BTree::reBuildTree(data, pos, len);
}

/**
 * 将树序列化为数组,层序遍历
 * label直接存储
 * 里面的代码写的极其不规范
 */
void BTree::serializeToArrays(std::string data[], Pos pos[], std::string path){
	std::queue <BTree*> q;
	data[0] = (char*)label;
	q.push(left);
	pos[0].leftPos = 1;
	q.push(right);
	pos[0].rightPos = 2;
	pos[0].nodeNum = nodeNum;
	pos[0].rank = rank;
	int i = 1;
	while (!q.empty()){
		BTree *node = q.front();
		q.pop();
		data[i] = (char*)node->label;
		pos[i].nodeNum = node->nodeNum;
		pos[i].rank = node->rank;
		if(node->left != NULL){
			q.push(node->left);
			pos[i].leftPos = i + q.size();
		}else {
			pos[i].leftPos = -1;
		}

		if(node->right != NULL){
			q.push(node->right);
			pos[i].rightPos = i + q.size();
		}else{
			pos[i].rightPos = -1;
		}
		i++;
	}
	std::string treePathNode = path + "-node";
	std::string treePathPos = path + "-pos";
	std::fstream out_node;
	out_node.open(treePathNode.c_str(), std::ios::out);
	for(int i = 0; i < nodeNum; i++){
		out_node <<data[i]<<std::endl;
	}
	out_node.close();

	std::fstream out_pos;
	out_pos.open(treePathPos.c_str(), std::ios::out);
	for(int i = 0; i < nodeNum; i++){
		out_pos<<pos[i].leftPos<<",";
		out_pos<<pos[i].rightPos<<",";
		out_pos<<pos[i].nodeNum<<",";
		out_pos<<pos[i].rank<<std::endl;
	}
	out_pos.close();
}
/**
 * 直接传路径存储下来
 * label用16进制存储
 */
void BTree::serializeToArrays(std::string path){
	std::string treePathNode = path + "-node";
	std::string treePathPos = path + "-pos";
	std::fstream out_node;
	out_node.open(treePathNode.c_str(), std::ios::out);
	std::fstream out_pos;
	out_pos.open(treePathPos.c_str(), std::ios::out);

	std::queue <BTree*> q;
	char buff[2 * SHA256_DIGEST_LENGTH + 1];
	Utils::BytesToHex(buff, (char*)label, SHA256_DIGEST_LENGTH);
	out_node <<buff<<std::endl;
	Pos pos;
	q.push(left);
	pos.leftPos = 1;
	q.push(right);
	pos.rightPos = 2;
	pos.nodeNum = nodeNum;
	pos.rank = rank;

	out_pos<<pos.leftPos<<",";
	out_pos<<pos.rightPos<<",";
	out_pos<<pos.nodeNum<<",";
	out_pos<<pos.rank<<std::endl;

	int i = 1;
	while (!q.empty()){
		BTree *node = q.front();
		q.pop();
		Utils::BytesToHex(buff, (char*)node->label, SHA256_DIGEST_LENGTH);
		out_node <<buff<<std::endl;

		pos.nodeNum = node->nodeNum;
		pos.rank = node->rank;
		if(node->left != NULL){
			q.push(node->left);
			pos.leftPos = i + q.size();
		}else {
			pos.leftPos = -1;
		}

		if(node->right != NULL){
			q.push(node->right);
			pos.rightPos = i + q.size();
		}else{
			pos.rightPos = -1;
		}
		out_pos<<pos.leftPos<<",";
		out_pos<<pos.rightPos<<",";
		out_pos<<pos.nodeNum<<",";
		out_pos<<pos.rank<<std::endl;
		i++;
	}
	out_node.close();
	out_pos.close();
}

/**
 * 定位叶节点
 * 给定树和叶节点的index（1,....)，查找此树中第index个节点
 * index 必须小于或等于root的rank
 */
BTree* BTree::locate(BTree *root, int index, std::vector<BTree*> &vec){
	BTree *v = root;
	int i = index;
	vec.push_back(v);
	while(v->left != NULL || v->right != NULL){
		if (i <= v->left->rank){
			v = v->left;
		}else {
			i = i - v->left->rank;
			v = v->right;
		}
		vec.push_back(v);
	}
	return v;
}
/**
 * 根据叶子节点的路径vec生成证明proof
 */
void BTree::proofGen(std::vector<Prof> &proof, std::vector<BTree*> &vec){
	// 先把此节点放进证明中
	BTree *node = vec.back();
	Prof prof;
//	prof.data = node->label;
	strcpy((char*)prof.data, (char*)node->label);
	prof.flag = node->flag;
	prof.rank = node->rank;
	proof.push_back(prof);
	// 再把所有的兄弟节点放进去
	while(!vec.empty()){
		BTree *node = vec.back();
		vec.pop_back();
		if(!vec.empty()){
			BTree *parent = vec.back();
			Prof prof;
			if(node->flag == 0){
//				prof.data = parent->right->label;
				strcpy((char*)prof.data, (char*)parent->right->label);
				prof.flag = parent->right->flag;
				prof.rank = parent->right->rank;
			}else if (node->flag == 1){
//				prof.data = parent->left->label;
				strcpy((char*)prof.data, (char*)parent->left->label);
				prof.flag = parent->left->flag;
				prof.rank = parent->left->rank;
			}
		}
	}
}
/**
 * 根据证明prof生成树根
 */
std::string BTree::buildRootFromProf(Prof prof[], int len){
	BTree *node = new BTree();
	node->rank = prof[len - 1].rank + prof[len - 2].rank;
	std::string str = "";
	if(prof[len - 1].flag == 0){
		str = str + (char*)prof[len-1].data;
		str = str + (char*)prof[len-2].data;
	}else {
		str = str + (char*)prof[len-2].data;
		str = str + (char*)prof[len-1].data;
	}
	std::stringstream ss;
	ss << node->rank;
	str = str + ss.str();
	SHA256((unsigned char*)str.c_str(), str.length(), node->label);

	for (int i = len - 3; i >= 0; i--){
		node->rank += prof[i].rank;
		str = "";
		if (prof[i].flag == 0){
			str = str + (char*)prof[i].data;
			str = str + (char*)node->label;
		}else {
			str = str + (char*)node->label;
			str = str + (char*)prof[i].data;
		}
		std::stringstream ss;
		ss << node->rank;
		str = str + ss.str();
		SHA256((unsigned char*)str.c_str(), str.length(), node->label);
	}
	return (char*)node->label;
}
/**
 * 在第index个数据块后插入数据data，并生成proof
 * index都是从1开始的索引
 */
void BTree::indertNode(std::string data, int dataLen, int index, std::vector<Prof> &proof){
	int hash_length = SHA256_DIGEST_LENGTH;
	// 生成新的叶子节点
	BTree *newNode = new BTree();
	std::string str = data + "1";
	SHA256((unsigned char*)str.c_str(), dataLen, newNode->label);
	newNode->flag = 1; // 右节点
	// 定位
	std::vector<BTree*> vec;
	BTree::locate(this, index, vec);
	// 生成证明
	std::vector<BTree*> vec_cpy(vec);
	BTree::proofGen(proof, vec_cpy);

	BTree *node = vec.back();
	vec.pop_back();
	// 新建一个父节点
	int rank = 2;
	std::string str2 = "";
	str2 = str2 + (char*)node->label;
	str2 = str2 + (char*)newNode->label; // 这里应该把自己的flage也连接到hash中
	std::stringstream ss;
	ss << rank;
	str2 = str2 + ss.str();
	unsigned char digest[hash_length + 1];
	SHA256((unsigned char*)str2.c_str(), str2.length(), digest);
	BTree *parent = new BTree(rank, 2, digest); // rank = nodeNum = 2
	parent->left = node;
	parent->right = newNode;
	parent->flag = node->flag;
	// 修改第i个叶子节点的flag
	node->flag = 0; // 左节点

	// 需要把parent作为子节点
	// 需要注意边界条件，最少有几个节点
	// 最少有三个节点，有一个节点的话也不算是树了
	node = vec.back();
	vec.pop_back();
	if(parent->flag == 0){
		// 将其
		node->left = parent;
	}else if (parent->flag == 1){
		node->right = parent;
	}
	node->rank = node->left->rank + node->right->rank;
	node->nodeNum = node->left->nodeNum + node->right->nodeNum;
	// while()
	// 修改剩余节点
	while(!vec.empty()){
		BTree *temp = vec.back();
		vec.pop_back();
		temp->rank = temp->left->rank + temp->right->rank;
		temp->nodeNum = temp->left->nodeNum + temp->right->nodeNum;
		std::string str = "";
		str = str + (char*)temp->left->label;
		str = str + (char*)temp->right->label; // 这里应该把自己的flag也连接到hash中
		std::stringstream ss;
		ss << temp->rank;
		str = str + ss.str();
		SHA256((unsigned char*)str.c_str(), str.length(), temp->label);
	}
}
/**
 * 删除第index个数据块
 * index都是从1开始的索引
 */
BTree* BTree::deleteNode(BTree *root, int index, std::vector<Prof> &proof){
	std::vector<BTree*> vec;
	BTree::locate(root, index, vec);
	std::vector<BTree*> vec_cpy(vec);
	BTree::proofGen(proof, vec_cpy);
	if(vec.size() == 2){ // 处理特殊情况
		BTree *node = vec.back(); // 被删除的节点
		if(node->flag == 0){
			root->right->flag = root->flag;
			return root->right;
		}else if (node->flag == 1){
			root->left->flag = root->flag;
			return root->left;
		}
	}else{
		BTree *node = vec.back();
		vec.pop_back();
		BTree *parent = vec.back();
		vec.pop_back();

		if(node->flag == 0){
			parent->right->flag = parent->flag;
			parent = parent->right;
		}else if (node->flag == 1){
			parent->left->flag = parent->flag;
			parent = parent->left;
		}

		while(!vec.empty()){
			BTree *node = vec.back();
			vec.pop_back();
			node->rank = node->left->rank + node->right->rank;
			node->nodeNum = node->left->nodeNum + node->right->nodeNum;
			std::string str = "";
			str = str + (char*)node->left->label;
			str = str + (char*)node->right->label; // 这里应该把自己的flag也连接到hash中
			std::stringstream ss;
			ss << node->rank;
			str = str + ss.str();
			SHA256((unsigned char*)str.c_str(), str.length(), node->label);
		}
	}
	return root;
}

/**
 * 修改第index个数据块，并生成proof
 * index都是从1开始的索引
 * data为修改后的hash, dataLen为data的字节数，proof为生成的证明
 */
void BTree::modify(std::string data, int dataLen, int index, std::vector<Prof> &proof){
//	int hash_length = SHA256_DIGEST_LENGTH;
	std::vector<BTree*> vec;
	BTree::locate(this, index, vec);
	std::vector<BTree*> vec_cpy(vec);
	BTree::proofGen(proof, vec_cpy);
	// 先修改叶子节点
	BTree *node = vec.back();
	vec.pop_back();
	std::string str = data + "1";
	SHA256((unsigned char*)str.c_str(), dataLen, node->label);

	while(!vec.empty()){
		BTree *node = vec.back();
		vec.pop_back();

		std::string str = "";
		str = str + (char*)node->left->label;
		str = str + (char*)node->right->label; // 这里应该把自己的flag也连接到hash中
		std::stringstream ss;
		ss << node->rank;
		str = str + ss.str();
		SHA256((unsigned char*)str.c_str(), str.length(), node->label);
	}

}
/**
 * 用于客户端插入数据，收到proof后生成新的根时使用
 * node表示插入前afterWhich节点
 */
void BTree::twoLeavesInOne(Prof node, std::string insertNode){
	int hash_length = SHA256_DIGEST_LENGTH;
	unsigned char insertLabel[hash_length + 1];
	std::string str = insertNode + "1";
	SHA256((unsigned char*)str.c_str(), str.length(), insertLabel);
	insertLabel[hash_length] = '\0';

	int rank = 2;
	std::string newStr = "";
	str = str + (char*)node.data;
	str = str + (char*)insertLabel;
	std::stringstream ss;
	ss << rank;
	str = str + ss.str();

	SHA256((unsigned char*)newStr.c_str(), newStr.length(), node.data);
	node.data[hash_length] = '\0';
	node.rank = rank;
}
