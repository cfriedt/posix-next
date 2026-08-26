/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_INCLUDE_POSIX_SPAWN_H_
#define ZEPHYR_INCLUDE_POSIX_SPAWN_H_

#include <sched.h>
#include <signal.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POSIX_SPAWN_RESETIDS      0x01
#define POSIX_SPAWN_SETPGROUP     0x02
#define POSIX_SPAWN_SETSCHEDPARAM 0x04
#define POSIX_SPAWN_SETSCHEDULER  0x08
#define POSIX_SPAWN_SETSIGDEF     0x10
#define POSIX_SPAWN_SETSIGMASK    0x20

#ifndef __posix_spawnattr_t_defined
#define __posix_spawnattr_t_defined
typedef struct {
	short flags;
	pid_t pgroup;
	sigset_t sigdefault;
	sigset_t sigmask;
	struct sched_param schedparam;
	int schedpolicy;
} posix_spawnattr_t;
#endif

#ifndef __posix_spawn_file_actions_t_defined
#define __posix_spawn_file_actions_t_defined
typedef struct {
	struct posix_spawn_file_action *actions;
	int num;
	int cap;
} posix_spawn_file_actions_t;
#endif

int posix_spawn(pid_t *pid, const char *path, const posix_spawn_file_actions_t *file_actions,
		const posix_spawnattr_t *attrp, char *const argv[], char *const envp[]);
int posix_spawnp(pid_t *pid, const char *file, const posix_spawn_file_actions_t *file_actions,
		 const posix_spawnattr_t *attrp, char *const argv[], char *const envp[]);
int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *file_actions, int fildes);
int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *file_actions, int fildes,
				     int newfildes);
int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *file_actions, int fildes,
				     const char *path, int oflag, mode_t mode);
int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *file_actions);
int posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions);
int posix_spawnattr_destroy(posix_spawnattr_t *attr);
int posix_spawnattr_getflags(const posix_spawnattr_t *attr, short *flags);
int posix_spawnattr_getpgroup(const posix_spawnattr_t *attr, pid_t *pgroup);
int posix_spawnattr_getschedparam(const posix_spawnattr_t *attr, struct sched_param *schedparam);
int posix_spawnattr_getschedpolicy(const posix_spawnattr_t *attr, int *schedpolicy);
int posix_spawnattr_getsigdefault(const posix_spawnattr_t *attr, sigset_t *sigdefault);
int posix_spawnattr_getsigmask(const posix_spawnattr_t *attr, sigset_t *sigmask);
int posix_spawnattr_init(posix_spawnattr_t *attr);
int posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags);
int posix_spawnattr_setpgroup(posix_spawnattr_t *attr, pid_t pgroup);
int posix_spawnattr_setschedparam(posix_spawnattr_t *attr, const struct sched_param *schedparam);
int posix_spawnattr_setschedpolicy(posix_spawnattr_t *attr, int schedpolicy);
int posix_spawnattr_setsigdefault(posix_spawnattr_t *attr, const sigset_t *sigdefault);
int posix_spawnattr_setsigmask(posix_spawnattr_t *attr, const sigset_t *sigmask);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SPAWN_H_ */
