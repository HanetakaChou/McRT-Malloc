#include "../../Runtime/Public/System/PTSMemoryAllocator.h"
#include "../../Runtime/Public/System/PTSTaskScheduler.h"
#include "../../Runtime/Public/System/PTSThread.h"
#include <iostream>

int main()
{
	::PTSMemoryAllocator_Initialize();
	::PTSTaskScheduler_Initialize();
	::PTSTaskScheduler_Initialize_ForThread();

	IPTSTaskScheduler *pTaskScheduler = ::PTSTaskScheduler_ForThread();

	int64_t TickPerSecond = PTSTick_Frequency();
	int64_t TickStart;
	int64_t TickEnd;

	int64_t SumForMap = 0;

	TickStart = ::PTSTick_Count();

	pTaskScheduler->Worker_Wake();

	::PTSParallel_Map(
		1, 0, 1000,
		[&SumForMap](uint32_t _0_Begin, uint32_t _0_End)->void
	{
		//Nested-Parallelism
		::PTSParallel_Map(
			1, 0, 1000,
			[&SumForMap,_0_Begin, _0_End](uint32_t _1_Begin, uint32_t _1_End)->void
		{
			//原子操作本质上仍是互斥，会抑制并发
			::PTSAtomic_GetAndAdd(&SumForMap, 1000 * _0_Begin + _1_Begin);
		});
	});

	pTaskScheduler->Worker_Sleep();

	TickEnd = ::PTSTick_Count();

	std::cout << (static_cast<float>(TickEnd - TickStart) / static_cast<float>(TickPerSecond))*1000.0f << std::endl;
	std::cout << SumForMap << std::endl;

	for (int i = 0; i < 5000000; ++i)
	{
		::PTS_Yield();
	}

	int64_t SumForReduce = 0;

	TickStart = ::PTSTick_Count();

	pTaskScheduler->Worker_Wake();

	::PTSParallel_Reduce(
		1, 0, 1000000, 
		SumForReduce,
		[](uint32_t Begin, uint32_t End)->uint32_t 
	{
		return Begin;
	}, 
		[](int64_t X1, int64_t X2)->int64_t
	{
		return X1 + X2;
	});

	pTaskScheduler->Worker_Sleep();

	TickEnd = ::PTSTick_Count();

	std::cout << (static_cast<float>(TickEnd - TickStart) / static_cast<float>(TickPerSecond))*1000.0f << std::endl;
	std::cout << SumForReduce << std::endl;

    return 0;
}

