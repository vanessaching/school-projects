CONTRIBUTIONS
Sam Sayyad: Task 1 & Task 2 
Vanessa Ching: Task 3 & Task 4

REPORT
1. Amount of time the parsort program took to sort the test data for each threshold value:  

Test run with threshold 2097152
real    0m0.386s
user    0m0.371s
sys     0m0.010s

Test run with threshold 1048576
real    0m0.225s
user    0m0.401s
sys     0m0.016s

Test run with threshold 524288
real    0m0.159s
user    0m0.429s
sys     0m0.032s

Test run with threshold 262144
real    0m0.135s
user    0m0.448s
sys     0m0.052s

Test run with threshold 131072
real    0m0.117s
user    0m0.440s
sys     0m0.060s

Test run with threshold 65536
real    0m0.113s
user    0m0.448s
sys     0m0.093s

Test run with threshold 32768
real    0m0.122s
user    0m0.492s
sys     0m0.102s

Test run with threshold 16384
real    0m0.130s
user    0m0.540s
sys     0m0.148s

2. Explanation for the times 
As the threshold decreased from 2,097,152 to 65,536, the total sorting time decreased. 
This happened because lowering the threshold allows the program to split the input data into smaller chunks earlier in the computation. 
Each of these smaller chunks can then be sorted in parallel by different processes, which the OS schedules across multiple CPU cores. 
When more chunks are being processed at the same time, the total sorting finishes faster. 
When the threshold is too large, the program delays splitting the data, so only a few large chunks are created. 
This means that fewer processes are active in parallel and most of the sorting happens sequentially within each process. 
The benefit of parallelism is reduced because of this. 
On the other hand, if the threshold becomes too small, the program ends up creating too many parallel processes. 
Each small task requires the OS to create, manage, and merge processes. 
The time required for this outweighs the benefit from the time saved from running parallel processes. 
Because of this, there is a middle ground for the threshold where the work is divided efficiently without excessive parallel processes. 
In the experiment, the optimal range was between 65,636 and 131,072. 