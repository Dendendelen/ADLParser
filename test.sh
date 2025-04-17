
# if this file is placed within the right directory, it will automatically parse ADL and run TIMBER on it
make
./main $1 > $1.py
python3 $1.py
