bin_PROGRAMS = bbs
man_MANS = bbs.1

bbs_SOURCES = \
	bbsrc.c \
	color.c \
	config.c \
	edit.c \
	filter.c \
	getline.c \
	global.c \
	info.c \
	inkey.c \
	main.c \
	queue.c \
	slist.c \
	sysio.c \
	telnet.c \
	utility.c \
	unix.c

if HAVE_CMOCKA
check_PROGRAMS = test_queue test_slist test_utility test_bbsrc
TESTS = $(check_PROGRAMS)
test_queue_SOURCES = test/test_queue.c queue.c
test_queue_CFLAGS = $(AM_CFLAGS) $(CMOCKA_CFLAGS)
test_queue_LDADD = $(CMOCKA_LIBS)
test_slist_SOURCES = test/test_slist.c slist.c
test_slist_CFLAGS = $(AM_CFLAGS) $(CMOCKA_CFLAGS)
test_slist_LDADD = $(CMOCKA_LIBS)
test_utility_SOURCES = test/test_utility.c utility.c global.c
test_utility_CFLAGS = $(AM_CFLAGS) $(CMOCKA_CFLAGS)
test_utility_LDADD = $(CMOCKA_LIBS)
test_bbsrc_SOURCES = test/test_bbsrc.c bbsrc.c queue.c slist.c global.c
test_bbsrc_CFLAGS = $(AM_CFLAGS) $(CMOCKA_CFLAGS)
test_bbsrc_LDADD = $(CMOCKA_LIBS)
else
TESTS =
endif

install-exec-hook:
	@echo 'Now hot Asian sluts will come out of your computer whenever you type "bbs".'

.PHONY: cppcheck analyze

cppcheck:
	@if ! command -v cppcheck >/dev/null 2>&1; then \
		echo "cppcheck is not installed. Install it with: brew install cppcheck"; \
		exit 1; \
	fi
	@if [ -f compile_commands.json ]; then \
		cppcheck --project=compile_commands.json \
			--enable=warning,style,performance,portability \
			--inconclusive; \
	else \
		echo "compile_commands.json not found; analyzing bbs sources directly"; \
		cppcheck --enable=warning,style,performance,portability \
			--inconclusive --std=c11 -DHAVE_CONFIG_H -I. $(bbs_SOURCES); \
	fi

analyze: cppcheck
	@if command -v clang-tidy >/dev/null 2>&1 && [ -f compile_commands.json ]; then \
		clang-tidy -p . $(bbs_SOURCES); \
	else \
		echo "Skipping clang-tidy (requires clang-tidy and compile_commands.json)"; \
	fi
