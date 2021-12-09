#include "BTree.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream>
#include <sstream>
#include <queue>
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
 * data[]:数据块，len：数据块的数量，dataLen：数据的长度（字节）
 */
BTree* BTree::buildTree(std::string data[], int len, int dataLen){
	int hash_length = SHA256_DIGEST_LENGTH;
	BTree *trees[len];
//				= new BTree();
	// 1. 数据全部hash，并new叶子节点
	for(int i = 0; i < len; i++){
		trees[i] = new BTree();
		SHA256((unsigned char*)data[i].c_str(), dataLen, trees[i]->label);
	}

	int levelLen = len;
	// 自底向上构建树
	while(levelLen > 1){
		for(int i = 0; i < levelLen; i += 2){
			int next = i + 1;
			if(next >= len){ // 若出现奇数
				next = -1;
			}
			if(next != -1){
				int rank = trees[i]->rank + trees[next]->rank;
//				unsigned char str[2 * hash_length + 1];
				std::string str = "";
				str = str + (char*)trees[i]->label;
				str = str + (char*)trees[next]->label; // 这里应该把自己的flage也连接到hash中
				std::stringstream ss;
				ss << rank;
				str = str + ss.str();

				unsigned char digest[hash_length + 1];
				SHA256((unsigned char*)str.c_str(), str.length(), digest);
				trees[i]->flag = 0;
				trees[next]->flag = 1;
				BTree *node = new BTree(rank, trees[i]->nodeNum + trees[next]->nodeNum + 1, digest);
				node->left = trees[i];
				node->right = trees[next];
				trees[i / 2] = node;
			}else{
				trees[i / 2] = trees[i]; // 奇数时，最后一个直接进入下一层
			}
			levelLen = (levelLen + 1) / 2;
		}
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

/**
 * 将树序列化为数组,层序遍历
 */
void BTree::serializeToArrays(std::string data[], Pos pos[]){
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
}
