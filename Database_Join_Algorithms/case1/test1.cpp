#include <bits/stdc++.h>
using namespace std;

class name_age {
	public:
		string name;
		string age;
		
		void set_name_age(string tuple)
		{
			stringstream tuplestr(tuple);
			string agestr;

			getline(tuplestr, name, ',');
			getline(tuplestr, age);
		}
};

class name_salary {
	public:
		string name;
		string salary;
		
		void set_name_salary(string tuple)
		{
			stringstream tuplestr(tuple);
			string salarystr;

			getline(tuplestr, name, ',');
			getline(tuplestr, salary);
		}
};

string make_tuple(string name, string age, string salary)
{
	return name+ ',' + age + ',' + salary + '\n';
}

int main(){

	string buffer[2];
	name_age temp0;
	name_salary temp1;
	int current_block[2] = {};
	fstream block[12];
	ofstream output;

	output.open("./output1.csv");
	
	if(output.fail())
	{
		cout << "output file opening fail.\n";
	}

	/*********************************************************************************/
	current_block[0] = 0; //name_age 블록에 대한 index
	current_block[1] = 0; //name_salary 블록에 대한 index
	int open_count = 0; //open 횟수를 세는 변수

	int inner_i = 0; //name_age 블록 내부 튜플의 index
	int inner_j = 0; //name_salary 블록 내부 튜플의 index



	//merge join
	while(1){ //반복

		if(current_block[0] == 0 && current_block[1] == 0 && inner_i == 0 && inner_j == 0){ //맨 처음 파일을 여는 상황
			block[0].open("./case1/name_age/" + to_string(current_block[0]) +".csv");
			if(block[0].fail()){
				cout << "block[0] file opening fail.\n";
			}
			open_count++; //open_count 횟수 증가
			

			getline(block[0], buffer[0]); //블록에서 한 줄 읽어오기
			temp0.set_name_age(buffer[0]); //인스턴스 변수 설정

			block[1].open("./case1/name_salary/" + to_string(current_block[1]) + ".csv");
			open_count++; //open_count 횟수 증가
			if(block[1].fail()){
				cout << "block[1] file opening fail.\n";
			}

			getline(block[1], buffer[1]); //블록에서 한 줄 읽어오기
			temp1.set_name_salary(buffer[1]); //인스턴스 변수 설정
		}

		
		if(temp0.name.compare(temp1.name) < 0){
			inner_i++;
			if(inner_i == 10){
				block[0].close();
				current_block[0]++;
				if(current_block[0] == 1000){ //반복문 나가는 조건
					break;
				}
				inner_i = 0;
				block[0].open("./case1/name_age/" + to_string(current_block[0]) + ".csv");
				open_count++; //open_count 횟수 증가
				if(block[0].fail()){
					cout << "block[0] file opening fail.\n";
				}
			}
			getline(block[0], buffer[0]); //블록에서 한 줄 읽어오기
			temp0.set_name_age(buffer[0]); //인스턴스 변수 설정
		}else if(temp0.name.compare(temp1.name) > 0){
			inner_j++;
			if(inner_j == 10){
				block[1].close();
				current_block[1]++;
				if(current_block[1] == 1000){ //반복문 나가는 조건
					break;
				}
				inner_j = 0;
				block[1].open("./case1/name_salary/" + to_string(current_block[1]) + ".csv");
				open_count++; //open_count 횟수 증가
				if(block[1].fail()){
					cout << "block[1] file opening fail.\n";
				}
			}
			getline(block[1], buffer[1]); //블록에서 한 줄 읽어오기
			temp1.set_name_salary(buffer[1]); //인스턴스 변수 설정
		}else{ //이름 같음
			output << make_tuple(temp0.name, temp0.age, temp1.salary);
			inner_i++;
			if(inner_i == 10){
				block[0].close();
				current_block[0]++;
				if(current_block[0] == 1000){ //반복문 나가는 조건
					break;
				}
				inner_i = 0;
				block[0].open("./case1/name_age/" + to_string(current_block[0]) + ".csv");
				open_count++; //open_count 횟수 증가
				if(block[0].fail()){
					cout << "block[0] file opening fail.\n";
				}
			}
			getline(block[0], buffer[0]); //블록에서 한 줄 읽어오기
			temp0.set_name_age(buffer[0]); //인스턴스 변수 설정
		}
	}
	
	cout << "open count : " + to_string(open_count + 1) << endl; //output의 open 까지
	
	/*********************************************************************************/


	output.close();
}
