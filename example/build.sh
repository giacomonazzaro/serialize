COMPILE="g++ -std=c++1z -O2"
MAIN=test

echo $COMPILE -o bin/$MAIN $MAIN.cpp ${@:2}
$COMPILE -o bin/$MAIN $MAIN.cpp ${@:2}

