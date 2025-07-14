/**
 * @file circular_queue.c
 * @brief Implementação da fila circular com proteção por mutex.
 */

#include "circular_queue.h"

void queue_init(CircularQueue *q)
{
    q->front = 0;
    q->back = -1;
    q->size = 0;
    mutex_init(&q->mutex);
}

bool queue_enqueue(CircularQueue *q, WiFiMessage m)
{
    bool success = false;
    mutex_enter_blocking(&q->mutex);

    if (q->size < QUEUE_SIZE)
    {
        q->back = (q->back + 1) % QUEUE_SIZE;
        q->queue[q->back] = m;
        q->size++;
        success = true;
    }

    mutex_exit(&q->mutex);
    return success;
}

bool queue_dequeue(CircularQueue *q, WiFiMessage *output)
{
    bool success = false;
    mutex_enter_blocking(&q->mutex);

    if (q->size > 0)
    {
        *output = q->queue[q->front];
        q->front = (q->front + 1) % QUEUE_SIZE;
        q->size--;
        success = true;
    }

    mutex_exit(&q->mutex);
    return success;
}

bool queue_is_empty(CircularQueue *q)
{
    return q->size == 0;
}
