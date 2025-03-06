#include "core_mem.h"
#include "mm_lib.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

// -------- Macros defined for the allocator --------

// --------- Definitions of the headers ---------
struct list_node
{
    size_t size;
    struct list_node *next;
};

struct header
{
    size_t size;
    int magic1;
    int magic2;
};

// --------- Global Variables ---------

struct list_node *list_node_head = NULL;

// --------- Helper function declarations ---------

void *search_for_free_block_first_fit(size_t aligned_size)
{
    struct list_node *search = list_node_head;

    while (search != NULL && search->size < aligned_size)
    {
        search = search->next;
    }
    return search;
}

void *search_for_previous_first_fit(size_t aligned_size)
{
    struct list_node *previous = NULL;
    struct list_node *search = list_node_head;
    while (search != NULL && search->size < aligned_size)
    {
        previous = search;
        search = search->next;
    }
    if (search == NULL)
    {
        previous = NULL;
    }
    return previous;
}

void *search_for_free_block_worst_fit(size_t aligned_size)
{
    struct list_node *search = list_node_head;
    size_t maximum_size = 0;
    struct list_node *block_with_max_size = NULL;

    while (search != NULL)
    {
        if (search->size >= aligned_size && search->size > maximum_size)
        {
            block_with_max_size = search;
            maximum_size = search->size;
        }
        search = search->next;
    }
    return block_with_max_size;
}

void *search_for_previous_worst_fit(size_t aligned_size)
{
    struct list_node *previous = NULL;
    struct list_node *previous_of_max = NULL;
    struct list_node *search = list_node_head;
    size_t maximum_size = 0;
    struct list_node *block_with_max_size = NULL;

    while (search != NULL)
    {
        if (search->size >= aligned_size && search->size > maximum_size)
        {
            previous_of_max = previous;
            block_with_max_size = search;
            maximum_size = search->size;
        }
        previous = search;
        search = search->next;
    }
    if (block_with_max_size == NULL)
    {
    }
    return previous_of_max;
}

void *search_for_free_block_best_fit(size_t aligned_size)
{
    struct list_node *search = list_node_head;
    size_t minimum_size = 100000000000000000;
    struct list_node *block_with_min_size = NULL;

    while (search != NULL)
    {
        if (search->size >= aligned_size && search->size < minimum_size)
        {
            block_with_min_size = search;
            minimum_size = search->size;
        }
        search = search->next;
    }
    return block_with_min_size;
}

void *search_for_previous_best_fit(size_t aligned_size)
{
    struct list_node *previous = NULL;
    struct list_node *previous_of_min = NULL;
    struct list_node *search = list_node_head;
    size_t minimum_size = 100000000000000000;
    struct list_node *block_with_min_size = NULL;

    while (search != NULL)
    {
        if (search->size >= aligned_size && search->size < minimum_size)
        {
            previous_of_min = previous;
            block_with_min_size = search;
            minimum_size = search->size;
        }
        previous = search;
        search = search->next;
    }
    if (block_with_min_size == NULL)
    {
    }
    return previous_of_min;
}

void *search_for_list_tail()
{
    struct list_node *search2 = list_node_head;
    if (list_node_head == NULL)
    {
        return search2;
    }
    while (search2->next != NULL)
    {
        search2 = search2->next;
    }
    return search2;
}

// --------- Function Definitions ---------
void mm_init()
{
    void *start_heap = NULL;
    size_t heap_size = 1024;
    start_heap = cm_sbrk(heap_size);

    if (start_heap == NULL)
    {
        return;
    }
    struct list_node *first_node = start_heap;
    first_node->size = heap_size - sizeof(struct list_node);
    first_node->next = NULL;
    list_node_head = first_node;
}

void *mm_malloc(size_t size)
{

    char *search_scheme = getenv("SEARCH_SCHEME");

    int allocation_found = 0;
    size_t aligned_size = size;
    while (aligned_size % 8 != 0)
    {
        aligned_size = aligned_size + 1;
    }
    if (size == 0)
    {
        aligned_size = 8; // minimum 8
    }

    struct list_node *returned_node = NULL;
    struct list_node *previous_node = NULL;
    void *return_malloc = NULL;
    while (allocation_found != 1)
    {

        if (strcmp(search_scheme, "FIRST_FIT") == 0)
        {
            returned_node = search_for_free_block_first_fit(aligned_size);
            previous_node = search_for_previous_first_fit(aligned_size);
        }

        else if (strcmp(search_scheme, "WORST_FIT") == 0)
        {
            returned_node = search_for_free_block_worst_fit(aligned_size);
            previous_node = search_for_previous_worst_fit(aligned_size);
        }

        else if (strcmp(search_scheme, "BEST_FIT") == 0)
        {
            returned_node = search_for_free_block_best_fit(aligned_size);
            previous_node = search_for_previous_best_fit(aligned_size);
        }

        if (returned_node == NULL)
        {
            size_t heap_size = 1024;
            void* heap_new = cm_sbrk(heap_size);
            if (heap_new == NULL)
            {
                return NULL;
            }
            struct list_node *extended_heap_node = (struct list_node *)heap_new;
            extended_heap_node->next = NULL;
            extended_heap_node->size = heap_size - sizeof(struct list_node);
            if (list_node_head == NULL)
            {
                list_node_head = extended_heap_node;
            }
            else
            {
                struct list_node *tail = search_for_list_tail();
                if (tail != NULL)
                {
                    tail->next = extended_heap_node;
                    size_t size_of_tail_node_and_its_holding = sizeof(struct list_node) + tail->size; // coalescing
                    struct list_node *check_if_coalesce = PTR_ADD(tail, size_of_tail_node_and_its_holding);
                    if (check_if_coalesce == extended_heap_node)
                    {
                        tail->size = tail->size + sizeof(struct list_node) + extended_heap_node->size;
                        tail->next = extended_heap_node->next;
                        extended_heap_node = NULL;
                    }
                }
            }

            continue;
        }
        allocation_found = 1;

        struct list_node *next_of_returned_node = returned_node->next;
        size_t size_of_retunred_node = returned_node->size;
        struct list_node *stored_returned_node = returned_node;

        struct header *header = (struct header *)returned_node;
        header->size = aligned_size;

        size_t remaining_size = size_of_retunred_node - aligned_size;

        if (remaining_size > sizeof(struct list_node))
        {
            size_t combined_size = aligned_size + sizeof(struct header);
            struct list_node *new_list_node = (struct list_node *)PTR_ADD(header, combined_size);
            new_list_node->size = remaining_size - sizeof(struct list_node);
            new_list_node->next = next_of_returned_node;

            if (stored_returned_node == list_node_head)
            {
                list_node_head = new_list_node;
            }
            else
            {
                previous_node->next = new_list_node;
            }
        }
        else
        {
            if (stored_returned_node == list_node_head)
            {
                list_node_head = next_of_returned_node;
            }
            else
            {
                previous_node->next = next_of_returned_node;
            }
        }
        return_malloc = PTR_ADD(header, sizeof(struct header));
    }
    return return_malloc;
}

void mm_free(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }
    struct header *header_of_free = (struct header *)PTR_SUB(ptr, sizeof(struct header));
    struct list_node *new_list_node_after_free = (struct list_node *)header_of_free;
    new_list_node_after_free->size = header_of_free->size;
    new_list_node_after_free->next = NULL;

    struct list_node *node_greater_than = list_node_head;
    struct list_node *previous_of_node_greater_than = NULL;
    while (node_greater_than != NULL)
    {
        if (node_greater_than < new_list_node_after_free)
        {
            previous_of_node_greater_than = node_greater_than;
            node_greater_than = node_greater_than->next;
            continue;
        }
        else
        {
            break;
        }
    }

    if (node_greater_than == list_node_head)
    {
        new_list_node_after_free->next = list_node_head;
        list_node_head = new_list_node_after_free;
    }
    else
    {
        new_list_node_after_free->next = previous_of_node_greater_than->next;
        previous_of_node_greater_than->next = new_list_node_after_free;
    }

    // coalescing
    struct list_node *search = list_node_head;
    struct list_node *prev = NULL;
    while (search != NULL)
    {
        if (search == new_list_node_after_free)
        {
            break;
        }
        prev = search;
        search = search->next;
    }
    if (new_list_node_after_free->next != NULL)
    {
        size_t size_of_new_node_combined = sizeof(struct list_node) + new_list_node_after_free->size;
        struct list_node *next_node_of_new = PTR_ADD(new_list_node_after_free, size_of_new_node_combined);
        if (next_node_of_new == new_list_node_after_free->next)
        {
            new_list_node_after_free->size = new_list_node_after_free->size + sizeof(struct list_node) + new_list_node_after_free->next->size;
            new_list_node_after_free->next = new_list_node_after_free->next->next;
            next_node_of_new = NULL;
        }
    }

    if (prev != NULL)
    {
        size_t size_of_prev_node_combined = sizeof(struct list_node) + prev->size;
        struct list_node *next_node_of_prev = PTR_ADD(prev, size_of_prev_node_combined);

        if (new_list_node_after_free == next_node_of_prev)
        {
            prev->size = prev->size + sizeof(struct list_node) + new_list_node_after_free->size;
            prev->next = new_list_node_after_free->next;
            new_list_node_after_free = NULL;
        }
    }
}

void *mm_realloc(void *ptr, size_t size)
{
    if (ptr == NULL)
    {
        void *allocated = mm_malloc(size);
        return allocated;
    }
    if (size == 0)
    {
        mm_free(ptr);
        return NULL;
    }

    void *ptr_of_new_allocation = mm_malloc(size);

    struct header *header_of_realloc = (struct header *)PTR_SUB(ptr, sizeof(struct header));
    if (size < header_of_realloc->size)
    {
        size_t content_to_copy = size;

        memcpy(ptr_of_new_allocation, ptr, content_to_copy);
        mm_free(ptr);
    }
    else
    {
        size_t content_to_copy = header_of_realloc->size;

        memcpy(ptr_of_new_allocation, ptr, content_to_copy);
        mm_free(ptr);
    }

    return ptr_of_new_allocation;
}
