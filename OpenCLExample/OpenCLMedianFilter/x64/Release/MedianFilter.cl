
#define m_width  512 
#define m_height  512 
#define vSkipSize  5
#define hSkipSize  5

void swap(int* A, int* B)
{
    int tmp;
	tmp = *A;
	*A = *B;
	*B = tmp;
}

void bubbleSort(int* kArray, int size)
{
	for(int i= 0; i< size -1 ; i++)
	{
		for(int j=0; j < size-i; j++ )
		{
		    if(kArray[j] > kArray[j+1])
		    {
			    swap(&kArray[j], &kArray[j+1]);
		    }
		}
	}
}

int calcMedian(int* kArray, int size)
{
    int median;
	
	bubbleSort(kArray, size);

	if (size %2 ==0)
	{
		median = (kArray[size / 2 - 1] + kArray[size / 2 ]) /2;
	}
	else
	{
		median = kArray[(size-1) / 2];
	}

	return median;
}


__kernel void MedianFilter(__global uchar *srcImageN, __global uchar *srcImageF)
{
    int idx = get_global_id(0);
    int idy = get_global_id(1); 
	
	int index = idy*m_width + idx;
	
	int kArray[25];
	
	int index_k = 0;
	int centerSeparate = 2;//(hSkipSize-1)/2;
	
	if(idx > 2 && idx < m_height-2 && idy > 2 && idy < m_height-2)
	{
		for(int i = idy-2; i< idy+3; i++)
		{
			for(int j =idx-2; j< idx+3; j++)
			{
				index = i*m_width+j;
				kArray[index_k] = srcImageN[index];
				index_k++;
			}
		}
	
		int median = calcMedian(kArray, 25);
		index = idy*m_width + idx;
		srcImageF[index] = median;
	}
	
}

