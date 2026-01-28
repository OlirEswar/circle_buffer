#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>

// ring buffer struct with buffer and head/tail index
typedef struct {
    int *buf;
    size_t size;
    _Atomic size_t head;
    _Atomic size_t tail;
} CircleBuffer;

// number of iterations and delay time for producer and consumer threads
int PRODUCER_ITERATIONS;
int CONSUMER_ITERATIONS;
int PRODUCER_DELAY;
int CONSUMER_DELAY;

// function to get values from the ring buffer
int get(CircleBuffer *circleBuffer, int *value) {
    // if head and tail are the same, do not consume value
    if (atomic_load(&circleBuffer->head) == atomic_load(&circleBuffer->tail)) {
        return 0;
    // else store element in value and update head
    } else {
        *value = circleBuffer->buf[circleBuffer->head];
        atomic_store(&circleBuffer->head, (circleBuffer->head + 1) % circleBuffer->size); 
        return 1;
    }
}

// function to put values into the ring buffer
int put(CircleBuffer *circleBuffer, int value) {
    // next index of tail
    size_t nextTail = (atomic_load(&circleBuffer->tail) + 1) % circleBuffer->size;

    // if nextTail is the same as head, do not produce value
    if (nextTail == atomic_load(&circleBuffer->head)) {
        return 0;
    }

    // if nextTail is not the same as head, produce value and update tail
    circleBuffer->buf[atomic_load(&circleBuffer->tail)] = value;
    atomic_store(&circleBuffer->tail, nextTail);

    return 1;
}

// consumer thread
void* consume(void* arg) {
    // pointer to ring_buffer
    CircleBuffer *circleBuffer = (CircleBuffer*)arg;

    // variable to store consumed value
    int value;

    for (int i = 0; i <= CONSUMER_ITERATIONS; i++) {
        while(!get(circleBuffer, &value)) {
            // wait
        }
        printf("Consumed %d\n", value);
        usleep(CONSUMER_DELAY);
    }
    return NULL;
}

// producer thread
void* produce(void* arg) {
    // pointer to ring_buffer
    CircleBuffer *circleBuffer = (CircleBuffer*)arg;

    for (int i = 0; i <= PRODUCER_ITERATIONS; i++) {
        while(!put(circleBuffer, i)) {
            // wait
        }
        printf("Produced %d\n", i);
        usleep(PRODUCER_DELAY);
    }
    return NULL;
}


int main(int argc, char *argv[]) {
    // Create a pthread_t variable for producer and consumer threads
    pthread_t producer;
    pthread_t consumer;

    // Declare ring buffer
    CircleBuffer circleBuffer;

    // Default values for buffer size, producer and consumer iterations, and thread delays
    size_t bufferSize = 5;
    PRODUCER_ITERATIONS = 1000;
    CONSUMER_ITERATIONS = 1000;
    PRODUCER_DELAY = 1000;
    CONSUMER_DELAY = 1000;

    // cycle through command line arguments
    int *params[] = {&PRODUCER_ITERATIONS, &CONSUMER_ITERATIONS,
                        &PRODUCER_DELAY, &CONSUMER_DELAY};

    for (int i = 0; i < 4; i++) {
        if (argc >= i + 2) {
            *params[i] = atoi(argv[i+1]);
            // return if any parameters is less than 0 to avoid unpredictable errors
            if (*params[i] < 0) {
                printf("Parameters must be positive\n");
                return 1;
            }
        }
    }

    if (argc >= 6) {
        bufferSize = atoi(argv[5]);
        // return if buffer size is less than 2 to avoid unpredictable errors
        if (bufferSize < 2) {
            printf("Buffer size must be at least 2\n");
            return 1;
        }
    }

    //Allocate buffer and initialie ring buffer
    circleBuffer.buf = malloc(bufferSize * sizeof(int));
    if (circleBuffer.buf == NULL) {
        printf("malloc failed \n");
        return 1;
    }

    // Initialize buffer size and head and tail indices
    circleBuffer.size = bufferSize;
    circleBuffer.head = 0;
    circleBuffer.tail = 0;
    
    // Creating a new thread. 
    pthread_create(&producer, NULL, produce, &circleBuffer);
    pthread_create(&consumer, NULL, consume, &circleBuffer);

    // Wait for thread to finish
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    printf("\nLock Free Check\n");
    printf("head is lock free: %d\n", atomic_is_lock_free(&circleBuffer.head));
    printf("tail is lock free: %d\n", atomic_is_lock_free(&circleBuffer.tail));

    free(circleBuffer.buf);
    return 0;
}