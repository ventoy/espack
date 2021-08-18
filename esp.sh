#!/bin/sh

if uname -m | egrep -q 'amd64|x86_64'; then
    espack=./tool/espack64
else
    espack=./tool/espack32
fi

chmod +x $espack

$espack -r ESP
