set -x 
BASE_PATH=$HOME/cs510/
cd $BASE_PATH/valgrind-3.18.1
make || exit 1
make install 

cd $BASE_PATH/testcases/
exe="test2"
file="${exe}.c"
out="${exe}.out"
res="${exe}.res"
gcc -o $exe -g -O0 $file
$BASE_PATH/val-tools/bin/valgrind --tool=depAnalysis --trace-file=$out $exe