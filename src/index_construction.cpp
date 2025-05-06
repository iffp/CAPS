#include <iostream>
#include <fstream>
#include "FilterIndex.h"

#include <thread>
#include "fanns_survey_helpers.cpp"

int main(int argc, char** argv)
{   
    // Get number of threads
    unsigned int nthreads = std::thread::hardware_concurrency();
    std::cout << "Number of threads: " << nthreads << std::endl;

    // Parameters
    std::string path_database_vectors;
    std::string path_database_attributes;
    std::string path_index;
    int M;
    int MaxM0;
    int efConstruction;
	size_t n_clusters;
	string metric;
    int mode;
    string algo;


    // Parse arguments
    if (argc != 11) {
        fprintf(stderr, "Usage: %s <path_database_vectors> <path_database_attributes> <path_index> <M> <MaxM0> <efConstruction> <n_clusters> <metric> <mode> <algo>\n", argv[0]);
        exit(1);
    }

    // Store parameters
    path_database_vectors = argv[1];
    path_database_attributes = argv[2];
    path_index = argv[3];
    M = atoi(argv[4]);
    MaxM0 = atoi(argv[5]);
    efConstruction = atoi(argv[6]);
	n_clusters = atoi(argv[7]);
	metric = argv[8];
	mode = atoi(argv[9]);
	algo = argv[10];

	// Load database vectors
	size_t d, n_items;
    float* database_vectors = fvecs_read(path_database_vectors.c_str(), &d, &n_items);

	// Load database attributes
    vector<int> database_attributes = read_one_int_per_line(path_database_attributes);
    assert(database_attributes.size() == n_items);

    // Transform database attributes into format required by CAPS
    std::vector<std::vector<std::string>> database_attributes_str;
    for (std::size_t i = 0; i < database_attributes.size(); ++i) {
        database_attributes_str.push_back({std::to_string(database_attributes[i])});
    }

	// Construct the index (timed)
	// TODO: Should the 2nd line here also be timed? Does this construct the index or only store it to disk?
	auto start_time = chrono::high_resolution_clock::now();
	FilterIndex caps_index(database_vectors, d, n_items, n_clusters, database_attributes_str, algo, mode);
	caps_index.get_index(metric, path_index, mode);
	auto end_time = chrono::high_resolution_clock::now();

    // Compute duration
    std::chrono::duration<double> diff = end_time - start_time;
    double duration = diff.count();

    // Report statistics
    printf("Index construction time: %.3f s\n", duration);
    peak_memory_footprint();
    return 0;
}
