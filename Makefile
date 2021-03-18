MAIN := main
SRC := main.c
SRC += ./src/*.c
INC := -I ./include/

${MAIN} : ${SRC}
	gcc $^ -o $@ ${INC} -lpthread
clean:
	rm ${MAIN}
