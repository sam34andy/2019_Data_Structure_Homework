// h1_UPGRADE.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <string>
#include <sstream>
using namespace std; 

struct TBH {
	int history[2];
	int CNT[4];
	int mis_pre;
	vector<string> h; // hsitory;
}; // TBH is Two Bit History. // SN-0, WN-1, WT-2, ST-3

struct Register {
	int counter;
	int value;
};

struct Instruction{
	string addr;
	string Type;
	int D_Reg;
	int InputA_Reg;
	int InputB_Reg;
	int immed; // immediate number
	string D_Addr; // 要跳去哪裡，這個的儲存法還要煩惱
}; // 

struct Jump_Addr {
	string addr;
	int inst_number;
};

vector<string>Inst_history;
vector<TBH>Tbh;
vector<int>Reg;
vector<Instruction>Inst;
vector<Jump_Addr>jump_addr;

void string_cut(string, int); // 進行字串切割
void Data_Saving(string, int, int); // 進行字符判別
void Calculating_Assembly(int);
int Find_matching_addr(string);
void TBH_predict(int, int, char);

int main()
{
	// 初始化20個Register
	for (int i = 0; i < 20; i++)
		Reg.push_back(0);

	// 決定entry的個數並初始化
	int entry_number = 0;
	cout << "Please input the entry number: ";
	cin >> entry_number;
	TBH initial = { {0,0}, { 0, 0, 0, 0}, 0};
	for (int i = 0; i < entry_number; i++)
		Tbh.push_back(initial);

	// 文本讀取與丟字串處理
	int inst_counter = 0;
	string content;
	ifstream myfile("ASSEMBLY.txt"); // ifstream myfile("ASSEMBLY_Test.txt");
	if (myfile.is_open())
		while (getline(myfile, content)) // 可以一次讀一行
		{		 
			Inst_history.push_back(content);
			string_cut(content, inst_counter);
			inst_counter++; 
		}

	Calculating_Assembly(entry_number);

	// Print out all the enrty histories
	int sum_misprediction = 0;
	cout << endl << endl << endl;
	for (int i = 0; i < Tbh.size(); i++)
	{
		cout << "Entry number: " << (i + 1) << endl;
		for (int j = 0; j < Tbh[i].h.size(); j++)
			cout << Tbh[i].h[j] << endl;

		cout << "Misprediction number: " << Tbh[i].mis_pre << endl << endl << endl;
		sum_misprediction = sum_misprediction + Tbh[i].mis_pre;
	}
	cout << "Sum misprediction number: " << sum_misprediction << endl << "OVER" << endl; // cout << Reg[2] << "\t" << Reg[3] << "\t" << Reg[4]; // Debug

	system("pause");
}

void string_cut(string content, int inst_n) // 進行字串處理
{
	Instruction inst = { "", "", -1, -1, -1, NULL, "" };
	Inst.push_back(inst);

	// cout << "Hello World!\n";
	string extract = "";

	// 截出一段文字
	while (true)
	{
		int start = content.find_first_not_of(" \t,:"); //cout << start << "\t";
		int end = content.find_first_of(" \t,:\n"); //cout << end << "\t";
		extract = content.substr(0, end);
		Data_Saving(extract, inst_n, content.length());
		if (end == -1)
			return;

		//對字串做整理
		content = content.substr(end, content.length() - extract.length());
		int split_n = content.find_first_not_of("\t, :");
		if (split_n == string::npos) // 如果已經到字串尾巴就跳出
			return;
		content = content.substr(split_n, content.length() - split_n + 1);
		//cout << "." << content << endl;
	}
}

void Data_Saving(string s, int inst, int rest_n)
{
	// cout << "\t";

	if (s.find("0x") != string::npos)
	{
		Inst[inst].addr = s;
		// cout << Inst[inst].addr;
		return;
	}

	if ((s.find("Loop") != string::npos) || (s.find("End") != string::npos)) // 如果Type為空，就是一個跳躍點
	{
		Inst[inst].D_Addr = s; 
		if (Inst[inst].Type != "")
			return;
		Jump_Addr ja = { s, inst };
		jump_addr.push_back(ja);
		return;
	}

	stringstream geek(s); // 取出immediate
	int x;
	if (geek >> x)
	{
		Inst[inst].immed = x;
		return;
	}

	if (s.find("R") != string::npos)
	{
		s = s.substr(1, s.length() - 1);

		if (Inst[inst].D_Reg == -1)
		{
			Inst[inst].D_Reg = stoi(s);
			return;
		}
		if (Inst[inst].InputA_Reg == -1)
		{
			Inst[inst].InputA_Reg = stoi(s);
			return;
		}

		Inst[inst].InputB_Reg = stoi(s);
		return;
	}

	if (s.find("j") != string::npos)
	{
		Inst[inst].Type = s;
		return;
	}

	Inst[inst].Type = s;
	// cout << Inst[inst].Type;
	return;
}

void Calculating_Assembly(int entry_counter) {
	int working = 0;
	int entry = -1;

	while (working < Inst.size())
	{
		entry++;
		entry = (entry >= entry_counter) ? 0 : entry;

		// Inst_print(working);
		// cout << Inst_history[working] << "\t";
		if (Inst[working].Type == "li") 
		{
			Reg[Inst[working].D_Reg] = Inst[working].immed;
			TBH_predict(working, entry, 'N');
			working++;
			continue;
		}

		if (Inst[working].Type == "add")
		{
			Reg[Inst[working].D_Reg] = Reg[Inst[working].InputA_Reg] + Reg[Inst[working].InputB_Reg];
			TBH_predict(working, entry, 'N');
			working++;
			continue;
		}

		if (Inst[working].Type == "addi")
		{
			Reg[Inst[working].D_Reg] = Reg[Inst[working].InputA_Reg] + Inst[working].immed;
			TBH_predict(working, entry, 'N');
			working++;
			continue;
		}

		if (Inst[working].Type == "sub")
		{
			Reg[Inst[working].D_Reg] = Reg[Inst[working].InputA_Reg] - Reg[Inst[working].InputB_Reg];
			TBH_predict(working, entry, 'N');
			working++;
			continue;
		}

		if (Inst[working].Type == "subi")
		{
			Reg[Inst[working].D_Reg] = Reg[Inst[working].InputA_Reg] - Inst[working].immed; // cout << "R1: " << Reg[1] << "\tR2: " << Reg[2] << endl;
			TBH_predict(working, entry, 'N');
			working++;
			continue;
		}

		if (Inst[working].Type == "beq")
		{
			if (Reg[Inst[working].D_Reg] == Reg[Inst[working].InputA_Reg])
			{
				TBH_predict(working, entry, 'T');
				working = Find_matching_addr(Inst[working].D_Addr);
				continue;
			}
			else
			{
				TBH_predict(working, entry, 'N');
				working++;
				continue;
			}
		}

		if (Inst[working].Type == "bne")
		{
			if (Reg[Inst[working].D_Reg] != Reg[Inst[working].InputA_Reg])
			{
				TBH_predict(working, entry, 'T');
				working = Find_matching_addr(Inst[working].D_Addr);
				continue;
			}
			else
			{
				TBH_predict(working, entry, 'N');
				working++;
				continue;
			}
		}

		if (Inst[working].Type == "j")
		{
			TBH_predict(working, entry, 'T');
			working = Find_matching_addr(Inst[working].D_Addr);
			continue;
		}

		if (Inst[working].Type == "")
		{
			working++;
			entry--;
			continue;
		}
	}
}

int Find_matching_addr(string addr)
{
	for (int i = 0; i < jump_addr.size(); i++)
	{
		if (jump_addr[i].addr == addr)
			return jump_addr[i].inst_number;
	}
}

void TBH_predict(int instruction_number, int entry, char outcome)
{
	// 根據history做出predict
	int current_h = Tbh[entry].history[0] * 2 + Tbh[entry].history[1];
	char predict = (Tbh[entry].CNT[current_h] >= 2) ? 'T' : 'N';

	// 根據predict與輸入的outcome判斷有沒有預測正確
	bool guess = (outcome == predict) ? true : false;
	if (guess == false)
		Tbh[entry].mis_pre++;

	// 更新History的值
	Tbh[entry].history[0] = Tbh[entry].history[1];
	Tbh[entry].history[1] = (outcome == 'T') ? 1 : 0;

	// 根據outcome改變我們的Two Bit History
	if (outcome == 'T')
		(Tbh[entry].CNT[current_h])++;
	if (outcome == 'N')
		(Tbh[entry].CNT[current_h])--;
		

	// 權重修正
	if (Tbh[entry].CNT[current_h] > 3)
		Tbh[entry].CNT[current_h] = 3;
	if (Tbh[entry].CNT[current_h] < 0)
		Tbh[entry].CNT[current_h] = 0;

	string tbh_result = "";

	cout << "Entry No." << (entry + 1);
	cout << "\t";
	tbh_result = "(" + to_string(Tbh[entry].history[0]) + to_string(Tbh[entry].history[1]);
	for (int i = 0; i < 4; i++)
	{
		tbh_result = tbh_result + ",";		
		switch (Tbh[entry].CNT[i]) {
		case 0:
			tbh_result = tbh_result + "SN";
			break;
		case 1:
			tbh_result = tbh_result + "WN";
			break;
		case 2:
			tbh_result = tbh_result + "WT";
			break;
		case 3:
			tbh_result = tbh_result + "ST";
			break;
		}
	}
	tbh_result = tbh_result + ")\tP:" + predict + "\tO:" + outcome + "\tG:" + to_string(guess);
	cout << tbh_result +"\t";
	cout << Inst_history[instruction_number] << endl;

	Tbh[entry].h.push_back(tbh_result +"\t"+ Inst_history[instruction_number]);

	//cout << "TBH entry No." << (entry+1);
	//cout << "\t(" << Tbh[entry].history[0] << Tbh[entry].history[1];
	//for (int i = 0; i < 4; i++)
	//{
	//	
	//	// 
	//	cout << ", ";
	//	switch (Tbh[entry].CNT[i]) {
	//	case 0:
	//		cout << "SN";
	//		break;
	//	case 1:
	//		cout << "WN";
	//		break;
	//	case 2:
	//		cout << "WT";
	//		break;
	//	case 3:
	//		cout << "ST";
	//		break;
	//	}
	//}
	//cout << ")" << "\tP:" << predict << "\tO:" << outcome << "\tG:" << guess << "\t";
	//cout << Inst_history[instruction_number] << endl;
}

// 0.確定可以讀取文本，逐行讀取 -> OK
// 1.確定可以讀取組合語言的文本 -> OK
// 2.確定可以分析出要做哪些事，Branch等等 ->
// 3.確定可以知道要使用哪個Register ->
// 4.確認end of file後，執行程式->
// 5.確認執行的正確性->
// 6.正確印出資料

// 【好用的東西】
//string delimiter = " ";
//size_t pos;
//std::string token;
//while ((pos = content.find(delimiter)) != string::npos) {
//	token = content.substr(0, pos);
//	cout << token << "\t";
//	content.erase(0, pos + delimiter.length());
//}
//cout << endl;

// void Inst_print(int);
//void Inst_print(int working)
//{
//	cout << "[" << working << "]\t";
//	cout << Inst[working].addr << " "
//		<< Inst[working].Type << " "
//		<< Inst[working].D_Reg << " "
//		<< Inst[working].InputA_Reg << " "
//		<< Inst[working].InputB_Reg << " "
//		<< Inst[working].immed << " "
//		<< Inst[working].D_Addr << endl;
//}
