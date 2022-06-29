#pragma once
#include <functional>
#include "threadpool.h"


namespace utility {

    using uint = unsigned int;

    class ArrayMTHelper {
    public:
        static void splitIndex(int dataSize, int numTask, int taskId, int& fr, int& to) {
            int taskSize = getTaskSize(dataSize, numTask);
            fr = taskSize * taskId;
            to = std::min(int(taskSize * taskId + taskSize), dataSize);        
        }

        template <typename T, template <typename> class U>
        static void splitRange(const U<T>& container, int numTask, int taskId, typename U<T>::iterator& l, typename U<T>::iterator& r) {
            int dataSize = container.size();
            int taskSize = (dataSize + numTask - 1) / numTask;
            l = container.begin() + taskSize * taskId;
            r = container.begin() + std::min(int(taskSize * taskId + taskSize), dataSize);   
        }

        static auto splitIndexAll(int dataSize, int numTask) {
            std::vector<int> ranges;
            ranges.resize(numTask+1);
            int taskSize = getTaskSize(dataSize, numTask);
            for (auto i = 0; i < numTask; i++) {
                ranges[i] = taskSize * i;
            }
            ranges[numTask] = dataSize;
            return ranges; 
        }

        template <typename T, template <typename> class U>
        static auto splitRangeAll(const U<T>& container, int numTask) {
            std::vector<typename U<T>::const_iterator> ranges;
            ranges.resize(numTask+1);

            auto iter = container.begin();
            int taskSize = getTaskSize(container.size(), numTask);

            for (auto i = 0; i < numTask; i++) {
                count = 0;
                ranges[i] = iter;
                for (; iter != container.end(); iter++) {
                    count++;
                    if (count > taskSize) {
                        break;
                    }
                }
            }
            ranges[numTask] = container.end();
            return ranges; 
        }

        static auto splitRangeAll(U<T>::iterator& begin, U<T>::iterator& end, int dataSize, int numTask) {
            std::vector<typename U<T>::const_iterator> ranges;
            ranges.resize(numTask+1);

            auto iter = begin;
            int taskSize = getTaskSize(dataSize, numTask);

            for (auto i = 0; i < numTask; i++) {
                count = 0;
                ranges[i] = iter;
                for (; iter.End(); iter++) {
                    count++;
                    if (count > taskSize) {
                        break;
                    }
                }
            }
            ranges[numTask] = end;
            return ranges; 
        }

        static int getTaskSize(int dataSize, int numTask) {
            return taskSize = (dataSize + numTask - 1) / numTask;
        }

        static int calcTaskNumber(int dataSize) {
            return 8;
        }
    };

    template <typename F>
    class sThreadHelper;

    template <typename F>
    class mThreadHelper;


    /// Task helper for array or other c++ style containers that are traversable.
    /// All are static members.
    class TaskHelper {
    public:
        /// interface for array-style types such as c-style array and c++ std::vector and std::array. 
        /// using index to traverse pointwise 
        /// require: container.size()
        /// require: container::iterator      

        template <typename T, template <typename> class U, typename F, typename... Ts>
        static auto map(F&& mapFunc, const U<T>& vec, int dataSize, int numTask, Ts... args) {
            using retType = decltype(mapFunc(vec[0]));
            U<retType> retvec;
            retvec.resize(vec.size());

            return std::move(map(mapFunc, vec, retvec, dataSize, numTask, args...));
        }

        template <typename U, typename V, typename F, typename... Ts>
        static void map(F&& mapFunc, const U& vec, V& retvec, int dataSize, int numTask, Ts... args) {
            mapImpl([&mapFunc, &vec, &retvec](int fr, int to, Ts... args) {
                for (int i = fr; i < to; i++) 
                    retvec[i] = mapFunc(vec[i], args...);
            }, ArrayMTHelper::splitIndexAll(dataSize, numTask), args...);
        }

        /// reduceFunc :: T -> T -> T
        template <typename U, typename F, typename RetType>
        static auto reduce(F&& reduceFunc, const U& vec, int dataSize, int numTask, RetType init_value) {
            std::vector<typename RetType> retvec;
            retvec.resize(ranges.size());
            return reduceImpl([&reducefunc, &vec](int fr, int to){
                RetType ret_seg(init_value);
                for (int i = fr; i < to; i++) 
                    ret_seg = reduceFunc(vec[i], ret_seg);
                return ret_seg;      
            }, [&reducefunc, &retvec](int fr, int to){
                RetType ret_seg = retvec[fr];
                for (int i = fr+1; i < to; i++) 
                    ret_seg = reduceFunc(retvec[i], ret_seg);
                return ret_seg;      
            }, ArrayMTHelper::splitIndexAll(dataSize, numTask), args...);
        }

        /// mapFunc :: T -> T2
        /// reduceFunc :: T2 -> T2 -> T2
        template <typename U, typename F1, typename F2, typename... Ts>
        static auto mapReduce(F1&& mapFunc, F2&& reduceFunc, U& vec, int dataSize, int numTask, Ts... args) {
            using RetType = decltype(mapFunc(vec[0]));
            std::vector<typename RetType> retvec;
            retvec.resize(ranges.size());

            return std::move(reduceImpl([&mapFunc, &reduceFunc, &vec](int fr, int to, Ts... args) {
                RetType ret_seg = mapFunc(vec[i], args...);
                for (int i = fr+1; i < to; i++) {
                    ret_seg = reduceFunc(mapFunc(vec[i], args...), ret_seg);
                }
                return ret_seg;
            }, [&reduceFunc, &retvec](int fr, int to){
                RetType ret = retvec[fr];
                for (int i = fr+1; i < to; i++) {
                    ret = reduceFunc(retvec[i], ret);
                }
                return ret;
            }, ArrayMTHelper::splitIndexAll(dataSize, numTask), args...));
        }

        // TODO.
        template <typename U, typename F1, typenameF F2, typename... Ts>
        static auto mapReduce(F1&& mapFunc, F2&& reduceFunc, const U& vec, V& tmpVec, RetType ret, int dataSize, int numTask, Ts... args) {
            std::vector<typename RetType> retvec;
            retvec.resize(ranges.size());

            return std::move(reduceImpl([&mapFunc, &reduceFunc, &vec](int fr, int to, Ts... args) {
                RetType ret_seg(vec[fr]);
                for (int i = fr+1; i < to; i++) {
                    ret_seg = reduceFunc(mapFunc(vec[i], args...), ret_seg);
                }
                return ret_seg;
            }, [&reduceFunc, &retvec](int fr, int to){
                RetType ret(vec[fr]);
                for (int i = fr+1; i < to; i++) {
                    ret = reduceFunc(retvec[i], ret);
                }
                return ret;
            }, ArrayMTHelper::splitIndexAll(dataSize, numTask), args...));
        }

    public:
        /// interface for c++-style containers such as std::list
        /// using iterator to traverse pointwise 
        /// require: container.size()
        /// require: container::iterator      

        /// mapFunc :: [](*iter) -> void
        template <typename R, typename F, typename... Ts>
        static auto mapC(F&& mapFunc, const R& range, Ts... args) {        
            using iterator = decltype(range[0]);
            mapImpl([&mapFunc](iterator fr, iterator to, Ts... args) {
                for (auto iter = fr; iter != to; iter++) {
                    mapFunc((*iter), args...);
                }
            }, range, args...);
        }

        /// reduceFunc1 :: [](*iter, RetType) -> RetType
        /// reduceFunc2 :: (RetType, RetType) -> RetType
        template <typename R, typename F1, typename F2, typename RetType>
        static auto reduceC(F1&& reduceFunc1, F2&& reduceFunc2, const R& range, RetType init_value) {        
            using iterator = decltype(range[0]);
            std::vector<typename RetType> retvec;
            retvec.resize(ranges.size());

            return std::move(reduceImpl([&reduceFunc1, &reduceFunc2, &init_value](iterator fr, iterator to) {
                RetType ret_seg(init_value);
                for (auto iter = fr; iter != to; iter++) {
                    ret_seg = reduceFunc1(*iter, ret_seg);
                }
                return ret_seg;
            }, [&reduceFunc2, &retvec, &init_value](int fr, int to){
                RetType ret(init_value);
                for (int i = fr+1; i < to; i++) {
                    ret = reduceFunc2(retvec[i], ret);
                }
                return ret;
            }, range));
        }

        /// mapFunc :: [](*iter) -> RetType
        /// reduceFunc :: (RetType, RetType) -> RetType
        template <typename R, typename F1, typename F2, typename... Ts>
        static auto mapReduceC(F1&& mapFunc, F2&& reduceFunc, const R& range, Ts... args) {        
            using iterator = decltype(range[0]);
            using RetType = decltype(mapFunc(*range[0], args...));
            std::vector<typename RetType> retvec;
            retvec.resize(ranges.size());

            return std::move(reduceImpl([&mapFunc, &reduceFunc, &container](iterator fr, iterator to, Ts... args) {
                RetType ret_seg = mapFunc(*fr, args...);
                for (auto iter = fr; iter != to; iter++) {
                    if (iter != fr) {
                        ret_seg = reduceFunc(mapFunc((*iter), args...), ret_seg);
                    }
                }
                return ret_seg;
            }, [&reduceFunc, &retvec](int fr, int to){
                RetType ret = retvec[fr];
                for (int i = fr+1; i < to; i++) {
                    ret = reduceFunc(retvec[i], ret);
                }
                return ret;
            }, range, args...));
        }

    public:

        /// interface for any data that could be traversed pointwise by index. 
        /// using index to traverse 

        /// mapFunc :: [this](int fr, int to, Ts...) -> void
        template <typename U, typename V, typename F, typename... Ts>
        static void mapM(F&& mapFunc, int dataSize, int numTask, Ts... args) {
            mapImpl([&mapFunc](int fr, int to, Ts... args) {
                for (int i = fr; i < to; i++) 
                    mapFunc(i, args...);
            }, ArrayMTHelper::splitIndexAll(dataSize, numTask), args...);
        }

        /// mapFunc :: T -> T2
        /// reduceFunc :: T2 -> T2 -> T2
        template <typename U, typename F1, typename F2, typename RetType>
        static auto reduceM(F1&& reduceFunc1, F2&& reduceFunc2, U& vec, int dataSize, int numTask, RetType init_value) {
            std::vector<typename RetType> retvec;
            retvec.resize(ranges.size());

            return std::move(reduceImpl([&reduceFunc1, &reduceFunc2, &vec](int fr, int to, Ts... args) {
                RetType ret_seg = vec[fr];
                for (int i = fr+1; i < to; i++) {
                    ret_seg = reduceFunc1(vec[i], ret_seg, args...);
                }
                return ret_seg;
            }, [&reduceFunc2, &retvec](int fr, int to){
                RetType ret = retvec[fr];
                for (int i = fr+1; i < to; i++) {
                    ret = reduceFunc2(retvec[i], ret);
                }
                return ret;
            }, ArrayMTHelper::splitIndexAll(dataSize, numTask), args...));
        }

        /// mapFunc :: T -> T2
        /// reduceFunc :: T2 -> T2 -> T2
        template <typename F1, typename F2, typename... Ts>
        static auto mapReduceM(F1&& mapFunc, F2&& reduceFunc, int dataSize, int numTask, Ts... args) {
            using RetType = decltype(mapFunc(vec[0]));
            std::vector<typename RetType> retvec;
            retvec.resize(ranges.size());

            return std::move(reduceImpl([&mapFunc, &reduceFunc](int fr, int to, Ts... args) {
                RetType ret_seg = mapFunc(fr, args...);
                for (int i = fr+1; i < to; i++) {
                    ret_seg = reduceFunc(mapFunc(i, args...), ret_seg);
                }
                return ret_seg;
            }, [&reduceFunc, &retvec](int fr, int to){
                RetType ret = retvec[fr];
                for (int i = fr+1; i < to; i++) {
                    ret = reduceFunc(retvec[i], ret);
                }
                return ret;
            }, ArrayMTHelper::splitIndexAll(dataSize, numTask), args...));
        }

        template <typename F1, typename F2, typename... Ts>
        static auto runM(F1&& mapFunc, F2&& reduceFunc, int dataSize, int numTask, Ts... args) {
            using RetType = decltype(mapFunc(vec[0]));
            std::vector<typename RetType> retvec;
            retvec.resize(ranges.size());

            return std::move(runImpl([&](auto threaid){[&mapFunc, &reduceFunc](int fr, int to, Ts... args) {
                RetType ret_seg = mapFunc(threadid)(fr, args...);
                for (int i = fr+1; i < to; i++) {
                    ret_seg = reduceFunc(mapFunc(threaid)(i, args...), ret_seg);
                }
                return ret_seg;
            }}, [&reduceFunc, &retvec](int fr, int to){
                RetType ret = retvec[fr];
                for (int i = fr+1; i < to; i++) {
                    ret = reduceFunc(retvec[i], ret);
                }
                return ret;
            }, ArrayMTHelper::splitIndexAll(dataSize, numTask), args...));
        }

    public:
        /// interface for any data that could be traversed in ranges. 

        template <typename R, typename F, typename ...Ts>
        static void mapR(F&& mapFunc, const R& ranges, Ts... args) {
            return mapImpl(mapFunc, ranges, args...);
        }

        template <typename R, typename F, typename... Ts>
        static auto reduceR(F&& reduceFunc, const R& ranges) {
            return reduceImpl(reduceFunc, reduceFunc, ranges);
        }

        template <typename R, typename F1, typename F2, typename... Ts>
        static auto reduceR(F1&& reduceFunc1, F2&& reduceFunc2, const R& ranges, Ts... args) {
            return reduceImpl(reduceFunc1, reduceFunc2, ranges, args...);
        }

        template <typename R, typename F1, typename F2, typename... Ts>
        static auto runR(F1&& func, F2&& reduceFunc, const R& ranges, Ts... args) {
            return runImpl(func, reduceFunc, ranges, args...);
        }

    public:
        /// Implementation

        template <typename R, typename F, typename ...Ts>
        static void mapImpl(F&& mapFunc, const R& ranges, Ts... args) {
            for (int i = 0; i < ranges.size() - 1; i++) {
                auto fr = ranges[i];
                auto to = ranges[i+1];
                solveThreadFunc([&, fr, to](){
                    mapFunc(fr, to, args...);
                });
            }

            // mapThreadImpl([&](auto i) {
            //     return [&, i]() {
            //         mapFunc(ranges[i], ranges[i+1], args..);
            //     }
            // }, ranges.size() - 1);
        }

        template <typename F, typename ...Ts>
        static void mapThreadImpl(F&& mapThreadFunc, int numTask) {
            mapTask(mapThreadFunc, numTask);
            waitAll();
        }

        template <typename F, typename ...Ts>
        static void mapThreadImpl2(F&& mapThreadFunc, int numTask, Ts... args) {
            taskQueue().addTasks(mapThreadFunc, numTask, args...);
            taskQueue().waitAll();
        }

        /// reduceFunc1 :: [...](l, r, Ts...) -> RetType
        /// reduceFunc2 :: [](const std::vector<retType>&, l, r) -> RetType 
        template <typename R, typename F1, typename F2, typename... Ts>
        static auto reduceImpl(F1&& reduceFunc1, F2&& reduceFunc2, const R& ranges, Ts... args) {
            for (int i = 0; i < ranges.size() - 1; i++) {
                auto fr = ranges[i];
                auto to = ranges[i+1];
                solveThreadFunc([&, fr, to](){
                    reduceFunc1(fr, to, args...);
                });
            }

            return reduceFunc2();

            // return mapThreadImpl([&](auto i) {
            //     return mapFunc(ranges[i], ranges[i+1], args..);
            // }, reduceFunc2, ranges.size() - 1);
        }

        template <typename F1, typename F2, typename... Ts>
        static auto reduceThreadImpl(F1&& reduceThreadFunc1, F2&& reduceFunc2, int numTask) {
            mapTask(reduceThreadFunc1, numTask);
            waitAll();
            return reduceFunc2();            
        }

        template <typename F>
        static auto mapTask(F&& func, uint numTask) {
            for (uint taskid = 0; taskid < numTask; taskid++) {
                addTask(func(taskid));
            }
        }

    public:

        class UseMTScope {
            UseMTScope(bool useMT = true) {
                _useMT = useMT;
                TaskHelper::setUseMT(useMT);
            }  
            ~UseMTScope() {
                TaskHelper::setUseMT(_useMT);
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
            if (useMT()) {
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

    class TaskHelper{
    public:
        template <typename F, typename ...Ts>
        static void Map(F&& mapFunc, int dataSize, int numTask, Ts...args) {

        }

        template <typename I, typename F, typename ...Ts>
        static void Map(F&& mapFunc, const std::vector<I>& ranges, Ts... args) {

        }

        template <typename F, typename ...Ts>
        static void MapThread(F&& mapThreadFunc, int dataSize, int numTask, Ts... args) {

        }

        template <typename I, typename F, typename ...Ts>
        static void MapThreadImpl(F&& mapThreadFunc, const std::vector<I>& ranges, Ts... args) {

        }

        template <typename F, typename ... Ts>
        static void MapReduce(F1&& mapFunc, F2&& reduceFunc, int dataSize, int numTask, Ts...args) {

        }

        template <typename I, typename F1, typename F2, typename ... Ts>
        static void MapReduceImpl(F1&& mapFunc, F2&& reduceFunc, const std::vector<I>& ranges, Ts...args) {
            
        }
    }
}