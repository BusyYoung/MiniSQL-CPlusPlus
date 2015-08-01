#include "BPlusTree.h"
using namespace std;
using namespace BPlusTree;


/**********************���⺯��*************************/

bool BPlusTree::compareInt(const Key& key1, const Key& key2) {
	char temp[sizeof(int)];
	for (int i = 0; i < sizeof(int); i++) {
		temp[i] = key1[i];
	}
	int a = *(int*)temp;
	for (int i = 0; i < sizeof(int); i++) {
		temp[i] = key2[i];
	}
	int b = *(int*)temp;
	return a < b;
}

bool BPlusTree::compareChar(const Key& key1, const Key& key2) {
	return key1 < key2;
}

bool BPlusTree::compareFloat(const Key& key1, const Key& key2) {
	char temp[sizeof(float)];
	for (int i = 0; i < sizeof(float); i++) {
		temp[i] = key1[i];
	}
	float a = *(float*)temp;
	for (int i = 0; i < sizeof(float); i++) {
		temp[i] = key2[i];
	}
	float b = *(float*)temp;
	return a < b;
}


Compare compares[4] = { compareInt, compareChar, compareFloat, NULL };


/**********************���ں���*************************/

Tree::Tree(int keySize, KeyType keyType, string fileName, BufferManager& bufferManager) : mKeySize(keySize), mKeyType(keyType), mFileName(fileName), mBufferManager(bufferManager), mOrder((blockSize - nodeHeaderSize - pointerSize) / (pointerSize + keySize) + 1) {
	loadRoot();
	
	setCompare(compares[keyType]);
}

Tree::~Tree() {
	//��дindex�ļ�ͷ
	TreeHeader treeHeader = getTreeHeader();
	char tempData[treeHeaderSize];
	*(TreeHeader*)tempData = treeHeader;
	vector<char> data;
	for (int i = 0; i < treeHeaderSize; i++) {
		data.push_back(tempData[i]);
	}
	Address address;
	address.mFileName = mFileName;
	address.mFileOffset = address.mBlockOffset = 0;
	mBufferManager.write(address, data);
}



/****************************����ӿ�*******************************/

Address Tree::searchKey(const Key& key) throw(NoSuchElementException) {
	Pointer pointer = search(key);
	int fileOffset = pointer >> 12;
	int blockOffset = pointer % 0x1000;
	return Address("", fileOffset, blockOffset);
}

vector<Address> Tree::searchKey(const Key& begin, const Key& end, bool beginEqualFlag, bool endEqualFlag) {
	vector<Address> addresses;
	vector<Pointer> pointers = search(begin, end, beginEqualFlag, endEqualFlag);
	for (int i = 0; i < pointers.size(); i++) {
		int fileOffset = pointers[i] >> 12;
		int blockOffset = pointers[i] % 0x1000;
		addresses.push_back(Address("", fileOffset, blockOffset));
	}
	return addresses;
}

void Tree::insertKey(const Key& key, Pointer recordPointer) throw(ElementExistException) {
	insert(key, recordPointer);
}

void Tree::deleteKey(const Key& k) throw (NoSuchElementException) {
	Node node = find(k);//�ҵ�k���ڵ�Ҷ��
	int index = lower_bound(node.mKeys.begin(), node.mKeys.end(), k, mCompare) - node.mKeys.begin();
	Pointer p = node.mPointers[index];
	deleteEntry(node, k, p);
}

void Tree::updateKey(const Key& oldKey, const Key& newKey, Pointer recordPointer) throw(ElementExistException, NoSuchElementException) {
	deleteKey(oldKey);
	insertKey(newKey, recordPointer);
}








/*****************�������ʺ���*******************/

int Tree::getKeySize() {
	return mKeySize;
}

KeyType Tree::getKeyType() {
	return mKeyType;
}

int Tree::getOrder() {
	return mOrder;
}

string Tree::getFileName() {
	return mFileName;
}

void Tree::setFileName(string fileName) {
	mFileName = fileName;
}

Compare Tree::getCompare() {
	return mCompare;
}

void Tree::setCompare(Compare compare) {
	mCompare = compare;
}







/*************************˽�к���**************************/

void Tree::loadRoot() {
	Address address;
	address.mFileName = mFileName;
	address.mFileOffset = address.mBlockOffset = 0;
	vector<char> data = mBufferManager.read(address, treeHeaderSize);
	char tempData[treeHeaderSize];
	for (int i = 0; i < treeHeaderSize; i++) {
		tempData[i] = data[i];
	}
	TreeHeader treeHeader = *(TreeHeader*)tempData;
	mRoot = treeHeader.mRoot;
}


TreeHeader Tree::getTreeHeader() {
	TreeHeader treeHeader;
	treeHeader.mRoot = mRoot;
	treeHeader.mKeySize = mKeySize;
	treeHeader.mKeyType = mKeyType;
	return treeHeader;
}

void Tree::deleteNode(Pointer pointer) {
	char temp[nodeHeaderSize];
	NodeHeader nodeHeader;
	nodeHeader.mIsUsed = false;
	*(NodeHeader*)temp = nodeHeader;
	vector<char> tempData;
	for (int i = 0; i < nodeHeaderSize; i++) {
		tempData.push_back(temp[i]);
	}
	Address address(mFileName, pointer, 0);
	mBufferManager.write(address, tempData);
}

inline Node Tree::getNode(Pointer pointer) {
	Node node(mKeySize, pointer, mBufferManager.readBlock(mFileName, pointer));
	return node;
}





Node Tree::findMinLeaf() throw(TreeEmptyException) {
	Node node = getNode(mRoot);
	while (!node.mIsLeaf) {
		if (node.mPointers[0] == NULLPOINTER) {
			throw TreeEmptyException();
		}
		node = getNode(node.mPointers[0]);
	}
	return node;
}

Key Tree::findMinKey() throw(TreeEmptyException) {
	Node node = findMinLeaf();
	return node.mKeys[0];
}

Node Tree::findMaxLeaf() throw(TreeEmptyException) {
	Node node = getNode(mRoot);
	while (!node.mIsLeaf) {
		if (node.mPointers[0] == NULLPOINTER) {
			throw TreeEmptyException();
		}
		node = getNode(node.mPointers[node.mPointers.size() - 2]);
	}
	return node;
}

Key Tree::findMaxKey() throw(TreeEmptyException) {
	Node node = findMaxLeaf();
	return node.mKeys[node.mKeys.size() - 1];
}

Pointer Tree::search(const Key& k) throw(NoSuchElementException) {
	Node node = find(k);
	int index = lower_bound(node.mKeys.begin(), node.mKeys.end(), k, mCompare) - node.mKeys.begin();
	return node.mPointers[index];
}

vector<Pointer> Tree::search(const Key& begin, const Key& end, bool beginEqualFlag, bool endEqualFlag) throw(NoSuchElementException) {//������[begin, end]�ڵļ�¼�����߶��Ǳ����䣩
	Node node = findShouldContain(begin);
	int index;
	if (beginEqualFlag) {
		index = lower_bound(node.mKeys.begin(), node.mKeys.end(), begin, mCompare) - node.mKeys.begin();
	}
	else {
		index = upper_bound(node.mKeys.begin(), node.mKeys.end(), begin, mCompare) - node.mKeys.begin();
	}
	vector<Pointer> result;
	if (index == node.mKeys.size()) {
		return result;
	}
	while (mCompare(node.mKeys[index], end) == true || (endEqualFlag && node.mKeys[index] == end)) {
		result.push_back(node.mPointers[index]);
		if (++index == node.mKeys.size()) {
			if (node.mPointers[index] == NULLPOINTER) {
				break;
			}
			node = getNode(node.mPointers[index]);
			index = 0;
		}
	}
	return result;
}

Key Tree::searchKeyForDelete(const Key& key) {
	Node node = find(key);
	int index = lower_bound(node.mKeys.begin(), node.mKeys.end(), key, mCompare) - node.mKeys.begin();
	return node.mKeys[index];
}

vector<Key> Tree::searchKeyForDelete(const Key& begin, const Key& end) {
	Node node = findShouldContain(begin);
	int index = lower_bound(node.mKeys.begin(), node.mKeys.end(), begin, mCompare) - node.mKeys.begin();
	vector<Key> result;
	if (index == node.mKeys.size()) {
		return result;
	}
	while (mCompare(node.mKeys[index], end) == true || node.mKeys[index] == end) {
		result.push_back(node.mKeys[index]);
		if (++index == node.mKeys.size()) {
			if (node.mPointers[index] == NULLPOINTER) {
				break;
			}
			node = getNode(node.mPointers[index]);
			index = 0;
		}
	}
	return result;
}

Node Tree::findShouldContain(const Key& k) {
	Node node = getNode(mRoot);
	while (!node.mIsLeaf) {
		int index = upper_bound(node.mKeys.begin(), node.mKeys.end(), k, mCompare) - node.mKeys.begin();
		node = getNode(node.mPointers[index]);
	}
	return node;
}

Node Tree::find(const Key& k) throw (NoSuchElementException) {
	Node node = getNode(mRoot);
	while (!node.mIsLeaf) {
		int index = upper_bound(node.mKeys.begin(), node.mKeys.end(), k, mCompare) - node.mKeys.begin();
		node = getNode(node.mPointers[index]);
	}
	if (binary_search(node.mKeys.begin(), node.mKeys.end(), k, mCompare) == true) {
		return node;
	}
	else {
		//B+����û�и�key
		throw NoSuchElementException();
	}
}


void Tree::insert(const Key& k, Pointer p) throw(ElementExistException) {
	Node node = findShouldContain(k);
	if (binary_search(node.mKeys.begin(), node.mKeys.end(), k, mCompare)) {
		throw ElementExistException();
	}

	//��ʱ�����node
	int index = lower_bound(node.mKeys.begin(), node.mKeys.end(), k, mCompare) - node.mKeys.begin();
	node.mKeys.insert(node.mKeys.begin() + index, k);
	node.mPointers.insert(node.mPointers.begin() + index, p);

	if (node.mKeys.size() < mOrder) {
		mBufferManager.writeBlock(mFileName, node.mFileOffset, node.toData());
	}
	else {
		//�����½ڵ㣬����
		Node newNode(mKeySize);
		newNode.mIsLeaf = true;
		newNode.mFather = node.mFather;//����������ڷ��ѣ��ֻ���Ҫ�����ӽڵ�ĸ�ָ��
		for (int i = (mOrder + 1) / 2; i < mOrder; i++) {
			newNode.mKeys.push_back(node.mKeys[i]);
			newNode.mPointers.push_back(node.mPointers[i]);
		}
		newNode.mPointers.push_back(node.mPointers[mOrder]);//��ԭ�ڵ���ָ���ֵܽڵ��ָ�븳ֵ���½ڵ�
		
		//��ԭ�ڵ���ɾ������
		for (int i = (mOrder + 1) / 2; i < mOrder; i++) {
			node.mKeys.pop_back();
			node.mPointers.pop_back();
		}
		node.mPointers.pop_back();

		int fileOffset = mBufferManager.appendBlock(mFileName, newNode.toData());//��ÿ�ţ�˳��д�ؽ��ļ�
		newNode.mFileOffset = fileOffset;
		node.mPointers.push_back(fileOffset);//��ԭ�ڵ���ָ���ֵܽڵ��ָ��ָ���´����Ľڵ�
		mBufferManager.writeBlock(mFileName, node.mFileOffset, node.toData());//��Ҫ���ھ�д�أ����ܵȵ�node����ʱд��

		Key leastKeyInNewNode = newNode.mKeys[0];
		insertInParent(node, leastKeyInNewNode, newNode);
	}
}

void Tree::insertInParent(Node& node1, const Key& k, Node& node2) {
	if (node1.mFileOffset == mRoot) {
		//�����¸�
		Node newRoot(mKeySize);
		newRoot.mKeys.push_back(k);
		newRoot.mPointers.push_back(node1.mFileOffset);
		newRoot.mPointers.push_back(node2.mFileOffset);
		newRoot.mIsLeaf = false;
		newRoot.mFather = NULLPOINTER;
		int fileOffset = mBufferManager.appendBlock(mFileName, newRoot.toData());//��д���¸�
		newRoot.mFileOffset = fileOffset;

		//����node1��node2�ĸ���ָ��
		node1.mFather = fileOffset;
		mBufferManager.writeBlock(mFileName, node1.mFileOffset, node1.toData());
		node2.mFather = fileOffset;
		mBufferManager.writeBlock(mFileName, node2.mFileOffset, node2.toData());

		//�������ĸ���Ϣ
		mRoot = newRoot.mFileOffset;
		return;
	}

	Node p = getNode(node1.mFather);

	//�ڸ����е�node1�������node2����Ϣ
	int index = lower_bound(p.mPointers.begin(), p.mPointers.end(), node1.mFileOffset) - p.mPointers.begin();
	assert(p.mPointers[index] == node1.mFileOffset);
	p.mKeys.insert(p.mKeys.begin() + index, k);
	p.mPointers.insert(p.mPointers.begin() + index + 1, node2.mFileOffset);

	if (p.mPointers.size() <= mOrder) {
		mBufferManager.writeBlock(mFileName, p.mFileOffset, p.toData());//д��
	}
	else {
		//�����½ڵ㣬����
		Node newP(mKeySize);
		newP.mIsLeaf = true;
		newP.mFather = p.mFather;//����������ڷ��ѣ��ֻ���Ҫ�����ӽڵ�ĸ�ָ��
		for (int i = (mOrder + 1) / 2; i < mOrder; i++) {
			newP.mKeys.push_back(p.mKeys[i]);
			newP.mPointers.push_back(p.mPointers[i]);
		}
		newP.mPointers.push_back(p.mPointers[mOrder]);
		int fileOffset = mBufferManager.appendBlock(mFileName, newP.toData());//д��
		newP.mFileOffset = fileOffset;
		//ɾ��ԭ���׵�����
		Key newKey = p.mKeys[(mOrder + 1) / 2 - 1];
		for (int i = (mOrder + 1) / 2; i <= mOrder; i++) { //��<=��������־�����������һ��
			p.mKeys.pop_back();
			p.mPointers.pop_back();
		}
		mBufferManager.writeBlock(mFileName, p.mFileOffset, p.toData());//д��

		//����������Ӱ����ӽڵ�ĸ���ָ�룬�����ڸ���ΪnewP���ӽڵ�ĸ���ָ��
		for (int i = 0; i < newP.mPointers.size(); i++) {
			Node node = getNode(newP.mPointers[i]);
			node.mFather = newP.mFileOffset;
			mBufferManager.writeBlock(mFileName, node.mFileOffset, node.toData());//д��
		}

		insertInParent(p, newKey, newP);

	}
}

void Tree::deleteEntry(Node& node, const Key& k, Pointer p) {
	auto it1 = lower_bound(node.mKeys.begin(), node.mKeys.end(), k, mCompare);
	node.mKeys.erase(it1);
	auto it2 = lower_bound(node.mPointers.begin(), node.mPointers.end(), p);
	node.mPointers.erase(it2);
	if (node.mFileOffset == mRoot) {
		if (node.mPointers.size() < 2) {//����Ҫɾ��
			mRoot = node.mPointers[0];
			if (node.mIsLeaf == false) {
				Node child = getNode(node.mPointers[0]);
				child.mFather = NULLPOINTER;
				mBufferManager.writeBlock(mFileName, child.mFileOffset, child.toData());//д��
			}
			deleteNode(node.mFileOffset);//д��
		}
		else {
			mBufferManager.writeBlock(mFileName, node.mFileOffset, node.toData());//����д��
		}
	}
	else if (node.mKeys.size() < (mOrder + 1) / 2) {
		int flag;//��¼�ҵ����ֵ�����ǰ�Ļ����ں��
		//Ѱ���ֵܽڵ�
		Node father = getNode(node.mFather);
		auto it = lower_bound(father.mPointers.begin(), father.mPointers.end(), node.mFileOffset);
		if (it == father.mPointers.begin()) {
			it++;
			flag = 1;
		}
		else {
			it--;
			flag = 0;
		}
		Node brother = getNode(*it);

		Key key = father.mKeys[it - father.mPointers.begin() - flag];//key��father��node��brother�м�ļ�ֵ

		if (node.mPointers.size() + brother.mPointers.size() - node.mIsLeaf <= mOrder) {//node��brother������һ���ڵ��зŵ���
			//���node��brother��ǰ�����򽻻����ǣ�֮��Ĳ�������brother��node��ǰ��Ϊ������
			if (flag == 1) {
				Node t = node;
				node = brother;
				brother = t;
			}
			if (node.mIsLeaf == false) {
				brother.mKeys.push_back(key);
				for (int i = 0; i < node.mKeys.size(); i++) {
					brother.mKeys.push_back(node.mKeys[i]);
				}
				for (int i = 0; i < node.mPointers.size(); i++) {
					brother.mPointers.push_back(node.mPointers[i]);
					Node child = getNode(node.mPointers[i]);
					child.mFather = brother.mFileOffset;//�����ӽڵ�ĸ���ָ��
					mBufferManager.writeBlock(mFileName, child.mFileOffset, child.toData());//д��
				}
			}
			else {
				brother.mPointers.pop_back();
				///////////////////////////////////////////////////
				for (int i = 0; i < node.mKeys.size(); i++) {
					brother.mKeys.push_back(node.mKeys[i]);
				}
				for (int i = 0; i < node.mPointers.size(); i++) {
					brother.mPointers.push_back(node.mPointers[i]);
				}
			}
			mBufferManager.writeBlock(mFileName, brother.mFileOffset, brother.toData());//д��
			
			deleteEntry(father, key, node.mFileOffset);
			deleteNode(node.mFileOffset);//��д��

		}
		else {//node��brother��һ���ڵ��зŲ��£����brother�н������node��
			//����ÿ��delete����ִ����Щ���裬����node�е��������ڴ������ֻ�������ֵ��һ�������Խ�һ�������node�о��㹻��
			//brother��node��ǰ�������
			if (flag == 0) {
				if (node.mIsLeaf == false) {
					//node����Ҷ�ڵ�
					int m = brother.mPointers.size() - 1;
					node.mKeys.insert(node.mKeys.begin(), key);
					node.mPointers.insert(node.mPointers.begin(), brother.mPointers[m]);
					auto it = lower_bound(father.mKeys.begin(), father.mKeys.end(), key, mCompare);
					*it = brother.mKeys[m - 1];
					brother.mPointers.pop_back();
					brother.mKeys.pop_back();
				}
				else {
					//node��Ҷ�ڵ�
					int m = brother.mKeys.size() - 1;
					node.mKeys.insert(node.mKeys.begin(), brother.mKeys[m]);
					node.mPointers.insert(node.mPointers.begin(), brother.mPointers[m]);
					auto it = lower_bound(father.mKeys.begin(), father.mKeys.end(), key, mCompare);
					*it = brother.mKeys[m];
					brother.mKeys.erase(brother.mKeys.begin() + m);
					brother.mPointers.erase(brother.mPointers.begin() + m);
				}
			}
			else {
				//node��brother��ǰ��
				if (node.mIsLeaf == false) {
					//node����Ҷ�ڵ�
					node.mKeys.push_back(key);
					node.mPointers.push_back(brother.mPointers[0]);
					auto it = lower_bound(father.mKeys.begin(), father.mKeys.end(), key, mCompare);
					*it = brother.mKeys[0];
					brother.mPointers.erase(brother.mPointers.begin());
					brother.mKeys.erase(brother.mKeys.begin());
				}
				else {
					//node��Ҷ�ڵ�
					int m = node.mKeys.size();
					node.mKeys.insert(node.mKeys.begin() + m, brother.mKeys[0]);
					node.mPointers.insert(node.mPointers.begin() + m, brother.mPointers[0]);
					auto it = lower_bound(father.mKeys.begin(), father.mKeys.end(), key, mCompare);
					*it = brother.mKeys[1];
					brother.mKeys.erase(brother.mKeys.begin());
					brother.mPointers.erase(brother.mPointers.begin());
				}
			}
			mBufferManager.writeBlock(mFileName, node.mFileOffset, node.toData());//д��
			mBufferManager.writeBlock(mFileName, brother.mFileOffset, brother.toData());//д��
			mBufferManager.writeBlock(mFileName, father.mFileOffset, father.toData());//д��
		}
	}
	else {
		//����Ҫ�ϲ�������д���޸�֮���node
		mBufferManager.writeBlock(mFileName, node.mFileOffset, node.toData());
	}
}


