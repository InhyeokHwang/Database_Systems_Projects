#include <bits/stdc++.h>
using namespace std;

class name_grade {
	public:
		string student_name;
		int korean;
		int math;
		int english;
		int science;
		int social;
		int history;

		void set_grade(string tuple)
		{
			stringstream tuplestr(tuple);
			string tempstr;

			getline(tuplestr, student_name, ',');

			getline(tuplestr, tempstr, ',');
			korean = stoi(tempstr);
			
			getline(tuplestr, tempstr, ',');
			math = stoi(tempstr);
			
			getline(tuplestr, tempstr, ',');
			english = stoi(tempstr);
			
			getline(tuplestr, tempstr, ',');
			science = stoi(tempstr);
			
			getline(tuplestr, tempstr, ',');
			social = stoi(tempstr);
			
			getline(tuplestr, tempstr);
			history = stoi(tempstr);
		}
};

class name_number{
	public :
		string student_name;
		string student_number;

		void set_number(string tuple)
		{
			stringstream tuplestr(tuple);
			string tempstr;


			getline(tuplestr, student_name, ',');
			getline(tuplestr, student_number, ',');
		}
};

string make_tuple(string name, string number)
{
	string ret = "";

	ret += name+ "," + number +"\n";

	return ret;
}

int main(){

	string buffer[2];
	name_grade temp0;
	name_grade temp1;
	name_number temp2;
	fstream block[12];
	ofstream output;

	output.open("./output3.csv");

	if(output.fail())
	{
		cout << "output file opening fail.\n";
	}

	/*********************************************************************/

	int i; //name_grade1 블록에 대한 index
	int j; //name_grade2 블록에 대한 index

	int open_count = 0; //open 횟수를 세는 변수

	int inner_i; //name_age 블록 내부 튜플의 index
	int inner_j; //name_salary 블록 내부 튜플의 index

	
	

	//block nested loop join
	for(i = 0; i < 1000; i++){
		block[0].open("./case3/name_grade1/" + to_string(i) + ".csv");
		if(block[0].fail()){
			cout << "block[0] file opening fail.\n";
		}
		open_count++; //open_count 횟수 증가
		
		for(j = 0; j < 1000; j++){
			block[1].open("./case3/name_grade2/" + to_string(j) + ".csv");
			if(block[1].fail()){
				cout << "block[1] file opening fail.\n";
			}
			open_count++; //open_count 횟수 증가
			

			//선택된 블록들에서 값을 비교 후 조인
			for(inner_i = 0; inner_i < 10;  inner_i++){
				getline(block[0], buffer[0]); //블록에서 한 줄 읽어오기
				temp0.set_grade(buffer[0]);//인스턴스 변수 설정
				
				for(inner_j = 0; inner_j < 10; inner_j++){
					getline(block[1], buffer[1]); //블록에서 한 줄 읽어오기
					temp1.set_grade(buffer[1]); //인스턴스 변수 설정

					if((temp0.student_name).compare(temp1.student_name) == 0){
						cout << "학생 카운트 " << endl;
						int cnt = 0; //성적 향상이 일어난 과목의 수
						if(temp0.english > temp1.english){
							cnt++;
						}
						if(temp0.history > temp1.history){
							cnt++;
						}
						if(temp0.korean > temp1.korean){
							cnt++;
						}
						if(temp0.math > temp1.math){
							cnt++;
						}
						if(temp0.science > temp1.science){
							cnt++;
						}
						if(temp0.social > temp1.social){
							cnt++;
						}
						
						if(cnt >= 2){			
							for(int x = 0; x < 1000; x++){ //pipelining을 고려해서 진행
								block[2].open("./case3/name_number/" + to_string(x) + ".csv"); //pipelining을 고려해서 진행
								if(block[2].fail()){
									cout << "block[2] file opening fail.\n";
								}
								open_count++;

								for( int h = 0; h < 10; h++){
									getline(block[2], buffer[0]);
									temp2.set_number(buffer[0]);

									if((temp0.student_name).compare(temp2.student_name) == 0){
										output << make_tuple(temp2.student_name, temp2.student_number);
									}	
								}
								block[2].close();
							}
						}
					}

					if(inner_j == 9){
						block[1].clear(); //파일의 처음 위치로 돌아감
						block[1].seekg(0, ios::beg);
					}
				}
				if(inner_i == 9){
					block[0].clear(); //파일의 처음 위치로 돌아감
					block[0].seekg(0, ios::beg);
				}
			}
			block[1].close();
		}
		block[0].close();
	}

	cout << "open count : " + to_string(open_count + 1) << endl;



	/*********************************************************************/


	output.close();

	
}
