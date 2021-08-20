#!/bin/sh

if uname -m | egrep -q 'amd64|x86_64'; then
    espack=./tool/espack64
elif uname -m | egrep -q 'i[3-6]86'; then
    espack=./tool/espack32
else
    echo Unsupported machine platform: $(uname -m)
    exit 1
fi

chmod +x $espack

$espack -r ESP
