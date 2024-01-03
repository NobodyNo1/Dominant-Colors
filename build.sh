clang -framework Cocoa -framework IOKit -Lexternal/lib -Iexternal/include -lraylib main.c -o build/main

if [ $? -ne 0 ]; then
    echo "Build Failed"
    exit 1
fi

./build/main
