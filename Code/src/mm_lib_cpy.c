#include "core_mem.h"
#include "mm_lib.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

// -------- Macros defined for the allocator --------
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

struct list_node *list_node_head = NULL;
void *start_heap = NULL;

// int counter = 0;
void *search_for_free_block_worst_fit(size_t aligned_size)
{
    struct list_node *search = list_node_head;
    size_t maximum_size = 0;
    struct list_node *block_with_max_size = NULL;
    // printf("NODE SEEARCH %p %ld\n", search, search->size);
    while (search != NULL && search->size <= aligned_size)
    {
        if (search->size > maximum_size)
        {
            block_with_max_size = search;
            maximum_size = search->size;
        }
        // printf("NODE SEEARCH %p %ld\n", search,search->size);
        search = search->next;
    }
    return block_with_max_size;
}

void *search_for_free_block_best_fit(size_t aligned_size)
{
    struct list_node *search = list_node_head;
    size_t minimum_size = 90000000000;
    struct list_node *block_with_min_size = NULL;
    // printf("NODE SEEARCH %p %ld\n", search, search->size);
    while (search != NULL && search->size <= aligned_size)
    {
        if (search->size < minimum_size)
        {
            block_with_min_size = search;
            minimum_size = search->size;
        }
        // printf("NODE SEEARCH %p %ld\n", search,search->size);
        search = search->next;
    }
    return block_with_min_size;
}

void *search_for_free_block_first_fit(size_t aligned_size)
{
    struct list_node *search = list_node_head;
    // printf("NODE SEEARCH %p %ld\n", search, search->size);
    while (search != NULL && search->size <= aligned_size)
    {
        // printf("NODE SEEARCH %p %ld\n", search,search->size);
        search = search->next;
    }
    return search;
}

void print_all_nodes()
{
    struct list_node *print_nodes = list_node_head;
    while (print_nodes != NULL)
    {
        printf("NODE At %p NEXT %p \n", print_nodes, print_nodes->next);
        print_nodes = print_nodes->next;
    }
}

void *search_for_list_tail()
{
    struct list_node *search2 = list_node_head;
    while (search2->next != NULL)
    {
        search2 = search2->next;
    }
    return search2;
}

void set_node(struct list_node *node, int length, struct list_node *next_node)
{
    node->size = length;
    node->next = next_node;
}

void *find_prev_free_list_node(struct list_node *node)
{
    struct list_node *temp_node = list_node_head;

    while (temp_node != NULL && temp_node->next != node)
    {
        temp_node = temp_node->next;
    }

    return temp_node;
}

void coalesce(struct list_node *node)
{
    struct list_node *prev_node = find_prev_free_list_node(node);

    if (node->next != NULL && PTR_ADD(node, (sizeof(struct list_node) + node->size)) == node->next)
    {
        node->size += sizeof(struct list_node) + node->next->size;
        node->next = node->next->next;
    }

    if (prev_node != NULL && PTR_ADD(prev_node, (sizeof(struct list_node) + prev_node->size)) == node)
    {
        prev_node->size += sizeof(struct list_node) + node->size;
        prev_node->next = node->next;
    }
}

// --------- Definitions of the headers ---------

// --------- Helper function declarations ---------

// --------- Function Definitions ---------
void mm_init()
{
    start_heap = cm_sbrk(1024);
    if(start_heap==NULL)
    {
        return;
    }
    struct list_node *first_node = start_heap;
    first_node->size = cm_heap_size() - sizeof(struct list_node);
    // printf("FIRST NODE SIZE %ld\n", first_node->size);
    first_node->next = NULL;
    list_node_head = first_node;

    LOG_DEBUG("HEAP start: %p\n", start_heap);
}

void *mm_malloc(size_t size)
{
    // char *search_scheme = getenv("SEARCH SCHEME");
    // printf("%s",search_scheme);
    int allocation_found = 0;
    // counter = counter + 1;
    // if (counter == 6)
    // {
    //     exit(1);
    // }
    // print_all_nodes();
    size_t aligned_size = size;
    while (aligned_size % 8 != 0)
    {
        aligned_size = aligned_size + 1;
    }

    if (size == 0) // minimum size is 8
    {
        aligned_size = 8;
    }
    // printf("ALIGNED SIZE %ld\n", aligned_size);
    struct list_node *returned_node = NULL;

    // if (strcmp(search_scheme, "FIRST_FIT") == 0)
    // {
    returned_node = search_for_free_block_first_fit(aligned_size);
    // }
    // else if (strcmp(search_scheme, "BEST_FIT") == 0)
    // {
    //     returned_node = search_for_free_block_best_fit(aligned_size);
    // }
    // else if (strcmp(search_scheme, "WORST_FIT") == 0)
    // {
    //     returned_node = search_for_free_block_worst_fit(aligned_size);
    // }

    while (allocation_found != 1)
    {
        // if (strcmp(search_scheme, "FIRST_FIT") == 0)
        // {
        returned_node = search_for_free_block_first_fit(aligned_size);
        // }
        // else if (strcmp(search_scheme, "BEST_FIT") == 0)
        // {
        //     returned_node = search_for_free_block_best_fit(aligned_size);
        // }
        // else if (strcmp(search_scheme, "WORST_FIT") == 0)
        // {
        //     returned_node = search_for_free_block_worst_fit(aligned_size);
        // }

        if (returned_node == NULL)
        {
            void *new_heap = cm_sbrk(1024);
            if (new_heap == NULL)
            {
                return NULL;
            }
            if (list_node_head == NULL)
            {
                size_t new_heap_allocation_size = 1024;
                struct list_node *new_extended_heap_node = new_heap;
                new_extended_heap_node->size = new_heap_allocation_size - sizeof(struct list_node);
                new_extended_heap_node->next = NULL;
                list_node_head = new_extended_heap_node;
                continue;
            }
            else
            {
                struct list_node *tail = search_for_list_tail(); // if last of previous heap and new heap are continous
                struct list_node *tail_and_size = PTR_ADD(tail, sizeof(struct list_node));
                struct list_node *check_if_coalesce = PTR_ADD(tail_and_size, tail->size);
                if (new_heap == check_if_coalesce)
                {
                    // printf("HERE\n");
                    tail->size = tail->size + 1024;
                }
                else // if last of previous heap and new heap are not continous
                {
                    // printf("HERE2\n");
                    size_t new_heap_allocation_size = 1024;
                    struct list_node *new_heap_node = new_heap;
                    new_heap_node->size = new_heap_allocation_size - sizeof(struct list_node);
                    new_heap_node->next = NULL;
                    tail->next = new_heap_node;
                }
                continue;
            }
        }
        allocation_found = 1;
    }
    size_t size_of_returned_node = returned_node->size;
    struct list_node *stored_returned_node = returned_node;
    struct list_node *next_of_returned_node = returned_node->next;

    // printf("FOUND AT %p\n", returned_node);
    // printf("SIZE OF RETURNED NODE %ld\n", returned_node->size);

    struct header *header_node = (struct header *)returned_node;
    header_node->size = aligned_size;

    // printf("NEW HEADER AT %p\n", header_node);
    size_t remaining_size = size_of_returned_node - aligned_size; // size after allocated memory block
    // printf("remaining size %ld\n", remaining_size);
    // if(new_size_free_block>0)
    // {
    // size_t size_of_header_and_aligned=aligned_size+sizeof(struct header);
    // struct list_node* new_free_node= PTR_ADD(returned_node,aligned_size);

    //}
    if (remaining_size > sizeof(struct list_node))
    {
        size_t size_of_header_and_aligned = aligned_size + sizeof(struct header);

        void *new_free_node = PTR_ADD(header_node, size_of_header_and_aligned);

        struct list_node *new_node = (struct list_node *)new_free_node;
        new_node->size = remaining_size - sizeof(struct list_node);
        new_node->next = next_of_returned_node;

        // printf("NEW FREE LIST NODE AT %p POINTING TO %p\n", new_node, new_node->next);

        if (returned_node == list_node_head)
        {
            list_node_head = new_node;
            // printf("NEW HEAD AT %p\n", list_node_head);
        }
        else
        {
            struct list_node *search_node = list_node_head;
            while (search_node->next != stored_returned_node)
            {
                search_node = search_node->next;
            }
            search_node->next = new_node;
        }
    }
    else
    {
        if (returned_node == list_node_head)
        {
            list_node_head = next_of_returned_node;
            // printf("NEW HEAD(else) AT %p\n", list_node_head);
        }
        else
        {
            struct list_node *search_node = list_node_head;
            while (search_node->next != returned_node)
            {
                search_node = search_node->next;
            }
            search_node->next = next_of_returned_node;
            // returned_node->next = NULL;
        }
    }

    void *return_malloc = PTR_ADD(header_node, sizeof(header_node));

    // printf("------------\n");
    // print_all_nodes();
    // printf("------------\n");
    // printf("------------\n");

    // LOG_DEBUG("malloc(%ld)\n", size);
    return return_malloc;
}

void mm_free(void *ptr)
{
    // printf("%p",ptr);
    // if(ptr==NULL)
    // {
    //     return;
    // }
    // void *find_header = PTR_SUB(ptr, sizeof(struct header));
    // // printf("HERE1\n");
    // struct header *header_of_free = (struct header *)find_header;
    // // printf("HERE2\n");
    // size_t header_size = header_of_free->size;
    // // printf("HERE3\n");
    // struct list_node *new_node_free = (struct list_node *)header_of_free;
    // // printf("HERE4\n");
    // new_node_free->size = header_size;
    // // printf("HERE5\n");
    // new_node_free->next = NULL;
    // // printf("HERE6\n");

    // struct list_node *prev_node = NULL;
    // // printf("HERE7\n");
    // struct list_node *next_node = list_node_head;
    // // printf("HERE8\n");
    // while (next_node != NULL && next_node < new_node_free)
    // {
    //     // printf("HERE9\n");
    //     prev_node = next_node;
    //     // printf("HERE10\n");
    //     next_node = next_node->next;
    //     // printf("HERE 11\n");
    // }

    // if (next_node == list_node_head)
    // {
    //     // printf("HERE 12\n");
    //     list_node_head = new_node_free;
    //     // printf("HERE 13\n");
    // }
    // else
    // {
    //     // printf("HERE 14\n");
    //     prev_node->next = new_node_free;
    //     // printf("HERE 15\n");
    // }
    // new_node_free->next=next_node;
    // printf("HERE 16\n");

    // if (list_node_head == NULL)
    // {
    //     list_node_head = new_node_free;
    //     return;
    // }
    // else
    // {
    //     struct list_node *search_for_prev = list_node_head;
    //     struct list_node *previous = NULL;
    //     if(list_node_head > new_node_free)
    //     {
    //         new_node_free->next=list_node_head;
    //         list_node_head=new_node_free;

    //         // struct list_node *list_node_after_size=PTR_ADD(list_node_head,sizeof(struct list_node));
    //         // struct list_node *list_node_after_block=PTR_ADD(list_node_after_size,list_node_head->size);
    //         // if(list_node_after_block==new_node_free)
    //         // {
    //         //     new_node_free->size= new_node_free->size + list_node_head->size + sizeof(struct list_node);
    //         //     new_node_free->next=list_node_head;
    //         //     list_node_head=new_node_free;
    //         // }

    //     }
    //     else
    //     {
    //         while (search_for_prev->next != NULL)
    //         {
    //             if (search_for_prev->next > new_node_free)
    //             {
    //                 previous = search_for_prev;
    //                 new_node_free->next=previous->next;
    //                 previous->next=new_node_free;
    //                 break;
    //             }
    //             search_for_prev = search_for_prev->next;
    //         }
    //         if(previous==NULL)
    //         {
    //             previous=search_for_prev;
    //             previous->next=new_node_free;
    //         }


            // if(new_node_free->next!=NULL)
            // {
            //     struct list_node *new_after_size=PTR_ADD(new_node_free,sizeof(struct list_node));
            //     struct list_node *new_after_block_size=PTR_ADD(new_after_size,new_node_free->size);
            //     if(new_after_block_size==new_node_free->next)
            //     {
            //         new_node_free->size = new_node_free->size + new_node_free->next->size + sizeof(struct list_node);
            //         new_node_free->next = new_node_free ->next->next;
            //     }
            // }

            // struct list_node *previous_of_new=list_node_head;
            // while(previous_of_new->next!=new_node_free)
            // {
            //     previous_of_new=previous_of_new->next;
            // }
            // struct list_node *previous_after_adding_size=PTR_ADD(previous_of_new,sizeof(struct list_node));
            // struct list_node *previous_after_adding_block_size=PTR_ADD(previous_after_adding_size,previous_of_new->size);
            // if(previous_after_adding_block_size==new_node_free)
            // {
            //     previous_of_new->size = previous_of_new->size + new_node_free ->size + sizeof(struct list_node);
            //     previous_of_new->next=new_node_free->next;
            // }

            

        //}

    //}
    if (ptr == NULL)
    {
        return;
    }

    struct header *ptr_header = (void *)PTR_SUB(ptr, sizeof(struct header));

    if (ptr_header->size == 0)
    {
        return;
    }

    size_t size_freed = ptr_header->size + sizeof(struct header);

    if (size_freed <= sizeof(struct list_node))
    {
        return;
    }

    struct list_node *new_free_node = (void *)ptr_header;
    set_node(new_free_node, (size_freed - sizeof(struct list_node)), NULL);

    struct list_node *prev_node = NULL;
    struct list_node *next_node = list_node_head;
    while (next_node != NULL && next_node < new_free_node)
    {
        prev_node = next_node;
        next_node = next_node->next;
    }

    if (next_node == list_node_head)
    {
        list_node_head = new_free_node;
    }
    else
    {
        prev_node->next = new_free_node;
    }
    new_free_node->next = next_node;

    coalesce(new_free_node);

    LOG_DEBUG("free(%p)\n", ptr);
}

void *mm_realloc(void *ptr, size_t size)
{
    LOG_DEBUG("realloc(%p, %ld)\n", ptr, size);
    return NULL;
}