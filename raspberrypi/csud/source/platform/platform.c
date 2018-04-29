/******************************************************************************
*	platform/platform.c
*	 by Alex Chadwick
*
*	A light weight implementation of the USB protocol stack fit for a simple
*	driver.
*
*	platform/platform.c contains code for generic system duties such as memory
*	management and logging, which can be optionally disabled. 
******************************************************************************/
#include <platform/platform.h>
#include <stdarg.h>

#ifdef MEM_NO_RESERVE

void* MemoryReserve(u32 length, void* physicalAddress) {
	return physicalAddress;
}

#endif

#ifdef MEM_INTERNAL_MANAGER_DEFAULT 

#define HEAP_END ((void*)0xFFFFFFFF)

struct HeapAllocation {
	u32 Length;
	void* Address;
	struct HeapAllocation *Next;
};

u8 Heap[0x4000] __attribute__((aligned(8))); // Support a maximum of 16KiB of allocations
struct HeapAllocation Allocations[0x100]; // Support 256 allocations
struct HeapAllocation *FirstAllocation = HEAP_END, *FirstFreeAllocation = NULL;
u32 allocated = 0;

void* MemoryAllocate(u32 size) {
	struct HeapAllocation *Current, *Next;
	if (FirstFreeAllocation == NULL) {
		LOG_DEBUG("Platform: First memory allocation, reserving 16KiB of heap, 256 entries.\n");
		MemoryReserve(sizeof(Heap), &Heap);
		MemoryReserve(sizeof(Allocations), &Allocations);

		FirstFreeAllocation = &Allocations[0];
	}

	size += (8 - (size & 7)) & 7; // Align to 8

	if (allocated + size > sizeof(Heap)) {
		LOG("Platform: Out of memory! We should've had more heap space in platform.c.\n");
		return NULL;
	}
	
	if (FirstFreeAllocation == HEAP_END) {
		LOG("Platform: Out of memory! We should've had more allocations in platform.c.\n");
		return NULL;
	}
	Current = FirstAllocation;

	while (Current != HEAP_END) {
		if (Current->Next != HEAP_END) {
			if ((u32)Current->Next->Address - (u32)Current->Address - Current->Length >= size) {
				FirstFreeAllocation->Address = (void*)((u8*)Current->Address + Current->Length);
				FirstFreeAllocation->Length = size;
				Next = FirstFreeAllocation;
				if (Next->Next == NULL)
					if ((u32)(FirstFreeAllocation + 1) < (u32)((u8*)Allocations + sizeof(Allocations)))
						FirstFreeAllocation = FirstFreeAllocation + 1;
					else
						FirstFreeAllocation = HEAP_END;
				else
					FirstFreeAllocation = Next->Next;
				Next->Next = Current->Next;
				Current->Next = Next;
				allocated += size;
				LOG_DEBUGF("Platform: malloc(%#x) = %#x. (%d/%d)\n", size, Next->Address, allocated, sizeof(Heap));
				return Next->Address;
			}
			else
				Current = Current->Next;
		} else {
			if ((u32)&Heap[sizeof(Heap)] - (u32)Current->Next - Current->Length >= size) {
				FirstFreeAllocation->Address = (void*)((u8*)Current->Address + Current->Length);
				FirstFreeAllocation->Length = size;
				Next = FirstFreeAllocation;
				if (Next->Next == NULL)
					if ((u32)(FirstFreeAllocation + 1) < (u32)((u8*)Allocations + sizeof(Allocations)))
						FirstFreeAllocation = FirstFreeAllocation + 1;
					else
						FirstFreeAllocation = HEAP_END;
				else
					FirstFreeAllocation = Next->Next;
				Next->Next = Current->Next;
				Current->Next = Next;
				allocated += size;
				LOG_DEBUGF("Platform: malloc(%#x) = %#x. (%d/%d)\n", size, Next->Address, allocated, sizeof(Heap));
				return Next->Address;
			}
			else {
				LOG("Platform: Out of memory! We should've had more heap space in platform.c.\n");
				LOG_DEBUGF("Platform: malloc(%#x) = %#x. (%d/%d)\n", size, NULL, allocated, sizeof(Heap));
				return NULL;
			}
		}
	}
	
	Next = FirstFreeAllocation->Next;
	FirstAllocation = FirstFreeAllocation;
	FirstAllocation->Next = HEAP_END;
	FirstAllocation->Length = size;
	FirstAllocation->Address = &Heap;
	if (Next == NULL)
		if ((u32)(FirstFreeAllocation + 1) < (u32)((u8*)Allocations + sizeof(Allocations)))
			FirstFreeAllocation = FirstFreeAllocation + 1;
		else
			FirstFreeAllocation = HEAP_END;
	else
		FirstFreeAllocation = Next;
	allocated += size;
	LOG_DEBUGF("Platform: malloc(%#x) = %#x. (%d/%d)\n", size, FirstAllocation->Address, allocated, sizeof(Heap));
	return FirstAllocation->Address;
}

void MemoryDeallocate(void* address) {
	struct HeapAllocation *Current, **CurrentAddress;

	CurrentAddress = &FirstAllocation;
	Current = FirstAllocation;

	while (Current != HEAP_END) {
		if (Current->Address == address) {
			allocated -= Current->Length;
			*CurrentAddress = Current->Next;
			Current->Next = FirstFreeAllocation;
			FirstFreeAllocation = Current;
			LOG_DEBUGF("Platform: free(%#x) (%d/%d)\n", address, allocated, sizeof(Heap));
			return;
		}
		else {
			Current = Current->Next;
			CurrentAddress = &((*CurrentAddress)->Next);
		}
	}
	
	LOG_DEBUGF("Platform: free(%#x) (%d/%d)\n", address, allocated, sizeof(Heap));
	LOG("Platform: Deallocated memory that was never allocated. Ignored, but you should look into it.\n");
}

void MemoryCopy(void* destination, void* source, u32 length) {
	u8 *d, *s;
	
	if (length == 0) return;

	d = (u8*)destination;
	s = (u8*)source;

	if ((u32)s < (u32)d)
		while (length-- > 0)
			*d++ = *s++;
	else {
		d += length;
		s += length;
		while (length-- > 0)
			*--d = *--s;
	}
}

#endif

#define FLOAT_TEXT "Floats unsupported."

#ifndef NO_LOG
void LogPrintF(char* format, u32 formatLength, ...) {
	va_list args;
	char messageBuffer[160];
	u32 messageIndex, width = 0, precision = 1, characters;
	s32 svalue; u32 value;
	bool opened, flagged, widthed = false, precisioned = false, length = false;
	bool left = false, plus = false, space = false, hash = false, zero = false, nwidth = false, period = false, nprecision = false;
	bool longInt = false, shortInt = false;
	char character; char* string;
	u8 base;
	messageIndex = 0;
	opened = false;
	
	va_start(args, formatLength);

	for (u32 index = 0; index < formatLength && messageIndex < sizeof(messageBuffer) - 1; index++) {
		if (opened) {
			if (!flagged)
				switch (format[index]) {
				case '-':
					if (!left) left = true;
					else flagged = true;
					break;
				case '+':
					if (!plus) plus = true;
					else flagged = true;
					break;
				case ' ':
					if (!space) space = true;
					else flagged = true;
					break;
				case '#':
					if (!hash) hash = true;
					else flagged = true;
					break;
				case '0':
					if (!zero) zero = true;
					else flagged = true;
					break;
				default:
					flagged = true;
				}
			if (flagged && !widthed) {
				switch (format[index]) {
				case '0': case '1':
				case '2': case '3':
				case '4': case '5':
				case '6': case '7':
				case '8': case '9':
					nwidth = true;
					width *= 10;
					width += format[index] - '0';
					continue;
				case '*':
					if (!nwidth) {
						widthed = true;
						width = va_arg(args, u32);
						continue;
					}
					else
						widthed = true;
					break;
				default:
					widthed = true;
				}			
			}
			if (flagged && widthed && !precisioned) {
				if (!period) {
					if (format[index] == '.') {
						period = true;
						precision = 0;
						continue;
					} else
						precisioned = true;
				}
				else {
					switch (format[index]) {
					case '0': case '1':
					case '2': case '3':
					case '4': case '5':
					case '6': case '7':
					case '8': case '9':
						nprecision = true;
						precision *= 10;
						precision += format[index] - '0';
						continue;
					case '*':
						if (!nprecision) {
							precisioned = true;
							precision = va_arg(args, u32);
							continue;
						}
						else
							precisioned = true;
						break;
					default:
						precisioned = true;
					}
				}
			}
			if (flagged && widthed && precisioned && !length) {
				switch (format[index]) {
				case 'h':
					length = true;
					shortInt = true;
					continue;
				case 'l':
					length = true;
					longInt = true;
					continue;
				case 'L':
					length = true;
					continue;
				default:
					length = true;
				}
			}
			if (flagged && widthed && precisioned && length) {
				character = '%';
				base = 16;
				switch (format[index]) {
				case 'c':
					character = va_arg(args, int) & 0x7f;
				case '%':
					messageBuffer[messageIndex++] = character;
					break;
				case 'd':
				case 'i':
					if (shortInt) svalue = (s32)((s16)va_arg(args, s32) & 0xffff);
					else if (longInt) svalue = va_arg(args, s64);
					else svalue = va_arg(args, s32);

					characters = 1;
					if (svalue < 0) {
						svalue = -svalue;
						messageBuffer[messageIndex++] = '-';
					}
					else if (plus)
						messageBuffer[messageIndex++] = '-';
					else if (space)
						messageBuffer[messageIndex++] = ' ';
					else 
						characters = 0;

					for (u32 digits = 0; digits < precision || svalue > 0; digits++, characters++) {
						for (u32 j = 0; j < digits; j++)
							if (messageIndex - j < sizeof(messageBuffer) - 1)
								messageBuffer[messageIndex - j] = messageBuffer[messageIndex - j - 1];
						if (messageIndex - digits < sizeof(messageBuffer) - 1)
							messageBuffer[messageIndex++ -digits] = '0' + (svalue % 10);
						svalue /= 10;
					}		
					
					if (characters < width) {
						if (!left)
							for (u32 i = 0; i <= characters; i++) {
								if (messageIndex - characters + width - i < sizeof(messageBuffer) - 1)
									messageBuffer[messageIndex - characters + width - i] = 
										messageBuffer[messageIndex - i];
							}

						for (u32 digits = characters; characters < width; characters++) {
							if (messageIndex - (!left ? digits : 0) < sizeof(messageBuffer) - 1)
								messageBuffer[messageIndex - (!left ? digits : 0)] = zero ? '0' : ' '; 
						}
					}
					break;
				case 'e':
				case 'E':
				case 'f':
				case 'g':
				case 'G':
					for (u32 i = 0; (i < width || i < sizeof(FLOAT_TEXT)) && messageIndex < sizeof(messageBuffer) - 1; i++) {
						if (i < sizeof(FLOAT_TEXT))
							messageBuffer[messageIndex++] = FLOAT_TEXT[i];
						else 
							messageBuffer[messageIndex++] = zero ? '0' : ' ';
					}
					break;
				case 'o':
					base = 8;
				case 'u':
					if (format[index] == 'u') base = 10;
				case 'x':
				case 'X':
				case 'p':
					if (shortInt) value = va_arg(args, u32) & 0xffff;
					else if (longInt) value = va_arg(args, u64);
					else value = va_arg(args, u32);

					characters = 1;
					if (plus)
						messageBuffer[messageIndex++] = '-';
					else if (space)
						messageBuffer[messageIndex++] = ' ';
					else 
						characters = 0;

					if (hash) {
						if (format[index] == 'o') {
							if (messageIndex < sizeof(messageBuffer) - 1) 
								messageBuffer[messageIndex++] = '0';
							characters++;
						}
						else if (format[index] != 'u') {
							if (messageIndex < sizeof(messageBuffer) - 1) 
								messageBuffer[messageIndex++] = '0';
							characters++;
							if (messageIndex < sizeof(messageBuffer) - 1) 
								messageBuffer[messageIndex++] = format[index];
							characters++;
						}
					}
							

					for (u32 digits = 0; digits < precision || value > 0; digits++, characters++) {
						for (u32 j = 0; j < digits; j++)
							if (messageIndex - j < sizeof(messageBuffer) - 1)
								messageBuffer[messageIndex - j] = messageBuffer[messageIndex - j - 1];
						if (messageIndex - digits < sizeof(messageBuffer) - 1)
							messageBuffer[messageIndex++ -digits] =
							 (value % base) >= 10 ? format[index] - ('X' - 'A') + ((value % base) - 10) : '0' + (value % base);
						value /= base;
					}		

					if (characters < width) {
						if (!left)
							for (u32 i = 0; i <= characters; i++) {
								if (messageIndex - characters + width - i < sizeof(messageBuffer) - 1)
									messageBuffer[messageIndex - characters + width - i] = 
										messageBuffer[messageIndex - i];
							}

						for (u32 digits = characters; characters < width; characters++) {
							if (messageIndex - (!left ? digits : 0) < sizeof(messageBuffer) - 1)
								messageBuffer[messageIndex++ - (!left ? digits : 0)] = zero ? '0' : ' '; 
						}
					}
					break;
				case 's':
					string = va_arg(args, char*);
					for (u32 i = 0; messageIndex < sizeof(messageBuffer) - 1 && string[i] != '\0' && (!period || i < precision); i++) {
						messageBuffer[messageIndex++] = string[i];
					}
					break;
				case 'n':
					*va_arg(args, u32*) = messageIndex;
					break;
				}
				opened = false;
			}
		} else if (format[index] == '%') {
			opened = true;
			flagged = false;
			widthed = false;
			precisioned = false;
			length = false;
			width = 0; precision = 1;
			left = false; plus = false; space = false; hash = false; zero = false; nwidth = false; period = false; nprecision = false;
			longInt = false; shortInt = false;
		}
		else
			messageBuffer[messageIndex++] = format[index];
	}

	va_end(args);
	
	LogPrint(messageBuffer, messageIndex);
}
#endif

void PlatformLoad()
{
#ifdef MEM_INTERNAL_MANAGER_DEFAULT 
	FirstAllocation = HEAP_END;
	FirstFreeAllocation = NULL;
#endif
}