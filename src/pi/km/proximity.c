/**
 * @file   proximity.c
 *
 * @brief  driver for distance sensors
 *
 * @author Michael You    <myou@andrew.cmu.edu>
 *         Benjamin Huang <zemingbh@andrew.cmu.edu>
 */

#include "motor_driver.h"    // for constant and func. declarations

/** @brief   Sets up the trigger GPIO pins.
 *
 *  @return  0 on success, negative error code otherwise
 */
int gpio_trig_start(void);

/** @brief   Releases the trigger GPIO pins.
 */
void gpio_trig_stop(void);

/** @brief file operations we wish to associate with our device */
static struct file_operations fops =
{
  .open    = dev_open,
  .read    = dev_read,
  .write   = dev_write,
  .release = dev_release,
};

// variables for the motor class
/** @brief  device major number */
static int majorNumber;
/** @brief  device minor number */
static int minorNumber = 0;
/** @brief  struct for the motor class */
static struct class* proximityClass = NULL;
/** @brief  buffer for messages */
static char message[MESSAGE_WIDTH] = {0};
/** @brief  reports to us the size of the message we sent */
static short messageSize;
/** @brief  keeps track of how many times the device has been opened */
static int deviceOpenCount = 0;
/** @brief  struct for the motor device */
static struct device* proximityDevice = NULL;

/** @brief var to keep track of the motor state */
volatile static int motorState = STATE_STOP; // default motor is off
/** @brief var that controls the speed, or PWM rate of motor */
volatile static int motorSpeed = 0;
/** @brief keep track of the motor position */
volatile static int motorPos = 0;
/** @brief keep track of the rotary position */
volatile static int rotPos = 0;
/** @brief keep track of which direction we want to move the motor */
volatile static int direction = 0;

/** @brief timer struct */
static struct hrtimer hr_timer;
/** @brief kernel time struct */
volatile ktime_t ktime;


/** @brief Module info: license */
MODULE_LICENSE("GPL");
/** @brief Module info: author(s) */
MODULE_AUTHOR("CycleSafe");
/** @brief Module info: description */
MODULE_DESCRIPTION("Interrupt driver for HC-SR04 Sensors");
/** @brief Module info: version */
MODULE_VERSION("1.0");

/** @brief The LKM init function */
static int __init prox_init(void) {

  int status;

  printk(KERN_INFO "Prox: Initializing...\n");

  // Allocate a major number for the device
  majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

  if (majorNumber < 0) {
    printk(KERN_ALERT "Prox: register_chrdev() error. Failed to register a major number!\n");
    return majorNumber;
  }
  printk(KERN_INFO "Prox: Registered major number %d.\n", majorNumber);

  // Register the device class
  proximityClass = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(proximityClass)) {
    printk(KERN_ALERT "Prox: class_create() error. Failed to register device class!\n");
    unregister_chrdev(majorNumber, DEVICE_NAME);
    return PTR_ERR(proximityClass);
  }
  printk(KERN_INFO "Prox: Device class registered correctly.\n");

  // Create device class
  proximityDevice = device_create(proximityClass, NULL, MKDEV(majorNumber, minorNumber), NULL, DEVICE_NAME);
  if (IS_ERR(proximityDevice)) {
    printk(KERN_ALERT "Prox: device_create() error. Failed to create the device!\n");
    class_destroy(proximityClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    return PTR_ERR(proximityDevice);
  }
  printk(KERN_INFO "Prox: Device class created.\n");

  // Setup GPIO pins
  status = gpio_trig_start();
  if (status) {
    printk(KERN_ALERT "Prox: gpio_trig_start() error!\n");
    device_destroy(proximityClass, MKDEV(majorNumber, minorNumber));
    class_destroy(proximityClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    return PTR_ERR(proximityDevice);
  }
  printk(KERN_INFO "Prox: GPIO setup complete.\n");

  // Setup GPIO pins
  status = gpio_echo_start();
  if (status) {
    printk(KERN_ALERT "Prox: gpio_echo_start() error!\n");
    gpio_trig_stop();
    device_destroy(proximityClass, MKDEV(majorNumber, minorNumber));
    class_destroy(proximityClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    return PTR_ERR(proximityDevice);
  }
  printk(KERN_INFO "Prox: GPIO setup complete.\n");


  // Setup GPIO pins
  status = gpio_irq_start();
  if (status) {
    printk(KERN_ALERT "Prox: gpio_irq_start() error!\n");
    gpio_echo_stop();
    gpio_trig_stop();
    device_destroy(proximityClass, MKDEV(majorNumber, minorNumber));
    class_destroy(proximityClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    return PTR_ERR(proximityDevice);
  }
  printk(KERN_INFO "Prox: GPIO setup complete.\n");



  printk(KERN_INFO "Prox: Initialization complete.\n");
  return 0;
}

/** @brief The LKM cleanup function
 */
static void __exit prox_exit(void) {

  printk(KERN_INFO "Prox: Exiting...\n");

  gpio_trig_stop();

  device_destroy(proximityClass, MKDEV(majorNumber, minorNumber));
  class_destroy(proximityClass);
  unregister_chrdev(majorNumber, DEVICE_NAME);

  printk(KERN_INFO "Prox: Exit completed.\n");

}

static int dev_open(struct inode* inodep, struct file* filep) {

  deviceOpenCount++;
  return 0;

}

static int dev_release(struct inode* inodep, struct file* filep) {

  deviceOpenCount--;
  return 0;

}

static ssize_t dev_read(struct file* filep, char* buffer, size_t len, loff_t* offset) {
  int errorCount = 0;

  // Send the rotary and motor encoder position to the user
  sprintf(message, "%d|%d", rotPos, motorPos);
  messageSize = strlen(message);
  errorCount = copy_to_user(buffer, message, messageSize+1);

  if (errorCount == 0) {
    return messageSize; // clear the messageSize
  } else {
    printk_ratelimited(KERN_INFO "Motor: Failed to send %d characters to the user\n", errorCount);
    return -EFAULT; // return bad address message
  }
}

static ssize_t dev_write(struct file* filep, const char* buffer, size_t len, loff_t* offset) {
  int num, tempDir;
  int res;

  sprintf(message, "%s", buffer);
  messageSize = strlen(message);

  // change motor speed based on command
  res = sscanf(buffer, "%d|%d", &num, &tempDir);

  if (res != 2) {
    printk(KERN_INFO "invalid write\n");
    return -1;
  }

  if (num > TIMER_RANGE) {
    printk(KERN_INFO "Speed must be between 0 and 100 (inclusive)\n");
  } else {
    motorSpeed = num;
  }

  if (tempDir != CW && tempDir != CCW) {
    printk(KERN_INFO "Direction must be either 0 or 1\n");
  } else {
    direction = tempDir;
  }

  // Tell the user the motor speed

  return len;
}

int gpio_trig_start(void) {

  int status = 0;
  int i = 0;
  while (i < SR_COUNT) {

    // Request the GPIO pin
    status = gpio_request(GPIO_TRIG_PIN[i], GPIO_TRIG_PIN_NAME[i]);
    if (status < 0) {
      printk(KERN_ALERT "Prox: gpio_request() error on trigger pin index %d!\n", i);
      break;
    }

    // Set to output low
    status = gpio_direction_output(GPIO_TRIG_PIN[i], false);
    if (status < 0) {
      printk(KERN_ALERT "Prox: gpio_direction_output() error on trigger pin index %d!\n", i);
      gpio_free(GPIO_TRIG_PIN[i]);
      break;
    }

    i++;

  }

  if (status < 0) {
    while (i > 0) {
      i--;
      gpio_free(GPIO_TRIG_PIN[i]);
    }
    return status;
  }

  return status;

}

void gpio_trig_stop(void) {
  int i = 0;
  while (i < SR_COUNT) {
    gpio_free(GPIO_TRIG_PIN[i]);
    i++;
  }
}

int gpio_echo_start(void) {

  int status = 0;
  int i = 0;

  while (i < SR_COUNT) {

    // Request the GPIO pin
    status = gpio_request(GPIO_ECHO_PIN[i], GPIO_ECHO_PIN_NAME[i]);
    if (status < 0) {
      printk(KERN_ALERT "Prox: gpio_request() error on echo pin index %d!\n", i);
      break;
    }

    // Set to input
    status = gpio_direction_input(GPIO_ECHO_PIN[i]);
    if (status < 0) {
      printk(KERN_ALERT "Prox: gpio_direction_input() error on echo pin index %d!\n", i);
      gpio_free(GPIO_ECHO_PIN[i]);
      break;
    }

    /*
    status = gpio_set_debounce(GPIO_ECHO_PIN[i], GPIO_ECHO_DEBOUNCE_MS);
    if (status < 0) {
      printk(KERN_ALERT "Prox: gpio_set_debounce() error on echo pin index %d!\n", i);
      gpio_free(GPIO_ECHO_PIN[i]);
      break;
    }
    */

    i++;
  }

  if (status < 0) {
    while (i > 0) {
      i--;
      gpio_free(GPIO_ECHO_PIN[i]);
    }
    return status;
  }

}

void gpio_echo_stop(void) {
  int i = 0;
  while (i > SR_COUNT) {
    gpio_free(GPIO_ECHO_PIN[i]);
    i++;
  }
}

void gpio_echo_irq_start(void) {

  int i = 0;
  while (i > SR_COUNT) {

    int irqNum = gpio_to_irq(GPIO_ECHO_PIN[i]);
    request_irq(irqNum,
      proxIrqHandler,
      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
      "proxIrqHandler",
      NULL
    );

    i++;
  }


}
/*
void gpio_input_setup(void) {
  int resMotor;
  int resRot;
  int irqNumMotor;
  int irqNumRot;


  irqNumMotor = gpio_to_irq(MOTOR_ENC_B);
  irqNumRot = gpio_to_irq(ROT_ENC_B);

  resMotor = request_irq(irqNumMotor,
                         (irq_handler_t) motor_enc_handler,
                         IRQF_TRIGGER_RISING,
                         "motor_enc_handler",
                         NULL);

  if (resMotor) {
    printk(KERN_INFO "Failed to set up IRQ handler for motor encoder\n");
  }

  resRot = request_irq(irqNumRot,
                       (irq_handler_t) rot_enc_handler,
                       IRQF_TRIGGER_RISING,
                       "rot_enc_handler",
                       NULL);
  if (resRot) {
    printk(KERN_INFO "Failed to set up IRQ handler for rot encoder\n");
  }
}

void timer_setup(void) {
  ktime = ktime_set(0, TIMER_TICK*TIMER_RANGE); // initialize ktime
  hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
  hr_timer.function = &my_hrtimer_callback;
  hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL); // start the timer
}

enum hrtimer_restart my_hrtimer_callback(struct hrtimer* timer) {
  // On cycle of PWM
  ktime_t currTime = ktime_get();

  // Figure out PWM settings
  if (motorSpeed <= 0) {
    motorState = STATE_STOP;
    ktime = ktime_set(0, TIMER_TICK*TIMER_RANGE);
    gpio_set_value(MOTOR_EN_PIN, false); // turn off PWM
  } else if (motorSpeed >= 100) {
    motorState = STATE_ON;
    ktime = ktime_set(0, TIMER_TICK*TIMER_RANGE);
    gpio_set_value(MOTOR_EN_PIN, true); // turn on PWM
  } else if (motorState == STATE_STOP) {
    motorState = STATE_ON;
    gpio_set_value(MOTOR_EN_PIN, true); // turn on PWM
    ktime = ktime_set(0, TIMER_TICK*motorSpeed); // on for motorSpeed time
  } else {
    motorState = STATE_STOP;
    gpio_set_value(MOTOR_EN_PIN, false); // turn off PWM
    ktime = ktime_set(0, TIMER_TICK*(TIMER_RANGE - motorSpeed)); // off for rest of cycle
  }

  // Figure out direction
  if (direction == CW) {
    // motor needs to turn cwise
    gpio_set_value(MOTOR_IN_1, MOTOR_IN_1_CW);
    gpio_set_value(MOTOR_IN_2, MOTOR_IN_2_CW);
  } else {
    // motor needs to turn ccwise
    gpio_set_value(MOTOR_IN_1, MOTOR_IN_1_CCW);
    gpio_set_value(MOTOR_IN_2, MOTOR_IN_2_CCW);
  }

  hrtimer_forward(&hr_timer, currTime, ktime);

  return HRTIMER_RESTART;              // DO reset the timer
}

static irq_handler_t rot_enc_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) {
  if (gpio_get_value(ROT_ENC_A)) {
    // clockwise
    rotPos += ENC_INCREMENT;
  } else {
    // counter-clockwise
    rotPos -= ENC_INCREMENT;
  }

  return (irq_handler_t) IRQ_HANDLED;
}

static irq_handler_t motor_enc_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) {
  if (gpio_get_value(MOTOR_ENC_A)) {
    // clockwise
    motorPos++;
  } else {
    // counter-clockwise
    motorPos--;
  }

  return (irq_handler_t) IRQ_HANDLED;
}
*/

// call module_init, module_exit modules from linux/init.h
/** @brief attach motor init function */
module_init(prox_init);
/** @brief attach motor exit function */
module_exit(prox_exit);
