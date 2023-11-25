# LSV-FINAL

## How to play with demo.cpp
1. Download the static library by running download_lib.py
```
python3 dowmload_lib.py
```
2. Link demo.cpp with the static library
```
g++ demo.cpp lib/libgenaag.a lib/libabc.a -lm -ldl -lreadline -lpthread -o demo
```
3. Usage of demo executable
```
./demo <filepath to the .v file>
```
