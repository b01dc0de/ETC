#ifndef HAVERSINE_REF0_INL
#define HAVERSINE_REF0_INL

namespace Haversine_Ref0
{
    template <typename T>
    struct DynamicArray
    {
        int Capacity;
        int Num;
        T* Data;

        static constexpr int DefaultCapacity = 32;
        static constexpr int GrowRate = 2;
        DynamicArray()
            : Capacity(DefaultCapacity)
            , Num(0)
            , Data(new T[DefaultCapacity])
        {
        }
        DynamicArray(int InitCapacity)
            : Capacity(InitCapacity)
            , Num(0)
            , Data(new T[DefaultCapacity])
        {
        }
        int Resize(int NewCapacity)
        {
            if (NewCapacity > Capacity)
            {
                int OldCapacity = Capacity;
                T* OldData = Data;

                Capacity = NewCapacity;
                Data = new T[NewCapacity];
                (void)memcpy(Data, OldData, sizeof(T)*Num);

                delete[] OldData;
            }

            return Capacity;
        }
        int Add(T Item)
        {
            if (Num >= Capacity)
            {
                Resize(Capacity * GrowRate);
            }
            int NewItemIdx = Num;
            Data[Num++] = Item;
            return NewItemIdx;
        }
        T* Add_RetPtr(T Item)
        {
            int NewItemIdx = Add(Item);
            return &Data[NewItemIdx];
        }
        void RemoveLast()
        {
            // NOTE: We aren't doing _any_ cleanup here for now
            Num--;
        }
        T& operator[](int Idx)
        {
            return Data[Idx];
        }
        T& Last()
        {
            if (Num > 0) return Data[Num-1];
            else return Data[0];
        }
    };
}

#endif // HAVERSINE_REF0_INL
