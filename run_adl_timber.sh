# if this file is placed within the right directory, it will automatically parse ADL and run TIMBER on it
set -e
start_time=$SECONDS

echo "Starting ADL -> TIMBER analyzer."
echo ""

echo "Making ADL Parser..."
make
echo ""

echo "Compiling to TIMBER..."
./adlparser $1 timber > $1.py

echo "Compiled to TIMBER."
echo ""

echo "Executing Python..."
python3 $1.py
echo ""

echo "Done."

duration=$(( SECONDS - start_time ))
echo "Elapsed time: $duration seconds."
