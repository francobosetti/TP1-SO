CC=gcc
GCCFLAGS = -g -Wall -std=c99
GCCLIBS = -lrt -lpthread
FILES =  shmADT.c lib.c
MAIN_FILES = app view slave
#CHECK = check

all: $(MAIN_FILES) #$(CHECK)

$(MAIN_FILES): %: %.c
	@$(CC) $(GCCFLAGS) $(FILES) -o $@ $< $(GCCLIBS)

.PHONY: clean
clean:
	@rm -rf $(MAIN_FILES)
	#@rm -rf $(CHECK)

check:
	@mkdir check
	@cppcheck --quiet --enable=all --force --inconclusive . 2> ./check/cppout.txt

	@pvs-studio-analyzer trace -- make
	@pvs-studio-analyzer analyze
	@plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o ./check/report.tasks ./PVS-Studio.log

	@mv strace_out check
	@rm PVS-Studio.log

.PHONY: all clean check