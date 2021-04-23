# ipc-sat-solving
Solves multiple propositional formulas in CNF in a distributed way.

## Compile
```
$ make
```
## Clean
```
$ make clean
```
## Requirements
```
$ apt-get install minisat
```
## Usage
```
$ ./solve "path_to_cnf_files"
```
This will generate a results.txt file with the results of minisat over all the given files. Alternatively you can use the view process to see the output from the terminal:
```
$ ./solve "path_to_cnf_files" | ./view
```
or
```
$ ./solve "path_to_cnf_files" &
$ ./view
```
You can also run both processes in separate terminals so you can check from which process the error (if there is one) is coming from.
