#include "Propose_functions.h"
int main(int argc, char *argv[]){
    if (argc < 2) {
    cerr << "No program parameters provided !" << endl;
    exit(EXIT_FAILURE);
    }
    input_id = stoi(argv[1]);
    t = stoi(argv[2]);
    k = stoi(argv[3]);
    c_num = stoi(argv[4]);
    cout << "input_id：" << input_id << " top：" << t << " display_size：" << k
    << " c_num：" << c_num <<endl;

    input_data();
    double norm_max = Preprocess();
    cout <<"preprocess done" << endl;

    mt19937 mt1(1);
    mt19937 mt2(1);
    uniform_int_distribution<> decide_category(0, c_items.size()-1);
    uniform_int_distribution<> decide_user(0, users.size()-1);

    vector<vector<double> > Record;
    for(int i=0; i<100; i++){
        vector<double> record;
        set<int> tmp_set;
        vector<int> c_decieded_vec;
        while(tmp_set.size() != c_num){
            tmp_set.insert(decide_category(mt1));
        }
        for(auto itr=tmp_set.begin(); itr != tmp_set.end(); ++itr){
            c_decieded_vec.push_back(*itr);
        }

        int query_id = decide_user(mt2);

        vector<vector<int> > result;
        int count =0;
        chrono::system_clock::time_point  start, end;
        start = chrono::system_clock::now();
        Online_Phase(query_id, c_decieded_vec, result, count, norm_max);
        end= chrono::system_clock::now();

        double msec = compute_time(start, end);

        record.push_back(query_id);
        record.push_back(count);
        record.push_back(msec);
        for(int j=0; j<result.size(); j++){
            for(int l=0; l<result[j].size(); l++){
                record.push_back(result[j][l]);
            }
        }
        Record.push_back(record);

        init_data(c_decieded_vec);
    }

    Output_result(Record);

    cout << "finish" << endl;
 
    return 0;
}