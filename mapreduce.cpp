#include <pthread.h>
#include <stdio.h>
#include <bits/stdc++.h>
#include <fstream>

using namespace std;

pthread_barrier_t barrier;

//data for each mapper
struct map_struct {
    unordered_set<int> *partial_lists;
    int num_files;
    char **files_names;
    int *indices;
    int R;
};

//data for each reducer
struct reduce_struct {
    struct map_struct *results;
    long id;
    int M;
};

void *map_function(void *arg) {

    //get the data for the current mapper
    struct map_struct *data = (struct map_struct *)arg;
    int num_files = data->num_files;
    char **files_names = data->files_names;
    int *indices = data->indices;
    int R = data->R;

    //local_lists will store R partial lists
    unordered_set<int> *local_lists;
    local_lists = new unordered_set<int>[R];

    //for each file assigned to the current mapper
    for (int i = 0; i < num_files; i++) {
        if (indices[i] == 1) {
            int numbers_in_file;
            ifstream file(files_names[i]);
            file >> numbers_in_file;

            for (int j = 0; j < numbers_in_file; j++) {
                int current;
                file >> current;

                //add one to all lists
                if (current == 1) {
                    for (int power = 2; power <= R + 1; power++) {
                        local_lists[power - 2].insert(current);
                    }
                }

                //check if current is a perfect power 
                else if (current != 0) {
                    for (int power = 2; power <= R + 1; power++) {
                        int left = 1;
                        int right = current;
                        int mid;

                        while (left <= right){
                            mid = (left + right) / 2;

                            if (pow(mid, power) == current) {

                                //add it to the corresponding list
                                local_lists[power - 2].insert(current);
                                break;
                            }
                            else if (pow(mid, power) < current) {
                                left = mid + 1;
                            }
                            else {
                                right = mid - 1;
                            }
                        }
                    }
                }
            }
            
            file.close();
        }
    }

    //put resulted lists in struct
    data->partial_lists = local_lists;

    //wait for all mappers to finish
    pthread_barrier_wait(&barrier);
    pthread_exit(NULL);
}

void *reduce_function(void *arg) {

    //wait for all mappers to finish
    pthread_barrier_wait(&barrier);

    //get the data for the current reducer
    struct reduce_struct *data = (struct reduce_struct *)arg;
    struct map_struct *results = data->results;
    long id = data->id;
    int M = data->M;

    unordered_set<int> perfect_powers;
    int count = 0;
    char file_name[20];

    //combine M lists found on the id position
    for (int i = 0; i < M; i++) {
        unordered_set<int> l = results[i].partial_lists[id];
        perfect_powers.insert(l.begin(), l.end());
    }

    //count & write to file
    count = perfect_powers.size();
    sprintf(file_name, "out%ld.txt", id + 2);
    ofstream out_file(file_name);
    out_file << count;
    out_file.close();

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

    //get number of mappers and reducers from command line
    int M = atoi(argv[1]);
    int R = atoi(argv[2]);

    int r;
    long id;
    void *status;

    int num_threads = M + R;
    int num_files;
    int job_size[M] = {0};
    int j = 0;
   
    pthread_t threads[num_threads];
    pthread_barrier_init(&barrier, NULL, num_threads);

    // arrays of structs for map and reduce
    struct map_struct map_array[M];
    struct reduce_struct reduce_array[R];

    //read files names
    ifstream file(argv[3]);
    file >> num_files;

    char **files_names = (char **)malloc(num_files * sizeof(char *));
    int file_size[num_files];

    for (int i = 0; i < num_files; i++) {
        files_names[i] = (char *)malloc(20 * sizeof(char));
        file >> files_names[i];
    }

    file.close();

    //get files sizes
    for (int i = 0; i < num_files; i++) {
        ifstream current(files_names[i]);
        current.seekg(0, ios::end);
        file_size[i] = current.tellg();
        current.close();
    }

    //init map structs
    for (int i = 0; i < M; i++) {
        map_array[i].R = R;
        map_array[i].num_files = num_files;
        map_array[i].files_names = files_names;

        //asign one file to each mapper
        int *indices = (int *)calloc(num_files, sizeof(int));
        job_size[i] += file_size[j];
        indices[j] = 1;
        j++;

        map_array[i].indices = indices;
    }

    //distribute remaining files
    while (j < num_files) {
        int min = job_size[0];
        int min_index = 0;

        for (int i = 1; i < M; i++) {
            if (job_size[i] < min) {
                min = job_size[i];
                min_index = i;
            }
        }

        job_size[min_index] += file_size[j];
        map_array[min_index].indices[j] = 1;
        j++;
    }

    //init reduce structs
    for (int i = 0; i < R; i++) {
        reduce_array[i].id = i;
        reduce_array[i].M = M;
        reduce_array[i].results = map_array;
    }
    
    //create threads
    for (id = 0; id < num_threads; id++) {
        if (id < M) {
            r = pthread_create(&threads[id], NULL, map_function, &map_array[id]);
        }
        else {
            r = pthread_create(&threads[id], NULL, reduce_function, &reduce_array[id - M]);
        }

        if (r) {
            cout << "Error! Could not create thread " << id << "\n";
            exit(-1);
        }
    }

    //join threads
    for (id = 0; id < num_threads; id++) {
        r = pthread_join(threads[id], &status);

        if (r) {
            cout << "Error! Waiting for thread " << id << "...\n";
            exit(-1);
        }
    }

    if (argc < 3) {
        perror("Not enough arguments.\n");
        exit(-1);
    }

    //free memory
    for (int i = 0; i < M; i++) {
        free(map_array[i].indices);
    }

    for (int i = 0; i < num_files; i++) {
        free(files_names[i]);
    }

    free(files_names);

    pthread_barrier_destroy(&barrier);
    pthread_exit(NULL);
    
    return 0;
}
