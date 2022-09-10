# Categorical-Diversity-Aware-k-IPS
## Introduction
* This repository provides an implementation for Categorical-Diversity-Aware k-IPS.


## Setting
* Linux OS (Ubuntu)
* g++ 5.5.0


## How to use
* Dataset
    * Create vectors from rating data by [MF](https://www.csie.ntu.edu.tw/~cjlin/libmf/) .
    * Add a category information to items. Category information is the third column of q (i.e. item). And, place your datasets at dataset directory.
        * Ex.) An example dataset amazon_M200_category25.txt is at the directory.
        * Ex.) In amazon_M200_category25.txt, q25 (item) belongs to category 0.   

* Algorithm (in `src` directory)
    * Complile: `g++ -O3  -o main.out main.cpp -std=c++11`
    * Input parameters: command line arguments.
        * `input_id, rank threshold, output size, the number of categories specified`
    * Run: `./main.out command line argument`
        * In setting input_id is 1, rank threshold is 100, out put size is 10, and the number of categories specified is 3, you run `./main.out 1 100 10 3`.

* Result output (in a csv file)
    * each column shows: queryID, the number of computing cosine similarity, time(ms), itemIDs in the result
