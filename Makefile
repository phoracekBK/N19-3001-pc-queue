RELEASE_PATH=Release
OUTPUT=pc_queue.exe
OPT=-Wall -O2 $$(pkg-config --libs --cflags aclib) $$(pkg-config --libs --cflags s7lib)
SRC=src/pc-queue


all:
	clear
	cc ${SRC}/pc_queue.c ${SRC}/controler.c ${SRC}/model.c ${SRC}/glass_info.c ${SRC}/visu.c ${OPT} -o ${RELEASE_PATH}/${OUTPUT}

run:
	clear
	cc ${SRC}/pc_queue.c ${SRC}/controler.c ${SRC}/model.c ${SRC}/glass_info.c ${SRC}/visu.c ${OPT} -o ${RELEASE_PATH}/${OUTPUT}
	${RELEASE_PATH}/${OUTPUT}

clean:
	rm -vf *.o
	rm -vf *.a
	rm -vf ${RELEASE_PATH}/${OUTPUT}
