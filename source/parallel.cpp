#include "parallel.hpp"

namespace utility {

    bool ArrayTaskHelper::s_useMT = false;
    
    template class sThreadHelper<std::function<void()>>;
    template class sThreadHelper<std::function<void(void*)>>;
    template class mThreadHelper<std::function<void()>>;
    template class mThreadHelper<std::function<void(void*)>>;

}