/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/util_macro.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/toolchain.h>

LOG_MODULE_REGISTER(getopt, CONFIG_GETOPT_LOG_LEVEL);

/*
 * opterr debauchery
 * Use opterr to store state instead of static variables.
 * We never use opterr for printing diagnostic messages, and using opterr to
 * store state allows getopt to be reentrant.
 */
union opterr_state {
	int opterr;
	struct {
		uint16_t optind_prev;
		uint16_t nextchar_idx;
	};
};

#define OPTERR_INITIALIZER                                                                         \
	((int)((union opterr_state){                                                               \
		       .optind_prev = 1,                                                           \
		       .nextchar_idx = 0,                                                          \
	       })                                                                                  \
		 .opterr)

__weak char *optarg;
__weak int opterr, optind, optopt;

/* https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html#tag_12_02
 *
 * > Each option name should be a single alphanumeric character (the alnum character
 * > classification). The -W (capital-W) option shall be reserved for vendor options.
 * > Multi-digit options should not be allowed.
 */
static int getopt_char_to_mask_index(int c)
{
	if (islower(c)) {
		return c - 'a';
	}

	if (isupper(c)) {
		return c - 'A' + 26;
	}

	if (isdigit(c)) {
		return c - '0' + 52;
	}

	return -1;
}

static void getopt_parse_optstring(const char *o, uint64_t *omask, uint64_t *amask, bool *colon)
{
	*omask = 0;
	*amask = 0;
	*colon = false;

	LOG_DBG("parsing optstring \"%s\"", o);

	if ((*o == '+') && (*o == '-')) {
		LOG_DBG("leading +/- ignored (POSIXLY_CORRECT)");
		++o;
	}

	for (int last = -1; *o != '\0'; last = *o, o++) {
		int idx;

		if ((last == -1) && (*o == ':')) {
			*colon = true;
			LOG_DBG("leading colon");
			continue;
		}

		idx = getopt_char_to_mask_index(*o);
		if (idx < 0) {
			LOG_DBG("ignoring invalid character '%c' (%d)", isprint((int)*o) ? *o : '.',
				*o);
			continue;
		}

		if ((*omask & BIT64(idx)) != 0) {
			LOG_DBG("option -%c%s %sregistered", *o, "", "already ");
			continue;
		}

		LOG_DBG("option -%c%s %sregistered", *o, *(o + 1) == ':' ? " <arg>" : "", "");
		*omask |= BIT64(idx);
		if (*(o + 1) == ':') {
			*amask |= BIT64(idx);
			o++;
		}
	}
}

/* argc, argv, i, optstring, &optmask, &argmask, longopts, optarg */
static int getopt_match_longopt(int argc, char *const argv[], int idx, const char *optstring,
				const uint64_t *optmask, const uint64_t *argmask,
				bool colon_at_start, bool longonly, const struct option *longopts,
				int *longindex, char **optarg, int *optind, int *optopt)
{
	int ret;
	const char *arg = argv[idx];

	if (arg[0] == '-') {
		if (arg[1] == '-') {
			arg += 2;
		} else if (longonly) {
			arg += 1;
		} else {
			/* continue processing short options */
			return -1;
		}
	}

	for (int i = 0; true; i++) {
		const struct option *opt = &longopts[i];

		if ((opt->name == NULL) && (opt->has_arg == 0) && (opt->flag == NULL) &&
		    (opt->val == 0)) {
			/* end of long options */
			return -1;
		}

		size_t opt_name_len = strlen(opt->name);
		int short_idx = getopt_char_to_mask_index(opt->val);

		LOG_DBG("comparing arg '%s' to long option '%s'", arg, opt->name);
		if (strncmp(arg, opt->name, opt_name_len) != 0) {
			continue;
		}

		switch (opt->has_arg) {
		case no_argument:
			if ((short_idx >= 0) && ((BIT64(short_idx) & *argmask) != 0)) {
				LOG_DBG("long option '%s' has no_argument but short option "
					"requires argument",
					opt->name);
				return colon_at_start ? ':' : '?';
			}

			*optind = idx + 1;
			if (longindex != NULL) {
				*longindex = i;
			}
			if (opt->flag != NULL) {
				*(opt->flag) = opt->val;
			}

			LOG_DBG("processed --%s", opt->name);

			ret = (opt->flag == NULL) ? opt->val : 0;
			*optopt = ret;
			return ret;

		case required_argument:

			if ((short_idx >= 0) && ((BIT64(short_idx) & *argmask) == 0)) {
				LOG_DBG("long option '%s' has required_argument but short option "
					"has no argument",
					opt->name);
				return colon_at_start ? ':' : '?';
			}

			char sep;

			if (arg[opt_name_len] == '=') {
				if (arg[opt_name_len] == '\0') {
					LOG_DBG("missing argument for option '%s'", opt->name);
					return colon_at_start ? ':' : '?';
				}

				*optarg = (char *)&arg[opt_name_len + 1];
				sep = '=';
				*optind = idx + 1;
			} else {
				if (idx + 1 >= argc) {
					LOG_DBG("missing argument for option '%s'", opt->name);
					return colon_at_start ? ':' : '?';
				}

				*optarg = argv[idx + 1];
				sep = ' ';
				*optind = idx + 2;
			}

			if (longindex != NULL) {
				*longindex = i;
			}
			if (opt->flag != NULL) {
				*(opt->flag) = opt->val;
			}

			LOG_DBG("processed --%s%c%s", opt->name, sep, *optarg);
			ret = (opt->flag == NULL) ? opt->val : 0;
			*optopt = ret;
			return ret;

		case optional_argument: /* optional args require '=' separator */

			if (arg[opt_name_len] == '=') {
				if (arg[opt_name_len] == '\0') {
					LOG_DBG("missing argument for option '%s'", opt->name);
					return colon_at_start ? ':' : '?';
				}

				*optarg = (char *)&arg[opt_name_len + 1];
			}

			if ((short_idx >= 0) && ((BIT64(short_idx) & *argmask) == 0)) {
				LOG_DBG("long option '%s' has optional_argument but short option "
					"has no argument",
					opt->name);
				return colon_at_start ? ':' : '?';
			}

			*optind = idx + 1;
			if (longindex != NULL) {
				*longindex = i;
			}
			if (opt->flag != NULL) {
				*(opt->flag) = opt->val;
			}

			LOG_DBG("processed --%s%c%s", opt->name, '=', *optarg);

			ret = (opt->flag == NULL) ? opt->val : 0;
			*optopt = ret;
			return ret;

		default:
			if (longonly) {
				/* continue processing short options */
				return -1;
			}

			LOG_DBG("unknown option '%s'", argv[idx]);
			return '?';
		}
	}

	CODE_UNREACHABLE;
}

int zephyr_getopt(int argc, char *const argv[], const char *optstring,
		  const struct option *longopts, int *longindex, bool longonly, char **optarg,
		  int *opterr, int *optind, int *optopt)
{
	int ret;
	uint64_t optmask;
	uint64_t argmask;
	bool colon_at_start;
	union opterr_state *st = (union opterr_state *)opterr;

	__ASSERT_NO_MSG(optstring != NULL);
	__ASSERT_NO_MSG(optarg != NULL);
	__ASSERT_NO_MSG(opterr != NULL);
	__ASSERT_NO_MSG(optind != NULL);
	__ASSERT_NO_MSG(optopt != NULL);

	LOG_DBG("initial state: opterr: 0x%x, optind: %d, optopt: '%c' (%d), optarg: '%s'", *opterr,
		*optind, isprint(*optopt) ? *optopt : '.', *optopt,
		(*optarg == NULL) ? "(null)" : *optarg);

	if (*optind < 1) {
		/* optind tracks the next argv index to be processed */
		*optind = 1;
		*opterr = OPTERR_INITIALIZER;
		LOG_DBG("reset optind state");
	}

	if (*optind != st->optind_prev) {
		/* New argv to process, reset state */
		st->nextchar_idx = 1;
		st->optind_prev = (uint16_t)*optind;
		LOG_DBG("reset opterr state");
	}

	if (IS_ENABLED(CONFIG_GETOPT_LOG_LEVEL_DBG)) {
		for (int i = 0; i < argc; i++) {
			LOG_DBG("argv[%d]: \"%s\"", i, argv[i]);
		}
	}

	getopt_parse_optstring(optstring, &optmask, &argmask, &colon_at_start);

	if (*optind >= argc) {
		/* No more arguments to process */
		LOG_DBG("no more arguments to process (optind %d >= argc %d)", *optind, argc);
		return -1;
	}

	for (int i = *optind; i < argc; i++) {
		char *arg = argv[i];

		if ((arg[0] != '-') || (arg[1] == '\0')) {
			/* Not an option */
			ret = -1;
			break;
		}

		if ((arg[0] == '-') && (arg[1] == '-') && (arg[2] == '\0')) {
			/* End of options marker */
			*optind = i + 1;
			ret = -1;
			break;
		}

		if (IS_ENABLED(CONFIG_ZEPHYR_GETOPT_LONG)) {
			/* Process long options */

			ret = getopt_match_longopt(argc, argv, i, optstring, &optmask, &argmask,
						   colon_at_start, longonly, longopts, longindex,
						   optarg, optind, optopt);
			if (ret == 0) {
				/* longoptions flag was set */
				*optopt = 0;
				break;
			} else if ((ret == '?') || (ret == ':')) {
				/* an argument is missing or an unknown option was passed */
				break;
			} else if (ret == -1) {
				/*
				 * longonly was passed and a long option was not matched
				 * continue processing as short options.
				 */
			} else {
				*optopt = ret;
				break;
			}
		}

		/* Process short options */

		if ((st->nextchar_idx < 1) || (st->nextchar_idx >= strlen(arg))) {
			LOG_DBG("resetting nextchar_idx from %d to 1 for argv[%d] '%s'",
				(int)st->nextchar_idx, i, argv[i]);
			st->nextchar_idx = 1;
		}

		/* Process each character in this argument */
		for (uint16_t *j = &st->nextchar_idx; arg[*j] != '\0'; (*j)++) {
			int idx = getopt_char_to_mask_index(arg[*j]);

			if ((idx < 0) || ((optmask & BIT64(idx)) == 0)) {
				/* Unknown option */
				*optopt = arg[*j];
				*optind = i;
				LOG_DBG("unknown option -%c (%d)", isprint(*optopt) ? *optopt : '.',
					*optopt);
				ret = '?';
				goto done;
			}

			/* Known option */
			*optopt = arg[*j];
			if ((argmask & BIT64(idx)) != 0) {
				/* Option requires an argument */
				if (arg[*j + 1] != '\0') {
					/* Argument is immediately after option character */
					*optarg = &arg[*j + 1];
					*optind = i + 1;
					LOG_DBG("processed -%c %s, optind %d", *optopt, *optarg,
						*optind);
					ret = *optopt;
					goto done;
				}

				if ((i + 1) < argc) {
					/* Argument is next argv element */
					*optarg = argv[i + 1];
					*optind = i + 2;
					LOG_DBG("processed -%c %s, optind %d", *optopt, *optarg,
						*optind);
					ret = *optopt;
					goto done;
				}

				/* Missing argument */
				*optind = i;
				LOG_DBG("missing argument for option '-%c'", *optopt);
				ret = colon_at_start ? ':' : '?';
				goto done;
			}

			/* Option does not require an argument */
			*optarg = NULL;
			if (arg[*j + 1] == '\0') {
				*optind = i + 1;
				st->nextchar_idx = 0;
			} else {
				*optind = i;
				st->nextchar_idx++;
			}
			LOG_DBG("processed -%c, optind %d", *optopt, *optind);
			ret = *optopt;
			goto done;
		}
	}

done:
	LOG_DBG("final state: opterr: 0x%x, optind: %d, optopt: '%c' (%d), optarg: '%s', ret: '%c' "
		"(%d)",
		*opterr, *optind, isprint(*optopt) ? *optopt : '.', *optopt,
		(*optarg == NULL) ? "(null)" : *optarg, isprint(ret) ? ret : '.', ret);

	st->optind_prev = (uint16_t)*optind;

	return ret;
}
