#include <iostream>
#include <set>
#include <vector>
#include <fstream>
#include <regex>
#include <algorithm>
#include <queue>

using namespace std;

bool isConnected(const vector<vector<set<int>>>& matrix) {
int n = matrix.size();
vector<bool>is_visited(n, false);
queue<int> q;

q.push(0);
while(!q.empty()) {
int cur_element = q.front();
q.pop();
is_visited[cur_element] = true;

for(int i = 0; i < n; ++i) {
if(is_visited[i]) {
continue;
}
if(!matrix[cur_element][i].empty() || !matrix[i][cur_element].empty()) {
q.push(i);
}
}
}
for(const auto& element: is_visited) {
if(!element) {
return false;
}
}
return true;
}

int main() {
    ifstream in("input.txt");
    ofstream out("output.txt");
    try {

        const regex regexes[5] = {
                regex(R"(states=\[((?:(?:\w+,)*\w+))\])"),
                regex(R"(alpha=\[((?:(?:[\w_]+,)*[\w_]+)?)\])"),
                regex(R"(initial=\[(\w*)\])"),
                regex(R"(accepting=\[((?:(?:\w+,)*\w+)?)\])"),
                regex(R"(trans=\[((?:(?:\w+\>[\w_]+\>\w+,)*\w+\>[\w_]+\>\w+)?)\])"),
        };
        const regex rr1(R"(\w+)");
        const regex rr2(R"([\w_]+)");

        string input_line;
        string unsplitted_data[5];
        for (int i = 0; i < 5; ++i) {
            in >> input_line;
//            getline(in, input_line);
//            if(i != 4) {
//                input_line = input_line.substr(0, input_line.size()-1);
//            }
            smatch m;
            if (!regex_match(input_line, m, regexes[i])) {
                throw runtime_error("0");
            }
            unsplitted_data[i] = m[1].str();
        }

        vector<string> states{
                sregex_token_iterator{cbegin(unsplitted_data[0]),
                                      cend(unsplitted_data[0]),
                                      rr1, 0},
                sregex_token_iterator{}
        };

        int n = states.size();

        vector<string> alphabet{
                sregex_token_iterator{cbegin(unsplitted_data[1]),
                                      cend(unsplitted_data[1]),
                                      rr2, 0},
                sregex_token_iterator{}
        };

        int initial_state = find(states.begin(), states.end(), unsplitted_data[2]) - states.begin();
        if (initial_state >= n) {
            if(unsplitted_data[2] == "") throw runtime_error("4");
            throw runtime_error("1"+unsplitted_data[2]);
        }
        vector<string> final_states_names{
                sregex_token_iterator{cbegin(unsplitted_data[3]),
                                      cend(unsplitted_data[3]),
                                      rr1, 0},
                sregex_token_iterator{}
        };
        vector<int> final_states;
        for (const auto &state: final_states_names) {
            auto pos = find(states.begin(), states.end(), state);
            if (pos == states.end()) {
                throw runtime_error("1" + state);
            }
            final_states.push_back(pos - states.begin());
        }

        vector<vector<set<int>>> translation_matrix(n, vector<set<int>>(n, set<int>()));
        vector<string> translates{
                sregex_token_iterator{cbegin(unsplitted_data[4]),
                                      cend(unsplitted_data[4]),
                                      rr2, 0},
                sregex_token_iterator{}
        };

        for (int i = 0; i < translates.size(); i += 3) {
            int state_from = find(states.begin(), states.end(), translates[i]) - states.begin();
            int translate = find(alphabet.begin(), alphabet.end(), translates[i + 1]) - alphabet.begin();
            int state_to = find(states.begin(), states.end(), translates[i + 2]) - states.begin();

            if (state_from >= n) {
                throw runtime_error("1" + translates[i]);
            }
            if (translate >= alphabet.size()) {
                throw runtime_error("3" + translates[i+1]);
            }
            if (state_to >= n) {
                throw runtime_error("1" + translates[i+2]);
            }
            for(int k = 0; k < n; ++k) {
                if(translation_matrix[state_from][k].find(translate) != translation_matrix[state_from][k].end()) {
                    throw runtime_error("5");
                }
            }
//            if (translation_matrix[state_from][state_to].find(translate) !=
//                translation_matrix[state_from][state_to].end()) {
//                throw runtime_error("5");
//            }
            translation_matrix[state_from][state_to].insert(translate);
        }

        if(!isConnected(translation_matrix)) {
            throw runtime_error("2");
        }

        int k = -1;
        vector<vector<string>> cur_R(n, vector<string>(n, "{}"));

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (!translation_matrix[i][j].empty()) {
                    cur_R[i][j] = alphabet[*translation_matrix[i][j].begin()];
                    for (auto it = ++translation_matrix[i][j].begin(); it != translation_matrix[i][j].end(); ++it) {
                        cur_R[i][j] += '|' + alphabet[*it];
                    }
                }
                if (i == j) {
                    if (cur_R[i][j] != "{}") {
                        cur_R[i][j] += "|eps";
                    } else {
                        cur_R[i][j] = "eps";
                    }
                }
            }
        }

        while (k != n - 1) {
            k++;
            auto prev_R = cur_R;
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    cur_R[i][j] =
                            '(' + prev_R[i][k] + ")(" + prev_R[k][k] + ")*(" + prev_R[k][j] + ")|(" + prev_R[i][j] +
                            ')';
                }
            }
        }

        string answer;

        for (const auto &final_state: final_states) {
            answer += '|'+cur_R[initial_state][final_state];
        }

        if (answer.empty()) {
            answer = "{}";
        }
        else {
            answer = answer.substr(1);
        }
        out << answer;
    }
    catch(const runtime_error& E) {
        auto error_type = string(E.what())[0];
        out << "Error:\n";
        if(error_type == '0') {
            out << "E0: Input file is malformed";
        }
        if(error_type == '1') {
            out << "E1: A state '" << string(E.what()).substr(1) <<"' is not in the set of states";
        }
        if(error_type == '2') {
            out << "E2: Some states are disjoint";
        }
        if(error_type == '3') {
            out << "E3: A transition '" << string(E.what()).substr(1) << "' is not represented in the alphabet";
        }
        if(error_type == '4') {
            out << "E4: Initial state is not defined";
        }
        if(error_type == '5') {
            out << "E5: FSA is nondeterministic";
        }
    }
    return 0;
}