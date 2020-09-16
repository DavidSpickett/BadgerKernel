#ifndef KERNEL_MESSAGE_H
#define KERNEL_MESSAGE_H

#include <stdbool.h>

bool k_get_msg(int* sender, int* message);
bool k_send_msg(int destination, int message);

#endif /* ifdef KERNEL_MESSAGE_H */
