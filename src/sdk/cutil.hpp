#pragma once

inline int UtlMemory_CalcNewAllocationCount(int nAllocationCount, int nGrowSize, int nNewSize, int nBytesItem)
{
	if (nGrowSize)
		nAllocationCount = ((1 + ((nNewSize - 1) / nGrowSize)) * nGrowSize);
	else
	{
		if (!nAllocationCount)
			nAllocationCount = (31 + nBytesItem) / nBytesItem;

		while (nAllocationCount < nNewSize)
			nAllocationCount *= 2;
	}

	return nAllocationCount;
}

template <class T, class N = int>
class CUtlMemory
{
public:
	T& operator[](N i)
	{
		return pMemory[i];
	}

	const T& operator[](N i) const
	{
		return pMemory[i];
	}

	T* Base()
	{
		return pMemory;
	}

	int NumAllocated() const
	{
		return iAllocationCount;
	}

	void Grow(int iNum = 1)
	{
		if (IsExternallyAllocated())
			return;

		int iOldAllocationCount = iAllocationCount;
		int iAllocationRequested = iAllocationCount + iNum;
		int iNewAllocationCount = UtlMemory_CalcNewAllocationCount(iAllocationCount, iGrowSize, iAllocationRequested, sizeof(T));

		if (static_cast<int>(static_cast<N>(iNewAllocationCount)) < iAllocationRequested)
		{
			if (static_cast<int>(static_cast<N>(iNewAllocationCount)) == 0 && static_cast<int>(static_cast<N>(iNewAllocationCount - 1)) >= iAllocationRequested)
				--iNewAllocationCount;
			else
			{
				if (static_cast<int>(static_cast<N>(iAllocationRequested)) != iAllocationRequested)
				{
					return;
				}

				while (static_cast<int>(static_cast<N>(iNewAllocationCount)) < iAllocationRequested)
					iNewAllocationCount = (iNewAllocationCount + iAllocationRequested) / 2;
			}
		}

		iAllocationCount = iNewAllocationCount;

		if (pMemory != nullptr)
		{
			//pMemory = reinterpret_cast<T*>(I::MemAlloc->Realloc(pMemory, iAllocationCount * sizeof(T)));

			std::byte* pData = new std::byte[iAllocationCount * sizeof(T)];
			memcpy(pData, pMemory, iOldAllocationCount * sizeof(T));
			pMemory = reinterpret_cast<T*>(pData);
		}
		else
			//pMemory = reinterpret_cast<T*>(I::MemAlloc->Alloc(iAllocationCount * sizeof(T)));
			pMemory = reinterpret_cast<T*>(new std::byte[iAllocationCount * sizeof(T)]);
	}

	bool IsExternallyAllocated() const
	{
		return iGrowSize < 0;
	}

protected:
	T*	pMemory;
	int iAllocationCount;
	int iGrowSize;
};


template <class T, class A = CUtlMemory<T>>
class CUtlVector
{
	using CAllocator = A;
public:
	auto begin() noexcept
	{
		return pMemory.Base();
	}

	auto end() noexcept
	{
		return pMemory.Base() + iSize;
	}

	auto begin() const noexcept
	{
		return pMemory.Base();
	}

	auto end() const noexcept
	{
		return pMemory.Base() + iSize;
	}

	T& operator[](int i)
	{
		return pMemory[i];
	}

	const T& operator[](int i) const
	{
		return pMemory[i];
	}

	T& Element(int i)
	{
		return pMemory[i];
	}

	const T& Element(int i) const
	{
		return pMemory[i];
	}

	T* Base()
	{
		return pMemory.Base();
	}

	int Count() const
	{
		return iSize;
	}

	int& Size()
	{
		return iSize;
	}

	void GrowVector(int nCount = 1)
	{
		if (iSize + nCount > pMemory.NumAllocated())
			pMemory.Grow(iSize + nCount - pMemory.NumAllocated());

		iSize += nCount;
	}

	void ShiftElementsRight(const int nElement, const int nShift = 1)
	{
		const int nToMove = iSize - nElement - nShift;

		if (nToMove > 0 && nShift > 0)
			memmove(&Element(nElement + nShift), &Element(nElement), nToMove * sizeof(T));
	}

	void ShiftElementsLeft(const int nElement, const int nShift = 1)
	{
		const int nToMove = iSize - nElement - nShift;

		if (nToMove > 0 && nShift > 0)
			memmove(&Element(nElement), &Element(nElement + nShift), nToMove * sizeof(T));
	}

	int InsertBefore(const int nElement)
	{
		// can insert at the end
		GrowVector();
		ShiftElementsRight(nElement);
		Construct(&Element(nElement));
		return nElement;
	}

	int InsertBefore(int nElement, const T& src)
	{
		// reallocate if can't insert something that's in the list
		// can insert at the end
		GrowVector();
		ShiftElementsRight(nElement);
		Copy(&Element(nElement), src);
		return nElement;
	}

	int AddToTail()
	{
		return InsertBefore(iSize);
	}

	int AddToTail(const T& src)
	{
		return InsertBefore(iSize, src);
	}

	int Find(const T& src) const
	{
		for (int i = 0; i < Count(); ++i)
		{
			if (Element(i) == src)
				return i;
		}

		return -1;
	}

	void Remove(const int nElement)
	{
		Destruct(&Element(nElement));
		ShiftElementsLeft(nElement);
		--iSize;
	}

	void RemoveAll()
	{
		for (int i = iSize; --i >= 0;)
			Destruct(&Element(i));

		iSize = 0;
	}

	bool FindAndRemove(const T& src)
	{
		if (const int nElement = Find(src); nElement != -1)
		{
			Remove(nElement);
			return true;
		}

		return false;
	}

protected:
	CAllocator pMemory;
	int iSize;
	T* pElements;
};