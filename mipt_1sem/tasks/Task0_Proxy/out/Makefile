# Change for any new project
JOB_NAME = Proxy_Job
JOB_SH = job.sh
EXEC = proxy
SRC = ../source/proxy.c

# Compilation flags
CFLAGS = -std=c99

# Color constants
RED=\033[0;31m
GREEN=\033[0;32m
NC=\033[0m

all:	
	@ mpicc ${CFLAGS} ${SRC} -o ${EXEC}
	@ qsub ${JOB_SH}
debug:
	@ mpicc ${CFLAGS} -D DEBUG ${SRC} -o ${EXEC}
	@ qsub ${JOB_SH}
clean:
	@ rm ${JOB_NAME}.* ${EXEC}
cat:
	@ echo -e "${RED}Errors:"
	@ cat ${JOB_NAME}.e*
	@ echo -e "${GREEN}Output:${NC}"
	@ cat ${JOB_NAME}.o*
