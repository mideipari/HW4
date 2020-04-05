#include "fifo_evictor.hh"

void FifoEvictor::touch_key(const key_type& key)  {
    //just insert at the beginning
    std::vector<key_type>::iterator it;

    it = FifoEvictor::touchedKeys.begin();
    FifoEvictor::touchedKeys.insert(it, key);


}


const key_type FifoEvictor::evict()  {
    if(!FifoEvictor::touchedKeys.empty()) {
        key_type key = FifoEvictor::touchedKeys[FifoEvictor::touchedKeys.size()-1];
        FifoEvictor::touchedKeys.pop_back();
        return key;
    }
    //return empty string if we don't have anything to evict. Delete will deal with it properly since it won't be found in the map
    return "";
}


