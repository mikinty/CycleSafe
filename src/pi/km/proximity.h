/**
 * @file   proximity.h
 *
 * @brief  Header file for proximity sensors driver.
 *
 * @date   3/16/2019
 * @author Benjamin Huang <zemingbh@andrew.cmu.edu>
 */

#ifndef _PROXIMITY_H_
#define _PROXIMITY_H_

#include <linux/init.h>      // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>    // Core header for loading LKMs into the kernel
#include <linux/device.h>    // Header to support the kernel Driver Model
#include <linux/kernel.h>    // Contains types, macros, functions for the kernel
#include <linux/fs.h>        // Header for the Linux file system support
#include <linux/gpio.h>      // Required for GPIO functions
#include <linux/interrupt.h> // Required for the IRQ code
#include <asm/uaccess.h>     // Required for the copy to user function
#include <linux/io.h>        // for iore/unmap()
#include <linux/hrtimer.h>   // linux timer
#include <linux/ktime.h>     // kernel time

/** @brief device name as it would appear under /dev/ */
#define DEVICE_NAME "proximity"
/** @brief name of the class */
#define CLASS_NAME  "proximityClass"

/** @brief number of sensors */
#define SR_COUNT 6

// PIN NUMBERS
/** @brief trigger pin numbers */
#define GPIO_TRIG_PIN {16, 18, 20, 22, 24, 26}
/** @brief echo pin numbers */
#define GPIO_ECHO_PIN {17, 19, 21, 23, 25, 27}
/** @brief trigger pin numbers */
#define GPIO_TRIG_PIN_NAME {"ProxTrig0", "ProxTrig1", "ProxTrig2", "ProxTrig3", "ProxTrig4", "ProxTrig5"}
/** @brief echo pin numbers */
#define GPIO_ECHO_PIN_NAME {"ProxEcho0", "ProxEcho1", "ProxEcho2", "ProxEcho3", "ProxEcho4", "ProxEcho5"}

/** @brief how many moves the motor should make for one encoder tick */
#define ENC_INCREMENT 1
/** @brief Rate of input debouncing in ms */
#define GPIO_ECHO_DEBOUNCE_MS 2

/** @brief the upper bound for the timer */
#define TIMER_RANGE 100
/** @brief length in ns of one timer tick */
#define TIMER_TICK  200000
/** @brief length of the message */
#define MESSAGE_WIDTH 30


/** @brief The function that is called each time the device is opened
 *
 *  @param inodep stores file information
 *  @param filep  file descriptor pointer
 */
static int dev_open(struct inode* inodep, struct file* filep);


/** @brief This function is claled whenever the device is read from the
 *         user space, i.e. data is being sent from the device to the user.
 *
 *  @param filep  file descriptor pointer
 *  @param buffer the buffer to read from
 *  @param len    the number of chars to read from buffer
 *  @param offset
 */
static ssize_t dev_read(struct file* filep, char* buffer, size_t len, loff_t* offset);


/** @brief This function is called whenever the device is being written to,
 *         i.e. data is sent to the device from the user.
 *
 *  @param filep  file descriptor pointer
 *  @param buffer the buffer to read from
 *  @param len    the number of chars to read from buffer
 *  @param offset file offset
 */
static ssize_t dev_write(struct file* filep, const char* buffer, size_t len, loff_t* offset);


/** @brief The function that is called whenever the device is closed/released
 *         by the userspace program.
 *
 *  @param inodep stores file information
 *  @param filep  file descriptor pointer
 */
static int dev_release(struct inode* inodep, struct file* filep);

#endif /* _PROXIMITY_H_ */
