#include <physicalallocator.h>

#include <paging.h>
#include <serial.h>
#include <string.h>
#include <logging.h>
#include <panic.h>
#include <lock.h>

extern void* _end;

namespace Memory{
    uint32_t physicalMemoryBitmap[PHYSALLOC_BITMAP_SIZE_DWORDS];

    uint64_t usedPhysicalBlocks = PHYSALLOC_BITMAP_SIZE_DWORDS * 32;
    uint64_t maxPhysicalBlocks = 0;

    lock_t allocatorLock = 0;

    // Initialize the physical page allocator
    void InitializePhysicalAllocator(memory_info_t* mem_info)
    {
        memset(physicalMemoryBitmap, 0xFFFFFFFF, PHYSALLOC_BITMAP_SIZE_DWORDS * sizeof(uint32_t));
        
        multiboot_memory_map_t* mem_map = mem_info->mem_map;
        multiboot_memory_map_t* mem_map_end = (multiboot_memory_map_t*)(mem_info->mem_map + mem_info->memory_map_len);

        maxPhysicalBlocks = (mem_info->memory_high + mem_info->memory_low) * 1024 / PHYSALLOC_BLOCK_SIZE;
        usedPhysicalBlocks = maxPhysicalBlocks;

        while (mem_map < mem_map_end)
        {
            Log::Info("Memory Region: [%x - %x] (Type %d)", mem_map->base, mem_map->base + mem_map->length, mem_map->type);

            if (mem_map->type == 1){
                MarkMemoryRegionFree(mem_map->base, mem_map->length);
            } else if (!mem_map->type) break;
            mem_map = (multiboot_memory_map_t*)(((uint64_t)mem_map) + mem_map->size + sizeof(mem_map->size));
        }

        MarkMemoryRegionUsed(0, (uintptr_t)&_end - KERNEL_VIRTUAL_BASE);
    }

    // Sets a bit in the physical memory bitmap
    inline void bit_set(uint64_t bit) {  
        physicalMemoryBitmap[bit >> 5] |= (1 << (bit % 32));
    }

    // Clears a bit in the physical memory bitmap
    inline void bit_clear(uint64_t bit) {
        physicalMemoryBitmap[bit >> 5] &= ~ (1 << (bit % 32));
    }

    // Tests a bit in the physical memory bitmap
    inline bool bit_test(uint64_t bit) {
        return physicalMemoryBitmap[bit >> 5] & (1 << (bit % 32));
    }

    // Finds the first free block in physical memory
    uint64_t GetFirstFreeMemoryBlock() {
        for (uint32_t i = 0; i < maxPhysicalBlocks / 32; i++)
            if (physicalMemoryBitmap[i] != 0xffffffff) // If all 32 bits at the index are used then ignore them
                for (uint32_t j = 0; j < 32; j++) // Test each bit in the dword
                    if (!(physicalMemoryBitmap[i] & (1 << j)) && (i || j))
                        return i * 32 + j;

        // The first block is always reserved
        return 0;
    }

    // Marks a region in physical memory as being used
    void MarkMemoryRegionUsed(uint64_t base, size_t size) {
        for (uint32_t blocks = size / PHYSALLOC_BLOCK_SIZE, align = base / PHYSALLOC_BLOCK_SIZE; blocks > 0; blocks--, usedPhysicalBlocks++)
            bit_set(align++);
    }

    // Marks a region in physical memory as being free
    void MarkMemoryRegionFree(uint64_t base, size_t size) {
        for (uint32_t blocks = (size + (PHYSALLOC_BLOCK_SIZE - 1)) / PHYSALLOC_BLOCK_SIZE, align = base / PHYSALLOC_BLOCK_SIZE; blocks > 0; blocks--, usedPhysicalBlocks--)
            bit_clear(align++);
    }

    // Allocates a block of physical memory
    uint64_t AllocatePhysicalMemoryBlock() {
        acquireLock(&allocatorLock);

        uint64_t index = GetFirstFreeMemoryBlock();
        if (!index){
            Log::Error("Out of memory!");
            KernelPanic((const char**)(&"Out of memory!"),1);
            for(;;);
            //return 0;
        }

        bit_set(index);
        usedPhysicalBlocks++;

        releaseLock(&allocatorLock);

        return index * PHYSALLOC_BLOCK_SIZE;
    }

    // Allocates a block of 2MB physical memory
    /*uint64_t AllocateLargePhysicalMemoryBlock() {
        uint64_t index = GetFirstFreeMemoryBlock();
        uint64_t blockCount = 0x200000
        uint64_t counter = 0;
        while(counter < blockCount){
            if(bit_test(index)){
                index++;
                counter = 0;
                continue;
            }

            index++;
            counter++;
        }

        if (index == 0)
            return 0;

        uint64_t addr = index * PHYSALLOC_BLOCK_SIZE;
        
        while(blockCount--)
            bit_set(index++);
        usedPhysicalBlocks += counter;

        return addr;
    }*/

    // Frees a block of physical memory
    void FreePhysicalMemoryBlock(uint64_t addr) {
        uint64_t index = addr / PHYSALLOC_BLOCK_SIZE;
        bit_clear(index);
        usedPhysicalBlocks--;
    }

    // Frees a block of physical memory
    void FreeLargePhysicalMemoryBlock(uint64_t addr) {
        uint64_t index = addr / PHYSALLOC_BLOCK_SIZE;
        uint64_t blockCount = 0x200000 /* 2MB */ / PHYSALLOC_BLOCK_SIZE;
        usedPhysicalBlocks-=blockCount;
        while(blockCount--)
            bit_clear(index++);
    }
}