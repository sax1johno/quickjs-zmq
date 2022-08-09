zmq: quickjs-zmq.c
	gcc quickjs-zmq.c ../quickjs/.obj/quickjs.o ../quickjs/.obj/quickjs-libc.o -g -o quickjs-zmq.so -ldl -lzmq -lz -shared -fPIC 
