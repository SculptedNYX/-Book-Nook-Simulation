#include <iostream>
#include <algorithm>
#include <ctime>
#include <utility>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <queue>

using namespace std;

// Inits a vector of pairs where the cumulative prob is the first value and the minute is the second value
vector<pair<float, int>> create_pairs_vec (int start_offset, vector<float> prob) {
	vector<pair <float, int>> m;
	float sum = 0;
	for (int i = 0; i < prob.size(); i++) {
		sum = sum + prob[i];
		m.emplace_back(make_pair(sum, i+start_offset));
	}
	return m;
}

vector<pair<int, int>> generate_sim_arr (vector<int> r, const vector<pair<float, int>> TBA, const vector<pair<float, int>> COT) {
	vector<pair<int, int>> sim_arr;
	int size = r.size();

	// Find the largest element in the array to be used to get c
	int largest = *max_element(r.begin(), r.end());

	/* The reason i used log10 is because for any number with digits c the value of the number is between 10^(c) and 10^(c-1)
	 * therefore accodring to the power rule of logs log10(number) is between c and c-1. So all we need to do is floor the fraction
	 * the add +1 leaving us with c being the number of digits
	 */
	int c = log10(largest) + 1;

	float r_poststep = 0;
	for (int i = 0; i < size; i++) {
		// The pair that will eventually be added to the sim arr
		pair<int, int> tmp = make_pair(0, 0);

		// Apply steps to R
		r_poststep = r[i]/pow(10, c);
		// Iterate over the TBD and COT vector to find the time according to r_poststep <= cumulative probability
		int s1 = TBA.size();
		for (int i = 0; i < s1; i++) {
			if (r_poststep <= TBA[i].first) {
				tmp.first = TBA[i].second;
				break;
			}
		}

		int s2 = COT.size();
		for (int i = 0; i < s2; i++) {
			if (r_poststep <= COT[i].first) {
				tmp.second = COT[i].second;
				break;
			}
		}

		sim_arr.emplace_back(tmp);
	}

	return sim_arr;
}

// Creates an array of n exact values from 1->n
vector<int> generate_exact_inc_arr (int n) {
	vector<int> a;
	for (int i = 0; i < n; i++) {
		a.emplace_back(i+1);
	}

	return a;
}

vector<int> generate_rand_arr (int n, int range_end) {
	// Generate the seed for the random numbers
	srand((unsigned) time(NULL));
	
	vector<int> a;

	for (int i = 0; i < n; i++) {
		a.emplace_back(rand() % range_end);
	}

	return a;
}

vector<vector<int>> generate_sim_table (vector<pair<int, int>> sim_arr) {
	int num_customers = sim_arr.size();
	int cumulative_arrival_time = 0;

	vector<vector<int>> t (num_customers+1, vector<int>(10, 0));
	queue <int> cq;
	int customers_in_q = 0;

	for (int i = 0; i < num_customers; i++) {
		int waiting_time = 0;
		for (int j = 0; j < 10; j++) {
			switch (j) {
				// The customer column
				case 0:
					t[i][j] = i+1;
					break;
				// The iterarrival time
				case 1:
					t[i][j] = sim_arr[i].first;
					t[num_customers][j] += t[i][j];
					break;
				// Arrival time is the cumulitve of all the arrival times + the current interarrival time
				case 2:
					cumulative_arrival_time += sim_arr[i].first;
					t[i][j] = cumulative_arrival_time;
					t[num_customers][j] += t[i][j];
					break;
				// Service time
				case 3:
					t[i][j] = sim_arr[i].second;
					t[num_customers][j] += t[i][j];
					break;
				// Service time begin
				case 4:
					if (i == 0) {
						t[i][j] = t[i][j];
					}
					else {
						// if the arrival time is later then the end time of the prev customer then it begins at the arrival time
						if (t[i][2] >= t[i-1][7]) {
							t[i][j] = t[i][2];
						}
						// if not then the service begins at the end time of the prev customer
						else {
							waiting_time = t[i-1][7]-t[i][2];
							t[i][j] = t[i-1][7];
						}
					}
					t[num_customers][j] += t[i][j];
					break;
				// Waiting time
				case 5:
					t[i][j] = waiting_time;
					t[num_customers][j] += t[i][j];
					break;
				// Customers in Queue
				case 6:	
					// Add person to queue through their end time
					cq.push(t[i][4] + t[i][3]);
					
					// Here we use this to clear the queue if the time we are at is later than the time the person first at the queue takes to get server
					while (!cq.empty()) {
						if (t[i][2] >= cq.front() && !cq.empty()) {
							cq.pop();
						}
						else {
							break;
						}
					}
					// This is simply just to remove the person being served from the queue since they arent technically part of the waiting queue
					if (cq.size() - 1 < 0 || cq.empty()){
						t[i][j] = 0;
					}
					else {
						t[i][j] = cq.size()-1;
					}
					t[num_customers][j] += t[i][j];
					break;
				// Time service ends is the time it starts and the time itself
				case 7:
					t[i][j] = t[i][4] + t[i][3];
					t[num_customers][j] += t[i][j];
					break;
				// Time customer spends in the system is time waited in queue plus the service time
				case 8:
					t[i][j] = waiting_time + t[i][3];	
					t[num_customers][j] += t[i][j];
					break;
				// Server idle time is the difference between the service begin for the current customer and end for the prev customer
				case 9:
					if(i == 0) {
						t[i][j] = t[i][2];
					}
					else {
						t[i][j] = t[i][4] - t[i-1][7];
					}
					t[num_customers][j] += t[i][j];
					break;
					
			}
		}
	}

	return t;
}

void print_sim_table (vector<vector<int>> table) {
	cout << "Cust\tIArr\tArr\tServ\tServBeg\tWaiting\tCIQueue\tServEnd\tTISys\tServIdle\n";
	int size = table.size();
	for (int i = 0; i < size-1; i++) {
		for (int j = 0; j < 10; j++) {
			cout << table[i][j] << "\t";
		}
		cout << endl;
	}
	// print the sum
	cout << "Sum\t";
	for(int j = 1; j < 10; j++) {
		cout << table[size-1][j] << "\t";
	}
	cout << endl;
	// print the avg
	cout << "Avg\t";
	for(int j = 1; j < 10; j++) {
		cout << (float) table[size-1][j]/(size-1) << "\t";
	}
	cout << endl;
	float sumrow = size-1;
	cout << "Average time in queue: " << table[sumrow][5]/sumrow << endl;
	cout << "Average time in system: " << table[sumrow][8]/sumrow << endl;
	cout << "Average queue length: " << table[sumrow][6]/sumrow << endl;
	cout << "Server utilization: " << (1-(float)table[sumrow][9]/(float)table[sumrow-1][7])*100 <<"%" << endl;
	cout << "Average waiting time: " << table[sumrow][5]/sumrow << endl;
	cout << "Probability of waitiing: " << ((float)table[sumrow][5]/(float)table[sumrow-1][7])*100 <<"%" << endl;
	cout << "Probability of server being idle: " << ((float)table[sumrow][9]/(float)table[sumrow-1][7])*100 <<"%" << endl;
	cout << "Average service time: " << table[sumrow][3]/sumrow << endl;
	cout << "Average time between arrivals: " << table[sumrow][1]/sumrow << endl;
	cout << "Average waiting time for those who wait: " << table[sumrow][5]/sumrow << endl;
	cout << "Average time in system for the customer: " << table[sumrow][8]/sumrow << endl;
}

int main () {
	vector<float> prob1 {0.1, 0.2, 0.3, 0.2, 0.1, 0.1};
	const vector<pair<float, int>> TBA = create_pairs_vec(0, prob1);

	vector<float> prob2 {0.15, 0.25, 0.3, 0.2, 0.1};
	const vector<pair<float, int>> COT = create_pairs_vec(2, prob2);

	
	// The exact trial
	vector<int> exact_arr = generate_exact_inc_arr(50);
	vector<pair<int, int>> exact_sim_arr = generate_sim_arr(exact_arr, TBA, COT);	
	vector<vector<int>> exact_t = generate_sim_table(exact_sim_arr);

	// The rand trial
	vector<int> rand_arr = generate_rand_arr(50, 1000);
	vector<pair<int, int>> rand_sim_arr = generate_sim_arr(rand_arr, TBA, COT);
	vector<vector<int>> rand_t = generate_sim_table(rand_sim_arr);
	

	// Test array with the solved section example
	vector<pair<int, int>> test {
		make_pair(0,3),
		make_pair(4,4),
		make_pair(2,2),
		make_pair(3,3),
		make_pair(2,4),
		make_pair(3,5),
		make_pair(3,2),
		make_pair(4,2),
		make_pair(2,3),
		make_pair(1,4)
	};	
	vector<vector<int>> test_t = generate_sim_table(test);

	//print_sim_table(test_t);
	//print_sim_table(exact_t);
	print_sim_table(rand_t);	
}
