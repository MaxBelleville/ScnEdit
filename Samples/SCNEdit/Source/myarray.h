#ifndef MY_ARRAY_H_
#define MY_ARRAY_H_

#include <irrlicht.h>
#include "irrTypes.h"
#include "irrAllocator.h"

#define arrayu myarray<u32>

#define foreach(T,x,y) for(myarray<T>::iterator x=y.startIteration(); !x.hasEnded(); x.next())

using namespace irr;
using namespace core;

template <class T, typename TAlloc = irrAllocator<T> >
class myarray
{

public:

        //doesn't seem to work properly

        /*class iterator      //added by AA, use in for loops (myarray::iterator r=rs.startIteration(); r.hasEnded(); r++) ... *r ...
        {
            public:
            iterator(myarray * theparent)
            {
                idx = 0;
                parent = theparent;
            }

            void begin()
            {
                idx = 0;
            }
            bool hasEnded()
            {
                return (idx>=parent->used);
            }
            void next() {idx++;}

            T data()
            {
                return parent->data[idx];
            }
            //T& operator * () {return parent->data[idx];}
            //T * operator -> () {return &parent->data[idx];}
            //iterator& operator= (iterator& iter) {return *this;}
            private:
                u32 idx;
                myarray * parent;
        };

        iterator& startIteration()
        {
            iter.begin();
            return iter;
        }

        friend class iterator;*/

        myarray()
                : data(0), allocated(0), used(0),
                        free_when_destroyed(true), is_sorted(true), it(0)//,iter(this)
        {
        }

        myarray(u32 start_count)
                : data(0), allocated(0), used(0),
                        free_when_destroyed(true), is_sorted(true), it(0)//,iter(this)
        {
                reallocate(start_count);
        }


        myarray(const myarray<T>& other)
                : data(0)//, iter(this)
        {
                *this = other;
        }

        myarray(core::array<T>& other) //added by AA: build myarray from irr array
                : data(0) //,iter(this)
        {
                *this = other; //use operator defined below
        }


        ~myarray()
        {
                if (free_when_destroyed)
                {
                        for (u32 i=0; i<used; ++i)
                                allocator.destruct(&data[i]);

                        allocator.deallocate(data);
                }
        }


        u32 it_index()
        {
            return it;
        }

        //iterator, added by AA, for while loops
        bool iterate(T& element)
        {
            if (it>=used)
            {
                it = 0;             //reset iterator
                return false;       //stop iterator
            }
            else
            {
                element = data[it++]; //does this copy element?
                return true;        //continue iterator
            }
        }


        void reallocate(u32 new_size)
        {
                T* old_data = data;

                data = allocator.allocate(new_size); //new T[new_size];
                allocated = new_size;

                // copy old data
                s32 end = used < new_size ? used : new_size;

                for (s32 i=0; i<end; ++i)
                {
                        // data[i] = old_data[i];
                        allocator.construct(&data[i], old_data[i]);
                }

                // destruct old data
                for (u32 j=0; j<used; ++j)
                        allocator.destruct(&old_data[j]);

                if (allocated < used)
                        used = allocated;

                allocator.deallocate(old_data); //delete [] old_data;
        }

        void push_back(const T& element)
        {
                if (used + 1 > allocated)
                {
                        // reallocate(used * 2 +1);
                        // this doesn't work if the element is in the same myarray. So
                        // we'll copy the element first to be sure we'll get no data
                        // corruption

                        T e(element);
                        reallocate(used * 2 +1); // increase data block

                        allocator.construct(&data[used++], e); // data[used++] = e;  // push_back
                }
                else
                {
                        //data[used++] = element;
                        // instead of using this here, we copy it the safe way:
                        allocator.construct(&data[used++], element);
                }

                is_sorted = false;
        }
        void push_back(core::array<T>& other) //added by AA
        {
            if (used+other.size() > allocated)
                reallocate(used + other.size());

            for (u32 i=0; i<other.size(); i++)
                push_back(other[i]);
        }
        void push_back(const core::array<T>& other) //added by AA
        {
            if (used+other.size() > allocated)
                reallocate(used + other.size());

            for (u32 i=0; i<other.size(); i++)
                push_back(other[i]);
        }


        void push_back(myarray<T>& other) //added by AA
        {
            for (u32 i=0; i<other.size(); i++)
                push_back(other[i]);
        }

        void push_front(const T& element)
        {
                if (used + 1 > allocated)
                        reallocate(used +1);

                for (u32 i=used; i>0; --i)
                {
                        //data[i] = data[i-1];
                        allocator.construct(&data[i], data[i-1]);
                }

                // data[0] = element;
                allocator.construct(&data[0], element);

                is_sorted = false;
                ++used;
        }


        void insert(const T& element, u32 index=0)
        {
                _IRR_DEBUG_BREAK_IF(index>used) // access violation

                if (used + 1 > allocated)
                        reallocate(used +1);

                for (u32 i=used++; i>index; --i)
                        allocator.construct(&data[i], data[i-1]); // data[i] = data[i-1];

                allocator.construct(&data[index], element); // data[index] = element;
                is_sorted = false;
        }




        void clear()
        {
                for (u32 i=0; i<used; ++i)
                        allocator.destruct(&data[i]);

                allocator.deallocate(data); // delete [] data;
                data = 0;
                used = 0;
                allocated = 0;
                is_sorted = true;
        }



        void set_pointer(T* newPointer, u32 size)
        {
                for (u32 i=0; i<used; ++i)
                        allocator.destruct(&data[i]);

                allocator.deallocate(data); // delete [] data;
                data = newPointer;
                allocated = size;
                used = size;
                is_sorted = false;
        }



        void set_free_when_destroyed(bool f)
        {
                free_when_destroyed = f;
        }




        void set_used(u32 usedNow)
        {
                if (allocated < usedNow)
                        reallocate(usedNow);

                used = usedNow;
        }



        void operator=(const myarray<T>& other)
        {
                if (data)
                {
                        for (u32 i=0; i<used; ++i)
                                allocator.destruct(&data[i]);

                        allocator.deallocate(data); // delete [] data;
                }

                //if (allocated < other.allocated)
                if (other.allocated == 0)
                        data = 0;
                else
                        data = allocator.allocate(other.allocated); // new T[other.allocated];

                used = other.used;
                free_when_destroyed = other.free_when_destroyed;
                is_sorted = other.is_sorted;
                allocated = other.allocated;
                it = other.it;

                for (u32 i=0; i<other.used; ++i)
                        allocator.construct(&data[i], other.data[i]); // data[i] = other.data[i];

        }

        void operator=(const core::array<T>& other) //added by AA, slightly different because we don't have access to array's privates
        {
                if (data)
                {
                        for (u32 i=0; i<used; ++i)
                                allocator.destruct(&data[i]);

                        allocator.deallocate(data); // delete [] data;
                }

                //if (allocated < other.allocated)
                if (other.allocated_size() == 0)
                        data = 0;
                else
                        data = allocator.allocate(other.allocated_size()); // new T[other.allocated];

                used = other.size();
                free_when_destroyed = true; //assume true
                is_sorted = false; //assume not
                allocated = other.allocated_size();
                it = 0; //set iterator 0

                for (u32 i=0; i<other.size(); ++i)
                        allocator.construct(&data[i], other[i]); // data[i] = other.data[i];

        }


        // equality operator
        bool operator == (const myarray<T>& other) const
        {
                if (used != other.used)
                        return false;

                for (u32 i=0; i<other.used; ++i)
                        if (data[i] != other[i])
                                return false;
                return true;
        }

        // inequality operator
        bool operator != (const myarray<T>& other) const
        {
                return !(*this==other);
        }

        T& operator [](u32 index)
        {
                _IRR_DEBUG_BREAK_IF(index>=used) // access violation

                return data[index];
        }



        const T& operator [](u32 index) const
        {
                _IRR_DEBUG_BREAK_IF(index>=used) // access violation

                return data[index];
        }

        T& getLast()
        {
                _IRR_DEBUG_BREAK_IF(!used) // access violation

                return data[used-1];
        }


        const T& getLast() const
        {
                _IRR_DEBUG_BREAK_IF(!used) // access violation

                return data[used-1];
        }

        T* pointer()
        {
                return data;
        }



        const T* const_pointer() const
        {
                return data;
        }



        u32 size() const
        {
                return used;
        }



        u32 allocated_size() const
        {
                return allocated;
        }



        bool empty() const
        {
                return used == 0;
        }



        void sort()
        {
                if (is_sorted || used<2)
                        return;

                heapsort(data, used);
                is_sorted = true;
        }



        s32 binary_search(const T& element)
        {
                sort();
                return binary_search(element, 0, used-1);
        }

        s32 binary_search_const(const T& element) const
        {
                return binary_search(element, 0, used-1);
        }



        s32 binary_search(const T& element, s32 left, s32 right) const
        {
                if (!used)
                        return -1;

                s32 m;

                do
                {
                        m = (left+right)>>1;

                        if (element < data[m])
                                right = m - 1;
                        else
                                left = m + 1;

                } while((element < data[m] || data[m] < element) && left<=right);

                // this last line equals to:
                // " while((element != myarray[m]) && left<=right);"
                // but we only want to use the '<' operator.
                // the same in next line, it is "(element == myarray[m])"

                if (!(element < data[m]) && !(data[m] < element))
                        return m;

                return -1;
        }


        s32 linear_search(const T& element) const
        {
                for (u32 i=0; i<used; ++i)
                        if (element == data[i])
                                return (s32)i;

                return -1;
        }


        s32 linear_reverse_search(const T& element) const
        {
                for (s32 i=used-1; i>=0; --i)
                        if (data[i] == element)
                                return i;

                return -1;
        }



        void erase(u32 index)
        {
                _IRR_DEBUG_BREAK_IF(index>=used) // access violation

                for (u32 i=index+1; i<used; ++i)
                {
                        allocator.destruct(&data[i-1]);
                        allocator.construct(&data[i-1], data[i]); // data[i-1] = data[i];
                }

                allocator.destruct(&data[used-1]);

                --used;
        }


        void erase(u32 index, s32 count)
        {
                _IRR_DEBUG_BREAK_IF(index>=used || count<1 || index+count>used) // access violation

                u32 i;
                for (i=index; i<index+count; ++i)
                        allocator.destruct(&data[i]);

                for (i=index+count; i<used; ++i)
                {
                        if (i > index+count)
                                allocator.destruct(&data[i-count]);

                        allocator.construct(&data[i-count], data[i]); // data[i-count] = data[i];

                        if (i >= used-count)
                                allocator.destruct(&data[i]);
                }

                used-= count;
        }


        void set_sorted(bool _is_sorted)
        {
                is_sorted = _is_sorted;
        }


        private:

                T* data;
                u32 allocated;
                u32 used;
                bool free_when_destroyed;
                bool is_sorted;
                TAlloc allocator;
                //iterator iter;

                u32 it;
};

#endif


