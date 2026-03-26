# Full Stack

This is a parallel python program where each thread increments a counter. The program has a lock to prevent race conditions

## Run

python3 parallel.py

## Screenshots

Location: /parallel/screenshots

# Contribution

parallel.py:    
    Imported threads for thread creation,
    Created a lock so threads avoid race conditions,
    Created a counter for threads to increment,
    Created a worker function that increments 5 times, while using a lock to prevent race conditions. The function then prints output of thread incrementation to termial each loop,
    Created 2 threads, t1 and t2,
    Used t#.start to start threads, and t#.join to force program to wait until threads are done working,
    Printed final output of counter,

## References

1. [Thread-based parallelism](https://docs.python.org/3/library/threading.html)