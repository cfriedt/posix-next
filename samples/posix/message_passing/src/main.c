/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * POSIX message queue walk-through: create a named queue, exchange
 * prioritised messages, and receive asynchronous notification of message
 * arrival - by signal, and by function call with the classic
 * re-register-then-drain idiom.
 */

#include <fcntl.h>
#include <mqueue.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define QUEUE_NAME "/inbox"
#define MSG_SIZE   48
#define MAX_MSGS   8

static sem_t drained;

static void print_all_pending(mqd_t mqd)
{
	char msg[MSG_SIZE];
	unsigned int prio;
	ssize_t len;

	while ((len = mq_receive(mqd, msg, sizeof(msg), &prio)) >= 0) {
		printf("received [prio %u]: %.*s\n", prio, (int)len, msg);
	}
}

/*
 * SIGEV_THREAD notification function. A registration is consumed when it
 * fires, and messages that arrive while the queue is non-empty do not fire
 * at all - so the robust idiom is: re-register first, then drain the queue
 * with non-blocking receives.
 */
static void on_message(union sigval val)
{
	mqd_t mqd = (mqd_t)val.sival_int;
	struct mq_attr attr;
	struct sigevent sev = {
		.sigev_notify = SIGEV_THREAD,
		.sigev_notify_function = on_message,
		.sigev_value.sival_int = (int)mqd,
	};
	char msg[MSG_SIZE];
	unsigned int prio;
	ssize_t len;

	if (mq_notify(mqd, &sev) == -1) {
		perror("mq_notify");
		return;
	}

	if (mq_getattr(mqd, &attr) == 0) {
		printf("notified: %ld message(s) pending\n", attr.mq_curmsgs);
	}

	while ((len = mq_receive(mqd, msg, sizeof(msg), &prio)) >= 0) {
		printf("drained [prio %u]: %.*s\n", prio, (int)len, msg);
	}

	sem_post(&drained);
}

int main(void)
{
	struct mq_attr attr = {
		.mq_msgsize = MSG_SIZE,
		.mq_maxmsg = MAX_MSGS,
	};
	struct sigevent sev;
	mqd_t mqd;

	(void)sem_init(&drained, 0, 0);

	/* a named queue exists from mq_open() until mq_unlink() */
	mqd = mq_open(QUEUE_NAME, O_RDWR | O_CREAT | O_NONBLOCK, 0600, &attr);
	if (mqd == (mqd_t)-1) {
		perror("mq_open");
		return 1;
	}

	/*
	 * Messages are delivered in descending priority order, and in FIFO
	 * order among messages of equal priority - not in arrival order.
	 */
	(void)mq_send(mqd, "newsletter", strlen("newsletter"), 0);
	(void)mq_send(mqd, "reminder: standup at 10", strlen("reminder: standup at 10"), 3);
	(void)mq_send(mqd, "urgent: fire drill", strlen("urgent: fire drill"), 6);
	(void)mq_send(mqd, "reminder: lunch at noon", strlen("reminder: lunch at noon"), 3);

	print_all_pending(mqd);

	/*
	 * mq_notify() fires once, when a message arrives at the empty queue,
	 * and runs the function in a new thread.
	 */
	sev = (struct sigevent){
		.sigev_notify = SIGEV_THREAD,
		.sigev_notify_function = on_message,
		.sigev_value.sival_int = (int)mqd,
	};
	if (mq_notify(mqd, &sev) == -1) {
		perror("mq_notify");
		return 1;
	}

	(void)mq_send(mqd, "notify works", strlen("notify works"), 1);
	(void)sem_wait(&drained);

	/* the notification function re-registered, so arrival fires again */
	(void)mq_send(mqd, "and re-arms, too", strlen("and re-arms, too"), 1);
	(void)sem_wait(&drained);

	/* it re-registered once more; only one registration may be armed */
	(void)mq_notify(mqd, NULL);

	/*
	 * SIGEV_SIGNAL notification: arrival at the empty queue generates a
	 * signal carrying the registered value, with si_code SI_MESGQ. The
	 * signal is blocked and accepted synchronously with sigtimedwait().
	 */
	sigset_t set;
	siginfo_t info;
	const struct timespec forever = {.tv_sec = 3600, .tv_nsec = 0};

	(void)sigemptyset(&set);
	(void)sigaddset(&set, SIGUSR1);
	(void)sigprocmask(SIG_BLOCK, &set, NULL);

	sev = (struct sigevent){
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = SIGUSR1,
		.sigev_value.sival_int = 1234,
	};
	if (mq_notify(mqd, &sev) == -1) {
		perror("mq_notify");
		return 1;
	}

	(void)mq_send(mqd, "signalled, even", strlen("signalled, even"), 2);

	if (sigtimedwait(&set, &info, &forever) == SIGUSR1) {
		printf("signalled: SIGUSR1, %s, value %d\n",
		       (info.si_code == SI_MESGQ) ? "SI_MESGQ" : "unexpected si_code",
		       info.si_value.sival_int);
	}
	print_all_pending(mqd);

	if (mq_getattr(mqd, &attr) == 0) {
		printf("queue geometry: %ld x %ld bytes\n", attr.mq_maxmsg, attr.mq_msgsize);
	}

	(void)mq_notify(mqd, NULL);
	(void)mq_close(mqd);
	(void)mq_unlink(QUEUE_NAME);

	printf("done\n");

	return 0;
}
