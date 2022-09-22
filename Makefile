zmq: quickjs-zmq.c
	gcc -Wall -g quickjs-zmq.c ../quickjs/.obj/quickjs.o ../quickjs/.obj/quickjs-libc.o -ldl -lzmq -lczmq -lz -shared -fPIC -o quickjs-zmq.so
