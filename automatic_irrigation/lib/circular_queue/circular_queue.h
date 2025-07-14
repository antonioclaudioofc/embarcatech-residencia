/**
 * @file circular_queue.h
 * @brief Interface da fila circular protegida por mutex para comunicação entre núcleos.
 */
#include "general_config.h"

#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

typedef struct
{
    uint16_t attempt;
    uint16_t status;
} WiFiMessage;

typedef struct
{
    WiFiMessage queue[QUEUE_SIZE];
    int front;
    int back;
    int size;
    mutex_t mutex;
} CircularQueue;

void queue_init(CircularQueue *q);
bool queue_enqueue(CircularQueue *q, WiFiMessage m);
bool queue_dequeue(CircularQueue *q, WiFiMessage *output);
bool queue_is_empty(CircularQueue *q);

#endif
