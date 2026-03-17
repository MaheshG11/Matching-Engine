template<typename T>
MemoryPool<T>::MemoryPool(int32_t size){
    arena_ = new Slot[size];
    for(int i=0;i<size;i++){
        arena_[i].next=&arena_[i+1];
    }
    arena_[size - 1].next = nullptr;
    free_list_ = arena_;
}

template<typename T>
MemoryPool<T>::~MemoryPool(){
    delete[] arena_;
}

template<typename T>
T* MemoryPool<T>::Allocate(){
    assert(free_list_!=nullptr);
    T* node = reinterpret_cast<T*> (free_list_);
    free_list_=free_list_->next;
    return node;
}

template<typename T>
void MemoryPool<T>::Deallocate(T* node){
    Slot* slot = reinterpret_cast<Slot*> (node);
    slot->next = free_list_;
    free_list_ = slot;
}
