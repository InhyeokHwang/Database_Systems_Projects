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
	fstream block[12];
	ofstream output;

	output.open("./output2.csv");

	if(output.fail())
	{
		cout << "output file opening fail.\n";
	}


	/******************************************************************/
	
	int open_count = 0; //open 횟수를 세는 변수
	int inner_i; //name_age 블록 내부 튜플의 index
	int inner_j; //name_salary 블록 내부 튜플의 index

	int first_letter[10]; //한 csv 파일에 있는 각 튜플의 이름 첫번째 글자
	

	
	for(int i = 0; i < 1000; i++){
		block[0].open("./case2/name_age/" + to_string(i) + ".csv");
		if(block[0].fail()){
			cout << "block[0] file opening fail.\n";
		}
		open_count++; //open_count 횟수 증가

		for(inner_i = 0; inner_i < 10; inner_i++){
			getline(block[0], buffer[0]);
			temp0.set_name_age(buffer[0]);
			first_letter[inner_i] = (int)((temp0.name)[0]) - 'a';

			int k;
			for(k = 0; k < inner_i; k++){
				if(first_letter[k] == first_letter[inner_i]){ //첫 글자에 대해서 이미 open된 파일이면 또 다시 open할 필요가 없음
					block[k + 2] << buffer[0] << endl; //probe input으로 옮기기
					break;
				}
			}

			//처음으로 등장한 첫글자라면
			if(k == inner_i){
				block[inner_i + 2].open("./buckets/probe_" + to_string(first_letter[inner_i]) + ".csv", ios::app); //파일을 여러번 실행하면 이미 쓰여진 값에 중복으로 쓰기 때문에 파일을 다 지우고 다시 실행해야함.
				if(block[inner_i + 2].fail()){
					cout << "block[inner_i+2] file opening fail.\n";
				}
				open_count++; //open_count 횟수 증가
				block[inner_i + 2] << buffer[0] << endl; //probe input으로 옮기기
			}
		}

		block[0].close();
		for(int p = 2; p < 12; p++){
			if(block[p].is_open()){
				block[p].close(); //연 블록들 다 닫아줌
			}
		}
	}


	for(int j = 0; j < 1000; j++){
		block[1].open("./case2/name_salary/" + to_string(j) + ".csv");
		if(block[1].fail()){
			cout << "block[1] file opening fail.\n";
		}
		open_count++; //open_count 횟수 증가

		for(inner_j = 0; inner_j < 10; inner_j++){
			getline(block[1], buffer[1]);
			temp1.set_name_salary(buffer[1]);
			first_letter[inner_j] = (int)((temp1.name)[0]) - 'a';

			int k;
			for(k = 0; k < inner_j; k++){
				if(first_letter[k] == first_letter[inner_j]){ //첫 글자에 대해서 이미 open된 파일이면 또 다시 open할 필요가 없음
					block[k + 2] << buffer[1] << endl; //probe input으로 옮기기
					break;
				}
			}

			//처음으로 등장한 첫글자라면
			if(k == inner_j){
				block[inner_j + 2].open("./buckets/build_" + to_string(first_letter[inner_j]) + ".csv", ios::app); //여러번 실행하면 이미 쓰여진 값에 중복으로 쓰기 때문에 파일을 다 지우고 다시 실행해야함.
				if(block[inner_j + 2].fail()){
					cout << "block[inner_j+2] file opening fail.\n";
				}
				open_count++; //open_count 횟수 증가
				block[inner_j + 2] << buffer[1] << endl; //probe input으로 옮기기
			}
		}

		block[1].close();
		for(int p = 2; p < 12; p++){
			if(block[p].is_open()){
				block[p].close(); //연 블록들 다 닫아줌
			}
		}
	}

	for(int hash_idx = 0; hash_idx < 10; hash_idx++){
		block[0].open("./buckets/build_" + to_string(hash_idx) + ".csv");
		if(block[0].fail()){
			cout << "block[0] file opening fail.\n";
		}
		open_count++;
		block[1].open("./buckets/probe_" + to_string(hash_idx) + ".csv");
		if(block[1].fail()){
			cout << "block[1] file opening fail.\n";
		}
		open_count++;
		while(getline(block[0], buffer[0])){
			temp0.set_name_age(buffer[0]);
			while(1){
				if(!getline(block[1], buffer[1])){
					block[1].clear(); //파일의 처음 위치로 돌아감
					block[1].seekg(0, ios::beg);
					break;
				}
				temp1.set_name_salary(buffer[1]);

				if(temp0.name.compare(temp1.name) == 0){
					output << make_tuple(temp0.name, temp0.age, temp1.salary);
				}
			}
		}

		block[0].close();
		block[1].close();

		
	}

	cout << "open count : " + to_string(open_count + 1) << endl; //output의 open 까지

	/******************************************************************/

	output.close();

	
}
