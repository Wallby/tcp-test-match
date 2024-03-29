#define TCP_MINI_MATCH
#include "buffer_message.h"

#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>

#define length(a) (sizeof a/sizeof *a)
//#define NULL 0

//#define NOTPTR_T long int //< NOTE: why are * and sizeof not in the same range?

#define MAX_MESSAGE_T struct { int b[TCP_MINI_MAX_MESSAGE_SIZE]; } //< NOTE: b to avoid confusion (don't use this directly)

MAX_MESSAGE_T messagesToProcess[64];
int numMessagesToProcess = 0;
MAX_MESSAGE_T* notTheMessagesToProcess[64];

void try_process_now(tm_message_t* a, int b)
{
  switch(a->type)
  {
  case EMessageType_Buffer:
	  {
		  buffer_message_t* c = (buffer_message_t*)a;
		  switch(c->format)
		  {
		  case EBufferFormat_None:
			  {
				  unformattedbuffer_t* e = (unformattedbuffer_t*)c;
				  if(e->size > 0)
				  {
					  void* msg = ((void*)e) + sizeof(unformattedbuffer_t);
					  char* d = new char[e->size + 1];
					  int i = 0;
					  memcpy(d, msg, e->size);
					  i += e->size;
					  d[i] = '\0';
					  printf("unformatted buffer message.. size: %i, \"msg\": %s\n", e->size, d);
					  delete d;
				  }
			  }
			  break;
		  default:
			  printf("buffer message.. format: %i\n", c->format);
			  break;
		  }
	  }
	  break;
  default:
	  printf("unhandled message is here\n");
	  break;
  }
}

#define byte char

int discard_message(tm_message_t* a)
{
	//int b = a - messagesToProcess / sizeof(MAX_MESSAGE_T);
	tm_message_t* c = (tm_message_t*)messagesToProcess;
	int b = (((byte*)a) - ((byte*)c)) / sizeof *messagesToProcess;
	if(b >= 0 && b < length(messagesToProcess))
	{
		// "free up a"
		notTheMessagesToProcess[b] = NULL;
		--numMessagesToProcess;
	}
}
void try_process_late_and_discard(tm_message_t* a)
{
  switch(a->type)
  {
  default:
	  printf("unhandled message is here\n");
	  break;
  }

  discard_message(a);
}

void my_on_receive(tm_message_t* message, int a)
{
	switch(message->type)
	{
	case EMessageType_Buffer:
		{
		  try_process_now(message, a); //< buffer messages may get very big, hence process them immediately instead of late.

		  unformattedbuffer_t b = unformattedbuffer_default;
		  b.size = 0;
		  TM_SEND((tm_message_t*)&b, NULL, 0);
		}
		break;
	default:
		{
			if(numMessagesToProcess == length(messagesToProcess))
			{
				return; //< discard the message
			}
			for(int i = 0; i < length(messagesToProcess); ++i)
			{
				// NOTE: I think that discard_message doesn't actually discard the message
				// NOTE: tcp_mini still has some code that should not be kept as is (referring to "blocking of sockets")

				// CHANGELIST: ..
				// .. String now copy constructs strings by copying the contents of the source buffer into a new one (internally in tcp-mini)
				// .. displaying of "hello" message is now working (fixed incorrect pointer offsetting)
				/*
				 * if a slot in messagesToProcess is not occupied, the "same indexed" slot in notTheMessagesToProcess will be NULL
				 * numMessagesToProcess is to be kept "up-to-date" accordingly
				 */
				if(notTheMessagesToProcess[i] == NULL)
				{
					//notTheMessagesToProcess[i] = &messagesToProcess[i];
					void** c = (void**)&notTheMessagesToProcess[i];
					*c = (void*)&messagesToProcess[i];
					++numMessagesToProcess;
					//messagesToProcess[i] = message;
					memcpy(&messagesToProcess[i], message, a);
					break;
				}
			}
		}
		break;
	}
}

void my_on_connected_to_us(char* ip)
{
	printf("%s connected\n", ip);
}
void my_on_hung_up(char* ip)
{
	printf("%s hung up\n", ip);
}

int main()
{
  tm_set_on_receive(my_on_receive);
  TM_SET_ON_HUNG_UP(my_on_hung_up);
  tm_set_on_connected_to_us(my_on_connected_to_us);

  printf("started\n");

  if(!tm_become_a_match(44444))
  {
    return -1;
  }

  printf("polling..\n");
  while(1)
  {
	  do
	  {
		  for(int i = 0; i < length(notTheMessagesToProcess); ++i)
		  {
			  if(notTheMessagesToProcess[i] == NULL)
			  {
				  continue;
			  }

			  try_process_late_and_discard((tm_message_t*)notTheMessagesToProcess[i]);
		  }
	  } while(tm_poll(length(messagesToProcess)) == 1);

	  sleep(1);
  }

  printf("stopped\n");

  tm_stop_being_a_match();

  tm_unset_on_connected_to_us();
  TM_UNSET_ON_HUNG_UP();
  tm_unset_on_receive(); //< NOTE: do we want to do this after/before stop_being_a_match?
}
