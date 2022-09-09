#include "DataInput.h"
vector<vector<vector<double> > > random_vectors;

vector<double> trans_function(double norm_max, vector<double> &vec, double norm){
    vector<double> vec_trans;
    for(int i=0; i< vec.size(); i++){
        vec_trans.push_back( (double)vec[i]/norm_max );
    }
    vec_trans.push_back( sqrt(norm_max * norm_max - norm * norm) );
    return vec_trans;
}


void make_random(int dimentions){
    mt19937 mt(1);
    normal_distribution<> dist(0.0, 1.0);

    for(int i=0; i< K; i++){
        vector<vector<double> > tmp1;
        for(int j=0; j< m; j++){
            vector<double> tmp2;
            for(int l=0; l< dimentions; l++){
                tmp2.push_back(dist(mt));
            }
            tmp1.push_back(tmp2);
        }
        random_vectors.push_back(tmp1);
    }
}


bitset<m> make_hash_vector(vector<vector<double> > &a, vector<double> &v){
    bitset <m> hash_vector;
    for(int i=0; i<m; i++){
        double ip = 0;//内積
        for(int j=0; j<v.size() ;j++){
            ip = ip + a[i][j]*v[j];
        }
        if(ip >=0){
            hash_vector.set(i,1);
        }else{
            hash_vector.set(i,0);
        }
    }
    return hash_vector;
}

double Preprocess(){//return norm_max
    double norm_max = -100000;

    //compute norm
    for(int i=0; i < c_items.size(); i++){
        for(int j=0; j < c_items[i].items.size(); j++){
            double norm = norm_com( c_items[i].items[j].vec );
            c_items[i].items[j].norm = norm;
            if(norm_max < norm){
                norm_max = norm;
            }
        }
        sort(c_items[i].items.begin(), c_items[i].items.end(), norm_comp_dec);//for all check
    }



    //transform vectors
    for(int i=0; i< c_items.size(); i++){
        for(int j=0; j< c_items[i].items.size(); j++){
            c_items[i].items[j].vec_tran = trans_function(norm_max, c_items[i].items[j].vec, c_items[i].items[j].norm);
        }
    }

    //prepare hash functions
    make_random(c_items[0].items[0].vec_tran.size());

    for(int i=0; i< c_items.size(); i++){
        for(int j=0; j<K ;j++){
            unordered_map<bitset<m>, Bucket> tmp_hash_table;
            for(int l=0; l<c_items[i].items.size(); l++){
                bitset<m> hash_vec = make_hash_vector(random_vectors[j], c_items[i].items[l].vec_tran);
                auto itr = tmp_hash_table.find(hash_vec);
                if(itr != tmp_hash_table.end()){
                    itr->second.index.push_back(l);
                }else{
                    Bucket tmp_bucket;
                    tmp_bucket.hash_vec = hash_vec;
                    tmp_bucket.index.push_back(l);
                    tmp_hash_table[hash_vec] = tmp_bucket;
                }

            }
            c_items[i].hash_tables.push_back(tmp_hash_table);
        }
    }
    return norm_max;
}

void trans_function_user(int &query_id){
    users[query_id].norm = norm_com(users[query_id].vec);
    for(int i=0; i< users[query_id].vec.size();i++){
        users[query_id].vec_tran.push_back( (double)users[query_id].vec[i]/users[query_id].norm );
    }
    users[query_id].vec_tran.push_back(0);
}

void decide_each_c_num(vector<int> &each_c_num){
    for(int i=0; i< c_num; i++){
        each_c_num.push_back((k+i)/c_num);
    }
}

double ip_com(vector<double> &a, vector<double> &b){
    double ip=0;
    for(int i=0; i< a.size(); i++){
        ip += a[i] * b[i];
    }
    return ip;
}




void all_check(int &query_id, int &category, int &num, vector<int> &result_c, int &count){
    cout <<"all check" <<endl;
    map<double, int, greater<double>> topk;
    double threshold = -10000;//min
    for(int i=0; i<c_items[category].items.size(); i++){
        if(users[query_id].norm * c_items[category].items[i].norm < threshold){
            break;
        }

        double ip;
        if(c_items[category].items[i].flag_ip ==1){
            ip = c_items[category].items[i].ip;
        }else{
            ip = ip_com(users[query_id].vec, c_items[category].items[i].vec);
            c_items[category].items[i].ip = ip;
            c_items[category].items[i].flag_ip =1;
            count++;
        }

        if(ip > threshold){
            topk.insert({ip,i});
        }
        if(topk.size()>num){
            auto it = topk.end();
            --it;
            topk.erase(it);
        }
        if(topk.size() == num){
            auto it = topk.end();
            --it;
            threshold = it->first;
        }
    }

    result_c.clear();
    for(auto itr = topk.begin(); itr != topk.end(); ++itr){
        int tmp = itr->second;
        result_c.push_back( c_items[category].items[tmp].id );
    }
}



void retrieval(int &query_id, vector<bitset<m> > &query_hash_v, int &category, int &num, vector<int> &result_c, int &count, double &norm_max){
    vector<set<pair<int, Bucket *> > >  topBuckets;
    
    int threshold_num =(int)log10(items.size())/c_items.size();//top log
    if(threshold_num ==0) threshold_num++;

    int check_count =0;

    for(int i=0; i< K; i++){
        set<pair<int, Bucket * > > topBuckets_tmp;
        int threshold = 10000;

        for(auto itr = c_items[category].hash_tables[i].begin(); itr != c_items[category].hash_tables[i].end(); ++itr){
            int dist = (itr->first ^ query_hash_v[i]).count();//hamming distance

            //insert
            if(dist < threshold){
                topBuckets_tmp.insert({dist, &(itr->second)});//hamming dist and pointer of bucket
                check_count += itr->second.index.size();
            }

            if(topBuckets_tmp.size() > threshold_num){
                auto it = topBuckets_tmp.end();
                --it;
                check_count = check_count - it->second->index.size();
                topBuckets_tmp.erase(it);
            }

            if(topBuckets_tmp.size() == threshold_num){
                auto it = topBuckets_tmp.end();
                --it;
                threshold = it->first;
            }
        }
        topBuckets.push_back(topBuckets_tmp);
    }


    if(check_count < num){
        all_check(query_id, category, num, result_c, count);
    }else{
        
        std::mt19937 mt(1);
        double threshold =(1-epsilon)*1;
        int flag =0;
        int flag1 =1;
        double cos_max = -10000;

        
        for(int i=0; i< K; i++){//shuffle
            for(auto itr = topBuckets[i].begin(); itr != topBuckets[i].end(); ++itr){
                shuffle(itr->second->index.begin(), itr->second->index.end(), mt);
            }
        }
        


        while(flag != num && threshold > 0.5){
            if(flag1 ==1){

                //retrieval
                for(int i=0; i< K; i++){
                    for(auto itr = topBuckets[i].begin(); itr != topBuckets[i].end(); ++itr){
                        for(int j=0; j< itr->second->index.size(); j++){
                            int id = itr->second->index[j];
                            double cosine;

                            if(c_items[category].items[id].flag_insert ==1){
                                continue;
                            }

                            if(c_items[category].items[id].flag_cos ==1){
                                cosine = c_items[category].items[id].cosine;
                            }else{//
                                double ip = ip_com(users[query_id].vec, c_items[category].items[id].vec);
                                cosine = (double)ip/(users[query_id].norm * norm_max);
                                c_items[category].items[id].flag_ip =1;
                                c_items[category].items[id].flag_cos =1;
                                c_items[category].items[id].cosine = cosine;
                                c_items[category].items[id].ip = ip;
                                count++;
                            }

                            if(cosine > threshold){
                                result_c.push_back( c_items[category].items[id].id );
                                c_items[category].items[id].flag_insert =1;
                                flag++;
                            }else{
                                if(cos_max < cosine){
                                    cos_max = cosine;
                                }
                            }
                            
                            if(flag == num) break;
                        }
                        if(flag == num) break;
                    }
                    if(flag == num) break;
                }
                if(flag == num) break;
            }

            //update threshold
            threshold = threshold * (1-epsilon);
            if(threshold > cos_max){
                flag1 =0;
            }else{
                flag1 =1;
            }
            
        }
        if(flag != num && threshold <= 0.5){
            cout << "ouch!" << endl; 
            all_check(query_id, category, num, result_c, count);
        }
    }    
}


void Online_Phase(int &query_id, vector<int> &c_decieded_vec, vector<vector<int> > &result, int &count, double &norm_max){
    vector<int> each_c_num;
    decide_each_c_num(each_c_num);
    trans_function_user(query_id);


    vector<bitset<m> > query_hash_v;
    for(int i=0; i< K; i++){
        query_hash_v.push_back( make_hash_vector(random_vectors[i],users[query_id].vec_tran) );
    }


    for(int i=0; i< c_num; i++){// retrieval for each category
        vector<int> result_c;
        retrieval(query_id, query_hash_v, c_decieded_vec[i], each_c_num[i], result_c, count, norm_max);
        result.push_back(result_c);
    }
}

void init_data(vector<int> &c_decided_vec){
    for(int i=0; i< c_decided_vec.size(); i++){
        int category = c_decided_vec[i];
        for(int j=0; j< c_items[category].items.size(); j++){
            c_items[category].items[j].ip =0;
            c_items[category].items[j].flag_ip =0;
            c_items[category].items[j].cosine =0;
            c_items[category].items[j].flag_cos =0;
            c_items[category].items[j].flag_insert =0;
        }
    }
}

double compute_time(chrono::system_clock::time_point start, chrono::system_clock::time_point end){
    auto time = end - start;
    double tmp = std::chrono::duration_cast<chrono::microseconds>(time).count();
    double msec =(double)tmp/1000;
    return msec;
}

void Output_result(vector<vector<double> > &Record){
    string output_file_name;
    string setting ="_k" + to_string(k) + "_c" + to_string(c_num) + "_t" + to_string(t) + 
    "_eps" + to_string(epsilon) + "_K" + to_string(K) + "_m" + to_string(m) + ".csv";
    if(input_id ==0){
        output_file_name = "result/netflix/netflix" + setting;
    }else if(input_id ==1){
        output_file_name = "result/amazonM/amazon_M" + setting;
    }else if(input_id ==2){
        output_file_name = "result/amazonK/amazon_K" + setting;
    }else if(input_id == 3){
        output_file_name = "result/MovieLens/MovieLens" + setting;
    }

    ofstream file(output_file_name);
    for(int i=0; i< Record.size(); i++){
        for(int j=0; j< Record[i].size() -1; j++){
            file<< Record[i][j];
            file << ',';
        }
        file << Record[i][ Record[i].size()-1 ];
        file << '\n';
    }
    file.close();
}