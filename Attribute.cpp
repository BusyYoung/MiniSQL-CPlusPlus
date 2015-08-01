#include "Attribute.h"

Attribute::Attribute(const vector<char>& tName, const vector<char>& aName, const char t, const char isU)
{
	tableName = tName;
	attrName = aName;
	type = t;
	isUnique = isU;
}

Attribute::~Attribute()
{
}

void Attribute::setAttrRecord(const vector<char>& tName, const vector<char>& aName, const char t, const char isU)
{
	tableName = tName;
	attrName = aName;
	type = t;
	isUnique = isU;
}

void Attribute::getAttrRecord(vector<char>& record){
	record.clear();
	record.insert(record.end(), tableName.begin(), tableName.end());
	record.insert(record.end(), attrName.begin(), attrName.end());
	record.push_back(type);
	record.push_back(isUnique);
	for (int i = 0; i < 8; i++){//����ļ�¼���ǵ�һ��Ҳ�����һ����ָ��(0,0)
		record.push_back(0);
	}
}
