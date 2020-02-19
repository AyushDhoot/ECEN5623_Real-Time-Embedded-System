all: 	
	gcc -pthread native_posix.c -o native_posix -lrt
	gcc -pthread native_heap.c -o native_heap -lrt

clean:
	rm -f *.o
	rm -f native_posix native_heap
