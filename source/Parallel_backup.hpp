#pragma once
#include <functional>
#include "threadpool.h"


namespace utility {

    class ArrayMTHelper {
    public:

        static void splitIndex(int arraySize, int numTask, int taskId, int& frIdx, int& toIdx) {
            int taskSize = (arraySize + numTask - 1) / numTask;
            frIdx = taskSize * taskId;
            toIdx = std::min(int(taskSize * taskId + taskSize), arraySize);        
        }

        template <typename T, template <typename> class U>
        static void splitRange(const U<T>& arr, int numTask, int taskId, typename U<T>::iterator& l, typename U<T>::iterator& r) {
            int arraySize = arr.size();
            int taskSize = (arraySize + numTask - 1) / numTask;
            l = arr.begin() + taskSize * taskId;
            r = arr.begin() + std::min(int(taskSize * taskId + taskSize), arraySize);   
        }

        static int getSegement(int arraySize) {
            return 10;
        }
    };

    class ListMTHelper {
    public:

        template <typenmae T, template <typename> class U>
        static void splitRange(const U<T>& arr, int numTask, U<T>::const_iterator* iters) {
            using const_iter = U<T>::const_iterator;

            for (auto i = 0; i < numTask; i++) {
                ArrayMTHelper::splitRange(arr, numTask, i, iters[i][0], iters[i][1]);
            }
        }
    }

    template <typename F>
    class sThreadHelper;

    template <typename F>
    class mThreadHelper;

    class ArrayTaskHelper {
    public:
        // interface for c++-style array
        // implemented: vec.size()
        // implemented: vec[i]

        template <typename T, template <typename> class U, typename F, typename... Ts>
        static auto map(F&& mapFunc, const U<T>& vec, int segment, Ts... args) {
            using retType = decltype(mapFunc(vec[0]));
            const U<retType>& retvec;
            retvec.resize(vec.size());

            return std::move(mapImpl([&mapFunc](const U<T>& vec, U<retType>& retvec, int frIdx, int toIdx, Ts... args) {
                for (int i = frIdx; i < toIdx; i++) 
                    retvec[i] = mapFunc(vec[i], args...);
            }, vec, segment, args...));
        }

        template <typename T1, typename T2, template <typename> class U, template <typename> class V, typename F, typename... Ts>
        static void map(F&& mapFunc, const U<T1>& vec, V<T2>& retvec, int segment, Ts... args) {
            return std::move(mapImpl([&mapFunc](const U<T1>& vec, U<T2>& retvec, int frIdx, int toIdx, Ts... args) {
                for (int i = frIdx; i < toIdx; i++) 
                    retvec[i] = mapFunc(vec[i], args...);
            }, vec, retvec, segment, args...));
        }

        template <typename T, template <typename> class U, typename F, typename... Ts>
        static void map_inplace(F&& mapFunc, U<T>& vec, int segment, Ts... args) {
            mapImpl_inplace([&mapFunc](U<T>& vec, int frIdx, int toIdx, Ts... args) {
                for (int i = frIdx; i < toIdx; i++) 
                    mapFunc(vec[i], args...);
            }, vec, segment, args...);
        }

        // reduceFunc :: T -> T -> T
        template <typename T, template <typename> class U, typename F>
        static T reduce(F&& reduceFunc, const U<T>& vec, int segment) {
            return reduceImpl([&reducefunc](const U<T>& vec, int frIdx, int toIdx){
                T ret_seg(init_value);
                for (int i = frIdx; i < toIdx; i++) 
                    ret_seg = reduceFunc(vec[i], ret_seg);
                return ret_seg;      
            }, vec, segment);
        }

        // mapFunc :: T -> T2
        // reduceFunc :: T2 -> T2 -> T2
        template <typename T, template <typename> class U, typename F1, typename F2, typename... Ts>
        static auto mapReduce(F1&& mapFunc, F2&& reduceFunc, const U<T>& vec, int segment, Ts... args) {
            using RetType = decltype(mapFunc(vec[0]));

            return std::move(reduceImpl([&mapFunc, &reduceFunc](const U<T>& vec, int frIdx, int toIdx, Ts... args) {
                RetType ret_seg;
                for (int i = frIdx; i < toIdx; i++) {
                    ret_seg = reduceFunc(mapFunc(vec[i], args...), ret_seg);
                }
                return ret_seg;
            }, [&reduceFunc](const U<RetType>& vec, int frIdx, int toIdx){
                RetType ret;
                for (int i = frIdx; i < toIdx; i++) {
                    ret = reduceFunc(vec[i], ret);
                }
                return ret;
            }, vec, segment));
        }

    public:
        // interface for c-style array
        // implemented: arr[i]

        template <typename T1, typename T2, typename F, typename... Ts>
        static void map(F&& mapFunc, const T1* vec, T2* retvec, int arraySize, int segment, Ts... args) {
            std::move(mapImpl([&mapFunc](const T1* vec, T2* retvec, int frIdx, int toIdx, Ts... args) {
                for (int i = frIdx; i < toIdx; i++) 
                    retvec[i] = mapFunc(vec[i], args...);
            }, vec, retvec, arraySize, segment, args...));
        }

        template <typename T, typename F, typename... Ts>
        static void map_inplace(F&& mapFunc, T* vec, int arraySize, int segment, Ts... args) {
            mapImpl_inplace([&mapFunc](T* vec, int frIdx, int toIdx, Ts... args) {
                for (int i = frIdx; i < toIdx; i++) 
                    mapFunc(vec[i], args...);
            }, vec, arraySize, segment, args...);
        }

        // reduceFunc :: T -> T -> T
        template <typename T, typename F>
        static T reduce(F&& reduceFunc, T* vec, int segment, T init_value = T()) {
            return reduceImpl([&](T* vec, int frIdx, int toIdx){
                T ret_seg(init_value);
                for (int i = frIdx; i < toIdx; i++) 
                    ret_seg = reduceFunc(vec[i], ret_seg);
                return ret_seg;      
            }, vec, segment);
        }

        // mapFunc :: T -> T2
        // reduceFunc :: T2 -> T2 -> T2
        template <typename T, typename F1, typename F2, typename... Ts>
        static auto mapReduce(F1&& mapFunc, F2&& reduceFunc, T* vec, int arraySize, int segment, Ts... args) {
            using RetType = decltype(mapFunc(vec[0]));

            return std::move(reduceImpl([&mapFunc, &reduceFunc](T* vec, int frIdx, int toIdx, Ts... args) {
                RetType ret_seg;
                for (int i = frIdx; i < toIdx; i++) {
                    ret_seg = reduceFunc(mapFunc(vec[i], args...), ret_seg);
                }
                return ret_seg;
            }, [&](const std::vector<RetType>& vec, int frIdx, int toIdx){
                RetType ret;
                for (int i = frIdx; i < toIdx; i++) {
                    ret = reduceFunc(vec[i], ret);
                }
                return ret;
            }, vec, arraySize, segment));
        }

    public:

        template <typename T, template <typename> class U, typename F, typename... Ts>
        static auto mapImpl(F&& func, const U<T>& vec, int segment, Ts... args) {
            using RetType = decltype(func(vec[0]));

            U<RetType> retvec;
            retvec.resize(vec.size());

            mapImpl(func, vec, retvec, segment, args...);

            return std::move(retvec);
        }

        template <typename T1, typename T2, template <typename> class U, template <typename> class V, typename F, typename... Ts>
        static void mapImpl(F&& func, U<T1>& vec, V<T2>& retvec, int segment, Ts... args) {
            retvec.resize(vec.size());

            for (int i = 0; i < segment; i++) {
                int frIdx = 0, toIdx = -1;
                ArrayMTHelper::splitIndex(vec.size(), segment, i, frIdx, toIdx);
                if (frIdx >= toIdx) break;

                solveThreadFunc([&, frIdx, toIdx](){
                    func(vec, retvec, frIdx, toIdx, args...);
                });
            }
        }

        template <typename T1, typename T2, typename F, typename... Ts>
        static void mapImpl(F&& func, T1* vec, T2* retvec, int arraySize, int segment, Ts... args) {

            for (int i = 0; i < segment; i++) {
                int frIdx = 0, toIdx = -1;
                ArrayMTHelper::splitIndex(arraySize, segment, i, frIdx, toIdx);
                if (frIdx >= toIdx) break;

                solveThreadFunc([&, frIdx, toIdx](){
                    func(vec, retvec, frIdx, toIdx, args...);
                });
            }
        }

        template <typename T, template <typename> class U, typename F, typename... Ts>
        static void mapImpl_inplace(F&& func, U<T>& vec, int segment, Ts... args) {
            for (int i = 0; i < segment; i++) {
                int frIdx = 0, toIdx = -1;
                ArrayMTHelper::splitIndex(vec.size(), segment, i, frIdx, toIdx);
                if (frIdx >= toIdx) break;

                solveThreadFunc([&, frIdx, toIdx](){
                    func(vec, frIdx, toIdx, args...);
                });
            }
        }

        template <typename T, typename F, typename... Ts>
        static void mapImpl_inplace(F&& func, T* vec, int arraySize, int segment, Ts... args) {
            for (int i = 0; i < segment; i++) {
                int frIdx = 0, toIdx = -1;
                ArrayMTHelper::splitIndex(arraySize, segment, i, frIdx, toIdx);
                if (frIdx >= toIdx) break;

                solveThreadFunc([&, frIdx, toIdx](){
                    func(vec, frIdx, toIdx, args...);
                });
            }
        }

        // reduceFunc :: [T] -> T
        template <typename T, template <typename> class U, typename F>
        static T reduceImpl(F&& reduceFunc, const U<T>& vec, int segment) {
            return reduceImpl(reduceFunc, reduceFunc, vec, segment);
        }

        // reduceFunc1 :: [T] -> T2
        // reduceFunc2 :: [T2] -> T2
        template <typename T, template <typename> class U, typename F1, typename F2>
        static auto reduceImpl(F1&& reduceFunc1, F2&& reduceFunc2, const U<T>& vec, int segment) {
            // static_assert(std::is_same_v<typename F1::return_type, typename F2::return_type>);
            using RetType = decltype(reduceFunc1(vec, 0, 1));

            U<RetType> retvec;
            retvec.resize(segment);

            for (int i = 0; i < segment; i++) {
                int frIdx = 0, toIdx = -1;
                ArrayMTHelper::splitIndex(vec.size(), segment, i, frIdx, toIdx);
                if (frIdx >= toIdx) break;

                solveThreadFunc([&, frIdx, toIdx](){
                    retvec[i] = reduceFunc1(vec, frIdx, toIdx);
                });
            }
            return std::move(reduceFunc2(retvec, 0, segment));
        }

    public:

        // mapFunc :: [this](int, Ts...) -> void
        template <typename F, typename ...Ts>
        static void mapM(F&& mapFunc, int arraySize, int segment, Ts...args) {
            mapMImpl([&mapFunc](int frIdx, int toIdx, Ts... args) {
                for (int i = frIdx; i < toIdx; i++) 
                    mapFunc(i, args...);
            }, arraySize, segment, args...);            
        }

        // mapFunc :: [this](int, int, Ts...) -> void
        template <typename F, typename... Ts>
        static void mapMImpl(F&& mapFunc, int arraySize, int segment, Ts... args) {
            for (int i = 0; i < segment; i++) {
                int frIdx = 0, toIdx = -1;
                ArrayMTHelper::splitIndex(arraySize, segment, i, frIdx, toIdx);
                if (frIdx >= toIdx) break;

                solveThreadFunc([&, frIdx, toIdx](){
                    mapFunc(frIdx, toIdx, args...);
                });
            }            
        }

        // mapFunc :: [this](int, Ts...) -> RetType
        // reduceFunc :: RetType -> RetType -> RetType
        template <typename F1, typename F2, typename... Ts>
        static auto mapReduceM(F1&& mapFunc, F2&& reduceFunc, int arraySize, int segment, Ts... args) {
            using RetType = decltype(mapFunc(0, args...));

            std::vector<RetType> vec;

            return std::move(reduceMImpl([&mapFunc, &reduceFunc](int frIdx, int toIdx, Ts... args) {
                RetType ret_seg;
                for (int i = frIdx; i < toIdx; i++) {
                    ret_seg = reduceFunc(mapFunc(i, args...), ret_seg);
                }
                return ret_seg;
            }, [&reduceFunc](const std::vector<RetType>& vec, int frIdx, int toIdx){
                RetType ret;
                for (int i = frIdx; i < toIdx; i++) {
                    ret = reduceFunc(vec[i], ret);
                }
                return ret;
            }, arraySize, segment));
        }

        // mapFunc :: [this](int, int, Ts...) -> RetType
        // reduceFunc :: RetType -> RetType -> RetType
        template <typename F1, typename F2, typename... Ts>
        static auto reduceMImpl(F1&& reduceFunc1, F2&& reduceFunc2, int arraySize, int segment, Ts... args) {
            using RetType = decltype(reduceFunc1(0, 1, args...));

            std::vector<RetType> retvec;
            retvec.resize(segment);

            for (int i = 0; i < segment; i++) {
                int frIdx = 0, toIdx = -1;
                ArrayMTHelper::splitIndex(arraySize, segment, i, frIdx, toIdx);
                if (frIdx >= toIdx) break;

                solveThreadFunc([&, frIdx, toIdx](){
                    retvec[i] = reduceFunc1(frIdx, toIdx, args...);
                });
            }            

            return reduceFunc2(retvec, 0, int(retvec.size()));
        }

        template <typename F1, typename F2, typename... Ts>
        static auto reducePImpl(F1&& reduceFunc1, F2&& reduceFunc2, int arraySize, int segment, Ts... args) {
            using RetType = decltype(reduceFunc1(0, 1, args...));

            std::vector<RetType> retvec;
            retvec.resize(segment);

            for (int i = 0; i < segment; i++) {
                T* fpointer = nullptr, tpointer = nullptr;
                ArrayMTHelper::splitIndex(arraySize, segment, i, fpointer, tpointer);
                if (fpointer ==  nullptr) break;

                solveThreadFunc([&, fpointer, tpointer](){
                    retvec[i] = reduceFunc1(fpointer, tpointer, args...);
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