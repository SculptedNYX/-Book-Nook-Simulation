#include <algorithm>
#include <utility>
#include <vector>
#include <cmath>

using namespace std;

// Inits a vector of pairs where the cumulative prob is the first value and the minute is the second value
vector<pair<float, int>> create_pairs_vec (int start, int end, float prob[]) {
	vector<pair <float, int>> m;
	float sum = 0;

	for (int i = start; i <= end; i++) {
		sum += prob[i];
		m.emplace_back(make_pair(sum, i));
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
			}
		}

		sim_arr.emplace_back(tmp);
	}

	return sim_arr;
}

// Creates an array of n exact values from 1->n
vector<int> exact_inc_arr (int n) {
	vector<int> a;
	for (int i = 0; i < n; i++) {
		a.emplace_back(i+1);
	}

	return a;
}

int main () {
	float prob1[] = {0.1, 0.2, 0.3, 0.2, 0.1, 0.1};
	const vector<pair<float, int>> TBA = create_pairs_vec(0, 5, prob1);

	float prob2[] = {0.15, 0.25, 0.3, 0.2, 0.1};
	const vector<pair<float, int>> COT = create_pairs_vec(2, 6, prob2);
	
	// The exact trial
	vector<int> exact_arr = exact_inc_arr(50);
	vector<pair<int, int>> sim_arr = generate_sim_arr(exact_arr, TBA, COT);

}
