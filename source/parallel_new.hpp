#pragma once
#include <functional>
#include "threadpool.h"


namespace utility {

    class ArrayMTHelper {
    public:

        static void splitIndex(int dataSize, int numTask, int taskId, int& frIdx, int& toIdx) {
            int taskSize = (dataSize + numTask - 1) / numTask;
            frIdx = taskSize * taskId;
            toIdx = std::min(int(taskSize * taskId + taskSize), dataSize);        
        }

        template <typename T, template <typename> class U>
        static void splitRange(const U<T>& arr, int numTask, int taskId, typename U<T>::iterator& l, typename U<T>::iterator& r) {
            int dataSize = arr.size();
            int taskSize = (dataSize + numTask - 1) / numTask;
            l = arr.begin() + taskSize * taskId;
            r = arr.begin() + std::min(int(taskSize * taskId + taskSize), dataSize);   
        }

        static std::vector<int> splitIndexAll(int dataSize, int numTask) {
            std::vector<int> ranges;
            ranges.resize(numTask*2);

            for (auto i = 0; i < numTask; i++) {
                splitIndex(dataSize, numTask, i, ranges[i*2], ranges[i*2+1]);
            }
            return std::move(ranges); 
        }

        template <typename T, template <typename> class U>
        static std::vector<int> splitRangeAll(const U<T>& arr, int numTask, int taskId) {
            std::vector<typename U<T>::const_iterator> ranges;
            ranges.resize(numTask*2);

            for (auto i = 0; i < numTask; i++) {
                splitRange(arr, numTask, i, ranges[i*2], ranges[i*2+1]);
            }
            return std::move(ranges); 
        }

        static int getSegement(int dataSize) {
            return 10;
        }
    };

    template <typename F>
    class sThreadHelper;

    template <typename F>
    class mThreadHelper;

    class ArrayTaskHelper {
    public:

        template <typename T, template <typename> class U, typename F, typename... Ts>
        static auto map(F&& mapFunc, const U<T>& vec, int dataSize, int numTask, Ts... args) {
            using retType = decltype(mapFunc(vec[0]));
            U<retType> retvec;
            retvec.resize(vec.size());

            return std::move(map(mapFunc, vec, retvec, dataSize, numTask, args...));
        }

        template <typename U, typename V, typename F, typename... Ts>
        static void map(F&& mapFunc, const U& vec, V& retvec, int dataSize, int numTask, Ts... args) {
            mapMImpl([&mapFunc, &vec, &retvec](int frIdx, int toIdx, Ts... args) {
                for (int i = frIdx; i < toIdx; i++) 
                    retvec[i] = mapFunc(vec[i], args...);
            }, ArrayMTHelper::splitIndexAll(dataSize, numTask), args...);
        }

        // reduceFunc :: T -> T -> T
        template <typename U, typename F>
        static auto reduce(F&& reduceFunc, const U& vec, int dataSize, int numTask) {
            return reduceMImpl([&reducefunc, &vec](const U& vec, int frIdx, int toIdx){
                T ret_seg(init_value);
                for (int i = frIdx; i < toIdx; i++) 
                    ret_seg = reduceFunc(vec[i], ret_seg);
                return ret_seg;      
            }, ArrayMTHelper::splitIndexAll(dataSize, numTask), args...);
        }

        // mapFunc :: T -> T2
        // reduceFunc :: T2 -> T2 -> T2
        template <typename U, typename F1, typename F2, typename... Ts>
        static auto reduce(F1&& reduceFunc1, F2&& reduceFunc2, U& vec, int dataSize, int numTask, Ts... args) {
            using RetType = decltype(mapFunc(vec[0]));

            return std::move(reduceMImpl([&reduceFunc1, &reduceFunc2, &vec](int fr, int to, Ts... args) {
                RetType ret_seg;
                for (int i = fr; i < to; i++) {
                    ret_seg = reduceFunc1(vec[i], ret_seg, args...);
                }
                return ret_seg;
            }, [&reduceFunc2, &vec](const std::vector<RetType>& vec, int fr, int to){
                RetType ret;
                for (int i = fr; i < to; i++) {
                    ret = reduceFunc(vec[i], ret);
                }
                return ret;
            }, ArrayMTHelper::splitIndexAll(dataSize, numTask), args...));
        }

        // mapFunc :: T -> T2
        // reduceFunc :: T2 -> T2 -> T2
        template <typename U, typename F1, typename F2, typename... Ts>
        static auto mapReduce(F1&& mapFunc, F2&& reduceFunc, U& vec, int dataSize, int numTask, Ts... args) {
            using RetType = decltype(mapFunc(vec[0]));

            return std::move(reduceMImpl([&mapFunc, &reduceFunc, &vec](int fr, int to, Ts... args) {
                RetType ret_seg;
                for (int i = fr; i < to; i++) {
                    ret_seg = reduceFunc(mapFunc(vec[i], args...), ret_seg);
                }
                return ret_seg;
            }, [&reduceFunc](const std::vector<RetType>& vec, int fr, int to){
                RetType ret;
                for (int i = fr; i < to; i++) {
                    ret = reduceFunc(vec[i], ret);
                }
                return ret;
            }, ArrayMTHelper::splitIndexAll(dataSize, numTask), args...));
        }

    public:
        // interface for c++-style container
        // require: container.size()
        // require: container::iterator      

        template <typename U, typename R, typename F, typename... Ts>
        static auto mapC(F&& mapFunc, const R& range, Ts... args) {        
            using iterator = decltype(range[0]);

            return std::move(mapMImpl([&mapFunc](Iterator fr, Iterator to, Ts... args) {
                for (auto iter = fr; iter != to; iter++) {
                    mapFunc((*iter), args...);
                }
            }));
        }

        template <typename U, typename R, typename F1, typename F2, typename RetType>
        static auto reduceC(F1&& reduceFunc1, F2&& reduceFunc, const R& range, RetType init_value) {        
            using iterator = decltype(range[0]);
            return std::move(reduceImpl([&reduceFunc1, &reduceFunc, &init_value](iterator fr, iterator to) {
                RetType ret_seg(init_value);
                for (auto iter = fr; iter != to; iter++) {
                    ret_seg = reduceFunc1((*iter), ret_seg, args...);
                }
                return ret_seg;
            }, [&reduceFunc, &init_value](const std::vector<RetType>& vec, int fr, int to){
                RetType ret(init_value);
                for (int i = fr; i < to; i++) {
                    ret = reduceFunc(vec[i], ret);
                }
                return ret;
            }, range, args...));
        }
        
        template <typename R, typename F1, typename F2, typename... Ts>
        static auto mapReduceC(F1&& mapFunc, F2&& reduceFunc, const R& range, Ts... args) {        
            using iterator = decltype(range[0]);
            using RetType = decltype(mapFunc(*range[0], args...));

            return std::move(reduceImpl([&mapFunc, &reduceFunc, &container](iterator fr, iterator to, Ts... args) {
                RetType ret_seg;
                for (auto iter = fr; iter != to; iter++) {
                    ret_seg = reduceFunc(mapFunc((*iter), args...), ret_seg);
                }
                return ret_seg;
            }, [&reduceFunc](const std::vector<RetType>& vec, int fr, int to){
                RetType ret;
                for (int i = fr; i < to; i++) {
                    ret = reduceFunc(vec[i], ret);
                }
                return ret;
            }, range, args...));
        }
   
    public:

        template <typename F, typename U, typename ...Ts>
        static void mapMImpl(F&& mapFunc, const U& ranges, Ts... args) {
            for (auto i = 0; i < ranges.size(); i++) {
                [auto fr, auto to] = ranges[i];
                solveThreadFunc([&, fr, to](){
                    mapFunc(fr, to, args...);
                });
            }
        }

        // reduceFunc :: [T] -> T
        template <typename F, typename U>
        static auto reduceMImpl(F&& reduceFunc, const U& ranges) {
            return reduceMImpl(reduceFunc, reduceFunc, ranges);
        }

        // mapFunc :: [this](int, int, Ts...) -> RetType
        // reduceFunc :: RetType -> RetType -> RetType
        template <typename F1, typename F2, typename U, typename... Ts>
        static auto reduceMImpl(F1&& reduceFunc1, F2&& reduceFunc2, const U& ranges, Ts... args) {
            using RetType = decltype(reduceFunc1(0, 1, args...));

            std::vector<RetType> retvec;
            retvec.resize(ranges.size());

            for (auto i = 0; i < ranges.size(); i++) {
                [auto fr, auto to] = ranges[i];
                solveThreadFunc([&, fr, to](){
                    retvec[i] = reduceFunc1(fr, to, args...);
                });
            }            

            return reduceFunc2(retvec, 0, int(retvec.size()));
        }

    public:

        class UseMTScope {
            UseMTScope(bool useMT = true) {
                _useMT = useMT;
                ArrayTaskHelper::setUseMT(useMT);
            }  
            ~UseMTScope() {
                ArrayTaskHelper::setUseMT(_useMT);
            }

            bool _useMT;
        };

        static bool useMT() {
            return s_useMT;
        }

        static void setUseMT(bool useMT = true){
            s_useMT = useMT;
        }

        template <typename F>
        inline static void solveThreadFunc(F&& func) {
            if (useMT) {
                mThreadHelper<F>::solveThreadFunc(func);
            } else {
                sThreadHelper<F>::solveThreadFunc(func);
            }
        }

    protected:
        static bool s_useMT;
    };

    template <typename F>
    class sThreadHelper {
    public:
        using ThreadFunc = F;
        inline static void solveThreadFunc(const ThreadFunc& func) {
            func();
        }
    };

    template <typename F>
    class mThreadHelper {
    public:
        using ThreadFunc = F;
        inline static void solveThreadFunc(const ThreadFunc& func) {
            threadpool().runTask(func);
        }
    };
}