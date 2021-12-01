class TaskMgr {
public:

    TaskMgr() {
        _segment = 100;
    }

    void splitter(int arraySize, int task_id, int& frIdx, int& toIdx) {
        int numTask = _segment;
        int taskSize = (arraySize + numTask - 1) / numTask;
        frIdx = taskSize * task_id;
        toIdx = UTMIN(int(taskSize * task_id + taskSize), arraySize);
    }

    template<class Func, class DataPtr>
    void runMT(const Func& func, DataPtr data, int arraySize) {
        static_assert(std::is_pointer<DataPtr>::value);

        for (int i = 0; i < _segment; i++) {
            int frIdx = 0, toIdx = -1;
            splitter(arraySize, _segment, i, frIdx, toIdx);
            if (frIdx >= toIdx) break;

            utThread::RunThreadFunc([&func, frIdx, toIdx](void* args){
                auto data = static_cast<DataPtr>(args);
                func(data, frIdx, toIdx);
            }, data);
        }   
    }

    template <class RetValue>
    using AggFunc = std::function<RetValue(std::vector<RetValue>)>;

    template <class DataPtr>
    using AggInPlaceFunc = std::function<void(DataPtr)>;

    template<class Func, class DataPtr, class RetValue>
    RetValue runMT(const Func& func, DataPtr data, int arraySize, const AggFunc<RetValue>& aggFunc) {
        static_assert(std::is_pointer<DataPtr>::value);

        std::vector<RetValue> ret;
        ret.resize(_segment);

        for (int i = 0; i < _segment; i++) {
            int frIdx = 0, toIdx = -1;
            splitter(arraySize, _segment, i, frIdx, toIdx);
            if (frIdx >= toIdx) break;

            utThread::RunThreadFunc([&func, frIdx, toIdx, &ret](void* args){
                auto data = static_cast<DataPtr>(args);
                ret[i] = func(data, frIdx, toIdx);
            }, data);
        }        
        
        return std::move(aggFunc(ret));
    }

    template<class Func, class DataPtr>
    void runMT2(const Func& func, DataPtr data, int arraySize, const AggInPlaceFunc<DataPtr>& aggFunc) {
        static_assert(std::is_pointer<DataPtr>::value);

        for (int i = 0; i < _segment; i++) {
            int frIdx = 0, toIdx = -1;
            splitter(arraySize, _segment, i, frIdx, toIdx);
            if (frIdx >= toIdx) break;

            utThread::RunThreadFunc([&func, frIdx, toIdx](void* args){
                auto data = static_cast<DataPtr>(args);
                func(data, frIdx, toIdx);
            }, data);
        }        
        
        aggFunc(data);    
    }

private:
    int _segment;
};