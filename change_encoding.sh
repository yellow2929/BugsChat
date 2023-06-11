#!/bin/bash

file_path=$1

if [ -f "$file_path" ]; then
    if [[ $file_path == *.txt ]]; then
        iconv -f UTF-8 -t GB2312 "$file_path" -o "$file_path"
        echo "the file is changed to GB2312"
    else
        echo "not txt file"
    fi
else
    echo "can not find the file"
fi

