/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX process spawning (<spawn.h>)
 *
 * Provides the POSIX spawn API for creating a new process from an executable
 * image, with optional file actions and attributes applied in the child
 * before it starts.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/spawn.h.html">
 *      POSIX.1-2017 &lt;spawn.h&gt;</a>
 *
 * @ingroup posix_option_spawn
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SPAWN_H_
#define ZEPHYR_INCLUDE_POSIX_SPAWN_H_

#include <sched.h>
#include <signal.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reset the child's effective user and group IDs to its real IDs.
 * @ingroup posix_option_spawn
 */
#define POSIX_SPAWN_RESETIDS      0x01
/**
 * @brief Place the child in the process group named by the pgroup attribute.
 * @ingroup posix_option_spawn
 */
#define POSIX_SPAWN_SETPGROUP     0x02
/**
 * @brief Set the child's scheduling parameters from the schedparam attribute.
 * @ingroup posix_option_spawn
 */
#define POSIX_SPAWN_SETSCHEDPARAM 0x04
/**
 * @brief Set the child's scheduling policy from the schedpolicy attribute.
 * @ingroup posix_option_spawn
 */
#define POSIX_SPAWN_SETSCHEDULER  0x08
/**
 * @brief Reset the signals named by the sigdefault attribute to their default actions.
 * @ingroup posix_option_spawn
 */
#define POSIX_SPAWN_SETSIGDEF     0x10
/**
 * @brief Set the child's signal mask from the sigmask attribute.
 * @ingroup posix_option_spawn
 */
#define POSIX_SPAWN_SETSIGMASK    0x20

#ifndef __posix_spawnattr_t_defined
#define __posix_spawnattr_t_defined
/**
 * @brief Spawn attributes object, initialized with posix_spawnattr_init().
 * @ingroup posix_option_spawn
 */
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
/**
 * @brief Ordered list of file actions performed in the child, initialized with
 *        posix_spawn_file_actions_init().
 * @ingroup posix_option_spawn
 */
typedef struct {
	struct posix_spawn_file_action *actions;
	int num;
	int cap;
} posix_spawn_file_actions_t;
#endif

/**
 * @brief Spawn a new process from the executable image named by @p path.
 *
 * File actions and attributes are applied in the child, in that order, before
 * the child begins execution.
 *
 * @ingroup posix_option_spawn
 * @param pid          Output: process ID of the child, or NULL.
 * @param path         Path naming the executable image.
 * @param file_actions File actions performed in the child, or NULL.
 * @param attrp        Spawn attributes, or NULL for defaults.
 * @param argv         NULL-terminated argument vector for the child.
 * @param envp         NULL-terminated environment vector for the child.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawn.html
 */
int posix_spawn(pid_t *pid, const char *path, const posix_spawn_file_actions_t *file_actions,
		const posix_spawnattr_t *attrp, char *const argv[], char *const envp[]);

/**
 * @brief Spawn a new process, resolving @p file as posix_spawn() resolves a path.
 * @ingroup posix_option_spawn
 * @param pid          Output: process ID of the child, or NULL.
 * @param file         Name of the executable image.
 * @param file_actions File actions performed in the child, or NULL.
 * @param attrp        Spawn attributes, or NULL for defaults.
 * @param argv         NULL-terminated argument vector for the child.
 * @param envp         NULL-terminated environment vector for the child.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnp.html
 */
int posix_spawnp(pid_t *pid, const char *file, const posix_spawn_file_actions_t *file_actions,
		 const posix_spawnattr_t *attrp, char *const argv[], char *const envp[]);

/**
 * @brief Add a close action for descriptor @p fildes in the child.
 * @ingroup posix_option_spawn
 * @param file_actions File actions object.
 * @param fildes       Descriptor to close in the child.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawn_file_actions_addclose.html
 */
int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *file_actions, int fildes);

/**
 * @brief Add a dup2 action in the child, duplicating @p fildes onto @p newfildes.
 * @ingroup posix_option_spawn
 * @param file_actions File actions object.
 * @param fildes       Source descriptor.
 * @param newfildes    Destination descriptor in the child.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawn_file_actions_adddup2.html
 */
int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *file_actions, int fildes,
				     int newfildes);

/**
 * @brief Add an open action in the child, opening @p path as descriptor @p fildes.
 * @ingroup posix_option_spawn
 * @param file_actions File actions object.
 * @param fildes       Descriptor to open in the child.
 * @param path         Path to open.
 * @param oflag        Open flags, as for open().
 * @param mode         Creation mode, as for open().
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawn_file_actions_addopen.html
 */
int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *file_actions, int fildes,
				     const char *path, int oflag, mode_t mode);

/**
 * @brief Destroy a file actions object.
 * @ingroup posix_option_spawn
 * @param file_actions File actions object.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawn_file_actions_destroy.html
 */
int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *file_actions);

/**
 * @brief Initialize a file actions object with no actions.
 * @ingroup posix_option_spawn
 * @param file_actions File actions object.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawn_file_actions_init.html
 */
int posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions);

/**
 * @brief Destroy a spawn attributes object.
 * @ingroup posix_option_spawn
 * @param attr Spawn attributes object.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_destroy.html
 */
int posix_spawnattr_destroy(posix_spawnattr_t *attr);

/**
 * @brief Get the POSIX_SPAWN_* flags of a spawn attributes object.
 * @ingroup posix_option_spawn
 * @param attr  Spawn attributes object.
 * @param flags Output: current flags.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_getflags.html
 */
int posix_spawnattr_getflags(const posix_spawnattr_t *attr, short *flags);

/**
 * @brief Get the process group attribute.
 * @ingroup posix_option_spawn
 * @param attr   Spawn attributes object.
 * @param pgroup Output: current process group attribute.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_getpgroup.html
 */
int posix_spawnattr_getpgroup(const posix_spawnattr_t *attr, pid_t *pgroup);

/**
 * @brief Get the scheduling parameter attribute.
 * @ingroup posix_option_spawn
 * @param attr       Spawn attributes object.
 * @param schedparam Output: current scheduling parameters.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_getschedparam.html
 */
int posix_spawnattr_getschedparam(const posix_spawnattr_t *attr, struct sched_param *schedparam);

/**
 * @brief Get the scheduling policy attribute.
 * @ingroup posix_option_spawn
 * @param attr        Spawn attributes object.
 * @param schedpolicy Output: current scheduling policy.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_getschedpolicy.html
 */
int posix_spawnattr_getschedpolicy(const posix_spawnattr_t *attr, int *schedpolicy);

/**
 * @brief Get the default-signals attribute.
 * @ingroup posix_option_spawn
 * @param attr       Spawn attributes object.
 * @param sigdefault Output: signals reset to their default actions in the child.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_getsigdefault.html
 */
int posix_spawnattr_getsigdefault(const posix_spawnattr_t *attr, sigset_t *sigdefault);

/**
 * @brief Get the signal mask attribute.
 * @ingroup posix_option_spawn
 * @param attr    Spawn attributes object.
 * @param sigmask Output: signal mask applied to the child.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_getsigmask.html
 */
int posix_spawnattr_getsigmask(const posix_spawnattr_t *attr, sigset_t *sigmask);

/**
 * @brief Initialize a spawn attributes object with default values.
 * @ingroup posix_option_spawn
 * @param attr Spawn attributes object.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_init.html
 */
int posix_spawnattr_init(posix_spawnattr_t *attr);

/**
 * @brief Set the POSIX_SPAWN_* flags of a spawn attributes object.
 * @ingroup posix_option_spawn
 * @param attr  Spawn attributes object.
 * @param flags New flags.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_setflags.html
 */
int posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags);

/**
 * @brief Set the process group attribute, applied with POSIX_SPAWN_SETPGROUP.
 * @ingroup posix_option_spawn
 * @param attr   Spawn attributes object.
 * @param pgroup Process group for the child; 0 starts a new group led by the child.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_setpgroup.html
 */
int posix_spawnattr_setpgroup(posix_spawnattr_t *attr, pid_t pgroup);

/**
 * @brief Set the scheduling parameter attribute, applied with POSIX_SPAWN_SETSCHEDPARAM.
 * @ingroup posix_option_spawn
 * @param attr       Spawn attributes object.
 * @param schedparam Scheduling parameters for the child.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_setschedparam.html
 */
int posix_spawnattr_setschedparam(posix_spawnattr_t *attr, const struct sched_param *schedparam);

/**
 * @brief Set the scheduling policy attribute, applied with POSIX_SPAWN_SETSCHEDULER.
 * @ingroup posix_option_spawn
 * @param attr        Spawn attributes object.
 * @param schedpolicy Scheduling policy for the child.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_setschedpolicy.html
 */
int posix_spawnattr_setschedpolicy(posix_spawnattr_t *attr, int schedpolicy);

/**
 * @brief Set the default-signals attribute, applied with POSIX_SPAWN_SETSIGDEF.
 * @ingroup posix_option_spawn
 * @param attr       Spawn attributes object.
 * @param sigdefault Signals reset to their default actions in the child.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_setsigdefault.html
 */
int posix_spawnattr_setsigdefault(posix_spawnattr_t *attr, const sigset_t *sigdefault);

/**
 * @brief Set the signal mask attribute, applied with POSIX_SPAWN_SETSIGMASK.
 * @ingroup posix_option_spawn
 * @param attr    Spawn attributes object.
 * @param sigmask Signal mask for the child.
 * @return 0 on success, or an errno value on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_setsigmask.html
 */
int posix_spawnattr_setsigmask(posix_spawnattr_t *attr, const sigset_t *sigmask);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SPAWN_H_ */
