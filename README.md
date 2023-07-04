Title: APD Project 1 - Multithreaded MapReduce File Processing in C++

Description:
This repository contains the implementation of a multithreaded MapReduce file processing system in C++. The project aims to implement a parallel program using Pthreads to find numbers greater than 0 that are perfect powers from a set of input files. It is implemented using the Map-Reduce model for the efficient processing of the input documents, distributing them evenly among multiple threads. The program will parse and verify the numbers in parallel, generating partial lists for each exponent. These lists will then be combined to obtain aggregated lists for each exponent. Finally, the program will count the unique values in parallel and write the results to respective output files.

The project aims to efficiently process and analyze large datasets using multiple threads. The threads are created based on the number of mappers (M) and reducers (R), and each thread performs specific functions. The mappers handle file processing and map data to partial lists, while the reducers perform data aggregation and produce the final output.

- Thread Creation: Threads are created using a for loop iterating up to the total number of threads (M + R). Map threads call the map_function, while reduce threads are created using the reduce_function.
- File Partitioning: File names are read and added to a word vector. The size of each file is calculated and stored in the file_size vector. Each map structure in the map_array initially stores the total number of files, the file names vector, and the number of reducers. A frequency vector called "indices" is used by mappers to determine the files they should process.
- Map Function: Each map thread iterates through the file names. If the frequency vector "indices" is set to 1 at that position, the file is assigned to the thread for processing. For each file, the numbers are read, and if a number is 1, it is added to all partial lists. Otherwise, if the number is not 0, a binary search algorithm checks if it is a perfect power from 2 to R+1. The numbers are added to the appropriate partial lists based on their powers. The partial lists are then stored in the map structure.
- Reduce Function: Each reduce thread receives a reduce structure from the reduce_array, which includes an ID, the number of mappers, and a reference to the mapper's processed structures. To synchronize the reducers with the mappers, a barrier of size M + R is created at the beginning of the reduce function. This barrier ensures that the reducers wait until all the mappers have finished their job before proceeding with the combination and processing steps. Each reducer creates an unordered set named "perfect_powers" and adds the corresponding lists of exponents from the M processed structures received from the mappers. When finished, the reducer counts the elements in "perfect_powers" and writes the result to the appropriate output file based on the power (ID + 2).
