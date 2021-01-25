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

int discard_message(tm_message_t* a)
{
	//int b = a - messagesToProcess / sizeof(MAX_MESSAGE_T);
	tm_message_t* c = (tm_message_t*)messagesToProcess;
	int b = (a - c) / sizeof *messagesToProcess;
	if(b >= 0 && b < length(messagesToProcess))
	{
		// "free up a"
		notTheMessagesToProcess[b] = NULL;
		--numMessagesToProcess;
	}
}
void try_process_and_discard(tm_message_t* a)
{
  printf("%s\n", "a message is here!");

  switch(a->type)
  {
  case EMessageType_Buffer:
	  buffer_message_t* b = (buffer_message_t*)a;
	  printf("format: %i\n", b->format);
	  switch(b->format)
	  {
	  case EBufferFormat_None:
		  unformattedbuffer_t* c = (unformattedbuffer_t*)b;
		  if(c->size > 0)
		  {
			  void* msg = c + 1;
			  printf("%s\n", msg);
		  }
		  break;
	  }
	  break;
  }

  discard_message(a);
}

void my_on_receive(tm_message_t* message, int a)
{
  if(numMessagesToProcess == length(messagesToProcess))
  {
    return; //< discard the message
  }
  for(int i = 0; i < length(messagesToProcess); ++i)
  {
	  // IDEA: perhaps if a message has additional data appended, process it directly, otherwise queue it up?
	  // NOTE: I think that discard_message doesn't actually discard the message
	  // NOTE: tcp_mini still has some code that should not be kept as is (referring to "blocking of sockets")
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
    }
  }
}


int main()
{
  tm_set_on_receive(my_on_receive);

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

			  try_process_and_discard((tm_message_t*)notTheMessagesToProcess[i]);
		  }
	  } while(tm_poll(length(messagesToProcess)) == 1);

	  sleep(1);
  }

  printf("stopped\n");

  tm_stop_being_a_match();

  tm_unset_on_receive(); //< NOTE: do we want to do this after/before stop_being_a_match?
}
