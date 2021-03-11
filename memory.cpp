#include<iostream>
#include<stdio.h>
#include<vector>
#include<string>
#include<string.h>
#include<fstream>
#include<list>
#include<utility>
using namespace std;

#define SIZE 20
#define PTRLISTSIZE 10000

const int checkHead = 0x1234abcd;		//vertif放在文件的开头和结尾用于简单判断是否正确读取数据 
const int checkTail = 0x1234dcba;		//vertif放在文件的开头和结尾用于简单判断是否正确读取数据 
const int checkHeadAll = 0x12341234;		//vertif放在文件的开头和结尾用于简单判断是否正确读取数据 
const int checkTailAll = 0x12344321;		//vertif放在文件的开头和结尾用于简单判断是否正确读取数据 
const int check = 0x1234dcba;
string filePath = "D:/temp/";


//每个obj代表一个记忆节点或者是节点的一部分
//本质是个可变长度数组 数组的大小位SIZE
class obj
{
public:
	int aaa[SIZE];
	int numb = 0;//数组内元素个数
	int identifier = 0;//该数组的编号 通过编号可以从列表里找到
	obj* next = NULL;// 类似链表的方式链接下一个数组

	obj(int id)
	{
		identifier = id;
	}

	obj()
	{
		//子节点 不用id
	}

	void push_back(int x)
	{
		numb++;
		if (numb <= SIZE)
		{
			aaa[numb - 1] = x;
		}
		else
		{
			if (next == NULL)
			{
				next = new obj();
			}
			next->push_back(x);
		}
	}

	//获取值 跟数组相同
	int get(int x)
	{
		if (x < numb)
		{
			if (x < SIZE)
			{
				return aaa[x];
			}
			else
			{
				return(next->get(x - SIZE));
			}
		}
		else
		{
			cout << "x = " << x << endl;
			cout << "size = " << numb << endl;

			cout << " 数组越界" << endl;
			exit(-1);
		}
	}

	//识别到checkHead后触发restoreData
	//count为0时是初始状态 count为正数时表示还需要读入多少信息
	//返回值为-1时出错 0正常运行
	int restoreData(ifstream& ifs, int count)
	{
		int check;
		if (count == 0)
		{
			ifs.read((char*)& check, sizeof(int));
			//cout <<"checkHead="<< check << endl;
			if (check == checkHead)
			{
				//cout << "check right" << endl;
				ifs.read((char*)& identifier, sizeof(int));
				ifs.read((char*)& numb, sizeof(int));
				//cout << "id = " << identifier << endl;
				//cout << "size = " << numb << endl;
				count = numb;
				if (numb > SIZE)
				{
					ifs.read((char*)& aaa[0], SIZE * sizeof(int));
					count = numb - SIZE;
					if (next == NULL)
					{
						next = new obj();
					}
					next->restoreData(ifs, count);
				}
				else
				{
					int totalSize = count * sizeof(int);
					ifs.read((char*)& aaa[0], totalSize);
				}
			}
			else
			{
				cout << "读取错误" << endl;
				return -1;
			}
			//检查是否是尾确认码 不是的话报错
			ifs.read((char*)& check, sizeof(int));
			//cout << "checkTail=" << check << endl;

			if (check != checkTail)
			{
				//cout << "restore fail!" << endl;
				return -1;
			}
			else
			{
				//cout << "success" << endl;
				return 0;
			}

		}
		else
		{
			numb = count;
			if (count > SIZE)
			{
				ifs.read((char*)& aaa[0], SIZE * sizeof(int));
				count = count - SIZE;
				if (next == NULL)
				{
					next = new obj();
				}
				next->restoreData(ifs, count);
			}
			else
			{
				ifs.read((char*)& aaa[0], count * sizeof(int));
			}
		}

	}

	//保存数据 输入为一个输出流和个数
	//保存格式 以一个包的形式保存 包头从头到尾是 
	//头确认码 该组编号 数据个数 数据本体 尾确认码
	//   int       int     int       n*int   int                 总长度为n+4
	void saveData(ofstream& ofs, int count)
	{
		if (count == 0)
		{
			count = numb;
			//确认头
			ofs.write((char*)& checkHead, sizeof(int));
			ofs.write((char*)& identifier, sizeof(int));
			ofs.write((char*)& count, sizeof(int));
			if (count > SIZE)
			{
				ofs.write((char*)& aaa[0], SIZE * sizeof(int));
				count = count - SIZE;
				next->saveData(ofs, count);
			}
			else
			{
				ofs.write((char*)& aaa[0], count * sizeof(int));
			}
			ofs.write((char*)& checkTail, sizeof(int));
		}
		else
		{
			if (count > SIZE)
			{
				ofs.write((char*)& aaa[0], SIZE * sizeof(int));
				count = count - SIZE;
				next->saveData(ofs, count);
			}
			else
			{
				ofs.write((char*)& aaa[0], count * sizeof(int));
			}
		}
	}




	//打印整个数组
	void display()
	{
		cout << "size=" << numb << endl;
		cout << "id=" << identifier << endl;
		for (int i = 0; i < numb; i++)
		{
			cout << get(i) << endl;
		}
	}
};


//记忆节点的地址   obj的地址
class ptrNode
{
public:
	obj* me = NULL;
	void creat(int id)
	{
		if (me == NULL)
		{
			me = new obj(id);
		}
		else
		{
			cout << "重复创建 创建失败" << endl;
			exit(-1);
		}

	}

	int restoreDataFromDisk(ifstream& ifs, int id)
	{
		creat(id);
		return(me->restoreData(ifs, 0));
	}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//由节点地址组成，管理节点列表
class ptrList
{
public:
	string name;
	string path;
	ptrNode lista[PTRLISTSIZE];
	int numb = 0;//目前有几个元素

	ptrList(string n)
	{
		name = n;
		path = filePath + n;
	}

	int pushBack(obj& a)
	{
		lista[numb].me = &a;
		lista[numb].me->identifier = numb;
		numb++;
		return numb - 1;
	}


	void pushBackab(int a, int b)
	{
		if (a < numb)
		{
			lista[a].me->push_back(b);
		}
		else
		{
			cout << "pushBackab越界" << endl;
		}
	}
	//checkHead numb body checkTail
	void readAll()
	{
		ifstream ifile(path.c_str(), ios::binary);
		int check;
		ifile.read((char*)& check, sizeof(int));
		if (check == checkHeadAll)
		{
			ifile.read((char*)& numb, sizeof(int));
			for (int i = 0; i < numb; i++)
			{
				if (lista[i].restoreDataFromDisk(ifile, i) == -1)
				{
					cout << "wrong i = " << i << endl;
					exit(-1);
				}
				if (lista[i].me->identifier != i)
				{
					cout << "id wrong, i = " << i << endl;
					cout << "id wrong, id = " << lista[i].me->identifier << endl;
					//exit(-1);
				}
				//lista[i].me->display();
			}
		}
		else
		{
			cout << "头校验码错误" << endl;
			cout << "checkHeadAll" << checkHeadAll << endl;
			cout << "check" << check << endl;
		}
		//检查是否是尾确认码 不是的话报错
		ifile.read((char*)& check, sizeof(int));
		if (check != checkTailAll)
		{
			cout << "尾效验码出错" << endl;
			cout << "checkTailAll" << checkTailAll << endl;
			cout << "check" << check << endl;
		}
		else
		{
			cout << name;
			cout << "读取成功！" << endl;
		}
		ifile.close();
	}
	/////////////////////////////////////
	void saveAll()
	{
		ofstream ofile(path.c_str(), ios::binary);
		ofile.write((char*)& checkHeadAll, sizeof(int));
		ofile.write((char*)& numb, sizeof(int));
		for (int i = 0; i < numb; i++)
		{
			lista[i].me->saveData(ofile, 0);
		}
		ofile.write((char*)& checkTailAll, sizeof(int));
		ofile.close();
	}

	void display()
	{
		cout << "当前打印的是" << name << endl;
		cout << "个数是" << numb << endl;
		for (int i = 0; i < numb; i++)
		{
			cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
			if (lista[i].me != NULL)
				lista[i].me->display();
			cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
		}
	}

	obj* get(int numb)
	{
		return lista[numb].me;
	}
};
ptrList total("total");



int main() {

	//obj x, y, z, xx, xxx, xxxx, yy, yyyy;


	//for (int i = 0; i < 100; i++)
	//{
	//	x.push_back(i);
	//	y.push_back(i * 5);
	//}

	//for (int i = 0; i < 200; i++)
	//{
	//	z.push_back(i);
	//	xx.push_back(i * 4);
	//}

	//for (int i = 0; i < 10; i++)
	//{
	//	xxx.push_back(i);
	//	xxxx.push_back(i * 3);
	//}

	//for (int i = 0; i < 180; i++)
	//{
	//	yyyy.push_back(i);
	//	yy.push_back(i * 2);
	//}


	//total.pushBack(x);
	//total.pushBack(y);
	//total.pushBack(z);
	//total.pushBack(yy);
	//total.pushBack(xx);
	//total.pushBack(yyyy);
	//total.pushBack(xxxx);
	//total.pushBack(xxx);

	//total.display();

	//total.saveAll();





	total.readAll();
	total.display();

	//int size = 300;
	//int a[300];
	//ifile.read((char*)& a[0], size * sizeof(int));
	//for (int i = 0; i < size; i++)
	//{
	//	cout <<  i << "=========" << a[i] << endl;
	//}
	//ifile.close();
}

