/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/libc-hooks.h>
#include <zephyr/sys/process_state.h>
#include <zephyr/sys/sem.h>

#define TRACK_ALLOC (IS_ENABLED(CONFIG_POSIX_ENV_LOG_LEVEL_DBG) || IS_ENABLED(CONFIG_ZTEST))

LOG_MODULE_REGISTER(posix_env, CONFIG_POSIX_ENV_LOG_LEVEL);

/*
 * The environment is process state. The boot image's lives in the globals
 * below, behind environ; an image process with C library state of its own
 * carries one in a slot there, seeded from the vector it was entered with.
 */
struct env_state {
	char ***environ;
	size_t allocated;
};

#ifdef CONFIG_USERSPACE
/* taken from user-mode processes: C library application memory */
static Z_LIBC_DATA SYS_SEM_DEFINE(environ_lock, 1, 1);
#else
static SYS_SEM_DEFINE(environ_lock, 1, 1);
#endif
static size_t allocated;
char **environ;

static struct env_state boot_env = {
	.environ = &environ,
};

struct process_env {
	struct env_state state;
	char **vars;
};

static struct env_state *env_state(struct process_state **lpp)
{
	struct process_state *lp = process_state_get();
#if PROCESS_STATE_SUPPORTED
	struct process_env *pe;
#endif /* PROCESS_STATE_SUPPORTED */

	*lpp = lp;
	if (lp == NULL) {
		return &boot_env;
	}
#if !PROCESS_STATE_SUPPORTED
	return NULL;
#else
	pe = lp->slot[PROCESS_STATE_SLOT_ENV];
	if (pe == NULL) {
		size_t n = 0;

		while ((lp->envp != NULL) && (lp->envp[n] != NULL)) {
			n++;
		}
		/* a copy the process owns: entries are freed and reallocated individually */
		pe = calloc(1, sizeof(*pe));
		if (pe == NULL) {
			return NULL;
		}
		pe->state.environ = &pe->vars;
		if (n > 0) {
			pe->vars = calloc(n + 1, sizeof(char *));
			if (pe->vars == NULL) {
				free(pe);
				return NULL;
			}
			for (size_t i = 0; i < n; i++) {
				pe->vars[i] = strdup(lp->envp[i]);
			}
		}
		lp->slot[PROCESS_STATE_SLOT_ENV] = pe;
	}

	return &pe->state;
#endif /* PROCESS_STATE_SUPPORTED */
}

static void env_lock(struct process_state *lp)
{
	if (lp != NULL) {
		process_state_lock(lp);
	} else {
		(void)sys_sem_take(&environ_lock, K_FOREVER);
	}
}

static void env_unlock(struct process_state *lp)
{
	if (lp != NULL) {
		process_state_unlock(lp);
	} else {
		(void)sys_sem_give(&environ_lock);
	}
}

#ifdef CONFIG_ZTEST
size_t posix_env_get_allocated_space(void)
{
	struct process_state *lp;
	struct env_state *st = env_state(&lp);

	return (st == NULL) ? 0 : ((lp == NULL) ? allocated : st->allocated);
}
#endif

static size_t environ_size(char **vars)
{
	size_t ret;

	if (vars == NULL) {
		return 0;
	}
	for (ret = 0; vars[ret] != NULL; ++ret) {
	}

	return ret;
}

static int findenv(char **vars, const char *name, size_t namelen)
{
	const char *env;

	if (name == NULL || namelen == 0 || strchr(name, '=') != NULL) {
		/* Note: '=' is not a valid name character */
		return -EINVAL;
	}
	if (vars == NULL) {
		return -ENOENT;
	}
	for (char **envp = &vars[0]; *envp != NULL; ++envp) {
		env = *envp;
		if (strncmp(env, name, namelen) == 0 && env[namelen] == '=') {
			return envp - vars;
		}
	}

	return -ENOENT;
}

static void track(struct env_state *st, struct process_state *lp, ssize_t delta)
{
	if (TRACK_ALLOC) {
		size_t *acc = (lp == NULL) ? &allocated : &st->allocated;

		*acc += delta;
		LOG_DBG("%s %zd bytes (allocated: %zu)", (delta < 0) ? "free" : "alloc", delta,
			*acc);
	}
}

char *z_getenv(const char *name)
{
	struct process_state *lp;
	struct env_state *st = env_state(&lp);
	char *val = NULL;
	int ret;

	if (st == NULL) {
		return NULL;
	}
	env_lock(lp);
	ret = findenv(*st->environ, name, (name == NULL) ? 0 : strlen(name));
	if (ret >= 0) {
		val = (*st->environ)[ret] + strlen(name) + 1;
	}
	env_unlock(lp);

	return val;
}

int z_getenv_r(const char *name, char *buf, size_t len)
{
	struct process_state *lp;
	struct env_state *st = env_state(&lp);
	size_t vsize;
	int ret;

	if (st == NULL) {
		errno = ENOMEM;
		return -1;
	}
	env_lock(lp);
	ret = findenv(*st->environ, name, (name == NULL) ? 0 : strlen(name));
	if (ret < 0) {
		env_unlock(lp);
		LOG_DBG("No entry for name '%s'", name);
		errno = -ret;
		return -1;
	}
	{
		const char *val = (*st->environ)[ret] + strlen(name) + 1;

		LOG_DBG("Found entry %s", (*st->environ)[ret]);
		vsize = strlen(val) + 1;
		if (vsize > len) {
			env_unlock(lp);
			errno = ERANGE;
			return -1;
		}
		strcpy(buf, val);
	}
	env_unlock(lp);

	return 0;
}

int z_setenv(const char *name, const char *val, int overwrite)
{
	struct process_state *lp;
	struct env_state *st = env_state(&lp);
	int ret = 0;
	char *env;
	char **envp;
	char **vars;
	size_t esize;
	const size_t vsize = (val == NULL) ? 0 : strlen(val);
	const size_t nsize = (name == NULL) ? 0 : strlen(name);
	/* total size of name + '=' + val + '\0' */
	const size_t tsize = nsize + 1 /* '=' */ + vsize + 1 /* '\0' */;

	if (name == NULL || val == NULL) {
		LOG_DBG("Invalid name '%s' or value '%s'", name, val);
		errno = EINVAL;
		return -1;
	}
	if (st == NULL) {
		errno = ENOMEM;
		return -1;
	}

	env_lock(lp);
	vars = *st->environ;
	ret = findenv(vars, name, nsize);
	if (ret == -EINVAL) {
		LOG_DBG("Invalid name '%s'", name);
		goto out;
	}
	if (ret >= 0) {
		/* name was found in environ */
		esize = strlen(vars[ret]) + 1;
		if (overwrite == 0) {
			LOG_DBG("Found entry %s", vars[ret]);
			ret = 0;
			goto out;
		}
	} else {
		/* name was not found in environ -> add new entry */
		esize = environ_size(vars);
		envp = realloc(vars, sizeof(void *) * (esize + 1 /* new entry */ + 1 /* NULL */));
		if (envp == NULL) {
			ret = -ENOMEM;
			goto out;
		}
		track(st, lp, sizeof(void *) * (esize + 2));
		vars = envp;
		*st->environ = vars;
		ret = esize;
		vars[ret] = NULL;
		vars[ret + 1] = NULL;
		esize = 0;
	}
	if (esize < tsize) {
		/* need to malloc or realloc space for new environ entry */
		env = realloc(vars[ret], tsize);
		if (env == NULL) {
			ret = -ENOMEM;
			goto out;
		}
		track(st, lp, tsize - esize);
		vars[ret] = env;
	}
	strcpy(vars[ret], name);
	vars[ret][nsize] = '=';
	strncpy(vars[ret] + nsize + 1, val, vsize + 1);
	LOG_DBG("Added entry %s", vars[ret]);
	ret = 0;
out:
	env_unlock(lp);
	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}

	return ret;
}

int z_unsetenv(const char *name)
{
	struct process_state *lp;
	struct env_state *st = env_state(&lp);
	int ret = 0;
	char **envp;
	char **vars;
	size_t esize;
	size_t nsize;

	if (st == NULL) {
		errno = ENOMEM;
		return -1;
	}
	nsize = (name == NULL) ? 0 : strlen(name);
	env_lock(lp);
	vars = *st->environ;
	ret = findenv(vars, name, nsize);
	if (ret < 0) {
		ret = (ret == -EINVAL) ? -EINVAL : 0;
		goto out;
	}
	esize = environ_size(vars);
	track(st, lp, -(ssize_t)(strlen(vars[ret]) + 1));
	free(vars[ret]);
	/* shuffle remaining environment variable pointers forward */
	for (; ret < esize; ++ret) {
		vars[ret] = vars[ret + 1];
	}
	/* environ must be terminated with a NULL pointer */
	vars[ret] = NULL;
	/* reduce environ size and update allocation */
	--esize;
	if (esize == 0) {
		free(vars);
		*st->environ = NULL;
	} else {
		envp = realloc(vars, (esize + 1 /* NULL */) * sizeof(void *));
		if (envp != NULL) {
			*st->environ = envp;
		}
	}
	track(st, lp, -(ssize_t)(((esize == 0) ? 2 : 1) * sizeof(void *)));
	ret = 0;
out:
	env_unlock(lp);
	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}

	return ret;
}
