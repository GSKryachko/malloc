#include <iostream>
#include <fstream>
#include <list>
#include <new>
#define NULL nullptr

using namespace std;

class Descriptor {
public:
	void* start;
	void* end;
	Descriptor* prev;
	Descriptor* next;
	Descriptor(void* start, size_t size) {
		this->start = start;
		this->end = (void*)((size_t)start + size);
		prev = NULL;
		next = NULL;
	}

};

class MemoryManager{
public:
	void* global_start;
	void* global_end;
	size_t size;
	Descriptor* first_descriptor = NULL;

	MemoryManager(size_t size) {
		global_start = malloc(size);
		global_end = (void*)((size_t)global_start + size);
		
	}

	~MemoryManager(){
		free(global_start);
	}

	void* allocate(size_t size);

	void deallocate(void* block_start);

	void dump();
};



void * MemoryManager::allocate(size_t size) {
	size += sizeof(Descriptor);
	if (first_descriptor == NULL) {
		if ((size_t)global_end - (size_t)global_start < size)
			throw std::bad_alloc();

		first_descriptor = new(global_start) Descriptor(global_start, size);
		return (void*)((size_t)first_descriptor->start + sizeof(Descriptor));

	}
	Descriptor* current = first_descriptor;
	while (current != NULL)
	{
		if (current->next != NULL) {
			if ((size_t)current->next->start - (size_t)current->end > size) {
				Descriptor* next = current->next;
				Descriptor* new_descriptor = new(current->end) Descriptor(current->end, size);
				current->next = new_descriptor;
				new_descriptor->next = next;
				new_descriptor->prev = current;
				next->prev = current;
				return (void*)((size_t)current->next->start + sizeof(Descriptor));
			}
		}else if ((size_t)global_end - (size_t)current->end > size) {
			current->next = new(current->end) Descriptor(current->end, size);
			current->next->prev = current;
			return (void*)((size_t)current->next->start + sizeof(Descriptor));
		}
		current = current->next;
	}
	throw std::bad_alloc();
}

void MemoryManager::deallocate(void* block_start) {
	block_start = (void*)((size_t)block_start - sizeof(Descriptor));
	Descriptor* current = first_descriptor;
	while (current != NULL)
	{
		if (current->start == block_start) {
			if (current->prev)
				current->prev->next = current->next;
			if (current->next)
				current->next->prev = current->prev;
			if (current == first_descriptor)
				first_descriptor = current->next;
			return;
		}
		
		current = current->next;
	}
	throw std::exception();
}

void MemoryManager::dump() {
	Descriptor* current = first_descriptor;
	while (current != NULL)
	{
		cout << current->start << "-" << current->end << endl;
	    current = current->next;	
	}
}

int main() {
	MemoryManager* memoryManager = new MemoryManager(2048);
	void *chunk = memoryManager->allocate(10);
	void *chunk2 = memoryManager->allocate(20);
	memoryManager ->dump();

	memoryManager->deallocate(chunk);
	memoryManager->dump();

	memoryManager->deallocate(chunk2);
	memoryManager->dump();

}

