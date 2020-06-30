#include "ring_buffer.h"
#include <stdio.h>
#include <assert.h>

int main()
{
    ring_buffer r;
    printf("TESTING INITIALIZE...");
    ring_buffer_init(&r);
    assert(r.head == 0);
    assert(r.tail == 0);
    assert(r.length == RING_BUFFER_LENGTH);
    assert(r.buffer != NULL);
    printf("Passed\n");

    printf("\nTESTING AVAILABLE SPACE\n");
    printf("Empty ring buffer available space: ");
    assert(ring_buffer_available_space(&r) == RING_BUFFER_LENGTH - 1);
    printf("Passed\n");

    printf("\nTESTING RING BUFFER WRITE\n");
    char test[] = "18864918344475718003462424500970977059896717062855";
    printf("Writing 50 characters to the ring buffer: ");
    assert(ring_buffer_write(&r, test, 50) == 0);
    printf("Passed\n");

    char test2[] = "E09PUOXKWKT6WNJDO5RPMEJAHOT0XD0NGV797BPQDT6X3UJX4BV29G1AS0AWUOP5XSQMNNXY1VB94C6AQL38SO0G9027V940DJWD";
    printf("Writing 100 characters to the ring buffer: ");
    assert(ring_buffer_write(&r, test2, 100) == 0);
    printf("Passed\n");

    printf("Writing 100 characters to the ring buffer (should fail): ");
    assert(ring_buffer_write(&r, test2, 100) == -1);
    printf("Passed\n");

    printf("Writing 50 characters to the ring buffer (should fail): ");
    assert(ring_buffer_write(&r, test, 50) == -1);
    printf("Passed\n");

    printf("Writing 49 characters to the ring buffer: ");
    assert(ring_buffer_write(&r, test, 49) == 0);
    printf("Passed\n");

    printf("\nTESTING AVAILABLE SPACE\n");
    printf("Full ring buffer available space: ");
    assert(ring_buffer_available_space(&r) == 0);
    printf("Passed\n");
    printf("Full ring buffer available data: ");
    assert(ring_buffer_available_data(&r) == RING_BUFFER_LENGTH - 1);
    printf("Passed\n");

    printf("\nTESTING READ\n");
    char test3[] = "18864918344475718003462424500970977059896717062855E09PUOXKWKT6WNJDO5RPMEJAHOT0XD0NGV797BPQDT6X3UJX4BV29G1AS0AWUOP5XSQMNNXY1VB94C6AQL38SO0G9027V940DJWD1886491834447571800346242450097097705989671706285";
    int i;
    char target;
    printf("Reading RING_BUFFER_LENGTH - 1 characters and matching with test string: \n");
    printf("Gold      : %s\n", test3);
    printf("Actual    : ");
    for (i = 0; i < RING_BUFFER_LENGTH - 1; i++)
    {
        assert(ring_buffer_read(&r, &target, 1) == 0);
        printf("%c", target);
    }
    printf("\n");

    printf("\nSPACE AFTER READ\n");
    printf("Space available after read : ");
    assert(ring_buffer_available_space(&r) == RING_BUFFER_LENGTH - 1);
    printf("Passed\n");
    printf("Data available after read : ");
    assert(ring_buffer_available_data(&r) == 0);
    printf("Passed\n");

    printf("\nTESTING CIRCULARNESS\n");
    printf("Inserting 25 characters : ");
    assert(ring_buffer_write(&r, test3, 25) == 0);
    printf("Passed\n");
    char targetarr[16];
    printf("Removing 15 characters : ");
    assert(ring_buffer_read(&r, targetarr, 15) == 0);
    printf("Passed\n");
    printf("Testing remaining space : ");
    assert(ring_buffer_available_space(&r) == 189);
    printf("Passed\n");
    printf("Inserting 189 characters (Remaining space) : ");
    assert(ring_buffer_write(&r, test3, ring_buffer_available_space(&r)) == 0);
    printf("Passed\n");
    char test4[] = "180034624218864918344475718003462424500970977059896717062855E09PUOXKWKT6WNJDO5RPMEJAHOT0XD0NGV797BPQDT6X3UJX4BV29G1AS0AWUOP5XSQMNNXY1VB94C6AQL38SO0G9027V940DJWD1886491834447571800346242450097097705989671706285";
    printf("Reading All Data : ");
    for (i = 0; i < RING_BUFFER_LENGTH - 1; i++)
        assert(ring_buffer_read(&r, &target, 1) == 0 && target == test4[i]);
    printf("Passed\n");

    printf("\nPASSED ALL UNIT TESTS\n");
    return 0;
}
    
