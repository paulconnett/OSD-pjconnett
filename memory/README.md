# memory

This is a modified version of bench_mem that instead pulls data from a directory and finds the timing for accessing this data in a map (scattered data) and a vector (adjacent data).
This simulates a system that searches for a department based on phone extensions. This testing program proves that vectors are faster in finding the stored data, as vectors 
store their data right next to each other, while maps store their data in tree nodes that are usually spread around, thus increasing the amount of needed jumps to grab the data.
This shows that vectors are better for memory optimization for cases like using a directory. ChatGpt build the input file and recommended it be 30000 entires, as this is better for testing
timing and better reflects an organization at scale. 

## Run

./bench_mem -all

## Screenshots

Location: /memory/screenshots

# Contribution

bench_mem.cpp:    
    Replaced generated data with an input file that simulates an organization's department directory,
    Added a file input logic to store data from departments.txt in the program,
    Removed any testing not associated with vec and map,
    Created DepartmentEntry struct which stores department name and extension (from 1 - 30000),
    Added choose_extension and load_departments functions to allow user to interact with program,
    Added presentation of department depending on extension to summary output,
    Used lower_bound() for vector extension searching.

## References

1. [ChatGPT] Created input file and helped build the Dataset struct. Additonally, it recommended and guided me through using lower_bound() vector lookup.
2. [Dr. MacEvoy's bench_mem](https://github.com/wmacevoy/os-wmacevoy-spring-2026/blob/main/memory/bench_mem.cpp)