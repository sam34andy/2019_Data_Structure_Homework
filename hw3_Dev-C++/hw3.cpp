// hw3.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <sstream>
#include <bitset>
using namespace std;

struct Addr {
	string Text_Original;
	string Text;
	string Tag;
	string Index;
	string Offset;
	int index_dec; // set_index
	int counter; // ?
};

struct History {
	string Word;
	bool Hit; 
};

struct Line {
	string line_word;
	string line_tag;
	int line_counter;
};

Line *cache; 
int *set_history; 
vector<Addr> addr;
vector<History> history;
void Data_Transfer(int, int, int);
void Cache_Replacement(int, int);
int Binary_To_Decimal(string);
void print(int, int, int);

int main()
{
	int block_size = 16;
	int cache_size = 16; // 輸入的是KB
	int set_associative_num = 4;
	int line_num = ((cache_size * 1024) / block_size);
	int set_num = line_num / set_associative_num;

	cout << "Block size is ? ";
	cin >> block_size;
	cout << "Cache total size(KB) ? ";
	cin >> cache_size;
	cout << "Set associative number? ";
	cin >> set_associative_num;

	int index_num = log2(set_num);
	int offset_num = log2(cache_size);
	int addr_num = 20;
	int tag_num = addr_num - index_num - offset_num;

	string content;
	ifstream myfile("HW3.txt");
	if (myfile.is_open())
	{
		
		while (getline(myfile, content))
		{
			Addr temp_addr;
			temp_addr = { content, "", "", "", "", 0, 0 };
			addr.push_back(temp_addr);
			History temp_history = { content, 0 };
			history.push_back(temp_history);
		}
	}

	Data_Transfer(addr_num, index_num, offset_num); 
	Cache_Replacement(line_num, set_associative_num);

	system("pause");
}

void Data_Transfer(int addr_num, int index, int offset)
{
	cout << endl << "Address / Address(binary) / Tag(binary) / Index(binary) / Offset(binary) / Index(Decimal)" << endl;
	for (int i = 0; i < addr.size(); i++)
	{
		cout << addr[i].Text_Original << "\t";
		// hexadecimal to binary
		// https://stackoverflow.com/questions/18310952/convert-strings-between-hex-format-and-binary-format
		stringstream adapter;
		adapter << hex << addr[i].Text_Original;
		unsigned n;
		adapter >> n;
		bitset<32> b(n);
		addr[i].Text = (b.to_string()).substr(32 - addr_num, addr_num);
			
		// Save data to tag, index, offset
		string  s = addr[i].Text;
		addr[i].Offset = addr[i].Text.substr(addr_num - offset, offset);
		addr[i].Index = addr[i].Text.substr(addr_num - offset - index, index);
		addr[i].Tag = addr[i].Text.substr(0, addr_num - offset - index);
		cout << addr[i].Text << "\t" << addr[i].Tag << "\t" << addr[i].Index << "\t" << addr[i].Offset;

		// Transfer index binary to index decimal
		addr[i].index_dec = Binary_To_Decimal(addr[i].Index);
		cout << "\t" << addr[i].index_dec << endl;
 	}
	cout << endl;
}

void Cache_Replacement(int line_number, int set_associative_num)
{
	
	cache = new Line[line_number]();
	set_history = new int[line_number / set_associative_num](); // to record used space in each space
	
	for (int i = 0; i < addr.size(); i++)
	{
		bool match_found = false;

		int set_counter = 0;
		int line_index = addr[i].index_dec * 4; 

		for (int j = 0; j < set_history[addr[i].index_dec]; j++) // check match Tag and update counter
		{
			cache[line_index + j].line_counter++;
			if (cache[line_index + j].line_tag == addr[i].Tag)
			{
				cache[line_index + j].line_word = addr[i].Text_Original;
				cache[line_index + j].line_counter = 0;
				match_found = true;
			}
		}

		if (match_found == true)
		{
			print(i, (line_number / set_associative_num), set_associative_num);
			continue;
		}

		if (set_history[addr[i].index_dec] < set_associative_num) // while there still have empty space
		{
			cache[line_index + set_history[addr[i].index_dec]].line_word = addr[i].Text_Original;
			cache[line_index + set_history[addr[i].index_dec]].line_tag = addr[i].Tag;
			cache[line_index + set_history[addr[i].index_dec]].line_counter = 0;
			set_history[addr[i].index_dec]++;
		}
		else if (set_history[addr[i].index_dec] == set_associative_num) //if the set is already full
		{
			// LRU -> Least Recently Used 
			int max_counter_tag = 0;
			int max_counter_number = 0;
			for (int t = 0; t < set_associative_num; t++) 
				if (cache[line_index + t].line_counter > max_counter_number)
				{
					max_counter_tag = t;
					max_counter_number = cache[line_index + t].line_counter;
				}
					 
			cache[line_index + max_counter_tag].line_word = addr[i].Text_Original;
			cache[line_index + max_counter_tag].line_tag = addr[i].Tag;
			cache[line_index + max_counter_tag].line_counter = 0;

		}
		print(i, (line_number / set_associative_num), set_associative_num);
	}
}

int Binary_To_Decimal(string addr)
{
	// https://stackoverflow.com/questions/7352099/stdstring-to-char
	int counter = 0;
	int result = 0;
	const char *Number = addr.c_str();
	for (int i = (addr.length() - 1); i >= 0; i--)
	{
		if (Number[i] == '1')
			result += pow(2, counter);
		counter++;
	}

	return result;
}

void print(int address_tag, int set_num,int set_associative_num)
{
	cout << "---------------------------------------------------------------" << endl;
	cout << ">>> " << addr[address_tag].Text_Original << " <<<" << endl;

	for (int i = 0; i < set_num; i++)
		if (set_history[i] != 0)
		{
			int temp = (i * 4);
			cout << "Set " << i << " counter: [" << set_history[i] << "]" << endl;
			for (int j = 0; j < set_associative_num; j++)
				cout << cache[temp + j].line_word << "\t" << cache[temp + j].line_counter << endl;
		}

	//cout << "Set 4C(76) counter: [" << set_history[76] << "]" << endl;
	//cout << cache[304].line_word << "\t" << cache[304].line_counter << endl
	//	<< cache[305].line_word << "\t" << cache[305].line_counter << endl
	//	<< cache[306].line_word << "\t" << cache[306].line_counter << endl
	//	<< cache[307].line_word << "\t" << cache[307].line_counter << endl << endl;

	//cout << "Set 4E(78) counter: [" << set_history[78] << "]" << endl;
	//cout << cache[312].line_word << "\t" << cache[312].line_counter << endl
	//	<< cache[313].line_word << "\t" << cache[313].line_counter << endl
	//	<< cache[314].line_word << "\t" << cache[314].line_counter << endl
	//	<< cache[315].line_word << "\t" << cache[315].line_counter << endl;
}
