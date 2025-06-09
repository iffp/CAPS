#include <iostream>
#include <fstream>
#include <thread>
#include "FilterIndex.h"

#include <atomic>
#include <omp.h>
#include "fanns_survey_helpers.cpp"
#include "global_thread_counter.h"


// Global atomic to store peak thread count
std::atomic<int> peak_threads(1);

int main(int argc, char** argv)
{   
    // Get number of WH threads and use that number of threads for the index construction
    unsigned int nthreads = std::thread::hardware_concurrency();
    omp_set_num_threads(nthreads);

    // Prepare thread monitoring
    std::atomic<bool> done(false);
    std::thread monitor(monitor_thread_count, std::ref(done));

    // Parameters
    std::string path_database_vectors;
    std::string path_database_attributes;
    std::string path_index;
	size_t n_clusters;
	string metric;
    int mode;
    string algo;

    // Parse arguments
    if (argc != 8) {
        fprintf(stderr, "Usage: %s <path_database_vectors> <path_database_attributes> <path_index> <n_clusters> <metric> <mode> <algo>\n", argv[0]);
        exit(1);
    }

    // Store parameters
    path_database_vectors = argv[1];
    path_database_attributes = argv[2];
    path_index = argv[3];
	n_clusters = atoi(argv[4]);
	metric = argv[5];
	mode = atoi(argv[6]);
	algo = argv[7];

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
	auto start_time = chrono::high_resolution_clock::now();
	FilterIndex caps_index(database_vectors, d, n_items, n_clusters, database_attributes_str, algo, mode);
	auto mid_time = chrono::high_resolution_clock::now();
	caps_index.get_index(metric, path_index, mode);
	auto end_time = chrono::high_resolution_clock::now();

    // Stop thread monitoring
    done = true;
    monitor.join();

    // Print statistics
    std::chrono::duration<double> diff1 = end_time - start_time;
    double duration1 = diff2.count();
	std::chrono::duration<double> diff2 = mid_time - start_time;
	double duration2 = diff1.count();
	printf("Maximum number of threads: %d\n", peak_threads.load()-1);   // Subtract 1 because of the monitoring thread
    printf("Index construction time 1: %.3f s\n", duration1);
    printf("Index construction time 2: %.3f s\n", duration2);
    peak_memory_footprint();
    return 0;
}
