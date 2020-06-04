# roblox_sample
sample client/server for roblox

# to build

cd to main folder

make

this will make rbclient and rbserver, both in their own folders.

running either with no params will show options available.

sample input files exist in each folder

server:

cd server && rbserver -p 15005 config.txt

client:

cd client && rbclient -d 100 -r 0 -h 127.0.0.1 -p 15005 input01.txt
