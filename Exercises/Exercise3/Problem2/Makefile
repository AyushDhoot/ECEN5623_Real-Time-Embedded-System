all: 	
	gcc -pthread localvaronly.c -o localvaronly -lrt
	gcc -pthread threadindexed.c -o threadindexed -lrt
	gcc -pthread withmutex.c -o withmutex -lrt
	gcc -pthread accelero.c -o accelero -lrt -lm


clean:
	rm -f *.o
	rm -f localvaronly threadindexed withmutex accelero

