MAIN := main
SRC := main.c
SRC += ./src/*.c
INC := -I ./include/

${MAIN} : ${SRC}
	gcc $^ -o $@ ${INC} -lpthread

share : ${SRC}
	gcc $^ -o libmblk.so -fPIC -shared ${INC} -lpthread

clean:
	rm ${MAIN}
