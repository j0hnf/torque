PROG_ROOT = ../..

AM_CFLAGS = -g -DTEST_FUNCTION -I${PROG_ROOT}/ -I${PROG_ROOT}/${PBS_MACH} --coverage -I${PROG_ROOT}/../../resmom/${PBS_MACH}

lib_LTLIBRARIES = libsite_mom_ckp.la

AM_LDFLAGS = @CHECK_LIBS@ ${lib_LTLIBRARIES}

check_PROGRAMS = test_site_mom_ckp

libsite_mom_ckp_la_SOURCES = scaffolding.c ${PROG_ROOT}/site_mom_ckp.c
libsite_mom_ckp_la_LDFLAGS = @CHECK_LIBS@ -shared -lgcov

test_site_mom_ckp_SOURCES = test_site_mom_ckp.c

check_SCRIPTS = ${PROG_ROOT}/../../test/coverage_run.sh

TESTS = ${check_PROGRAMS} ${check_SCRIPTS}

CLEANFILES = *.gcno *.gcda *.gcov core *.lo
