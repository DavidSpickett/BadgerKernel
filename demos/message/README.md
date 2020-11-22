# Message

In this demo we will show the message passing mechanism. Since messages are not sharable to other threads, this is a way for a thread to communicate with others privately.

To explain this demo, we will first introduce the following:

* `send_msg` (in `src/user/thread.c`)
* `get_msg` (in `src/user/thread.c`)

## `send_msg`

In AMT, each thread owns a ring buffer to store pending messages. When this function is called, the kernel will check the corresponding buffer of the destination thread.

If the buffer is not full, the message will be copied into the destination thread's buffer according to the `destination` and `message` we specified and remains there until the thread takes it out.

If the buffer is full, the kernel will refuse to send the message, and thus the return value of this function will be `false`.

## `get_msg`

When a thread calls this function, the kernel checks whether there is a message in the buffer.

If the buffer is non-empty, the kernel will copy the information of the message to the variables we gave and remove it from the message buffer. So we can view the `content` and `src` variables to know the content and which thread sent this message.

## Walkthrough

First, the `setup` thread creates two threads `sender` and `receiver` then it jumps to the `spammer` function.

`spammer` doesn't end or give away the use of the CPU, so it repeatedly sends messages to `receiver`, which makes `receiver`'s message buffer full. As a result, when `sender` runs, it fails to send a message to `receiver`.

When `receiver` runs, it drops the messages which are not from `sender`. Since the message buffer only contains the messages from `spammer`, they will all be dropped and `receiver` will yield.

`spammer` will clog `receiver`'s buffer again and the process repeats. Finally `spammer` will send only one message, then exit. At this time, `sender` can send the message to `receiver` as the buffer is not full. So `receiver` will get the message sent from `sender` and end the demo.
