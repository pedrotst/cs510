BASE_PATH=$HOME/cs510
cd $BASE_PATH/valgrind-3.18.1
./autogen.sh
./configure --prefix=$BASE_PATH/val-tools
make || exit 1
make install 

cd $BASE_PATH/testcases/
for file in `ls`
do
	if [[ $file == *.c ]]
	then
		exe="${file%.*}"
		out="${exe}.out"
		res="${exe}.res"
		gcc -o $exe -g -O0 $file
		$BASE_PATH/val-tools/bin/valgrind --tool=depAnalysis --trace-file=$out $exe &> /dev/null
		# rm -rf $exe
	fi
done
