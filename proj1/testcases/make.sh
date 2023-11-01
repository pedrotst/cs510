set -x 
BASE_PATH=$HOME/cs510/
cd $BASE_PATH/valgrind-3.18.1
make || exit 1
make install 
