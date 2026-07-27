/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2024 BayLibre, SAS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/math_extras.h>
#include <zephyr/sys/timeutil.h>

/* snapshot handed to the SIGEV_THREAD notification thread; owns no queue reference */
struct mq_notify_job {
	void (*func)(union sigval value);
	union sigval value;
	pthread_attr_t attr;
};

typedef struct mqueue_object {
	sys_snode_t snode;
	char *mem_buffer;
	char *mem_obj;
	struct k_msgq queue;
	atomic_t ref_count;
	char *name;
	struct sigevent not;              /* registered when sigev_notify != 0 */
	struct mqueue_desc *notify_mqd;   /* descriptor used to register */
	struct mq_notify_job *job;        /* non-NULL only when SIGEV_THREAD registered */
} mqueue_object;

typedef struct mqueue_desc {
	char *mem_desc;
	mqueue_object *mqueue;
	uint32_t  flags;
} mqueue_desc;

K_SEM_DEFINE(mq_sem, 1, 1);

/* Initialize the list */
sys_slist_t mq_list = SYS_SLIST_STATIC_INIT(&mq_list);

static mqueue_object *find_in_list(const char *name);
static int32_t send_message(mqueue_desc *mqd, const char *msg_ptr, size_t msg_len,
			  k_timeout_t timeout);
static int32_t receive_message(mqueue_desc *mqd, char *msg_ptr, size_t msg_len,
			   k_timeout_t timeout);
static void remove_notification(mqueue_object *msg_queue);
static void remove_mq(mqueue_object *msg_queue);
static void *mq_notify_thread(void *arg);

/**
 * @brief Open a message queue.
 *
 * Number of message queue and descriptor to message queue are limited by
 * heap size. increase the size through CONFIG_HEAP_MEM_POOL_SIZE.
 *
 * See IEEE 1003.1
 */
mqd_t mq_open(const char *name, int oflags, ...)
{
	va_list va;
	mode_t mode;
	struct mq_attr *attrs = NULL;
	long msg_size = 0U, max_msgs = 0U;
	mqueue_object *msg_queue;
	mqueue_desc *msg_queue_desc = NULL, *mqd = (mqueue_desc *)(-1);
	char *mq_desc_ptr, *mq_obj_ptr, *mq_buf_ptr, *mq_name_ptr;

	va_start(va, oflags);
	if ((oflags & O_CREAT) != 0) {
		BUILD_ASSERT(sizeof(mode_t) <= sizeof(int));
		mode = va_arg(va, unsigned int);
		attrs = va_arg(va, struct mq_attr*);
	}
	va_end(va);

	if (attrs != NULL) {
		msg_size = attrs->mq_msgsize;
		max_msgs = attrs->mq_maxmsg;
	}

	if ((name == NULL) || ((oflags & O_CREAT) != 0 && (msg_size <= 0 ||
						      max_msgs <= 0))) {
		errno = EINVAL;
		return (mqd_t)mqd;
	}

	if ((strlen(name) + 1)  > CONFIG_MQUEUE_NAMELEN_MAX) {
		errno = ENAMETOOLONG;
		return (mqd_t)mqd;
	}

	/* Check if queue already exists */
	k_sem_take(&mq_sem, K_FOREVER);
	msg_queue = find_in_list(name);
	k_sem_give(&mq_sem);

	if ((msg_queue != NULL) && (oflags & O_CREAT) != 0 &&
	    (oflags & O_EXCL) != 0) {
		/* Message queue has already been opened and O_EXCL is set */
		errno = EEXIST;
		return (mqd_t)mqd;
	}

	if ((msg_queue == NULL) && (oflags & O_CREAT) == 0) {
		errno = ENOENT;
		return (mqd_t)mqd;
	}

	mq_desc_ptr = k_malloc(sizeof(struct mqueue_desc));
	if (mq_desc_ptr != NULL) {
		(void)memset(mq_desc_ptr, 0, sizeof(struct mqueue_desc));
		msg_queue_desc = (struct mqueue_desc *)mq_desc_ptr;
		msg_queue_desc->mem_desc = mq_desc_ptr;
	} else {
		goto free_mq_desc;
	}


	/* Allocate mqueue object for new message queue */
	if (msg_queue == NULL) {
		size_t buf_size;

		/* Check for message quantity and size in message queue */
		if (attrs->mq_msgsize > CONFIG_MSG_SIZE_MAX ||
		    attrs->mq_maxmsg > CONFIG_POSIX_MQ_OPEN_MAX) {
			goto free_mq_desc;
		}

		mq_obj_ptr = k_malloc(sizeof(mqueue_object));
		if (mq_obj_ptr != NULL) {
			(void)memset(mq_obj_ptr, 0, sizeof(mqueue_object));
			msg_queue = (mqueue_object *)mq_obj_ptr;
			msg_queue->mem_obj = mq_obj_ptr;

		} else {
			goto free_mq_object;
		}

		mq_name_ptr = k_malloc(strlen(name) + 1);
		if (mq_name_ptr != NULL) {
			(void)memset(mq_name_ptr, 0, strlen(name) + 1);
			msg_queue->name = mq_name_ptr;

		} else {
			goto free_mq_name;
		}

		strcpy(msg_queue->name, name);

		if (size_mul_overflow((size_t)msg_size, (size_t)max_msgs, &buf_size)) {
			goto free_mq_buffer;
		}

		mq_buf_ptr = k_malloc(buf_size);
		if (mq_buf_ptr != NULL) {
			(void)memset(mq_buf_ptr, 0, buf_size);
			msg_queue->mem_buffer = mq_buf_ptr;
		} else {
			goto free_mq_buffer;
		}

		(void)atomic_set(&msg_queue->ref_count, 1);
		/* initialize zephyr message queue */
		k_msgq_init(&msg_queue->queue, msg_queue->mem_buffer, msg_size,
			    max_msgs);
		k_sem_take(&mq_sem, K_FOREVER);
		sys_slist_append(&mq_list, (sys_snode_t *)&(msg_queue->snode));
		k_sem_give(&mq_sem);

	} else {
		atomic_inc(&msg_queue->ref_count);
	}

	msg_queue_desc->mqueue = msg_queue;
	msg_queue_desc->flags = (oflags & O_NONBLOCK) != 0 ? O_NONBLOCK : 0;
	return (mqd_t)msg_queue_desc;

free_mq_buffer:
	k_free(mq_name_ptr);
free_mq_name:
	k_free(mq_obj_ptr);
free_mq_object:
	k_free(mq_desc_ptr);
free_mq_desc:
	errno = ENOSPC;
	return (mqd_t)mqd;
}

/**
 * @brief Close a message queue descriptor.
 *
 * See IEEE 1003.1
 */
int mq_close(mqd_t mqdes)
{
	mqueue_desc *mqd = (mqueue_desc *)mqdes;

	if (mqd == NULL) {
		errno = EBADF;
		return -1;
	}

	/* a notification registered through this descriptor is removed */
	if (mqd->mqueue->notify_mqd == mqd) {
		remove_notification(mqd->mqueue);
	}

	atomic_dec(&mqd->mqueue->ref_count);

	/* remove mq if marked for unlink */
	if (mqd->mqueue->name == NULL) {
		remove_mq(mqd->mqueue);
	}

	k_free(mqd->mem_desc);
	return 0;
}

/**
 * @brief Remove a message queue.
 *
 * See IEEE 1003.1
 */
int mq_unlink(const char *name)
{
	mqueue_object *msg_queue;

	k_sem_take(&mq_sem, K_FOREVER);
	msg_queue = find_in_list(name);

	if (msg_queue == NULL) {
		k_sem_give(&mq_sem);
		errno = EBADF;
		return -1;
	}

	k_free(msg_queue->name);
	msg_queue->name = NULL;
	k_sem_give(&mq_sem);
	remove_mq(msg_queue);
	return 0;
}

/**
 * @brief Send a message to a message queue.
 *
 * All messages in message queue are of equal priority.
 *
 * See IEEE 1003.1
 */
int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
	    unsigned int msg_prio)
{
	mqueue_desc *mqd = (mqueue_desc *)mqdes;

	return send_message(mqd, msg_ptr, msg_len, K_FOREVER);
}

/**
 * @brief Send message to a message queue within abstime time.
 *
 * All messages in message queue are of equal priority.
 *
 * See IEEE 1003.1
 */
int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
		 unsigned int msg_prio, const struct timespec *abstime)
{
	mqueue_desc *mqd = (mqueue_desc *)mqdes;

	if ((abstime == NULL) || !timespec_is_valid(abstime)) {
		errno = EINVAL;
		return -1;
	}

	return send_message(mqd, msg_ptr, msg_len,
			    sys_timepoint_timeout(timespec_abs_rt_to_timepoint(abstime)));
}

/**
 * @brief Receive a message from a message queue.
 *
 * All messages in message queue are of equal priority.
 *
 * See IEEE 1003.1
 */
int mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
		   unsigned int *msg_prio)
{
	mqueue_desc *mqd = (mqueue_desc *)mqdes;

	return receive_message(mqd, msg_ptr, msg_len, K_FOREVER);
}

/**
 * @brief Receive message from a message queue within abstime time.
 *
 * All messages in message queue are of equal priority.
 *
 * See IEEE 1003.1
 */
int mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
			unsigned int *msg_prio, const struct timespec *abstime)
{
	mqueue_desc *mqd = (mqueue_desc *)mqdes;

	if ((abstime == NULL) || !timespec_is_valid(abstime)) {
		errno = EINVAL;
		return -1;
	}

	return receive_message(mqd, msg_ptr, msg_len,
			       sys_timepoint_timeout(timespec_abs_rt_to_timepoint(abstime)));
}

/**
 * @brief Get message queue attributes.
 *
 * See IEEE 1003.1
 */
int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat)
{
	mqueue_desc *mqd = (mqueue_desc *)mqdes;
	struct k_msgq_attrs attrs;

	if (mqd == NULL) {
		errno = EBADF;
		return -1;
	}

	k_sem_take(&mq_sem, K_FOREVER);
	k_msgq_get_attrs(&mqd->mqueue->queue, &attrs);
	mqstat->mq_flags = mqd->flags;
	mqstat->mq_maxmsg = attrs.max_msgs;
	mqstat->mq_msgsize = attrs.msg_size;
	mqstat->mq_curmsgs = attrs.used_msgs;
	k_sem_give(&mq_sem);
	return 0;
}

/**
 * @brief Set message queue attributes.
 *
 * See IEEE 1003.1
 */
int mq_setattr(mqd_t mqdes, const struct mq_attr *mqstat,
	       struct mq_attr *omqstat)
{
	mqueue_desc *mqd = (mqueue_desc *)mqdes;

	if (mqd == NULL) {
		errno = EBADF;
		return -1;
	}

	if (mqstat->mq_flags != 0 && mqstat->mq_flags != O_NONBLOCK) {
		errno = EINVAL;
		return -1;
	}

	if (omqstat != NULL) {
		mq_getattr(mqdes, omqstat);
	}

	k_sem_take(&mq_sem, K_FOREVER);
	mqd->flags = mqstat->mq_flags;
	k_sem_give(&mq_sem);

	return 0;
}

/**
 * @brief Notify process that a message is available.
 *
 * See IEEE 1003.1
 */
int mq_notify(mqd_t mqdes, const struct sigevent *notification)
{
	mqueue_desc *mqd = (mqueue_desc *)mqdes;
	struct mq_notify_job *job = NULL;

	if (mqd == NULL) {
		errno = EBADF;
		return -1;
	}

	mqueue_object *msg_queue = mqd->mqueue;

	if (notification == NULL) {
		/* removing a non-existent registration is a harmless no-op */
		remove_notification(msg_queue);
		return 0;
	}

	switch (notification->sigev_notify) {
	case SIGEV_NONE:
		break;
	case SIGEV_SIGNAL:
		errno = ENOSYS;
		return -1;
	case SIGEV_THREAD:
		if (notification->sigev_notify_function == NULL) {
			errno = EINVAL;
			return -1;
		}
		job = k_malloc(sizeof(*job));
		if (job == NULL) {
			errno = ENOMEM;
			return -1;
		}
		job->func = notification->sigev_notify_function;
		job->value = notification->sigev_value;
		/*
		 * The notification thread is never joinable: force the detach
		 * state on a private copy of the caller's attributes.
		 */
		if (notification->sigev_notify_attributes != NULL) {
			job->attr = *(pthread_attr_t *)notification->sigev_notify_attributes;
		} else {
			(void)pthread_attr_init(&job->attr);
		}
		(void)pthread_attr_setdetachstate(&job->attr, PTHREAD_CREATE_DETACHED);
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	k_sem_take(&mq_sem, K_FOREVER);
	if (msg_queue->not.sigev_notify != 0) {
		k_sem_give(&mq_sem);
		if (job != NULL) {
			(void)pthread_attr_destroy(&job->attr);
			k_free(job);
		}
		errno = EBUSY;
		return -1;
	}
	msg_queue->not = *notification;
	msg_queue->notify_mqd = mqd;
	msg_queue->job = job;
	k_sem_give(&mq_sem);

	return 0;
}

static void *mq_notify_thread(void *arg)
{
	struct mq_notify_job *job = arg;
	void (*func)(union sigval) = job->func;
	union sigval value = job->value;

	(void)pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	(void)pthread_attr_destroy(&job->attr);
	k_free(job);

	func(value);

	return NULL;
}

/* Internal functions */
static mqueue_object *find_in_list(const char *name)
{
	sys_snode_t *mq;
	mqueue_object *msg_queue;

	mq = mq_list.head;

	while (mq != NULL) {
		msg_queue = (mqueue_object *)mq;
		if ((msg_queue->name != NULL) && (strcmp(msg_queue->name, name) == 0)) {
			return msg_queue;
		}

		mq = mq->next;
	}

	return NULL;
}

static int32_t send_message(mqueue_desc *mqd, const char *msg_ptr, size_t msg_len,
			  k_timeout_t timeout)
{
	int32_t ret = -1;

	if (mqd == NULL) {
		errno = EBADF;
		return ret;
	}

	if ((mqd->flags & O_NONBLOCK) != 0U) {
		timeout = K_NO_WAIT;
	}

	if (msg_len >  mqd->mqueue->queue.msg_size) {
		errno = EMSGSIZE;
		return ret;
	}

	uint32_t used_before = k_msgq_num_used_get(&mqd->mqueue->queue);

	if (k_msgq_put(&mqd->mqueue->queue, (void *)msg_ptr, timeout) != 0) {
		errno = K_TIMEOUT_EQ(timeout, K_NO_WAIT) ? EAGAIN : ETIMEDOUT;
		return ret;
	}

	/*
	 * The notification fires when a message arrives on a previously empty
	 * queue and no thread is blocked in mq_receive(); a blocked receiver
	 * takes the message directly, leaving the used count at 0. The pre-put
	 * sample is taken outside the lock, so a sender racing a receiver at
	 * the 0/1 boundary may see or miss the transition.
	 */
	if ((used_before == 0) && (k_msgq_num_used_get(&mqd->mqueue->queue) > 0)) {
		mqueue_object *mq = mqd->mqueue;
		struct sigevent sev = {0};
		struct mq_notify_job *job = NULL;

		k_sem_take(&mq_sem, K_FOREVER);
		if (mq->not.sigev_notify != 0) {
			/*
			 * The event consumes the registration before anything
			 * is delivered; for SIGEV_NONE nothing is delivered at
			 * all.
			 */
			sev = mq->not;
			job = mq->job;
			mq->job = NULL;
			mq->notify_mqd = NULL;
			(void)memset(&mq->not, 0, sizeof(mq->not));
		}
		k_sem_give(&mq_sem);

		if (sev.sigev_notify == SIGEV_THREAD) {
			pthread_t th;

			if (pthread_create(&th, &job->attr, mq_notify_thread, job) != 0) {
				/* the notification is lost; the send still succeeded */
				(void)pthread_attr_destroy(&job->attr);
				k_free(job);
			}
		}
	}

	return 0;
}

static int32_t receive_message(mqueue_desc *mqd, char *msg_ptr, size_t msg_len,
			     k_timeout_t timeout)
{
	int ret = -1;

	if (mqd == NULL) {
		errno = EBADF;
		return ret;
	}

	if (msg_len < mqd->mqueue->queue.msg_size) {
		errno = EMSGSIZE;
		return ret;
	}

	if ((mqd->flags & O_NONBLOCK) != 0U) {
		timeout = K_NO_WAIT;
	}

	if (k_msgq_get(&mqd->mqueue->queue, (void *)msg_ptr, timeout) != 0) {
		errno = K_TIMEOUT_EQ(timeout, K_NO_WAIT) ? EAGAIN : ETIMEDOUT;
	} else {
		ret = mqd->mqueue->queue.msg_size;
	}

	return ret;
}

static void remove_mq(mqueue_object *msg_queue)
{
	if (atomic_cas(&msg_queue->ref_count, 0, 0)) {
		k_sem_take(&mq_sem, K_FOREVER);
		sys_slist_find_and_remove(&mq_list, (sys_snode_t *) msg_queue);
		k_sem_give(&mq_sem);

		/* free a registration that never fired */
		remove_notification(msg_queue);

		/* Free mq buffer and pbject */
		k_free(msg_queue->mem_buffer);
		k_free(msg_queue->mem_obj);
	}
}

static void remove_notification(mqueue_object *msg_queue)
{
	struct mq_notify_job *job;

	k_sem_take(&mq_sem, K_FOREVER);
	job = msg_queue->job;
	msg_queue->job = NULL;
	msg_queue->notify_mqd = NULL;
	(void)memset(&msg_queue->not, 0, sizeof(msg_queue->not));
	k_sem_give(&mq_sem);

	if (job != NULL) {
		(void)pthread_attr_destroy(&job->attr);
		k_free(job);
	}
}
