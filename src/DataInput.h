#include<iostream>
#include<algorithm>
#include<cmath>
#include<fstream>
#include<string>
#include<sstream>
#include<vector>
#include<map>
#include<time.h>
#include<random>
#include<unordered_map>
#include<bitset>
#include<set>
#include<chrono>

using namespace std;

//parameter
int input_id=1;//0:netflix, 1:amazon_M, 2:amazon_K, 3:MovieLens
const int K=3;//table number
const int m=6;//lengh of hash vector

const double epsilon = 0.01;
int t=100;//rating threshold,100,150,200
int k=10;//display size,5,10,15,20
int c_num=3;//category number,2,3,4


struct Item{
    int id;
    int category;
    vector<double> vec;
    vector<double> vec_tran;
    double ip =0;
    double cosine =0;
    double norm =0;
    int flag_ip =0;
    int flag_cos =0;
    int flag_insert =0;
};


bool norm_comp_dec(const Item& a, const Item& b){
    return a.norm > b.norm;
}

struct Bucket{
    bitset<m> hash_vec;
    vector<int> index;//index in bucket
};

struct Category_Data{
    int category;
    vector<Item> items;
    vector<unordered_map<bitset<m>, Bucket> > hash_tables;
};


struct User{
    int id;
    vector<double> vec;
    vector<double> vec_tran;
    double norm;
};


vector<Item> items;
vector<User> users;
vector<Category_Data> c_items;



//input data
vector<string> split(string& input, char delimiter)
{
    istringstream stream(input);
    string field;
    vector<string> result;
    while (getline(stream, field, delimiter)) {
        result.push_back(field);
    }
    return result;
}

double norm_com(vector<double> &vec){
    double tmp =0;
    for(int i=0; i< vec.size(); i++){
        tmp += vec[i]*vec[i];
    }
    return sqrt(tmp);
}

void input_data(){
    string input_file;
    if(input_id ==0){
        input_file = "../dataset/netflix200_category25.txt";
    }else if(input_id ==1){
        input_file = "../dataset/amazon_M200_category25.txt";
    }else if(input_id ==2){
        input_file = "../dataset/amazon_K200__category25.txt";
    }else if(input_id == 3){
        input_file = "../dataset/MovieLens200_category.txt";
    }

    ifstream ifs(input_file);
    if(!ifs){
        cout<<"Error! File can not be opened"<<endl;
    }

    string line;
    set<int> category_set;
    while (getline(ifs, line)){
        vector<string> strvec = split(line,' ');
        if(strvec[1]=="F"){
            continue;
        }
        string tmp_str = strvec[0];
        if(tmp_str[0] == 'p'){//user
            User tmp_user;
            tmp_user.id = stoi(tmp_str.substr(1));

            strvec.erase(strvec.begin());//erase meta information
            strvec.erase(strvec.begin());

            for(int i=0;i<strvec.size();i++){
                double d= stod(strvec.at(i));
                tmp_user.vec.push_back(d);
            }
            users.push_back(tmp_user);
        }


        if(tmp_str[0] == 'q'){//item
            Item tmp_item;
            tmp_item.id = stoi(tmp_str.substr(1));
            tmp_item.category = stoi(strvec[2]);
            category_set.insert(stoi(strvec[2]));

            strvec.erase(strvec.begin());//erase meta information
            strvec.erase(strvec.begin());
            strvec.erase(strvec.begin());

            for(int i=0;i<strvec.size();i++){
                double d= stod(strvec.at(i));
                tmp_item.vec.push_back(d);
            }
            items.push_back(tmp_item);
            
        }
    }

    cout << "item " << items.size() << "user " << users.size() << endl;
    
    for(int i=0; i < category_set.size(); i++){
        Category_Data tmp;
        tmp.category = i;
        c_items.push_back(tmp);
    }
    for(int i=0; i < items.size(); i++){
        int tmp_category = items[i].category;
        if(c_items[tmp_category].category != items[i].category){
            cout<< "error!" <<endl; 
            exit(0);
        }
        c_items[tmp_category].items.push_back(items[i]);
    }
}


