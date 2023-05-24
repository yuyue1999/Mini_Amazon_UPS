protoc --cpp_out=./protobuf ./protobuf/amazon_ups.proto
protoc --cpp_out=./protobuf ./protobuf/world_amazon.proto

make -B

sleep 3

chmod ug+w ./server

./server

while true; do sleep 1; done
