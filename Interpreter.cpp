#include "Interpreter.h"
#include<sstream>
#include<fstream>

Interpreter::~Interpreter()
{
}

void Interpreter::handleError(int num, string name, string s){
	if (num == 0)//syntax error
		cout << name << "ERROR: You have an error in your SQL syntax;" << endl;
}

int Interpreter::isLegal_name(string name){
	int flag = 0;

	if (isLegal_type(name))//����Ϊ�ؼ���
		return 0;

	for (int i = 0; i < name.size(); i++){
		if (name[i] < '0' || name[i] > '9'){//����һ��������
			flag = 1;
			if (name[i] == '_' 
				|| 'a' <= name[i] && name[i] <= 'z' 
				|| 'A' <= name[i] && name[i] <= 'Z')
				;
			else//���ַǷ��ַ�
				return 0;
		}
	}

	if (flag == 0)//ȫΪ����
		return 0;

	return 1;
}

int Interpreter::isLegal_type(string type){
	if (type == "int")
		return 1;
	else if (type == "float")
		return 2;
	else if (type == "char")
		return 3;

	return 0;
}

int Interpreter::isLegal_op(string op){
	if (op == "=" || op == "<>" || op == ">"
		|| op == "<" || op == ">=" || op == "<=")
		return 1;

	return 0;
}

int Interpreter::isTypeMatch(string &value, int type){
	//cout << value << " " << type << endl;
	//ȥ��������
	int flag = 0;
	if (value[0] == '"' && value[value.size() - 1] == '"' || value[0] == '\'' && value[value.size() - 1] == '\''){
		value.erase(value.begin());
		value.erase(value.end() - 1);//end()��������һ����������ָ���ַ�����ĩβ(���һ���ַ�����һ��λ��).
		flag = 1;
	}

	if (type == 1){//int
		for (int j = 0; j <value.size(); j++){
			if (value[j]<'0' || value[j]>'9'){
				cout << "ERROR: Data type error." << endl;
				return 0;
			}
		}
	}
	else if (type == 2){//float
		for (int j = 0; j < value.size(); j++){
			if ((value[j]<'0' || value[j]>'9') && value[j] != '.'){
				cout << "ERROR: Data type error." << endl;
				return 0;
			}
		}
	}
	else{//char
		if (flag == 0 && (value[0] == '"' && value[value.size() - 1] != '"' || value[0] == '\'' && value[value.size() - 1] != '\'')){//���Ų��ɶ�
			return -1;
		}
	}

	//cout << value << " ";
	return 1;
}

string Interpreter::readIn(){
	string temp;
	string instr;
	int flag;

	//����һ�������ȥ����;��
	instr = "";
	while (1){
		getline(cin, temp);
		flag = 0;
		int i;
		//for (i = temp.size() - 1; i >= 0; i--){//�Ӻ���ǰ�����������Ƿ��� ;
		//	if (temp[i] == ';'){
		//		flag = 1;
		//		break;
		//	}
		//}
		for (i = 0; i < temp.size(); i++){//�Ӻ���ǰ�����������Ƿ��� ;
			if (temp[i] == ';'){
				flag = 1;
				break;
			}
		}
		if (flag == 1){
			temp.erase(i, temp.size() - i);//Ĩ����iλ�������ڵ�λ�ã�����֮�����е��ַ�
			instr += temp;
			break;
		}
		else{
			temp += " ";
			instr += temp;
			cout << "      -> ";
		}
	}
	instr += " ";//��β��һ���ո񣬱��ڷָ�

	return instr;
}

vector<string> Interpreter::separate(string instr){
	vector<string> element;
	string temp = "";
	int n = 0;
	int flag = 0;

	//�ָ������ȥ���հ׷�
	for (int i = 0; i < instr.size(); i++){
		if (instr[i] == '*' || instr[i] == ',' || instr[i] == '(' || instr[i] == ')'
			|| instr[i] == '>' || instr[i] == '<' || instr[i] == '='){//���ǣ�������ַ��ڵ�
			if (flag == 1){
				element.push_back(temp);
				n++;
			}
			temp = "";
			temp += instr[i];
			if (i<instr.size() - 1 && ((instr[i] == '<'&&instr[i + 1] == '=') || 
				(instr[i] == '>'&&instr[i + 1] == '=') || (instr[i] == '<'&&instr[i + 1] == '>'))){
				i++;
				temp += instr[i];
			}
			element.push_back(temp);
			temp = "";
			flag = 0;
		}
		else if (instr[i] != ' ' &&instr[i] != '\t'){
			temp.push_back(instr[i]);
			flag = 1;
		}
		else if (flag == 1){
			element.push_back(temp);
			n++;
			temp = "";
			flag = 0;
		}
	}

	return element;
}

int Interpreter::createTable(vector<string> &element, int size){
	if (isLegal_name(element[2])){//������ȷ
		if (cmanager.isExistTable(element[2])){//�ѽ������˱�
			cout << "ERROR: Table '" << element[2] << "' already exists" << endl;
			return 0;
		}
		vector<string> attr;
		vector<int> type;//intΪ1��floatΪ2��char(n)Ϊ3+n
		vector<int> isKey;
		string temp_attr;
		int temp_type;

		if (size == 3){
			cout << "ERROR: A table must have at least 1 column" << endl;
			return 0;
		}
		else if (size == 4 || size == 5)
			goto ct_error;
		else if (element[3] == "(" && element[size - 1] == ")"){
			//��������
			int i = 4;
			while (element[i] != "primary" && i<size){
				//�ж��������������Ƿ���ȷ
				int isType = isLegal_type(element[i + 1]);
				if (isLegal_name(element[i]) && isType){
					//������
					temp_attr = element[i++];
					attr.push_back(temp_attr);
					//������
					if (isType == 3){//char
						if (element[i + 1] == "(" &&element[i + 3] == ")"){
							int number = atoi(element[i + 2].c_str());
							if (number > 255){
								cout << "ERROR: Column length too big for column '" << temp_attr << "' (max = 255);" << endl;
								return 0;
							}
							else if (number < 1)
								goto ct_error;
							else{
								temp_type = isType;
								type.push_back(temp_type + number);
								i += 4;
							}
						}
						else
							goto ct_error;
					}
					else{//int & float
						temp_type = isType;
						type.push_back(temp_type);
						i++;
					}
					//����unique
					if (element[i] == "unique"){
						isKey.push_back(1);
						i++;
					}
					else
						isKey.push_back(0);
					//������
					if (element[i] != "," && i != size - 1)
						goto ct_error;
					else if (i == size - 1)
						break;
					else
						i++;
				}//if_end: �������������;���ȷʱ
				else
					goto ct_error;
			}//while_end: ��������

			//����primary
			if (element[i] == "primary"){
				if (i == size - 6 && element[i + 1] == "key" && element[i + 2] == "(" && element[i + 4] == ")"){
					if (i == 4){//primary֮ǰû�ж��������
						cout << "ERROR: A table must have at least 1 column" << endl;
						return 0;
					}
					else{
						int isExist = 0;
						int j;
						for (j = 0; j < attr.size(); j++){
							if (element[i + 3] == attr[j]){
								isExist = 1;
								break;
							}
						}
						if (!isExist){
							cout << "ERROR: Key column '" << element[i + 3] << "' doesn't exist in table" << endl;
							return 0;
						}
						else{
							isKey[j] = 2;
						}
					}
				}
				else
					goto ct_error;
			}//if_end: ����primary
		}
		else
			goto ct_error;

		//�鿴�������Ƿ��ظ��������ִ�Сд
		for (int i = 0; i < attr.size(); i++)
			transform(attr[i].begin(), attr[i].end(), attr[i].begin(), tolower);

		for (int i = 0; i < attr.size(); i++)
			for (int j = i + 1; j < attr.size(); j++)
				if (attr[i] == attr[j]){
					cout << "ERROR: Duplicate column name '" << attr[j] << "'" << endl;
					return 0;
				}

		//������
		if (api.createTable(element[2],attr, type, isKey)){
			//describeTable(attr, type, isKey);//���Ҫ�Ӹ�tableName����
			return 1;
		}

	}//if_end: ��������ȷʱ
	else{
	ct_error:
		handleError(0, "create table ", "");
		return 0;
	}
}

int Interpreter::insertRecord(vector<string> &element, int size){
	if (size > 6 && isLegal_name(element[2]) && element[3] == "values" && element[4] == "(" && element[size - 1] == ")"){
		if (cmanager.isExistTable(element[2])){
			if (size - 6 == 2 * cmanager.getAttrNum(element[2]) - 1){//���϶��ţ������value�������Ƿ���ȷ
				int i = 5;
				int j = 0;
				vector<string> attrName;
				vector<int> type;
				vector<int> isKey;
				cmanager.getAttrInfo(element[2], attrName, type, isKey);
				
				vector<string> value;
				do{
					int isTM = isTypeMatch(element[i], type[j]);
					if (isTM){
						if (isKey[j]){//�����key��Ҫ�ж��Ƿ�Ϊ�ظ�Ԫ
							string s[] = { "select", "*", "from", element[2], "where", attrName[j],"=",element[i]};
							vector<string> temp(s, s + 8);
							
							if (select(temp, temp.size(), 0, 1) == 0){
								value.push_back(element[i]);
							}
							else{
								cout << "ERROR: Duplicate entry '"<<element[i]<<"' for key." << endl;
								return 0;
							}
						}
						else
							value.push_back(element[i]);
					}
					else if (isTM == -1)
						goto i_error;
					else//isTM==0
						return 0;

					i += 2;
					j++;
				} while (i < size - 1);
								
				api.insert(element[2], value, type);
			}
			else{
				cout << "ERROR: Column count doesn't match value count" << endl;
				return 0;
			}
		}
		else{
			cout << "ERROR: Table '" << element[2] << "' doesn't exist" << endl;
			return 0;
		}
	}
	else {
	i_error:
		handleError(0, "insert ", "");
		return 0;
	}

	return 1;
}

int Interpreter::select(vector<string> &element, int size, int toDelete, int isInsert){
	if (isLegal_name(element[3-toDelete])){
		if (cmanager.isExistTable(element[3 - toDelete])){
			vector<string> attr;
			vector<string> op;
			vector<string> value;
			vector<int> type;
			vector<int> begin;
			int recordSize=0;
			vector<int> type_all;
			vector<int> noUse;
			vector<string> attr_all;
			cmanager.getAttrInfo(element[3 - toDelete], attr_all, type_all, noUse);
			for (int i = 0; i < type_all.size(); i++)
				if (type_all[i] == 1 || type_all[i] == 2)
					recordSize += 4;
				else
					recordSize += type_all[i] - 3;
			recordSize += 8;

			if (size > 4 - toDelete){//������
				if (element[4 - toDelete] == "where" && (size - 5 + 1 + toDelete) % 4 == 0){//�����Ƿ�����
					int i = 5 - toDelete;
					while (i<size){
						int tempBegin=0;
						int attrIndexPlusOne = cmanager.isExistAttr(element[3 - toDelete], element[i]);//�����ڣ�����(attr�����+1)
						if (attrIndexPlusOne){
							if (isLegal_op(element[i + 1])){
								if (isTypeMatch(element[i + 2], type_all[attrIndexPlusOne - 1]) == 1){
									attr.push_back(element[i]);
									op.push_back(element[i + 1]);
									value.push_back(element[i + 2]);
									type.push_back(type_all[attrIndexPlusOne - 1]);
									for (int k = 0; k < attrIndexPlusOne - 1; k++){
										if (type_all[k] == 1 || type_all[k] == 2)
											tempBegin += 4;
										else
											tempBegin += type_all[k] - 3;
									}
									begin.push_back(tempBegin);
								}
								else
									return 0;
							}
							else
								goto s_error;
						}
						else{
							cout << "ERROR: Key column '" << element[i] << "' doesn't exist in table" << endl;
							return 0;
						}

						i += 3;
						if (i<size && element[i] == "and")
							i++;
					}//while end
				}
				else
					goto s_error;
			}
			
			//������&û����
			vector<vector<char>> returnRecord;
			int n = api.select(element[3 - toDelete], attr, op, value, type, begin, recordSize, returnRecord, toDelete);
			if (!isInsert  && n!=0 && !toDelete){
				showTable(returnRecord, type_all, attr_all);
				//for (int i = 0; i < returnRecord.size(); i++)//output
				//	printvc(returnRecord[i]);
			}
			
			return n;	
		}
		else{
			cout << "ERROR: Table '" << element[3 - toDelete] << "' doesn't exist" << endl;
			return 0;
		}	
	}
	else s_error:
	handleError(0, "", "");

	return 0;
}

int Interpreter::execute(vector<string> &element){
	//��¼Ԫ�ظ���
	int size = element.size();

	/////////////////////empty/////////////////////
	if (size == 0)
		cout << "No query specified" << endl;

	/////////////////////quit/////////////////////
	else if (element[0] == "quit"){
		if (size == 1){
			cout << "Bye" << endl;
			return 0;
		}
		else{
			handleError(0, "quit ","");
		}
	}

	/////////////////////create table/////////////////////
	else if (size >= 3 && element[0] == "create" && element[1] == "table"){
		if(createTable(element, size))//����̫���ˣ����������
			cout<<"Query OK"<<endl;
	}

	/////////////////////drop table/////////////////////
	else if (size == 3 && element[0] == "drop" && element[1] == "table"){
		if (isLegal_name(element[2])){
			if (cmanager.isExistTable(element[2])){
				if(api.dropTable(element[2]))
					cout<<"Query OK"<<endl;
			}
			else
				cout << "ERROR: Table '" << element[2] << "' doesn't exist" << endl;
		}
		else

			handleError(0, "drop table ", "");
	}

	/////////////////////create index/////////////////////
	else if (size == 8 && element[0] == "create"&&element[1] == "index"&&element[3] == "on"){
		if (isLegal_name(element[2]) && isLegal_name(element[4]) && isLegal_name(element[6])){
			if (cmanager.isExistTable(element[4])){
				int n = cmanager.isExistAttr(element[4], element[6]);
				vector<string> noUse;
				vector<int> noUse1;
				vector<int> type;
				cmanager.getAttrInfo(element[4], noUse, type, noUse1);
				if (n){
					if (!cmanager.isExistIndex(element[2])){
						if (api.createIndex(element[4], element[6], element[2], type, n-1))
							cout << "Query OK" << endl;
					}
					else
						cout << "ERROR: Index '" << element[2] << "' already exists." << endl;
				}
				else
					cout << "ERROR: Key column '" << element[6] << "' doesn't exist in table" << endl;
			}
			else
				cout << "ERROR: Table '" << element[4] << "' doesn't exist" << endl;
		}
		else
			handleError(0, "create index ", "");
	}

	/////////////////////drop index/////////////////////
	else if (size == 3 && element[0] == "drop" && element[1] == "index"){
		if (isLegal_name(element[2])){
			if (cmanager.isExistIndex(element[2])){
				if(api.dropIndex(element[2]))
					cout << "Query OK" << endl;
			}
			else
				cout << "ERROR: Index '" << element[2] << "' doesn't exist" << endl;
		}
		else
			handleError(0, "drop index ", "");
	}

	/////////////////////insert/////////////////////
	else if (size >= 3 && element[0] == "insert" && element[1] == "into"){
		if(insertRecord(element, size))
			cout << "Query OK" << endl;
	}

	/////////////////////select/////////////////////
	else if (size >= 4 && element[0] == "select" && element[1] == "*" && element[2] == "from"){
		int rValue = select(element, size,0, 0);
		//if(rValue)
		cout << rValue << " rows in set." << endl;
	}

	/////////////////////delete/////////////////////
	else if (size >= 3 && element[0] == "delete"&& element[1] == "from"){
		int rValue = select(element, size,1, 0);
		//if (rValue)
		cout << rValue << " rows in set." << endl;
	}

	/////////////////////describe table/////////////////////
	else if (size == 2 && element[0] == "describe"){
		if (isLegal_name(element[1])){
			if (cmanager.isExistTable(element[1])){
				vector<string> attr;
				vector<int> type;
				vector<int> isKey;
				cmanager.getAttrInfo(element[1], attr, type, isKey); 
				describeTable(attr, type, isKey);
			}
			else
				cout << "ERROR: Table '" << element[1] << "' doesn't exist" << endl;
		}
		else
			handleError(0, "describe table ", "");
	}

	/////////////////////show tables/////////////////////
	else if (size == 2 && element[0] == "show" && element[1] == "tables"){
		api.showTables();
		/*Table table;
		vector<vector<char>> returnRecord;
		rmanager.getAllRecord("table.catalog", table.recordSize, returnRecord);
		vector<int> type;
		type.push_back(53);
		vector<string> title;
		title.push_back("Tables_in_miniSQL");
		cout << showTable(returnRecord, type, title) << " rows in set." << endl;*/
	}

	/////////////////////execfile/////////////////////
	else if (size==2 && element[0] == "execfile"){
		string temp;
		string instr;
		int flag;
		ifstream infile(element[1].c_str());
		if (infile.is_open()){
			while (1){
				//����һ�������ȥ����;��
				instr = "";
				while (1){
					getline(infile, temp);
					flag = 0;
					int i;
					for (i = temp.size() - 1; i >= 0; i--){//�Ӻ���ǰ�����������Ƿ��� ;
						if (temp[i] == ';'){
							flag = 1;
							break;
						}
					}
					if (flag == 1){
						temp.erase(i, temp.size() - i);//Ĩ����iλ�������ڵ�λ�ã�����֮�����е��ַ�
						instr += temp;
						break;
					}
					else{
						temp += " ";
						instr += temp;
					}
				}
				instr += " ";//��β��һ���ո񣬱��ڷָ�

				vector<string> element = separate(instr);
				if (execute(element) == 0 || infile.eof())//quit
					break;
			}
		}
		else{
			cout << "ERROR: Fail to open file " << element[1] << endl;
		}
	}

	/////////////////////others/////////////////////
	else{
		handleError(0, "", "");
	}

	return 1;
}