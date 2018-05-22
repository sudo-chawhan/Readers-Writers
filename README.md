# Readers-Writers
Implementing Readers-Writers problem and Fair Readers-Writers problem using Semaphores in C++.

## For Linux only

* Clone the repository, using

 ``` 
 $ git clone https://github.com/sudo-chawhan/Readers-Writers.git
 ```
 
* In terminal, change the current directory to cloned repo using

```
$ cd Readers-Writers
```

* Edit input.txt to the desired inputs
 
* in the terminal, compile the .cpp files using

```
$ g++ -std=c++11 -o rw -pthread Assgn3-RW-CS16BTECH11037.cpp -lrt
$ g++ -std=c++11 -o fairrw -pthread Assgn3-RW_Fair-CS16BTECH11037.cpp -lrt

```
* now run

```
$ ./rw
$ ./fairrw
```

* You will find that two files are generated ,i.e , RW-log.txt(contains the log of threads) and Average_time.txt which contains the average time of all threads. 
