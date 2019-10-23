/* Copyright (C) 2002-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define printf printk
#define puts printk

/* Note that this test requires more than the standard.  It is
   required that there are no spurious wakeups if only more readers
   are added.  This is a reasonable demand.  */


static pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mut3 = PTHREAD_MUTEX_INITIALIZER;


#define N3 10

static void
delayed_exit_thread (int seconds_as_ptr)
{
  int seconds = seconds_as_ptr;
  struct timespec delay = { seconds, 0 };
  struct timespec remaining = { 0 };
  if (nanosleep (&delay, &remaining) != 0)
    printf("Cant nanosleep\n");
  return;
}

static void *
tf3 (void *arg)
{
  int i = (long int) arg;
  int err;

  /* Get the mutex.  */
  err = pthread_mutex_lock (&mut3);
  if (err != 0)
    {
      printf ("child %d mutex_lock failed: %s\n", i, strerror (err));
      exit (1);
    }

  /* This call should never return.  */
  pthread_cond_wait (&cond3, &mut3);
  puts ("error: pthread_cond_wait in tf3 returned");

  /* We should never get here.  */
  exit (1);

  return NULL;
}


int
do_test3 (void)
{
  int err;
  int i;

  for (i = 0; i < N3; ++i)
    {
      pthread_t th;

      if (i != 0)
	{
	  /* Release the mutex.  */
	  err = pthread_mutex_unlock (&mut3);
	  if (err != 0)
	    {
	      printf ("mutex_unlock %d failed: %s\n", i, strerror (err));
	      return 1;
	    }
	}

      err = pthread_create (&th, NULL, tf3, (void *) (long int) i);
      if (err != 0)
	{
	  printf ("create %d failed: %s\n", i, strerror (err));
	  return 1;
	}

      /* Get the mutex.  */
      err = pthread_mutex_lock (&mut3);
      if (err != 0)
	{
	  printf ("mutex_lock %d failed: %s\n", i, strerror (err));
	  return 1;
	}
    }

  delayed_exit_thread(1);

  /* This call should never return.  */
  pthread_cond_wait (&cond3, &mut3);

  puts ("error: pthread_cond_wait in do_test3 returned");
  return 1;
}
