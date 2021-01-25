#ifndef Array_h
#define Array_h

#ifndef STORAGE_SIZE
#define STORAGE_SIZE 10
#endif
#include "Arduino.h"
template<class T>
struct CSimpleArraySleepStorage
{
  typedef int size_type;
  T data_storage[STORAGE_SIZE];
  size_type data_size;
  size_type elements;
};
template<class T>
class CSimpleArray
{
    typedef int size_type;

  public:
    CSimpleArray() : data_storage(NULL), data_size(0), elements(0) {
      dummy = (T*)new char[sizeof(T)];
    }
    ~CSimpleArray() {
      // RemoveAll();
      if (data_storage)
        delete[] (char*)data_storage;
       if(dummy)
        delete[] (char*)dummy;
    }
	void RemoveAll() {
		if (data_storage)
			delete[](char*)data_storage;
		data_storage = NULL;
		data_size = 0;
		elements = 0;
	}
    //void SerializeToStorage(){
    //  SerializeToStorage(ArrayStorage);
    //}
    void SerializeToStorage(CSimpleArraySleepStorage<T> &storage){
        size_type count=data_size;
        count=count>STORAGE_SIZE?STORAGE_SIZE:count;
        storage.data_size=count;
        storage.elements=elements>STORAGE_SIZE?STORAGE_SIZE:elements;
        memcpy(storage.data_storage,data_storage,sizeof(T)*count);
    }
    //void RestoreFromStorage(){
    //  RestoreFromStorage(ArrayStorage);
    //}
    void RestoreFromStorage(CSimpleArraySleepStorage<T> &storage){
        if(data_storage)
          delete[] data_storage;
        data_size=storage.data_size;
        elements=storage.elements;
        data_storage= new T[data_size];
        memcpy(data_storage,storage.data_storage,sizeof(T)*data_size);
    }
    size_type GetSize() const {
      return elements;
    }
    void Add(T & t) {
      if (data_size <= elements) {
        Realloc(data_size + 10);
      }
      elements++;
      SetAt(elements - 1, t);

    }
    void AddWithShiftLeft(T & t) {
      if (data_size == 0)
        return;
      if (data_size == elements) {
        memcpy(data_storage, data_storage + 1, sizeof(T) * (data_size - 1));
        data_storage[data_size - 1] = t;
      }
      else {
        Add(t);
      }
    }
	bool Dequeue(T* element) {
		if (elements == 0)
			return false;
		*element = GetAt(0);
		memcpy(data_storage, data_storage + 1, sizeof(T) * (data_size - 1));
		elements--;
		return true;
	}
    bool SetAt(size_type index, T & t) {
      if (index < 0 || index >= data_size)
        return false;
      data_storage[index] = t;
      return true;
    }
    //Just reset  without deleting buffer
    size_type SetSize(size_type size){
      if(size>elements)
        return -1;
        elements=size;
      return elements;
    }
    T Sum(){
      T sum=0;
      for (int i = 0; i<elements; i++){
        sum+=data_storage[i];
      }
      return sum;
    }
    T& Max() {
      return Select([](const T & t1, const T & t2)-> bool{return t1 > t2;});
      /*
        if (elements == 0)
        return dummy[0];

        size_type idx_cur = 0;

        for (int i = 1; i<elements; i++){
        idx_cur = data_storage[idx_cur]>data_storage[i] ? idx_cur : i;
        }
        return data_storage[idx_cur];
      */
    }
    T& Min() {
      return Select([](const T & t1, const T & t2)-> bool {return  t1 < t2;});
      /*
        if (elements == 0)
        return dummy[0];

        size_type idx_cur = 0;

        for (int i = 1; i<elements; i++){

        idx_cur = data_storage[idx_cur]<data_storage[i] ? idx_cur : i;
        }
        return data_storage[idx_cur];
      */
    }
    T& GetAt(size_type index) const {
      if (index < 0 || index >= data_size)
        return dummy[0];
      return data_storage[index];
    }
    T& operator[] (size_type index) const {
      return GetAt(index);
    }
    T& Select(bool(*fcompare)(const T&, const T&)) {
      if (elements == 0)
        return dummy[0];
      size_type idx_cur = 0;
      for (int i = 1; i < elements; i++) {
        idx_cur =  fcompare(data_storage[idx_cur], data_storage[i]) ? idx_cur : i;
      }
      return data_storage[idx_cur];
    }
	String toJsonArray(T min, T max) {
		String res = "[";

		for (int i = 0; i < elements; i++) {
			res += String(constrain(data_storage[i], min, max));
			if (i != (elements - 1))
				res += ",";
		}
		res += "]";
		return res;
	}
  private:
    void Realloc(size_type newsize) {
      T* olddata = data_storage;
      data_storage =(T*) new char[sizeof(T)*newsize]; //new T[newsize];
      memset((char*)data_storage, 0, sizeof(T)*newsize);
      if (data_size != 0) {
        memcpy(data_storage, olddata, sizeof(T)*data_size);
       // delete[] (char*)olddata;
      }
	  if(olddata)
		  delete[](char*)olddata;
      data_size = newsize;
    }

  
  protected:
    T* data_storage;
    size_type data_size;
    size_type elements;
    T* dummy;
    //RTC_DATA_ATTR  CSimpleArraySleepStorage<T> ArrayStorage;
};

#endif
