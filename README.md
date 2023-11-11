# MCRT-Malloc

## License  
```  
Copyright (C) YuqiaoZhang(HanetakaChou)

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>
```  

## Slab Allocator

By "3. Slab Allocator Implementation" of \[Bonwick 1994\], the name "slab" is originally from the source code of SunOS, and has no special meaning. (The name "slab" was chosen because people felt "slab" was more recognizable than "object" or "cache" when they were writing the source code of SunOS.)  
   
By "4.4. Arena Management" of \[Bonwick 1994\], the strategies how the allocators manage the arenas ([region](https://en.wikipedia.org/wiki/Region-based_memory_management)s) can be divided into three categories: "sequential-fit", "buddy" and "segregated-storage".  

Evidently, the slab allocator belongs to the "segregated-storage" category. The slab allocator is introduced in detail by "3. Slab Allocator Implementation" and "4. Hardware Cache Effects" of \[Bonwick 1994\].  

![](README-1.svg)  

All slabs are the same size. And the memory of the slab is allocated from the backend allocator. (The backend allocator is an allocator of which the grain is more coarse than the slab allocator. For example, we can request memory directly from the operating system, and the minimum allowed size is the page size, which can be used as the slab size. On POSIX operating systems, **sysconf(_SC_PAGESIZE)** can be used to query the size of a page, and **mmap** can be used to request the memory from the operating system).  

One slab consists of several buffers of the same size. The control block (slab data) is placed at the end of a slab. (By "3.2.2. Slab Layout for Small objects" of \[Bonwick 1994\], according to the empirical observation, when the application attempts to illegaly modify the memory which has been freed, the data at the beginning of the slab is more likely to be modified. Evidently, empirical observation is usually unreliable. In McRT-Malloc by \[Kukanov 2007\], the control block is placed at the beginning of a slab. And this makes it much more efficient to distinguish whether the memory is a buffer or a large object, which we will cover later.)  

One cache has several slabs. All buffers in all slabs within the same cache are the same size. The control blocks (slab data) of the slabs within the same cache form a doubly-linked list. Each cache holds a header pointer (freelist pointer) which points to one slab from the doubly-linked list and ensure that this slab and all succeeding (next) slabs contain at least one free buffer.  

When a buffer is freed, we can check the number of the unfreed buffers within same slab as the current buffer. When all buffers are freed, the slab will be insert to the tail of the doubly-linked list to ensure that the slab, which the header pointer (freelist pointer) of the cache points to, and all succeeding slabs contain at least one free buffer. (When a buffer is freed, we only need to align the address of the buffer to the slab size to get the address of the slab where the buffer is located. It is a trivial operation to check the number of the unfreed buffers within a slab, since a counter can be maintained in the control block (slab data) of a slab.)       

Since the memory of the slab is allocated from the backend allocator, the memory of the slab should be returned to the backend allocator when all buffers within the slab are freed. However, by "3.4. Reclaiming Memory" of \[Bonwick 1994\], if the number of the slabs within one cache is less than a certain threshould (working-set), the slab will not be returned to the backend allocator, but instead will be inserted into the doubly-linked list within the cache. Evidently, we can avoid the overhead of allocating and initializing slabs from the backend allocator.  

The "segregated-storage" means that the size of the buffer can only be several predefines values (for exmaple, 8b, 16b, 32b, 48b ...), and the same number of the slab doubly-linked lists, as the number of the possible buffer size values, are maintained. Each slab doubly-linked list has a one-to-one correspondence with each cache, and with each possible buffer size value. This means that all buffers within the same cache must be the same size, and buffers of which the sizes are different are segregated and stored in different caches.  

When we allocate from the slab allocator, for the request size which is less than half the slab size, we can match the smallest possible buffer size which is not less than the request size. Evidently, the matching is a trivial operation. Accodring to the matched buffer size value, we will index into the corresponding cache, and find the free buffer from a slab within the cache. For the request size which is larger than half slab size, we will request the memory from the backend allocator directly.  

By "3.2.3. Slab Layout for Large Objects" of \[Bonwick 1994\], when we return the memory to the slab allocator, a hash table should be used to distinguish whether the memory is a buffer or a large object.  

Evidently, the hash table is not efficient. Actually, the control block (slab data) can be placed at the beginning of a slab, and thus the address of a buffer can not be aligned to the slab size. However, since the large object is requested directly from the backend allocator, the address of a large object must be aligned to the slab size. Hence, when we return the memory to the slab allocator, we can check whether the address is aligned to the slab size to distinguish whether the memory is a buffer or a large object. This is the approach of McRT-Malloc by \[Kukanov 2007\].  

## McRT-Malloc  
   
By \[Bonwick 1994\], McRT-Malloc is "segregated-storage".  
   
McRT-Malloc | Slab Allocator   
:-: | :-:   
Bin | Cache  
Block | Slab  
Object | Buffer  

## References
\[Bonwick 1994\] [Jeff Bonwick. "The Slab Allocator: An Object-Caching Kernel Memory Allocator." USENIX 1994](https://www.usenix.org/legacy/publications/library/proceedings/bos94/bonwick.html)  
\[Hudson 2006\] Richard Hudson, Bratin Saha, Ali-Reza Adl-Tabatabai, Benjamin C. Hertzberg. "McRT-Malloc: a scalable transactional memory allocator". ISMM 2006.  
\[McCool 2012\] [Michael McCool, James Reinders, Arch Robison. "Structured Parallel Programming: Patterns for Efficient Computation." Morgan Kaufmann 2012.](http://parallelbook.com/)  
