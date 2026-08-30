/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX terminal interface (<termios.h>)
 *
 * Values follow the common Linux ABI.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/termios.h.html">
 *      POSIX.1-2017 &lt;termios.h&gt;</a>
 *
 */

#ifndef ZEPHYR_INCLUDE_POSIX_TERMIOS_H_
#define ZEPHYR_INCLUDE_POSIX_TERMIOS_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Terminal special-character and flag types
 * @ingroup posix_option_group_device_specific
 * @{
 */
/** @brief Type used for terminal special characters */
typedef unsigned char cc_t;
/** @brief Type used for terminal baud rates */
typedef unsigned int speed_t;
/** @brief Type used for terminal modes */
typedef unsigned int tcflag_t;
/** @} */

/**
 * @brief Size of the c_cc control-character array
 * @ingroup posix_option_group_device_specific
 */
#define NCCS 32

/**
 * @brief Terminal attributes.
 * @ingroup posix_option_group_device_specific
 */
struct termios {
	/** Input modes */
	tcflag_t c_iflag;
	/** Output modes */
	tcflag_t c_oflag;
	/** Control modes */
	tcflag_t c_cflag;
	/** Local modes */
	tcflag_t c_lflag;
	/** Line discipline */
	cc_t c_line;
	/** Control characters */
	cc_t c_cc[NCCS];
	/** Input baud rate */
	speed_t c_ispeed;
	/** Output baud rate */
	speed_t c_ospeed;
};

/**
 * @name Subscripts for the c_cc array
 * @ingroup posix_option_group_device_specific
 * @{
 */
#define VINTR 0    /**< INTR character */
#define VQUIT 1    /**< QUIT character */
#define VERASE 2   /**< ERASE character */
#define VKILL 3    /**< KILL character */
#define VEOF 4     /**< EOF character */
#define VTIME 5    /**< TIME value */
#define VMIN 6     /**< MIN value */
#define VSWTC 7    /**< SWTC character */
#define VSTART 8   /**< START character */
#define VSTOP 9    /**< STOP character */
#define VSUSP 10   /**< SUSP character */
#define VEOL 11    /**< EOL character */
#define VREPRINT 12 /**< REPRINT character */
#define VDISCARD 13 /**< DISCARD character */
#define VWERASE 14 /**< WERASE character */
#define VLNEXT 15  /**< LNEXT character */
#define VEOL2 16   /**< Alternate EOL character */
/** @} */

/**
 * @name Input modes (c_iflag)
 * @ingroup posix_option_group_device_specific
 * @{
 */
#define IGNBRK 0000001 /**< Ignore break condition */
#define BRKINT 0000002 /**< Signal interrupt on break */
#define IGNPAR 0000004 /**< Ignore characters with parity errors */
#define PARMRK 0000010 /**< Mark parity errors */
#define INPCK 0000020  /**< Enable input parity check */
#define ISTRIP 0000040 /**< Strip character */
#define INLCR 0000100  /**< Map NL to CR on input */
#define IGNCR 0000200  /**< Ignore CR */
#define ICRNL 0000400  /**< Map CR to NL on input */
#define IUCLC 0001000  /**< Map uppercase to lowercase on input */
#define IXON 0002000   /**< Enable start/stop output control */
#define IXANY 0004000  /**< Enable any character to restart output */
#define IXOFF 0010000  /**< Enable start/stop input control */
/** @} */

/**
 * @name Output modes (c_oflag)
 * @ingroup posix_option_group_device_specific
 * @{
 */
#define OPOST 0000001  /**< Post-process output */
#define OLCUC 0000002  /**< Map lowercase to uppercase on output */
#define ONLCR 0000004  /**< Map NL to CR-NL on output */
#define OCRNL 0000010  /**< Map CR to NL on output */
#define ONOCR 0000020  /**< No CR output at column 0 */
#define ONLRET 0000040 /**< NL performs CR function */
/** @} */

/**
 * @name Control modes (c_cflag)
 * @ingroup posix_option_group_device_specific
 * @{
 */
#define CSIZE 0000060  /**< Character size mask */
#define CS5 0000000    /**< 5 bits */
#define CS6 0000020    /**< 6 bits */
#define CS7 0000040    /**< 7 bits */
#define CS8 0000060    /**< 8 bits */
#define CSTOPB 0000100 /**< Send two stop bits, else one */
#define CREAD 0000200  /**< Enable receiver */
#define PARENB 0000400 /**< Parity enable */
#define PARODD 0001000 /**< Odd parity, else even */
#define HUPCL 0002000  /**< Hang up on last close */
#define CLOCAL 0004000 /**< Ignore modem status lines */
/** @} */

/**
 * @name Local modes (c_lflag)
 * @ingroup posix_option_group_device_specific
 * @{
 */
#define ISIG 0000001   /**< Enable signals */
#define ICANON 0000002 /**< Canonical input */
#define ECHO 0000010   /**< Enable echo */
#define ECHOE 0000020  /**< Echo erase character as error-correcting backspace */
#define ECHOK 0000040  /**< Echo KILL */
#define ECHONL 0000100 /**< Echo NL */
#define NOFLSH 0000200 /**< Disable flush after interrupt or quit */
#define TOSTOP 0000400 /**< Send SIGTTOU for background output */
#define IEXTEN 0100000 /**< Enable extended input character processing */
/** @} */

/**
 * @name Baud rates
 * @ingroup posix_option_group_device_specific
 * @{
 */
#define B0 0000000     /**< Hang up */
#define B50 0000001    /**< 50 baud */
#define B75 0000002    /**< 75 baud */
#define B110 0000003   /**< 110 baud */
#define B134 0000004   /**< 134.5 baud */
#define B150 0000005   /**< 150 baud */
#define B200 0000006   /**< 200 baud */
#define B300 0000007   /**< 300 baud */
#define B600 0000010   /**< 600 baud */
#define B1200 0000011  /**< 1200 baud */
#define B1800 0000012  /**< 1800 baud */
#define B2400 0000013  /**< 2400 baud */
#define B4800 0000014  /**< 4800 baud */
#define B9600 0000015  /**< 9600 baud */
#define B19200 0000016 /**< 19200 baud */
#define B38400 0000017 /**< 38400 baud */
#define B57600 0010001 /**< 57600 baud */
#define B115200 0010002 /**< 115200 baud */
/** @} */

/**
 * @name Attribute selection (tcsetattr)
 * @ingroup posix_option_group_device_specific
 * @{
 */
#define TCSANOW 0   /**< Change attributes immediately */
#define TCSADRAIN 1 /**< Change attributes when output has drained */
#define TCSAFLUSH 2 /**< Change attributes when output has drained; flush input */
/** @} */

/**
 * @name Queue selectors (tcflush)
 * @ingroup posix_option_group_device_specific
 * @{
 */
#define TCIFLUSH 0  /**< Flush pending input */
#define TCOFLUSH 1  /**< Flush untransmitted output */
#define TCIOFLUSH 2 /**< Flush both */
/** @} */

/**
 * @name Line control (tcflow)
 * @ingroup posix_option_group_device_specific
 * @{
 */
#define TCOOFF 0 /**< Suspend output */
#define TCOON 1  /**< Restart output */
#define TCIOFF 2 /**< Transmit a STOP character */
#define TCION 3  /**< Transmit a START character */
/** @} */

/**
 * @brief Get the input baud rate.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/cfgetispeed.html
 */
speed_t cfgetispeed(const struct termios *termios_p);

/**
 * @brief Get the output baud rate.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/cfgetospeed.html
 */
speed_t cfgetospeed(const struct termios *termios_p);

/**
 * @brief Set the input baud rate.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/cfsetispeed.html
 */
int cfsetispeed(struct termios *termios_p, speed_t speed);

/**
 * @brief Set the output baud rate.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/cfsetospeed.html
 */
int cfsetospeed(struct termios *termios_p, speed_t speed);

/**
 * @brief Wait until output has been transmitted.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/tcdrain.html
 */
int tcdrain(int fildes);

/**
 * @brief Suspend or restart data transmission or reception.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/tcflow.html
 */
int tcflow(int fildes, int action);

/**
 * @brief Discard non-transmitted output or non-read input.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/tcflush.html
 */
int tcflush(int fildes, int queue_selector);

/**
 * @brief Get the terminal attributes.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/tcgetattr.html
 */
int tcgetattr(int fildes, struct termios *termios_p);

/**
 * @brief Send a break to the terminal.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/tcsendbreak.html
 */
int tcsendbreak(int fildes, int duration);

/**
 * @brief Set the terminal attributes.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/tcsetattr.html
 */
int tcsetattr(int fildes, int optional_actions, const struct termios *termios_p);

/**
 * @brief Get the foreground process group of the terminal.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/tcgetpgrp.html
 */
pid_t tcgetpgrp(int fildes);

/**
 * @brief Set the foreground process group of the terminal.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/tcsetpgrp.html
 */
int tcsetpgrp(int fildes, pid_t pgid_id);

/**
 * @brief Get the session leader of the terminal's session.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/tcgetsid.html
 */
pid_t tcgetsid(int fildes);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_TERMIOS_H_ */
