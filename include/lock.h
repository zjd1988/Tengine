/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * License); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * AS IS BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*
 * Copyright (c) 2020, OPEN AI LAB
 * Author: haitao@openailab.com
 */

#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__

#ifdef CONFIG_BAREMETAL_BUILD

typedef int lock_t;

static inline void init_lock(lock_t* lock)
{
    lock[0] = 0;
}

static inline void lock(lock_t* l)
{
    l[0] = 1;
}

static inline void unlock(lock_t* l)
{
    l[0] = 0;
}

#else
#ifdef _WIN32
#include <stdbool.h>
#include <windows.h>

typedef CRITICAL_SECTION pthread_mutex_t;
typedef void pthread_mutexattr_t;

static inline int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
    (void)attr;

    if (mutex == NULL)
        return 1;

    InitializeCriticalSection(mutex);
    return 0;
}

static inline int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    if (mutex == NULL)
        return 1;
    EnterCriticalSection(mutex);
    return 0;
}

static inline int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    if (mutex == NULL)
        return 1;
    LeaveCriticalSection(mutex);
    return 0;
}
#else

#include <pthread.h>
#endif //_WIN32

typedef pthread_mutex_t lock_t;

static inline void init_lock(lock_t* lock)
{
    pthread_mutex_init(lock, NULL);
}

static inline void lock(lock_t* l)
{
    pthread_mutex_lock(l);
}

static inline void unlock(lock_t* l)
{
    pthread_mutex_unlock(l);
}

#endif //CONFIG_BAREMETAL_BUILD
#endif //__SYS_LOCK_H__
