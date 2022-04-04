# Change for any new project
JOB_NAME = Exp_Job
JOB_SH = job.sh
EXEC = exp
SRC = ../source/exp.c

# Compilation flags
GMP = -I${HOME}/gmp/6.1.0/include -L${HOME}/gmp/6.1.0/lib -lgmp
CFLAGS = -std=c99

# Color constants
RED=\033[0;31m
GREEN=\033[0;32m
NC=\033[0m

all:	
	@ mpicc ${CFLAGS} ${SRC} -o ${EXEC} ${GMP}
	@ qsub ${JOB_SH}
debug:
	@ mpicc ${CFLAGS} -D DEBUG ${SRC} -o ${EXEC} ${GMP}
	@ qsub ${JOB_SH}
clean:
	@ rm ${JOB_NAME}.* ${EXEC}
cat:
	@ echo -e "${RED}Errors:${NC}"
	@ cat ${JOB_NAME}.e*
	@ echo -e "${GREEN}Output:${NC}"
	@ cat ${JOB_NAME}.o*
