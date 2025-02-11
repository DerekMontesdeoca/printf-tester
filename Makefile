################################################################################
# PRINTF/TESTS                                                                 #
################################################################################
.SUFFIXES:

CC := cc

TO_TEST_DIR := ../

LIBFT_LIB := ../libft/libft.a

CMOCKA_DIR := cmocka
CMOCKA_INCLUDE_DIR := $(CMOCKA_DIR)/include
CMOCKA_BUILD_DIR := $(CMOCKA_DIR)/build
CMOCKA_LIB_DIR := $(CMOCKA_BUILD_DIR)/src
CMOCKA_LIB := $(CMOCKA_LIB_DIR)/libcmocka.so

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
TESTS := $(SRCS:.c=)

INCLUDES := $(CMOCKA_INCLUDE_DIR) $(TO_TEST_DIR)
override CFLAGS += -Wall -Wextra -Werror -g3 -fno-omit-frame-pointer
override CPPFLAGS += $(addprefix -I, $(INCLUDES))
override LDFLAGS += -L$(CMOCKA_LIB_DIR) -lcmocka 

################################################################################
# Rules                                                                        #
################################################################################

run_tests: $(CMOCKA_LIB) tests
	$(foreach test, $(TESTS), LD_LIBRARY_PATH=$(CMOCKA_LIB_DIR) ./$(test))

all: tests

tests: to_test $(TESTS)

$(TESTS): %: %.o ../libftprintf.a $(LIBFT_LIB) 
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

to_test:
	$(MAKE) -C $(TO_TEST_DIR) DEBUG="TRUE"

clean:
	rm -f $(OBJS)

fclean: clean
	rm -rf $(TESTS) $(CMOCKA_BUILD_DIR)

$(CMOCKA_LIB):
	mkdir -p $(CMOCKA_BUILD_DIR)
	cd $(CMOCKA_BUILD_DIR) && cmake .. && make 

re: fclean to_test_fclean tests

.PHONY: all clean fclean re to_test tests
