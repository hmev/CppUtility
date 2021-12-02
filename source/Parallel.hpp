#include <functional>

namespace utility {

    class ThreadPool {
        template <typename F, typename ...Ts>
        void run(const F& f, Ts... args) {

        }
    };

    class ArrayMTHelper {
    public:

        static void splitIndex(int arraySize, int numTask, int taskId, int& frIdx, int& toIdx) {
            int taskSize = (arraySize + numTask - 1) / numTask;
            frIdx = taskSize * taskId;
            toIdx = std::min(int(taskSize * taskId + taskSize), arraySize);        
        }

        template <typename T, template <typename T> class U>
        static void splitRange(const U<T>& arr, int taskId, U<T>::iterator& l, U<T>::iterator& r) {
            int numTask = _segment;
            int numTask = _segment;
            int taskSize = (arraySize + numTask - 1) / numTask;
            l = arr.begin() + taskSize * taskId;
            r = arr.begin() + UTMIN(int(taskSize * taskId + taskSize), arraySize);   
        }
    };

    class TaskMgr {
    public:

        TaskMgr() {}
        
        template <typename T1, typename T2, template <typename T1> class U, template <typename T2> class V, typename F, typename... Ts>
        static V<T2> mapMT(const F& mapFunc, const U<T1>& vec, int segment, Ts... args) {
            return std::move(runMT([](const U<T>& vec, V<T2>& retvec, int frIdx, int toIdx, Ts... args) {
                for (int i = frIdx; i < toIdx; i++) 
                    retvec[i] = mapFunc(vec[i], args...);
            }, vec, segment, args...));
        }

        template <typename T, template <typename T> class U, typename F, typename... Ts>
        static void mapInPlaceMT(const F& mapFunc, U<T>& vec, int segment, Ts... args) {
            runInPlaceMT([](U<T>& vec, int frIdx, int toIdx, Ts... args) {
                for (int i = frIdx; i < toIdx; i++) 
                    mapFunc(vec[i], args...);
            }, vec, segment, args...);
        }

        template <typename T, template <typename T> class U, typename F>
        static T reduceMT(const F& reduceFunc, const U<T>& vec, int segment) {
            return runMT([](const U<T>& vec, int frIdx, int toIdx) {
                T ret;
                for (int i = frIdx; i < toIdx; i++) {
                    ret = reduceFunc(vec[i], ret);
                }
                return ret;
            }, vec, segment);
        }

        template <typename T, template <typename T> class U, typename F, typename... Ts>
        static U<T> runMT(const F& func, const U<T>& vec, int segment, Ts... args) {
            U<T> retvec;
            retvec.resize(vec.size());

            runMT(func, vec, retvec, segment, args...);

            return std::move(retvec);
        }

        template <typename T1, typename T2, template <typename T1> class U, template <typename T2> class V, typename F, typename... Ts>
        static void runMT(const F& func, U<T1>& vec, V<T2>& retvec, int segment, Ts... args) {
            retvec.resize(vec.size());

            for (int i = 0; i < segment; i++) {
                int frIdx = 0, toIdx = -1;
                ArrayMTHelper.splitIndex(vec.size(), segment, i, frIdx, toIdx);
                if (frIdx >= toIdx) break;

                utThread::RunThreadFunc([=, &func, &retvec](void*){
                    func(vec, retvec, frIdx, toIdx, args...);
                });
            }
        }

        template <typename T, template <typename T> class U, typename F, typename... Ts>
        static void runInPlaceMT(const F& func, U<T>& vec, int segment, Ts... args) {
            for (int i = 0; i < segment; i++) {
                int frIdx = 0, toIdx = -1;
                ArrayMTHelper.splitIndex(vec.size(), segment, i, frIdx, toIdx);
                if (frIdx >= toIdx) break;

                utThread::RunThreadFunc([=, &func, &vec](void*){
                    func(vec, frIdx, toIdx, args...);
                });
            }
        }


        template <class U, class V, typename F, typename... Ts>
        static void runMT(const F& func, U* vec, V* retvec, int arraySize, int segment, Ts... args) {
            
        }        

        template <typename DataType, typename F, typename... Ts>
        static void runInPlaceMT(const F& func, DataType* data, int arraySize, int segment, Ts... args) {

        }
    };

}