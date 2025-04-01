#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include<stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include "ft_printf.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct s_stdout_test 
{
	int pipefd[2];
	int stdout_backup;
	char *buf;
	size_t size;
} t_stdout_test;

void stdout_test_zero_init(t_stdout_test *t)
{
	t->size = 0;
	t->stdout_backup = -1;
	t->pipefd[0] = -1;
	t->pipefd[0] = -1;
	t->buf = NULL;
}

void stdout_test_destroy_contents(t_stdout_test *t)
{
	if (t->pipefd[0] >= 0)
		close(t->pipefd[0]);
	if (t->pipefd[1] >= 0)
		close(t->pipefd[1]);
	if (t->stdout_backup >= 0)
	{
		int return_value = dup2(t->stdout_backup, STDOUT_FILENO);
		if (return_value < 0)
			exit(EXIT_FAILURE);
		close(t->stdout_backup);
	}
	free(t->buf);
	stdout_test_zero_init(t);
}

void stdout_test_init(t_stdout_test *t, size_t size)
{
	stdout_test_zero_init(t);

	t->size = size;

	if (pipe(t->pipefd) == -1) 
	{
		stdout_test_destroy_contents(t);
		fail_msg("Failed to open pipe to redirect stdout for printf tests.");
	}

	t->stdout_backup = dup(STDOUT_FILENO);
	if (t->stdout_backup < 0)
	{
		stdout_test_destroy_contents(t);
		fail_msg("Error duplicating stdout to backup");
	}

	if (dup2(t->pipefd[1], STDOUT_FILENO) < 0)
	{
		stdout_test_destroy_contents(t);
		fail_msg("Error redirecting stdout to pipe");
	}

	t->buf = malloc(t->size);
	if (t->buf == NULL)
	{
		stdout_test_destroy_contents(t);
		fail_msg("Error allocating buffer");
	}
}

int stdout_test_assert_stdout(t_stdout_test *t, const char *expected)
{
	ssize_t bytes_read = 0;;
	size_t total_read = 0;
	dup2(t->stdout_backup, STDOUT_FILENO);
	t->stdout_backup = -1;
	close(t->pipefd[1]);
	t->pipefd[1] = -1;
	do {
		bytes_read = read(t->pipefd[0], t->buf + total_read, t->size - total_read);
		total_read += bytes_read;
	} while (bytes_read > 0);
	assert_true(bytes_read >= 0);
	assert_memory_equal(t->buf, expected, t->size);
	return (0);
}

static void	test_char(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "a\nb        cd";
	stdout_test_init(&t, strlen(expected));
	ft_printf("%c\n%c%9c%c", 'a','b','c','d');
	stdout_test_assert_stdout(&t, expected);
	stdout_test_destroy_contents(&t);
}

static void	test_strings_no_specifier(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "hola como estas";
	stdout_test_init(&t, strlen(expected));
	ft_printf("hola como estas");
	stdout_test_assert_stdout(&t, expected);
	stdout_test_destroy_contents(&t);
}

static void	test_null_str(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "(null)";
	stdout_test_init(&t, strlen(expected));
	ft_printf("%s", NULL);
	stdout_test_assert_stdout(&t, expected);
	stdout_test_destroy_contents(&t);
}

static void	test_null_str_prec_less_6(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "";
	stdout_test_init(&t, strlen(expected));
	ft_printf("%.5s", NULL);
	stdout_test_assert_stdout(&t, expected);
	stdout_test_destroy_contents(&t);
}

static void	test_empty_string(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "";
	stdout_test_init(&t, strlen(expected));
	ft_printf("");
	stdout_test_assert_stdout(&t, expected);
	stdout_test_destroy_contents(&t);
}

static void	test_many_chars(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "hola\ta   mi   go";
	stdout_test_init(&t, strlen(expected));
	ft_printf("hol%c\t%c%4c%--4cg%c", 'a', 'a', 'm', 'i', 'o');
	stdout_test_assert_stdout(&t, expected);
	stdout_test_destroy_contents(&t);
}

static void	test_asterisk_char_width(void **state)
{
	(void) state;
	t_stdout_test t;
	int result;
	const char *expected = "hola\ta     mi   go ";
	stdout_test_init(&t, strlen(expected));
	result = ft_printf("hol%*c\t%*c%4c%*cg%-*c", 0, 'a', -3, 'a', 'm', -4, 'i', -2, 'o');
	assert_int_equal(result, strlen(expected));
	stdout_test_assert_stdout(&t, expected);
	stdout_test_destroy_contents(&t);
}

static void	test_basic_string(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "your love";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%s", "your love");
	assert_int_equal(result, strlen(expected));
	stdout_test_assert_stdout(&t, expected);
	stdout_test_destroy_contents(&t);
}

static void test_string_precision_null(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%.s", "your love");
	assert_int_equal(result, strlen(expected));
	stdout_test_assert_stdout(&t, expected);
	stdout_test_destroy_contents(&t);
}

static void test_string_precision_null_zero(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%.0s", "your love");
	assert_int_equal(result, strlen(expected));
	stdout_test_assert_stdout(&t, expected);
	stdout_test_destroy_contents(&t);
}

static void test_string_precision_invalid(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "%.-s";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%.-s", "your love");
	assert_int_equal(result, strlen(expected));
	stdout_test_assert_stdout(&t, expected);
	stdout_test_destroy_contents(&t);
}

static void test_string_precision_larger(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "cutirimiguaro";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf( "%.45s", "cutirimiguaro");
	assert_int_equal(result, strlen(expected));
	stdout_test_assert_stdout(&t, expected);
	stdout_test_destroy_contents(&t);
}

static void test_string_all_flags(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "I'm t       R";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf( "%-10.5s %*.*s", "I'm the king of the world", 2, 1, "Rose");
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_2_printf_in_a_row(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "%.-sholas";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%.-s", "your love");
	assert_int_equal(result, 4);
	result = ft_printf("%s", "holas");
	assert_int_equal(result, 5);
	stdout_test_assert_stdout(&t, expected);
	stdout_test_destroy_contents(&t);
}

static void test_ptr_basic(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = " 0x10 ";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf( " %p ", 0x10);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_ptr_nil(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "(nil)";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf( "%p", NULL);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_nil_s_with_width_and_prec(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "          ";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf( "%10.2s", NULL);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_nil_p_with_width_and_prec(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "     (nil)";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf( "%10.2p", NULL);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_ptr_address_basic(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "0x1";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%p", (void *)0x1);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_ptr_large(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected;
	if (sizeof(long) == 8)
		expected = "0xffffffffffffffff";
	else
		expected = "0xffffffff";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%p", (void *)(0ul - 1));
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_ptr_width(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "0xf  ";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%-5p", (void *)15);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_integer_basic(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "0";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%d", 0);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_integer_negative(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "-8934";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%d", -8934);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_integer_min(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "-2147483648";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%d", INT_MIN);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_integer_max(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "2147483647";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%d", INT_MAX);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_integer_zero_precision_zero(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%.d", 0);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_integer_zero_flag_1(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "-000098734";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%010d", -98734);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_integer_zero_flag_2(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "+004";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%+04d", 4);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_integer_zero_precision_padding(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = " +0004";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%+06.4d", 4);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_integer_null_with_flags(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "     +";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%+ 06.d", 0);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_integer_all_flags(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "+00000987";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%-+ 06.8d", 987);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_integer_all_flags_2(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "+00000987 ";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%-+ 010.8d", 987);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_integer_many_flags_1(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "         000002147483647";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("% 024.15d", INT_MAX);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_integer_many_flags_2(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "        -000002147483648";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("% 024.15d", INT_MIN);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_unsigned_basic(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "4294967295";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%u", UINT_MAX);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_unsigned_null(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%.u", 0);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_unsigned_flags(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "0004294967295";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%+013u", UINT_MAX);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_unsigned_flags_2(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "0004294967295  ";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%0*.13u", -15, UINT_MAX);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_unsigned_flags_3(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "0004294967295  ";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%0*.*u", -15, 13, UINT_MAX);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_unsigned_flags_4(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "0000000000000  ";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%-0*.*u", -15, 13, 0);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_hex_basic(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "2389";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%x", 0x2389);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_hex_null(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%.x", 0x0);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_hex_null_hash(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%#.x", 0x0);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_hex_null_hash_width(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "     ";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%#*.x", 5, 0x0);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_hex_upper(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "0X6D0X6E6F";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%#X%#X%X", 'm', 'n', 'o');
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_all_1(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "0X6D% %.-30X6E6F";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%#X%% %.-3%#X%X", 'm', 'n', 'o');
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_all_2(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "100   099 Is what I'm saying (null) 0xff 10";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf(
		"%*d %4.3u%*.18s %1s %#x %X",
		-4, 100, 99, 19,
		"Is what I'm saying my friend", NULL, 0xff, 0x10
	);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_contents_bigger_than_buf(void **state)
{
	(void) state;
	t_stdout_test t;
	char *expected = malloc(3015);
	assert_non_null(expected);
	for (int i = 0; i < 3014; ++i)
	{
		switch (i % 4)
		{
			case 0:
				expected[i] = 'h';
				break;
			case 1:
				expected[i] = 'o';
				break;
			case 2:
				expected[i] = 'l';
				break;
			case 3:
				expected[i] = 'a';
				break;
		}
	}
	expected[3014] = '\0';
	char *str = malloc(3015);
	assert_non_null(str);
	strcpy(str, expected);
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%s", str);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
	free(expected);
	free(str);
}

static void test_karla(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "hola%";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("hola%");
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_karla2(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "0";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%d");
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_karla3(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "(nil)";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf("%p");
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, strlen(expected));
	stdout_test_destroy_contents(&t);
}

static void test_guillermo(void **state)
{
	(void) state;
	t_stdout_test t;
	const char *expected = "";
	stdout_test_init(&t, strlen(expected));
	int result = ft_printf(NULL);
	stdout_test_assert_stdout(&t, expected);
	assert_int_equal(result, -1);
	stdout_test_destroy_contents(&t);
}


int	main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_char),
		cmocka_unit_test(test_strings_no_specifier),
		cmocka_unit_test(test_empty_string),
		cmocka_unit_test(test_many_chars),
		cmocka_unit_test(test_asterisk_char_width),
		cmocka_unit_test(test_null_str),
		cmocka_unit_test(test_null_str_prec_less_6),
		cmocka_unit_test(test_basic_string),
		cmocka_unit_test(test_string_precision_null),
		cmocka_unit_test(test_string_precision_null_zero),
		cmocka_unit_test(test_string_precision_invalid),
		cmocka_unit_test(test_2_printf_in_a_row),
		cmocka_unit_test(test_string_precision_larger),
		cmocka_unit_test(test_string_all_flags),
		cmocka_unit_test(test_ptr_basic),
		cmocka_unit_test(test_ptr_nil),
		cmocka_unit_test(test_nil_p_with_width_and_prec),
		cmocka_unit_test(test_nil_s_with_width_and_prec),
		cmocka_unit_test(test_ptr_address_basic),
		cmocka_unit_test(test_ptr_large),
		cmocka_unit_test(test_ptr_width),
		cmocka_unit_test(test_integer_basic),
		cmocka_unit_test(test_integer_negative),
		cmocka_unit_test(test_integer_min),
		cmocka_unit_test(test_integer_max),
		cmocka_unit_test(test_integer_zero_precision_zero),
		cmocka_unit_test(test_integer_zero_flag_1),
		cmocka_unit_test(test_integer_zero_flag_2),
		cmocka_unit_test(test_integer_zero_precision_padding),
		cmocka_unit_test(test_integer_null_with_flags),
		cmocka_unit_test(test_integer_all_flags),
		cmocka_unit_test(test_integer_all_flags_2),
		cmocka_unit_test(test_integer_many_flags_1),
		cmocka_unit_test(test_integer_many_flags_2),
		cmocka_unit_test(test_unsigned_basic),
		cmocka_unit_test(test_unsigned_null),
		cmocka_unit_test(test_unsigned_flags),
		cmocka_unit_test(test_unsigned_flags_2),
		cmocka_unit_test(test_unsigned_flags_3),
		cmocka_unit_test(test_unsigned_flags_4),
		cmocka_unit_test(test_hex_basic),
		cmocka_unit_test(test_hex_null),
		cmocka_unit_test(test_hex_null_hash),
		cmocka_unit_test(test_hex_null_hash_width),
		cmocka_unit_test(test_hex_upper),
		cmocka_unit_test(test_all_1),
		cmocka_unit_test(test_all_2),
		cmocka_unit_test(test_contents_bigger_than_buf),
		cmocka_unit_test(test_karla),
		cmocka_unit_test(test_karla2),
		cmocka_unit_test(test_karla3),
		cmocka_unit_test(test_guillermo),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
