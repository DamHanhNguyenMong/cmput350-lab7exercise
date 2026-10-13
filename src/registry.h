#ifndef REGISTRY_H
#define REGISTRY_H

#include <vector>
#include <cstddef>
#include <cassert>
#include <utility>

template <typename T>
class Registry {
private:
    std::vector<T> mData;
    std::vector<bool> mValid;
    std::vector<size_t> mFreeList;
    size_t mSize = 0;
public:
    // Store by copying
    size_t store(const T& value) {
        // if there are free slots, use one of them
        if (!mFreeList.empty()) {
            size_t index = mFreeList.back(); // get the most recently freed ticket
            mFreeList.pop_back(); // remove it from the free list
            mData[index] = value; // add the value to the data vector at index
            mValid[index] = true;
            mSize++;
            return index;
        } else {
            // else add a new slot at the end of the vector
            mData.push_back(value);
            mValid.push_back(true);
            mSize++;
            return mData.size() - 1;
        }
    }

    // Store by moving
    size_t store(T&& value) {
        if (!mFreeList.empty()) {
            size_t index = mFreeList.back();
            mFreeList.pop_back();
            mData[index] = std::move(value); // move the value into the data vector at index
            mValid[index] = true;
            ++mSize;

            return index;
        } else {
            mData.push_back(std::move(value));
            mValid.push_back(true);
            ++mSize;

            return mData.size() - 1;
        }
    }

    // Remove a component
    void remove(size_t index) {
        assert(valid(index)); // check that the index is valid before removing

        mData[index] = T{}; // reset the value at index to default-constructed T
        mValid[index] = false; //invalidate the index
        mFreeList.push_back(index); // add the index to the free list
        --mSize;
    } 

    // Access a component
    T& operator[](size_t index) { 
        assert(valid(index));
        return mData[index];
    }
    // Access a component (const version)
    const T& operator[](size_t index) const {
        assert(valid(index));
        return mData[index];
    }

    // Check whether a ticket is valid
    bool valid(size_t index) const {
        // it is valid if the index is within the bounds of the mValid vector 
        // and the corresponding entry in mValid is true
        return index < mValid.size() && mValid[index];
    }

    // Number of active components
    size_t size() const {
        return mSize;
    }

};

#endif  // REGISTRY_H
