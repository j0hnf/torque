PROG_ROOT = ../..

AM_CFLAGS = -g -DTEST_FUNCTION -I${PROG_ROOT}/ -I${PROG_ROOT}/${PBS_MACH} --coverage

lib_LTLIBRARIES = libenc_QueueJob_hash.la

AM_LDFLAGS = @CHECK_LIBS@ ${lib_LTLIBRARIES}

check_PROGRAMS = test_enc_QueueJob_hash

libenc_QueueJob_hash_la_SOURCES = scaffolding.c ${PROG_ROOT}/enc_QueueJob_hash.c
libenc_QueueJob_hash_la_LDFLAGS = @CHECK_LIBS@ -shared -lgcov

test_enc_QueueJob_hash_SOURCES = test_enc_QueueJob_hash.c

check_SCRIPTS = ${PROG_ROOT}/../../test/coverage_run.sh

TESTS = ${check_PROGRAMS} ${check_SCRIPTS} 

CLEANFILES = *.gcno *.gcda *.gcov core *.lo
