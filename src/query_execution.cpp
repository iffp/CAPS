#include <iostream>
#include <fstream>
#include "FilterIndex.h"

#include <threads>
#include "fanns_survey_helpers.cpp"


int main(int argc, char** argv)
{

	// Get number of threads
    unsigned int nthreads = std::thread::hardware_concurrency();
    std::cout << "Number of threads: " << nthreads << std::endl;

    // Parameters
    std::string path_query_vectors;
    std::string path_query_attributes;
    std::string path_groundtruth;
    std::string path_index;
    string metric;
    int mode;
    string algo;
    size_t n_clusters;
    size_t n_probe;
	int k;

    // Check if the number of arguments is correct
    if (argc != 11)
    {
        fprintf(stderr, "Usage: %s <path_query_vectors> <path_query_attributes> <path_groundtruth> <path_index> <metric> <mode> <algo> <n_clusters> <n_probe> <k>\n", argv[0]);
        exit(1);
    }

    // Read command line arguments
    path_query_vectors = argv[1];
    path_query_attributes = argv[2];
    path_groundtruth = argv[3];
    path_index = argv[4];
	metric = argv[5];
	mode = atoi(argv[6]);
	algo = argv[7];
	n_clusters = atoi(argv[8]);
	n_probe = atoi(argv[9]);
	k = atoi(argv[10]);

    // Read query vectors
	size_t n_queries, d;
	float* query_vectors = fvecs_read(path_query_vectors.c_str(), &d, &n_queries);

    // Read query attributes
    vector<int> query_attributes = read_one_int_per_line(path_query_attributes);
    assert(query_attributes.size() == n_queries);

    // Transform query attributes into format required by NHQ
    std::vector<std::vector<std::string>> query_attributes_str;
    for (std::size_t i = 0; i < query_attributes.size(); ++i) {
        query_attributes_str.push_back({std::to_string(query_attributes[i])});
    }

    // Read groundtruth
    vector<vector<int>> groundtruth = read_ivecs(path_groundtruth);
    assert(groundtruth.size() == n_queries);

    // Truncate ground-truth to at most k items
    for (std::vector<int>& vec : groundtruth) {
        if (vec.size() > k) {
            vec.resize(k);
        }
    }

	// Load the CAPS index
	// TODO: We currently use empty database vectors since we load the pre-constructed index form file.
	// If this turns out to not work, we need to change it.
	float* empty_vectors = nullptr;
	std::vector<std::vector<std::string>> empty_attributes;
	FilterIndex caps_index(empty_vectors, d, 0, n_clusters, empty_attributes, algo, mode);
	caps_index.loadIndex(path_index);

	// Execute the queries (timed)
	auto start_time = std::chrono::high_resolution_clock::now();
	caps_index.query(query_vectors, n_queries, query_attributes_str, k, n_probe);
	auto end_time = std::chrono::high_resolution_clock::now();
	
    // Compute search time
    chrono::duration<double> time_diff = end_time - start_time;
    double query_execution_time = time_diff.count();

    // Compute recall
    int32_t* all_result = caps_index.neighbor_set;
    size_t match_count = 0;
    size_t total_count = 0;
    for (int i = 0; i < n_queries; i++){
        int n_valid_neighbors = min(k, (int)groundtruth[i].size());
        vector<int> groundtruth_q = groundtruth[i];
        vector<int> result_q;
        for (int j = 0; j < k; j++){
            result_q.push_back(all_result[i * k + j]);		// TODO: What happens if there are less than k neighbors?
        }
        sort(groundtruth_q.begin(), groundtruth_q.end());
        sort(result_q.begin(), result_q.end());
        vector<int> intersection;
        set_intersection(groundtruth_q.begin(), groundtruth_q.end(), result_q.begin(), result_q.end(), back_inserter(intersection));
        match_count += intersection.size();
        total_count += n_valid_neighbors;
    }

    // Report results   
    double recall = (double)match_count / total_count;
    double qps = n_queries / query_execution_time;
    peak_memory_footprint();
    printf("Queries per second: %.3f\n", qps);
    printf("Recall: %.3f\n", recall);

    return 0;
}

