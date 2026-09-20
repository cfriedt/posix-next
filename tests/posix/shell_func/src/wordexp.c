/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shell_func_tests.h"

#include <stdlib.h>
#include <string.h>
#include <wordexp.h>

static void expect_words(const wordexp_t *we, const char *const *words, size_t n, size_t offs)
{
	zassert_equal(we->we_wordc, n, "we_wordc: %zu, want %zu", we->we_wordc, n);
	for (size_t i = 0; i < offs; i++) {
		zassert_is_null(we->we_wordv[i]);
	}
	for (size_t i = 0; i < n; i++) {
		zassert_str_equal(we->we_wordv[offs + i], words[i], "[%zu]: %s, want %s", i,
				  we->we_wordv[offs + i], words[i]);
	}
	zassert_is_null(we->we_wordv[offs + n]);
}

#define EXPECT_WORDS(we, ...)                                                                      \
	do {                                                                                       \
		const char *const words_[] = {__VA_ARGS__};                                        \
		expect_words((we), words_, ARRAY_SIZE(words_), 0);                                 \
	} while (0)

ZTEST(posix_shell_func, test_wordexp)
{
	static const char *const two[] = {"x", "y"};
	wordexp_t we;

	/* field splitting and quoting */
	zassert_equal(wordexp("a b  c", &we, 0), 0);
	EXPECT_WORDS(&we, "a", "b", "c");
	wordfree(&we);

	zassert_equal(wordexp("'a b' c", &we, 0), 0);
	EXPECT_WORDS(&we, "a b", "c");
	wordfree(&we);

	zassert_equal(wordexp("", &we, 0), 0);
	zassert_equal(we.we_wordc, 0);
	wordfree(&we);

	/* variables come from the caller's environment */
	zassert_ok(setenv("WE_VAR", "x y", 1));
	zassert_equal(wordexp("$WE_VAR", &we, 0), 0);
	EXPECT_WORDS(&we, "x", "y");
	wordfree(&we);
	zassert_equal(wordexp("\"$WE_VAR\" ${WE_VAR}z", &we, 0), 0);
	EXPECT_WORDS(&we, "x y", "x", "yz");
	wordfree(&we);
	zassert_ok(unsetenv("WE_VAR"));

	/* pathname expansion, sorted */
	zassert_equal(wordexp(TEST_GLOB "/a*", &we, 0), 0);
	EXPECT_WORDS(&we, TEST_GLOB "/a1", TEST_GLOB "/a2");
	wordfree(&we);

	/* command substitution, refused before the shell runs */
	zassert_equal(wordexp("$(echo x)", &we, WRDE_NOCMD), WRDE_CMDSUB);
	zassert_equal(wordexp("`echo x`", &we, WRDE_NOCMD), WRDE_CMDSUB);
	zassert_equal(wordexp("'$(not run)'", &we, WRDE_NOCMD), 0);
	EXPECT_WORDS(&we, "$(not run)");
	wordfree(&we);

	/* reserved slots, then more words */
	memset(&we, 0, sizeof(we));
	we.we_offs = 2;
	zassert_equal(wordexp("x y", &we, WRDE_DOOFFS), 0);
	expect_words(&we, two, 2, 2);
	zassert_equal(wordexp("z", &we, WRDE_DOOFFS | WRDE_APPEND), 0);
	zassert_equal(we.we_wordc, 3);
	zassert_str_equal(we.we_wordv[4], "z");
	zassert_is_null(we.we_wordv[5]);
	wordfree(&we);

	/* the shell refuses what it cannot parse, and an unset variable under WRDE_UNDEF */
	zassert_equal(wordexp("a 'b", &we, 0), WRDE_SYNTAX);
	zassert_true(wordexp("$WE_UNSET_VARIABLE", &we, WRDE_UNDEF) != 0);
	zassert_equal(wordexp("$WE_UNSET_VARIABLE", &we, 0), 0);
	zassert_equal(we.we_wordc, 0);
	wordfree(&we);

	/* a flag nobody defined (the host takes it) */
	IF_NOT_NATIVE_LIBC({
		zassert_equal(wordexp("a", &we, 0x1000), WRDE_BADVAL);
	});
}

ZTEST(posix_shell_func, test_wordfree)
{
	wordexp_t we;

	/* nothing to free */
	memset(&we, 0, sizeof(we));
	wordfree(&we);

	zassert_equal(wordexp("a b", &we, 0), 0);
	zassert_equal(we.we_wordc, 2);
	wordfree(&we);
	zassert_is_null(we.we_wordv);
	/* already freed */
	wordfree(&we);

	/* reserved slots are released with the words */
	memset(&we, 0, sizeof(we));
	we.we_offs = 3;
	zassert_equal(wordexp("a b", &we, WRDE_DOOFFS), 0);
	wordfree(&we);
	zassert_is_null(we.we_wordv);
}
