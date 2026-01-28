#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdatomic.h>

// size of the ring buffer
// maximum number of possible elements stored is SIZE-1
#define SIZE 5 

// number of iterations for producer and consumer 
#define PRODUCER_ITERATIONS 10
#define CONSUMER_ITERATIONS 10

// delay in threads to help simulate different timings
#define PRODUCER_DELAY 10000
#define CONSUMER_DELAY 100

// ring buffer struct with buffer and head/tail index
typedef struct {
    int buf[SIZE];
    _Atomic size_t head;
    _Atomic size_t tail;
} CircleBuffer;

CircleBuffer circleBuffer;

int get(CircleBuffer *circleBuffer, int *value) {
    // if head and tail are the same, do not consume value
    if (atomic_load(&circleBuffer->head) == atomic_load(&circleBuffer->tail)) {
        return 0;
    // else store element in value and update head
    } else {
        *value = circleBuffer->buf[circleBuffer->head];
        atomic_store(&circleBuffer->head, (circleBuffer->head + 1) % SIZE); 
        return 1;
    }
}

int put(CircleBuffer *circleBuffer, int value) {
    // next index of tail
    int nextTail = (atomic_load(&circleBuffer->tail) + 1) % SIZE;

    // if nextTail is the same as head, do not produce value
    if (nextTail == atomic_load(&circleBuffer->head)) {
        return 0;
    }

    // if nextTail is not the same as head, produce value and update tail
    circleBuffer->buf[atomic_load(&circleBuffer->tail)] = value;
    atomic_store(&circleBuffer->tail, nextTail);

    return 1;
}

void* consume(void* arg) {
    // variable to store consumed value
    int value;

    for (int i = 0; i <= CONSUMER_ITERATIONS; i++) {
        while(!get(&circleBuffer, &value)) {
            // wait
        }
        printf("Consumed %d\n", value);
        usleep(CONSUMER_DELAY);
    }
    return NULL;
}

void* produce(void* arg) {
    for (int i = 0; i <= PRODUCER_ITERATIONS; i++) {
        while(!put(&circleBuffer, i)) {
            // wait
        }
        printf("Produced %d\n", i);
        usleep(PRODUCER_DELAY);
    }
    return NULL;
}


int main() {
    // Create a pthread_t variable for producer and consumer threads
    pthread_t producer;
    pthread_t consumer;

    // Initialize head and tail indices
    circleBuffer.head = 0;
    circleBuffer.tail = 0;
    
    // Creating a new thread. 
    pthread_create(&producer, NULL, produce, NULL);
    pthread_create(&consumer, NULL, consume, NULL);

    // Wait for thread to finish
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    printf("\nLock Free Check\n");
    printf("head is lock free: %d\n", atomic_is_lock_free(&circleBuffer.head));
    printf("tail is lock free: %d\n", atomic_is_lock_free(&circleBuffer.tail));
    return 0;
}